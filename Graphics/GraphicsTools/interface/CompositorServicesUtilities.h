/*
 *  Copyright 2026 Diligent Graphics LLC
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#pragma once

/// \file
/// CompositorServices utilities for visionOS

#include "../../GraphicsEngine/interface/RenderDevice.h"
#include "../../GraphicsEngine/interface/DeviceContext.h"
#include "../../../Primitives/interface/BasicTypes.h"
#include "../../../Common/interface/BasicMath.hpp"

#include <CompositorServices/CompositorServices.h>

DILIGENT_BEGIN_NAMESPACE(Diligent)

#include "../../../Primitives/interface/DefineRefMacro.h"

/// Type of texture to retrieve from a CompositorServices drawable.
DILIGENT_TYPED_ENUM(COMPOSITOR_SERVICES_TEXTURE_TYPE, Uint8){
    /// Color texture (render target).
    COMPOSITOR_SERVICES_TEXTURE_TYPE_COLOR = 0,

    /// Depth texture.
    COMPOSITOR_SERVICES_TEXTURE_TYPE_DEPTH};

/// Returns the number of views (eyes) in a CompositorServices drawable.
///
/// \param [in] Drawable - CompositorServices drawable (cp_drawable_t).
/// \return Number of views (typically 2 for stereo rendering).
Uint32 DILIGENT_GLOBAL_FUNCTION(GetCompositorServicesViewCount)(cp_drawable_t Drawable);

/// Returns a Diligent ITexture wrapping the Metal texture from a CompositorServices drawable.
///
/// \param [in]  pDevice   - Pointer to the render device.
/// \param [in]  Drawable  - CompositorServices drawable (cp_drawable_t).
/// \param [in]  ViewIndex - Index of the view (eye) to get the texture for.
/// \param [in]  TexType   - Type of texture to retrieve (color or depth).
/// \param [out] ppTexture - Address of the memory location where the pointer to the
///                          texture object will be stored.
void DILIGENT_GLOBAL_FUNCTION(GetCompositorServicesSwapchainImage)(IRenderDevice*                   pDevice,
                                                                   cp_drawable_t                    Drawable,
                                                                   Uint32                           ViewIndex,
                                                                   COMPOSITOR_SERVICES_TEXTURE_TYPE TexType,
                                                                   ITexture**                       ppTexture);

/// Returns the projection matrix for a specific view in a CompositorServices drawable.
///
/// \param [in] Drawable  - CompositorServices drawable (cp_drawable_t).
/// \param [in] ViewIndex - Index of the view (eye).
/// \param [in] NearZ     - Near clipping plane distance.
/// \param [in] FarZ      - Far clipping plane distance.
/// \return The 4x4 projection matrix for the specified view.
float4x4 DILIGENT_GLOBAL_FUNCTION(GetCompositorServicesProjectionMatrix)(cp_drawable_t Drawable,
                                                                         Uint32        ViewIndex,
                                                                         float         NearZ,
                                                                         float         FarZ);

/// Returns the view transform matrix for a specific view in a CompositorServices drawable.
///
/// \param [in] Drawable  - CompositorServices drawable (cp_drawable_t).
/// \param [in] ViewIndex - Index of the view (eye).
/// \return The 4x4 view transform matrix for the specified view.
float4x4 DILIGENT_GLOBAL_FUNCTION(GetCompositorServicesViewTransform)(cp_drawable_t Drawable,
                                                                      Uint32        ViewIndex);

/// Presents the CompositorServices drawable by encoding a present command into the
/// Metal command buffer obtained from the device context.
///
/// \param [in] pContext  - Pointer to the device context.
/// \param [in] Drawable  - CompositorServices drawable (cp_drawable_t) to present.
///
/// This function hides all ObjC++/Metal details from the caller,
/// allowing tutorials to be pure C++.
void DILIGENT_GLOBAL_FUNCTION(PresentCompositorServicesDrawable)(IDeviceContext* pContext,
                                                                 cp_drawable_t   Drawable);

#include "../../../Primitives/interface/UndefRefMacro.h"

DILIGENT_END_NAMESPACE // namespace Diligent
