/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkRandom.h"
#include "src/core/SkAAClip.h"
#include "src/core/SkMask.h"
#include "src/core/SkRasterClip.h"
#include "tests/Test.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <string>

static bool operator==(const SkMask& a, const SkMask& b) {
    if (a.fFormat != b.fFormat || a.fBounds != b.fBounds) {
        return false;
    }
    if (!a.fImage && !b.fImage) {
        return true;
    }
    if (!a.fImage || !b.fImage) {
        return false;
    }

    size_t wbytes = a.fBounds.width();
    switch (a.fFormat) {
        case SkMask::kBW_Format:
            wbytes = (wbytes + 7) >> 3;
            break;
        case SkMask::kA8_Format:
        case SkMask::k3D_Format:
            break;
        case SkMask::kLCD16_Format:
            wbytes <<= 1;
            break;
        case SkMask::kARGB32_Format:
            wbytes <<= 2;
            break;
        default:
            SkDEBUGFAIL("unknown mask format");
            return false;
    }

    const int h = a.fBounds.height();
    const char* aptr = (const char*)a.fImage;
    const char* bptr = (const char*)b.fImage;
    for (int y = 0; y < h; ++y) {
        if (0 != memcmp(aptr, bptr, wbytes)) {
            return false;
        }
        aptr += wbytes;
        bptr += wbytes;
    }
    return true;
}

static void copyToMask(const SkRegion& rgn, SkMaskBuilder* mask) {
    mask->format() = SkMask::kA8_Format;

    if (rgn.isEmpty()) {
        mask->bounds().setEmpty();
        mask->rowBytes() = 0;
        mask->image() = nullptr;
        return;
    }

    mask->bounds() = rgn.getBounds();
    mask->rowBytes() = mask->fBounds.width();
    mask->image() = SkMaskBuilder::AllocImage(mask->computeImageSize());
    sk_bzero(mask->image(), mask->computeImageSize());

    SkImageInfo info = SkImageInfo::Make(mask->fBounds.width(),
                                         mask->fBounds.height(),
                                         kAlpha_8_SkColorType,
                                         kPremul_SkAlphaType);
    SkBitmap bitmap;
    bitmap.installPixels(info, mask->image(), mask->fRowBytes);

    // canvas expects its coordinate system to always be 0,0 in the top/left
    // so we translate the rgn to match that before drawing into the mask.
    //
    SkRegion tmpRgn(rgn);
    tmpRgn.translate(-rgn.getBounds().fLeft, -rgn.getBounds().fTop);

    SkCanvas canvas(bitmap);
    canvas.clipRegion(tmpRgn);
    canvas.drawColor(SK_ColorBLACK);
}

static void copyToMask(const SkRasterClip& rc, SkMaskBuilder* mask) {
    if (rc.isBW()) {
        copyToMask(rc.bwRgn(), mask);
    } else {
        rc.aaRgn().copyToMask(mask);
    }
}

static bool operator==(const SkRasterClip& a, const SkRasterClip& b) {
    if (a.isEmpty() && b.isEmpty()) {
        return true;
    } else if (a.isEmpty() != b.isEmpty() || a.isBW() != b.isBW() || a.isRect() != b.isRect()) {
        return false;
    }

    SkMaskBuilder mask0, mask1;
    copyToMask(a, &mask0);
    copyToMask(b, &mask1);
    SkAutoMaskFreeImage free0(mask0.image());
    SkAutoMaskFreeImage free1(mask1.image());
    return mask0 == mask1;
}

static SkIRect rand_rect(SkRandom& rand, int n) {
    int x = rand.nextS() % n;
    int y = rand.nextS() % n;
    int w = rand.nextU() % n;
    int h = rand.nextU() % n;
    return SkIRect::MakeXYWH(x, y, w, h);
}

static void make_rand_rgn(SkRegion* rgn, SkRandom& rand) {
    int count = rand.nextU() % 20;
    for (int i = 0; i < count; ++i) {
        rgn->op(rand_rect(rand, 100), SkRegion::kXOR_Op);
    }
}

static bool operator==(const SkRegion& rgn, const SkAAClip& aaclip) {
    SkMaskBuilder mask0, mask1;

    copyToMask(rgn, &mask0);
    aaclip.copyToMask(&mask1);
    SkAutoMaskFreeImage free0(mask0.image());
    SkAutoMaskFreeImage free1(mask1.image());
    return mask0 == mask1;
}

