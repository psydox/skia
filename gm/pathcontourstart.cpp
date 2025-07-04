/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"

using namespace skia_private;

namespace skiagm {

class ContourStartGM : public GM {
protected:
    void onOnceBeforeDraw() override {
        const SkScalar kMaxDashLen = 100;
        const SkScalar kDashGrowth = 1.2f;

        STArray<100, SkScalar> intervals;
        for (SkScalar len = 1; len < kMaxDashLen; len *= kDashGrowth) {
            intervals.push_back(len);
            intervals.push_back(len);
        }

        fDashPaint.setAntiAlias(true);
        fDashPaint.setStyle(SkPaint::kStroke_Style);
        fDashPaint.setStrokeWidth(6);
        fDashPaint.setColor(0xff008000);
        fDashPaint.setPathEffect(SkDashPathEffect::Make(intervals, 0));

        fPointsPaint.setColor(0xff800000);
        fPointsPaint.setStrokeWidth(3);

        fRect = SkRect::MakeLTRB(10, 10, 100, 70);
    }

    SkString getName() const override { return SkString("contour_start"); }

    SkISize getISize() override { return SkISize::Make(kImageWidth, kImageHeight); }

    void onDraw(SkCanvas* canvas) override {

        drawDirs(canvas, [](const SkRect& rect, SkPathDirection dir, unsigned startIndex) {
            return SkPath::Rect(rect, dir, startIndex);
        });

        drawDirs(canvas, [](const SkRect& rect, SkPathDirection dir, unsigned startIndex) {
            return SkPath::Oval(rect, dir, startIndex);
        });

        drawDirs(canvas, [](const SkRect& rect, SkPathDirection dir, unsigned startIndex) {
            SkRRect rrect;
            const SkVector radii[4] = { {15, 15}, {15, 15}, {15, 15}, {15, 15}};
            rrect.setRectRadii(rect, radii);
            return SkPath::RRect(rrect, dir, startIndex);
        });

        drawDirs(canvas, [](const SkRect& rect, SkPathDirection dir, unsigned startIndex) {
            SkRRect rrect;
            rrect.setRect(rect);
            return SkPath::RRect(rrect, dir, startIndex);
        });

        drawDirs(canvas, [](const SkRect& rect, SkPathDirection dir, unsigned startIndex) {
            SkRRect rrect;
            rrect.setOval(rect);
            return SkPath::RRect(rrect, dir, startIndex);
        });

    }

private:
    inline static constexpr int kImageWidth = 1200;
    inline static constexpr int kImageHeight = 600;

    SkPaint fDashPaint, fPointsPaint;
    SkRect  fRect;

    void drawDirs(SkCanvas* canvas,
                  SkPath (*makePath)(const SkRect&, SkPathDirection, unsigned)) const {
        drawOneColumn(canvas, SkPathDirection::kCW, makePath);
        canvas->translate(kImageWidth / 10, 0);
        drawOneColumn(canvas, SkPathDirection::kCCW, makePath);
        canvas->translate(kImageWidth / 10, 0);
    }

    void drawOneColumn(SkCanvas* canvas, SkPathDirection dir,
                       SkPath (*makePath)(const SkRect&, SkPathDirection, unsigned)) const {
        SkAutoCanvasRestore acr(canvas, true);

        for (unsigned i = 0; i < 8; ++i) {
            const SkPath path = makePath(fRect, dir, i);
            canvas->drawPath(path, fDashPaint);

            const int n = path.countPoints();
            AutoTArray<SkPoint> points(n);
            path.getPoints(points);
            canvas->drawPoints(SkCanvas::kPoints_PointMode, points, fPointsPaint);

            canvas->translate(0, kImageHeight / 8);
        }
    }

    using INHERITED = GM;
};

DEF_GM( return new ContourStartGM(); )

} // namespace skiagm
