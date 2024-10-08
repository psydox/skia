/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkBackendSurfacePriv_DEFINED
#define GrVkBackendSurfacePriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/vk/GrVkTypes.h"
#include "include/private/base/SkAPI.h"

class GrBackendTexture;
class GrBackendRenderTarget;

namespace skgpu {
class MutableTextureState;
}

namespace GrBackendTextures {

SK_API GrBackendTexture MakeVk(int width,
                               int height,
                               const GrVkImageInfo&,
                               sk_sp<skgpu::MutableTextureState>);

}  // namespace GrBackendTextures


namespace GrBackendRenderTargets {

SK_API GrBackendRenderTarget MakeVk(int width,
                                    int height,
                                    const GrVkImageInfo&,
                                    sk_sp<skgpu::MutableTextureState>);

}  // namespace GrBackendRenderTargets

#endif
