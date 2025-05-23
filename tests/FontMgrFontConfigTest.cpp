/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkPngEncoder.h"
#include "include/ports/SkFontMgr_fontconfig.h"

#ifdef SK_TYPEFACE_FACTORY_FONTATIONS
#include "include/ports/SkFontScanner_Fontations.h"
#endif
#ifdef SK_TYPEFACE_FACTORY_FREETYPE
#include "include/ports/SkFontScanner_FreeType.h"
#endif
#include "tests/Test.h"
#include "tools/Resources.h"

#include <fontconfig/fontconfig.h>

#include <array>
#include <memory>

namespace {

bool bitmap_compare(const SkBitmap& ref, const SkBitmap& test) {
    auto count = 0;
    for (int y = 0; y < test.height(); ++y) {
        for (int x = 0; x < test.width(); ++x) {
            SkColor testColor = test.getColor(x, y);
            SkColor refColor = ref.getColor(x, y);
            if (refColor != testColor) {
                ++count;
                if ((false)) {
                    SkDebugf("%d: (%d,%d) ", count, x, y);
                }
            }
        }
    }
    return (count == 0);
}

FcConfig* build_fontconfig_with_fontfile(const char* fontFilename) {
    FcConfig* config = FcConfigCreate();

    // FontConfig may modify the passed path (make absolute or other).
    FcConfigSetSysRoot(config, reinterpret_cast<const FcChar8*>(GetResourcePath("").c_str()));
    // FontConfig will lexically compare paths against its version of the sysroot.
    SkString fontFilePath(reinterpret_cast<const char*>(FcConfigGetSysRoot(config)));
    fontFilePath += fontFilename;
    FcConfigAppFontAddFile(config, reinterpret_cast<const FcChar8*>(fontFilePath.c_str()));

    FcConfigBuildFonts(config);
    return config;
}

FcConfig* build_fontconfig_from_resources() {

    SkString path = GetResourcePath("");
    SkString content;
    content.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">"
                   "<fontconfig>\n"
                   "<dir>/fonts</dir>\n");
    content.append("<match target=\"font\">\n"
                   "   <edit name=\"embolden\" mode=\"assign\">\n"
                   "       <bool>true</bool>\n"
                   "   </edit>\n"
                   "</match>");
    content.append("</fontconfig>\n");
    FcConfig* fc_config = FcConfigCreate();
    FcConfigSetSysRoot(fc_config, reinterpret_cast<const FcChar8*>(path.c_str()));
    if (FcConfigParseAndLoadFromMemory(
                fc_config, reinterpret_cast<const FcChar8*>(content.c_str()),
                FcTrue) != FcTrue) {
        SkDebugf("FcConfigParseAndLoadFromMemory\n");
    }
    if (FcConfigBuildFonts(fc_config) != FcTrue) {
        SkDebugf("!FcConfigBuildFonts\n");
    }
    return fc_config;
}

[[maybe_unused]]
static void write_bitmap(const SkBitmap* bm, const char fileName[]) {
    SkFILEWStream file(fileName);
    SkAssertResult(file.isValid());
    SkAssertResult(SkPngEncoder::Encode(&file, bm->pixmap(), {}));
}

}  // namespace

DEF_TEST(FontMgrFontConfig, reporter) {
    FcConfig* config = build_fontconfig_with_fontfile("/fonts/Distortable.ttf");

    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config, SkFontScanner_Make_FreeType()));
    sk_sp<SkTypeface> typeface(fontMgr->legacyMakeTypeface("Distortable", SkFontStyle()));
    if (!typeface) {
        ERRORF(reporter, "Could not find typeface. FcVersion: %d", FcGetVersion());
        return;
    }

    SkBitmap bitmapStream;
    bitmapStream.allocN32Pixels(64, 64);
    SkCanvas canvasStream(bitmapStream);
    canvasStream.drawColor(SK_ColorWHITE);

    SkBitmap bitmapClone;
    bitmapClone.allocN32Pixels(64, 64);
    SkCanvas canvasClone(bitmapClone);
    canvasStream.drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorGRAY);

    constexpr float kTextSize = 20;

    std::unique_ptr<SkStreamAsset> distortableStream(
            GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortableStream) {
        return;
    }

    SkPoint point = SkPoint::Make(20.0f, 20.0f);
    SkFourByteTag tag = SkSetFourByteTag('w', 'g', 'h', 't');

    for (int i = 0; i < 10; ++i) {
        SkScalar styleValue =
            SkDoubleToScalar(0.5 + i * ((2.0 - 0.5) / 10));
        SkFontArguments::VariationPosition::Coordinate
            coordinates[] = {{tag, styleValue}};
        SkFontArguments::VariationPosition
            position = {coordinates, std::size(coordinates)};

        SkFont fontStream(
            fontMgr->makeFromStream(distortableStream->duplicate(),
                                    SkFontArguments().setVariationDesignPosition(position)),
            kTextSize);
        fontStream.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        SkFont fontClone(
            typeface->makeClone(SkFontArguments().setVariationDesignPosition(position)), kTextSize);
        fontClone.setEdging(SkFont::Edging::kSubpixelAntiAlias);

        constexpr char text[] = "abc";

        canvasStream.drawColor(SK_ColorWHITE);
        canvasStream.drawString(text, point.fX, point.fY, fontStream, paint);

        canvasClone.drawColor(SK_ColorWHITE);
        canvasClone.drawString(text, point.fX, point.fY, fontClone, paint);

        bool success = bitmap_compare(bitmapStream, bitmapClone);
        REPORTER_ASSERT(reporter, success);
    }
}