static bool equalsAAClip(const SkRegion& rgn) {
    SkAAClip aaclip;
    aaclip.setRegion(rgn);
    return rgn == aaclip;
}

static void setRgnToPath(SkRegion* rgn, const SkPath& path) {
    SkIRect ir;
    path.getBounds().round(&ir);
    rgn->setPath(path, SkRegion(ir));
}

DEF_TEST(AAClip_setPath_RandomRegion_MatchesSkRegion, reporter) {
    SkRandom rand;
    for (int i = 0; i < 1000; i++) {
        SkRegion rgn;
        make_rand_rgn(&rgn, rand);
        REPORTER_ASSERT(reporter, equalsAAClip(rgn));
    }

    {
        SkRegion rgn;
        SkPath path;
        path.addCircle(0, 0, SkIntToScalar(30));
        setRgnToPath(&rgn, path);
        REPORTER_ASSERT(reporter, equalsAAClip(rgn));

        path.reset();
        path.moveTo(0, 0);
        path.lineTo(SkIntToScalar(100), 0);
        path.lineTo(SkIntToScalar(100 - 20), SkIntToScalar(20));
        path.lineTo(SkIntToScalar(20), SkIntToScalar(20));
        setRgnToPath(&rgn, path);
        REPORTER_ASSERT(reporter, equalsAAClip(rgn));
    }
}

static void imoveTo(SkPath& path, int x, int y) {
    path.moveTo(SkIntToScalar(x), SkIntToScalar(y));
}

static void icubicTo(SkPath& path, int x0, int y0, int x1, int y1, int x2, int y2) {
    path.cubicTo(SkIntToScalar(x0), SkIntToScalar(y0),
                 SkIntToScalar(x1), SkIntToScalar(y1),
                 SkIntToScalar(x2), SkIntToScalar(y2));
}

DEF_TEST(AAClip_setPath_ClipBoundsMatchExpectations, reporter) {
    SkPath path;
    SkAAClip clip;
    const int height = 40;
    const SkScalar sheight = SkIntToScalar(height);

    path.addOval(SkRect::MakeWH(sheight, sheight));
    REPORTER_ASSERT(reporter, sheight == path.getBounds().height());
    clip.setPath(path, path.getBounds().roundOut(), true);
    REPORTER_ASSERT(reporter, height == clip.getBounds().height());

    // this is the trimmed height of this cubic (with aa). The critical thing
    // for this test is that it is less than height, which represents just
    // the bounds of the path's control-points.
    //
    // This used to fail until we tracked the MinY in the BuilderBlitter.
    //
    const int teardrop_height = 12;
    path.reset();
    imoveTo(path, 0, 20);
    icubicTo(path, 40, 40, 40, 0, 0, 20);
    REPORTER_ASSERT(reporter, sheight == path.getBounds().height());
    clip.setPath(path, path.getBounds().roundOut(), true);
    REPORTER_ASSERT(reporter, teardrop_height == clip.getBounds().height());
}

DEF_TEST(AAClip_BasicFunctionality, reporter) {
    SkAAClip clip;

    REPORTER_ASSERT(reporter, clip.isEmpty());
    REPORTER_ASSERT(reporter, clip.getBounds().isEmpty());

    clip.translate(10, 10, &clip);    // should have no effect on empty
    REPORTER_ASSERT(reporter, clip.isEmpty());
    REPORTER_ASSERT(reporter, clip.getBounds().isEmpty());

    SkIRect r = { 10, 10, 40, 50 };
    clip.setRect(r);
    REPORTER_ASSERT(reporter, !clip.isEmpty());
    REPORTER_ASSERT(reporter, !clip.getBounds().isEmpty());
    REPORTER_ASSERT(reporter, clip.getBounds() == r);

    clip.setEmpty();
    REPORTER_ASSERT(reporter, clip.isEmpty());
    REPORTER_ASSERT(reporter, clip.getBounds().isEmpty());

    SkMaskBuilder mask;
    clip.copyToMask(&mask);
    REPORTER_ASSERT(reporter, nullptr == mask.fImage);
    REPORTER_ASSERT(reporter, mask.fBounds.isEmpty());
}

static void rand_irect(SkIRect* r, int N, SkRandom& rand) {
    r->setXYWH(0, 0, rand.nextU() % N, rand.nextU() % N);
    int dx = rand.nextU() % (2*N);
    int dy = rand.nextU() % (2*N);
    // use int dx,dy to make the subtract be signed
    r->offset(N - dx, N - dy);
}

