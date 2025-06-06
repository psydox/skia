/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkTypes.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/chromium/SkPMColor.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkRandom.h"
#include "src/core/SkColorData.h"

#include "tests/Test.h"

DEF_TEST(ColorPremul, reporter) {
    for (int a = 0; a <= 255; a++) {
        for (int x = 0; x <= 255; x++) {
            SkColor c0 = SkColorSetARGB(a, x, x, x);
            SkPMColor p0 = SkPreMultiplyColor(c0);

            SkColor c1 = SkUnPreMultiply::PMColorToColor(p0);
            SkPMColor p1 = SkPreMultiplyColor(c1);

            // we can't promise that c0 == c1, since c0 -> p0 is a many to one
            // function, however, we can promise that p0 -> c1 -> p1 : p0 == p1
            REPORTER_ASSERT(reporter, p0 == p1);

            {
                int ax = SkMulDiv255Ceiling(x, a);
                REPORTER_ASSERT(reporter, ax <= a);
            }
        }
    }
}

DEF_TEST(SkPMColor_SetAndRetrieveChannels, reporter) {
    SkPMColor pmc = SkPMColorSetARGB(0xFE, 0xDC, 0xBA, 0x98);

#if defined(SK_PMCOLOR_IS_RGBA)
    REPORTER_ASSERT(reporter, pmc == 0xFE98BADC);
#else
    REPORTER_ASSERT(reporter, pmc == 0xFEDCBA98);
#endif

    REPORTER_ASSERT(reporter, SkPMColorGetA(pmc) == 0xFE);
    REPORTER_ASSERT(reporter, SkPMColorGetR(pmc) == 0xDC);
    REPORTER_ASSERT(reporter, SkPMColorGetG(pmc) == 0xBA);
    REPORTER_ASSERT(reporter, SkPMColorGetB(pmc) == 0x98);
}

/**
  This test fails: SkFourByteInterp does *not* preserve opaque destinations.
  SkAlpha255To256 implemented as (alpha + 1) is faster than
  (alpha + (alpha >> 7)), but inaccurate, and Skia intends to phase it out.
*/
DEF_TEST(ColorInterp, reporter) {
    SkRandom r;

    U8CPU a0 = 0;
    U8CPU a255 = 255;
    for (int i = 0; i < 200; i++) {
        SkColor colorSrc = r.nextU();
        SkColor colorDst = r.nextU();
        SkPMColor src = SkPreMultiplyColor(colorSrc);
        SkPMColor dst = SkPreMultiplyColor(colorDst);

        if ((false)) {
            REPORTER_ASSERT(reporter, SkFourByteInterp(src, dst, a0) == dst);
            REPORTER_ASSERT(reporter, SkFourByteInterp(src, dst, a255) == src);
        }
    }
}

DEF_TEST(ColorFastIterp, reporter) {
    SkRandom r;

    U8CPU a0 = 0;
    U8CPU a255 = 255;
    for (int i = 0; i < 200; i++) {
        SkColor colorSrc = r.nextU();
        SkColor colorDst = r.nextU();
        SkPMColor src = SkPreMultiplyColor(colorSrc);
        SkPMColor dst = SkPreMultiplyColor(colorDst);

        REPORTER_ASSERT(reporter, SkFastFourByteInterp(src, dst, a0) == dst);
        REPORTER_ASSERT(reporter, SkFastFourByteInterp(src, dst, a255) == src);
    }
}