void testAllBold(sk_sp<SkFontMgr> fontMgr, skiatest::Reporter* reporter) {
    constexpr float kTextSize = 20;
    constexpr char text[] = "abc";

    SkString filePath = GetResourcePath("fonts/Roboto-Regular.ttf");
    sk_sp<SkTypeface> dataTypeface(fontMgr->makeFromFile(filePath.c_str(), 0));
    if (!dataTypeface) {
        ERRORF(reporter, "Could not find data typeface. FcVersion: %d", FcGetVersion());
        return;
    }

    SkFont dataFont(dataTypeface, kTextSize);
    dataFont.setEmbolden(true);

    sk_sp<SkTypeface> matchTypeface(fontMgr->matchFamilyStyle("Roboto", SkFontStyle()));
    if (!matchTypeface) {
        ERRORF(reporter, "Could not find match typeface. FcVersion: %d", FcGetVersion());
        return;
    }
    SkFont matchFont(matchTypeface, kTextSize);

    SkBitmap bitmapData;
    bitmapData.allocN32Pixels(64, 64);
    SkCanvas canvasData(bitmapData);

    SkBitmap bitmapMatch;
    bitmapMatch.allocN32Pixels(64, 64);
    SkCanvas canvasMatch(bitmapMatch);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    canvasData.drawColor(SK_ColorGRAY);
    canvasData.drawString(text, 20.0f, 20.0f, dataFont, paint);
    if ((false)) {
        // In case we wonder what's been painted
        SkString dataPath = GetResourcePath("/fonts/data.png");
        write_bitmap(&bitmapData, dataPath.c_str());
    }

    canvasMatch.drawColor(SK_ColorGRAY);
    canvasMatch.drawString(text, 20.0f, 20.0f, matchFont, paint);
    if ((false)) {
        SkString matchPath = GetResourcePath("/fonts/match.png");
        write_bitmap(&bitmapMatch, matchPath.c_str());
    }

    bool success = bitmap_compare(bitmapData, bitmapMatch);
    REPORTER_ASSERT(reporter, success);
}

#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
DEF_TEST(FontMgrFontConfig_FreeType_AllBold, reporter) {

    FcConfig* config = build_fontconfig_from_resources();
    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config, SkFontScanner_Make_FreeType()));

    testAllBold(fontMgr, reporter);
}
#endif

#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
DEF_TEST(FontMgrFontConfig_Fontations_AllBold, reporter) {

    FcConfig* config = build_fontconfig_from_resources();
    sk_sp<SkFontMgr> fontMgr(SkFontMgr_New_FontConfig(config, SkFontScanner_Make_Fontations()));

    testAllBold(fontMgr, reporter);
}
#endif

#if defined(SK_TYPEFACE_FACTORY_FREETYPE) && defined(SK_TYPEFACE_FACTORY_FONTATIONS)
// The results may not match but it's still interesting to run sometimes
DEF_TEST_DISABLED(FontMgrFontConfig_MatchFonts, reporter) {
    FcConfig* config = build_fontconfig_from_resources();
    sk_sp<SkFontMgr> freeTypeFontMgr(
            SkFontMgr_New_FontConfig(config, SkFontScanner_Make_FreeType()));
    sk_sp<SkFontMgr> fontationsFontMgr(
            SkFontMgr_New_FontConfig(config, SkFontScanner_Make_Fontations()));

    constexpr float kTextSize = 20;
    constexpr char text[] = "abc";

    sk_sp<SkTypeface> dataTypeface(freeTypeFontMgr->matchFamilyStyle("Roboto", SkFontStyle()));
    if (!dataTypeface) {
        ERRORF(reporter, "Could not find data typeface. FcVersion: %d", FcGetVersion());
        return;
    }
    SkFont dataFont(dataTypeface, kTextSize);

    sk_sp<SkTypeface> matchTypeface(fontationsFontMgr->matchFamilyStyle("Roboto", SkFontStyle()));
    if (!matchTypeface) {
        ERRORF(reporter, "Could not find match typeface. FcVersion: %d", FcGetVersion());
        return;
    }
    SkFont matchFont(matchTypeface, kTextSize);

    SkBitmap bitmapData;
    bitmapData.allocN32Pixels(64, 64);
    SkCanvas canvasData(bitmapData);

    SkBitmap bitmapMatch;
    bitmapMatch.allocN32Pixels(64, 64);
    SkCanvas canvasMatch(bitmapMatch);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    canvasData.drawColor(SK_ColorGRAY);
    canvasData.drawString(text, 20.0f, 20.0f, dataFont, paint);
    if ((false)) {
        // In case we wonder what's been painted
        SkString dataPath = GetResourcePath("/fonts/data.png");
        write_bitmap(&bitmapData, dataPath.c_str());
    }

    canvasMatch.drawColor(SK_ColorGRAY);
    canvasMatch.drawString(text, 20.0f, 20.0f, matchFont, paint);
    if ((false)) {
        SkString matchPath = GetResourcePath("/fonts/match.png");
        write_bitmap(&bitmapMatch, matchPath.c_str());
    }

    bool success = bitmap_compare(bitmapData, bitmapMatch);
    REPORTER_ASSERT(reporter, success);
}
#endif
