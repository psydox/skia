// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Paint_getShader, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("nullptr %c= shader\n", paint.getShader() ? '!' : '=');
   paint.setShader(SkShaders::Empty());
   SkDebugf("nullptr %c= shader\n", paint.getShader() ? '!' : '=');
}
}  // END FIDDLE