DEF_TEST(AAClip_setRect_RandomRects_MatchesSkRegion, reporter) {
    SkRandom rand;

    for (int i = 0; i < 10000; i++) {
        SkAAClip clip0, clip1;
        SkRegion rgn0, rgn1;
        SkIRect r0, r1;

        rand_irect(&r0, 10, rand);
        rand_irect(&r1, 10, rand);
        clip0.setRect(r0);
        clip1.setRect(r1);
        rgn0.setRect(r0);
        rgn1.setRect(r1);
        for (SkClipOp op : {SkClipOp::kDifference, SkClipOp::kIntersect}) {
            SkAAClip clip2 = clip0; // leave clip0 unchanged for future iterations
            SkRegion rgn2;
            bool nonEmptyAA = clip2.op(clip1, op);
            bool nonEmptyBW = rgn2.op(rgn0, rgn1, (SkRegion::Op) op);
            if (nonEmptyAA != nonEmptyBW || clip2.getBounds() != rgn2.getBounds()) {
                ERRORF(reporter, "%s %s "
                       "[%d %d %d %d] %s [%d %d %d %d] = BW:[%d %d %d %d] AA:[%d %d %d %d]\n",
                       nonEmptyAA == nonEmptyBW ? "true" : "false",
                       clip2.getBounds() == rgn2.getBounds() ? "true" : "false",
                       r0.fLeft, r0.fTop, r0.right(), r0.bottom(),
                       op == SkClipOp::kDifference ? "DIFF" : "INTERSECT",
                       r1.fLeft, r1.fTop, r1.right(), r1.bottom(),
                       rgn2.getBounds().fLeft, rgn2.getBounds().fTop,
                       rgn2.getBounds().right(), rgn2.getBounds().bottom(),
                       clip2.getBounds().fLeft, clip2.getBounds().fTop,
                       clip2.getBounds().right(), clip2.getBounds().bottom());
            }

            SkMaskBuilder maskBW, maskAA;
            copyToMask(rgn2, &maskBW);
            clip2.copyToMask(&maskAA);
            SkAutoMaskFreeImage freeBW(maskBW.image());
            SkAutoMaskFreeImage freeAA(maskAA.image());
            REPORTER_ASSERT(reporter, maskBW == maskAA);
        }
    }
}

DEF_TEST(AAClip_setPath_PathHasHole_MaskIsCorrect, reporter) {
    static const uint8_t gExpectedImage[] = {
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
    };
    SkMask expected(gExpectedImage, SkIRect::MakeWH(4, 6), 4, SkMask::kA8_Format);

    SkPath path;
    path.addRect(SkRect::MakeXYWH(0, 0,
                                  SkIntToScalar(4), SkIntToScalar(2)));
    path.addRect(SkRect::MakeXYWH(0, SkIntToScalar(4),
                                  SkIntToScalar(4), SkIntToScalar(2)));

    {
        skiatest::ReporterContext context(reporter, "noAA");
        SkAAClip clip;
        clip.setPath(path, path.getBounds().roundOut(), false);

        SkMaskBuilder mask;
        clip.copyToMask(&mask);
        SkAutoMaskFreeImage freeM(mask.image());

        REPORTER_ASSERT(reporter, expected == mask);
    }
    {
        skiatest::ReporterContext context(reporter, "withAA");
        SkAAClip clip;
        clip.setPath(path, path.getBounds().roundOut(), true);

        SkMaskBuilder mask;
        clip.copyToMask(&mask);
        SkAutoMaskFreeImage freeM(mask.image());

        REPORTER_ASSERT(reporter, expected == mask);
    }
}

DEF_TEST(AAClip_RRectIsReallyARect_ClipIsRect, reporter) {
    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeWH(100, 100), 5, 5);

    SkPath path;
    path.addRRect(rrect);

    SkAAClip clip;
    clip.setPath(path, path.getBounds().roundOut(), true);

    REPORTER_ASSERT(reporter, clip.getBounds() == SkIRect::MakeWH(100, 100));
    REPORTER_ASSERT(reporter, !clip.isRect());

    // This rect should intersect the clip, but slice-out all of the "soft" parts,
    // leaving just a rect.
    const SkIRect ir = SkIRect::MakeLTRB(10, -10, 50, 90);

    clip.op(ir, SkClipOp::kIntersect);

    REPORTER_ASSERT(reporter, clip.getBounds() == SkIRect::MakeLTRB(10, 0, 50, 90));
    // the clip recognized that that it is just a rect!
    REPORTER_ASSERT(reporter, clip.isRect());
}

