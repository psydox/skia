/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkDashPathEffect.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/SkDashImpl.h"
#include "src/utils/SkDashPathPriv.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>

class SkPathBuilder;

using namespace skia_private;

SkDashImpl::SkDashImpl(SkSpan<const SkScalar> intervals, SkScalar phase)
        : fIntervals(intervals.size())
        , fPhase(0)
        , fInitialDashLength(-1)
        , fIntervalLength(0)
        , fInitialDashIndex(0)
{
    SkASSERT(intervals.size() > 1 && SkIsAlign2(intervals.size()));

    memcpy(fIntervals.data(), intervals.data(), intervals.size_bytes());

    // set the internal data members
    SkDashPath::CalcDashParameters(phase, fIntervals,
            &fInitialDashLength, &fInitialDashIndex, &fIntervalLength, &fPhase);
}

bool SkDashImpl::onFilterPath(SkPathBuilder* builder, const SkPath& src, SkStrokeRec* rec,
                              const SkRect* cullRect, const SkMatrix&) const {
    return SkDashPath::InternalFilter(builder, src, rec, cullRect, fIntervals,
                                      fInitialDashLength, fInitialDashIndex, fIntervalLength,
                                      fPhase);
}

static void outset_for_stroke(SkRect* rect, const SkStrokeRec& rec) {
    SkScalar radius = SkScalarHalf(rec.getWidth());
    if (0 == radius) {
        radius = SK_Scalar1;    // hairlines
    }
    if (SkPaint::kMiter_Join == rec.getJoin()) {
        radius *= rec.getMiter();
    }
    rect->outset(radius, radius);
}

// Attempt to trim the line to minimally cover the cull rect (currently
// only works for horizontal and vertical lines).
// Return true if processing should continue; false otherwise.
static bool cull_line(SkPoint* pts, const SkStrokeRec& rec,
                      const SkMatrix& ctm, const SkRect* cullRect,
                      const SkScalar intervalLength) {
    if (nullptr == cullRect) {
        SkASSERT(false); // Shouldn't ever occur in practice
        return false;
    }

    SkScalar dx = pts[1].x() - pts[0].x();
    SkScalar dy = pts[1].y() - pts[0].y();

    if ((dx && dy) || (!dx && !dy)) {
        return false;
    }

    SkRect bounds = *cullRect;
    outset_for_stroke(&bounds, rec);

    // cullRect is in device space while pts are in the local coordinate system
    // defined by the ctm. We want our answer in the local coordinate system.

    SkASSERT(ctm.rectStaysRect());
    SkMatrix inv;
    if (!ctm.invert(&inv)) {
        return false;
    }

    inv.mapRect(&bounds);

    if (dx) {
        SkASSERT(dx && !dy);
        SkScalar minX = pts[0].fX;
        SkScalar maxX = pts[1].fX;

        if (dx < 0) {
            using std::swap;
            swap(minX, maxX);
        }

        SkASSERT(minX < maxX);
        if (maxX <= bounds.fLeft || minX >= bounds.fRight) {
            return false;
        }

        // Now we actually perform the chop, removing the excess to the left and
        // right of the bounds (keeping our new line "in phase" with the dash,
        // hence the (mod intervalLength).

        if (minX < bounds.fLeft) {
            minX = bounds.fLeft - SkScalarMod(bounds.fLeft - minX, intervalLength);
        }
        if (maxX > bounds.fRight) {
            maxX = bounds.fRight + SkScalarMod(maxX - bounds.fRight, intervalLength);
        }

        SkASSERT(maxX > minX);
        if (dx < 0) {
            using std::swap;
            swap(minX, maxX);
        }
        pts[0].fX = minX;
        pts[1].fX = maxX;
    } else {
        SkASSERT(dy && !dx);
        SkScalar minY = pts[0].fY;
        SkScalar maxY = pts[1].fY;

        if (dy < 0) {
            using std::swap;
            swap(minY, maxY);
        }

        SkASSERT(minY < maxY);
        if (maxY <= bounds.fTop || minY >= bounds.fBottom) {
            return false;
        }

        // Now we actually perform the chop, removing the excess to the top and
        // bottom of the bounds (keeping our new line "in phase" with the dash,
        // hence the (mod intervalLength).

        if (minY < bounds.fTop) {
            minY = bounds.fTop - SkScalarMod(bounds.fTop - minY, intervalLength);
        }
        if (maxY > bounds.fBottom) {
            maxY = bounds.fBottom + SkScalarMod(maxY - bounds.fBottom, intervalLength);
        }

        SkASSERT(maxY > minY);
        if (dy < 0) {
            using std::swap;
            swap(minY, maxY);
        }
        pts[0].fY = minY;
        pts[1].fY = maxY;
    }

    return true;
}

// Currently asPoints is more restrictive then it needs to be. In the future
// we need to:
//      allow kRound_Cap capping (could allow rotations in the matrix with this)
//      allow paths to be returned
bool SkDashImpl::onAsPoints(PointData* results, const SkPath& src, const SkStrokeRec& rec,
                            const SkMatrix& matrix, const SkRect* cullRect) const {
    // width < 0 -> fill && width == 0 -> hairline so requiring width > 0 rules both out
    if (0 >= rec.getWidth()) {
        return false;
    }

    // TODO: this next test could be eased up. We could allow any number of
    // intervals as long as all the ons match and all the offs match.
    // Additionally, they do not necessarily need to be integers.
    // We cannot allow arbitrary intervals since we want the returned points
    // to be uniformly sized.
    if (fIntervals.size() != 2 ||
        !SkScalarNearlyEqual(fIntervals[0], fIntervals[1]) ||
        !SkScalarIsInt(fIntervals[0]) ||
        !SkScalarIsInt(fIntervals[1])) {
        return false;
    }

    SkPoint pts[2];

    if (!src.isLine(pts)) {
        return false;
    }

    // TODO: this test could be eased up to allow circles
    if (SkPaint::kButt_Cap != rec.getCap()) {
        return false;
    }

    // TODO: this test could be eased up for circles. Rotations could be allowed.
    if (!matrix.rectStaysRect()) {
        return false;
    }

    // See if the line can be limited to something plausible.
    if (!cull_line(pts, rec, matrix, cullRect, fIntervalLength)) {
        return false;
    }

    SkScalar length = SkPoint::Distance(pts[1], pts[0]);

    SkVector tangent = pts[1] - pts[0];
    if (tangent.isZero()) {
        return false;
    }

    tangent.scale(SkScalarInvert(length));

    // TODO: make this test for horizontal & vertical lines more robust
    bool isXAxis = true;
    if (SkScalarNearlyEqual(SK_Scalar1, tangent.fX) ||
        SkScalarNearlyEqual(-SK_Scalar1, tangent.fX)) {
        results->fSize.set(SkScalarHalf(fIntervals[0]), SkScalarHalf(rec.getWidth()));
    } else if (SkScalarNearlyEqual(SK_Scalar1, tangent.fY) ||
               SkScalarNearlyEqual(-SK_Scalar1, tangent.fY)) {
        results->fSize.set(SkScalarHalf(rec.getWidth()), SkScalarHalf(fIntervals[0]));
        isXAxis = false;
    } else if (SkPaint::kRound_Cap != rec.getCap()) {
        // Angled lines don't have axis-aligned boxes.
        return false;
    }

    if (results) {
        results->fFlags = 0;
        SkScalar clampedInitialDashLength = std::min(length, fInitialDashLength);

        if (SkPaint::kRound_Cap == rec.getCap()) {
            results->fFlags |= PointData::kCircles_PointFlag;
        }

        results->fNumPoints = 0;
        SkScalar len2 = length;
        if (clampedInitialDashLength > 0 || 0 == fInitialDashIndex) {
            SkASSERT(len2 >= clampedInitialDashLength);
            if (0 == fInitialDashIndex) {
                if (clampedInitialDashLength > 0) {
                    if (clampedInitialDashLength >= fIntervals[0]) {
                        ++results->fNumPoints;  // partial first dash
                    }
                    len2 -= clampedInitialDashLength;
                }
                len2 -= fIntervals[1];  // also skip first space
                if (len2 < 0) {
                    len2 = 0;
                }
            } else {
                len2 -= clampedInitialDashLength; // skip initial partial empty
            }
        }
        // Too many midpoints can cause results->fNumPoints to overflow or
        // otherwise cause the results->fPoints allocation below to OOM.
        // Cap it to a sane value.
        SkScalar numIntervals = len2 / fIntervalLength;
        if (!SkIsFinite(numIntervals) || numIntervals > SkDashPath::kMaxDashCount) {
            return false;
        }
        int numMidPoints = SkScalarFloorToInt(numIntervals);
        results->fNumPoints += numMidPoints;
        len2 -= numMidPoints * fIntervalLength;
        bool partialLast = false;
        if (len2 > 0) {
            if (len2 < fIntervals[0]) {
                partialLast = true;
            } else {
                ++numMidPoints;
                ++results->fNumPoints;
            }
        }

        results->fPoints = new SkPoint[results->fNumPoints];

        SkScalar    distance = 0;
        int         curPt = 0;

        if (clampedInitialDashLength > 0 || 0 == fInitialDashIndex) {
            SkASSERT(clampedInitialDashLength <= length);

            if (0 == fInitialDashIndex) {
                if (clampedInitialDashLength > 0) {
                    // partial first block
                    SkASSERT(SkPaint::kRound_Cap != rec.getCap()); // can't handle partial circles
                    SkScalar x = pts[0].fX + tangent.fX * SkScalarHalf(clampedInitialDashLength);
                    SkScalar y = pts[0].fY + tangent.fY * SkScalarHalf(clampedInitialDashLength);
                    SkScalar halfWidth, halfHeight;
                    if (isXAxis) {
                        halfWidth = SkScalarHalf(clampedInitialDashLength);
                        halfHeight = SkScalarHalf(rec.getWidth());
                    } else {
                        halfWidth = SkScalarHalf(rec.getWidth());
                        halfHeight = SkScalarHalf(clampedInitialDashLength);
                    }
                    if (clampedInitialDashLength < fIntervals[0]) {
                        // This one will not be like the others
                        results->fFirst.addRect(x - halfWidth, y - halfHeight,
                                                x + halfWidth, y + halfHeight);
                    } else {
                        SkASSERT(curPt < results->fNumPoints);
                        results->fPoints[curPt].set(x, y);
                        ++curPt;
                    }

                    distance += clampedInitialDashLength;
                }

                distance += fIntervals[1];  // skip over the next blank block too
            } else {
                distance += clampedInitialDashLength;
            }
        }

        if (0 != numMidPoints) {
            distance += SkScalarHalf(fIntervals[0]);

            for (int i = 0; i < numMidPoints; ++i) {
                SkScalar x = pts[0].fX + tangent.fX * distance;
                SkScalar y = pts[0].fY + tangent.fY * distance;

                SkASSERT(curPt < results->fNumPoints);
                results->fPoints[curPt].set(x, y);
                ++curPt;

                distance += fIntervalLength;
            }

            distance -= SkScalarHalf(fIntervals[0]);
        }

        if (partialLast) {
            // partial final block
            SkASSERT(SkPaint::kRound_Cap != rec.getCap()); // can't handle partial circles
            SkScalar temp = length - distance;
            SkASSERT(temp < fIntervals[0]);
            SkScalar x = pts[0].fX + tangent.fX * (distance + SkScalarHalf(temp));
            SkScalar y = pts[0].fY + tangent.fY * (distance + SkScalarHalf(temp));
            SkScalar halfWidth, halfHeight;
            if (isXAxis) {
                halfWidth = SkScalarHalf(temp);
                halfHeight = SkScalarHalf(rec.getWidth());
            } else {
                halfWidth = SkScalarHalf(rec.getWidth());
                halfHeight = SkScalarHalf(temp);
            }
            results->fLast.addRect(x - halfWidth, y - halfHeight,
                                   x + halfWidth, y + halfHeight);
        }

        SkASSERT(curPt == results->fNumPoints);
    }

    return true;
}

std::optional<SkPathEffectBase::DashInfo> SkDashImpl::asADash() const {
    return {{fIntervals, fPhase}};
}

void SkDashImpl::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fPhase);
    buffer.writeScalarArray(fIntervals);
}

sk_sp<SkFlattenable> SkDashImpl::CreateProc(SkReadBuffer& buffer) {
    const SkScalar phase = buffer.readScalar();
    uint32_t count = buffer.getArrayCount();

    // Don't allocate gigantic buffers if there's not data for them.
    if (!buffer.validateCanReadN<SkScalar>(count)) {
        return nullptr;
    }

    AutoSTArray<32, SkScalar> intervals(count);
    if (buffer.readScalarArray(intervals)) {
        return SkDashPathEffect::Make(intervals, phase);
    }
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkDashPathEffect::Make(SkSpan<const SkScalar> intervals, SkScalar phase) {
    if (!SkDashPath::ValidDashPath(phase, intervals)) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkDashImpl(intervals, phase));
}