static void did_dx_affect(skiatest::Reporter* reporter, const SkScalar dx[],
                          size_t count, bool changed) {
    SkIRect ir = { 0, 0, 10, 10 };

    for (size_t i = 0; i < count; ++i) {
        SkRect r;
        r.set(ir);

        SkRasterClip rc0(ir);
        SkRasterClip rc1(ir);
        SkRasterClip rc2(ir);

        rc0.op(r, SkMatrix::I(), SkClipOp::kIntersect, false);
        r.offset(dx[i], 0);
        rc1.op(r, SkMatrix::I(), SkClipOp::kIntersect, true);
        r.offset(-2*dx[i], 0);
        rc2.op(r, SkMatrix::I(), SkClipOp::kIntersect, true);

        REPORTER_ASSERT(reporter, changed != (rc0 == rc1));
        REPORTER_ASSERT(reporter, changed != (rc0 == rc2));
    }
}

DEF_TEST(AAClip_op_NearlyIntegral_GenerateSameRasterClips, reporter) {
    // All of these should generate equivalent rasterclips

    static const SkScalar gSafeX[] = {
        0, SK_Scalar1/1000, SK_Scalar1/100, SK_Scalar1/10,
    };
    did_dx_affect(reporter, gSafeX, std::size(gSafeX), false);

    static const SkScalar gUnsafeX[] = {
        SK_Scalar1/4, SK_Scalar1/3,
    };
    did_dx_affect(reporter, gUnsafeX, std::size(gUnsafeX), true);
}

DEF_TEST(AAClip_setPath_AvoidAssertion, reporter) {
    // Should not assert in the debug build
    // bug was introduced in rev. 3209
    SkAAClip clip;
    SkRect r;
    r.fLeft = 129.892181f;
    r.fTop = 10.3999996f;
    r.fRight = 130.892181f;
    r.fBottom = 20.3999996f;
    REPORTER_ASSERT(reporter, clip.setPath(SkPath::Rect(r), r.roundOut(), true));
}

// Building aaclip meant aa-scan-convert a path into a huge clip.
// the old algorithm sized the supersampler to the size of the clip, which overflowed
// its internal 16bit coordinates. The fix was to intersect the clip+path_bounds before
// sizing the supersampler.
//
// Before the fix, the following code would assert in debug builds.
//
DEF_TEST(AAClip_crbug_422693_AvoidOverflow, reporter) {
    SkRasterClip rc(SkIRect::MakeLTRB(-25000, -25000, 25000, 25000));
    SkPath path;
    path.addCircle(50, 50, 50);
    REPORTER_ASSERT(reporter, rc.op(path, SkMatrix::I(), SkClipOp::kIntersect, true));
}

DEF_TEST(AAClip_setRect_HugeRect_ReturnsFalse, reporter) {
    SkAAClip clip;
    int big = 0x70000000;
    SkIRect r = { -big, -big, big, big };
    SkASSERT(r.width() < 0 && r.height() < 0);

    REPORTER_ASSERT(reporter, !clip.setRect(r));
}

DEF_TEST(AAClip_setPath_LargePathSmallClip_StillBlits, reporter) {
    // This test verifies the root cause of https://bugzilla.mozilla.org/show_bug.cgi?id=1909796
    // does not regress.
    SkAAClip clip;

    // Be advised that 2^31 will get turned into a float...
    SkPath largePath = SkPath::Rect(SkRect::MakeLTRB(-1000, 10,
         std::numeric_limits<int32_t>::max(), 20));
    SkIRect bounds;
    // ... and then back into an integer, so it won't be 2^31 any more
    // (e.g. 2147483520). Therefore, we pick the left to be big enough
    // to make the bounds exceed 31 bits again.
    largePath.getBounds().roundOut(&bounds);
    // SkIRect expects to work with 32 bit integers. If the width
    // or height exceeds 32 bits, isEmpty() returns •true•
    SkASSERT(bounds.isEmpty());
    // But the 64 bit version works fine.
    SkASSERT(!bounds.isEmpty64());

    // Make sure the clip overlaps a little bit
    SkIRect smallClip = SkIRect::MakeLTRB(5, 5, 15, 15);
    SkASSERT(bounds.intersect(smallClip));

    REPORTER_ASSERT(reporter, clip.setPath(largePath, smallClip, true));
    REPORTER_ASSERT(reporter, clip.setPath(largePath, smallClip, false));
}
