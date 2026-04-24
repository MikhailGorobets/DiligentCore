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

#include "CompositorServicesUtilities.h"

#include "RenderDeviceMtl.h"
#include "DeviceContextMtl.h"
#include "DebugUtilities.hpp"

#import <Metal/Metal.h>
#import <Spatial/Spatial.h>
#include <CompositorServices/CompositorServices.h>
#include <simd/simd.h>

namespace Diligent
{

Uint32 GetCompositorServicesViewCount(cp_drawable_t Drawable)
{
    if (Drawable == nullptr)
    {
        UNEXPECTED("Drawable must not be null");
        return 0;
    }

    return static_cast<Uint32>(cp_drawable_get_view_count(Drawable));
}

void GetCompositorServicesSwapchainImage(IRenderDevice*                   pDevice,
                                         cp_drawable_t                    Drawable,
                                         Uint32                           ViewIndex,
                                         COMPOSITOR_SERVICES_TEXTURE_TYPE TexType,
                                         ITexture**                       ppTexture)
{
    if (pDevice == nullptr)
    {
        UNEXPECTED("pDevice must not be null");
        return;
    }

    if (Drawable == nullptr)
    {
        UNEXPECTED("Drawable must not be null");
        return;
    }

    if (ppTexture == nullptr)
    {
        UNEXPECTED("ppTexture must not be null");
        return;
    }

    id<MTLTexture> mtlTexture = nil;
    switch (TexType)
    {
        case COMPOSITOR_SERVICES_TEXTURE_TYPE_COLOR:
            mtlTexture = cp_drawable_get_color_texture(Drawable, ViewIndex);
            break;
        case COMPOSITOR_SERVICES_TEXTURE_TYPE_DEPTH:
            mtlTexture = cp_drawable_get_depth_texture(Drawable, ViewIndex);
            break;
        default:
            UNEXPECTED("Unknown texture type");
            return;
    }

    if (mtlTexture == nil)
    {
        UNEXPECTED("Failed to get Metal texture from CompositorServices drawable");
        return;
    }

    RefCntAutoPtr<IRenderDeviceMtl> pDeviceMtl{pDevice, IID_RenderDeviceMtl};
    if (!pDeviceMtl)
    {
        UNEXPECTED("Failed to query IRenderDeviceMtl interface");
        return;
    }

    RESOURCE_STATE InitialState = (TexType == COMPOSITOR_SERVICES_TEXTURE_TYPE_DEPTH) ?
        RESOURCE_STATE_DEPTH_WRITE : RESOURCE_STATE_RENDER_TARGET;

    pDeviceMtl->CreateTextureFromMtlResource(mtlTexture, InitialState, ppTexture);
}

float4x4 GetCompositorServicesProjectionMatrix(cp_drawable_t Drawable,
                                               Uint32        ViewIndex,
                                               float         NearZ,
                                               float         FarZ)
{
    if (Drawable == nullptr)
    {
        UNEXPECTED("Drawable must not be null");
        return float4x4::Identity();
    }

    cp_view_t   View     = cp_drawable_get_view(Drawable, ViewIndex);
    simd_float4 Tangents = cp_view_get_tangents(View);

    // Tangents vector: [left, right, top, bottom], all positive values.
    // Negate left/bottom for the frustum convention used in the projection matrix.
    float Left   = -Tangents[0];
    float Right  =  Tangents[1];
    float Top    =  Tangents[2];
    float Bottom = -Tangents[3];

    float Width  = Right - Left;
    float Height = Top - Bottom;

    // clang-format off
    float4x4 Proj{
        2.0f / Width,            0.0f,                        0.0f,                          0.0f,
        0.0f,                    2.0f / Height,               0.0f,                          0.0f,
        -(Right + Left) / Width, -(Top + Bottom) / Height,    FarZ / (FarZ - NearZ),         1.0f,
        0.0f,                    0.0f,                        -NearZ * FarZ / (FarZ - NearZ), 0.0f
    };
    // clang-format on

    return Proj;
}

float4x4 GetCompositorServicesViewTransform(cp_drawable_t Drawable,
                                            Uint32        ViewIndex)
{
    if (Drawable == nullptr)
    {
        UNEXPECTED("Drawable must not be null");
        return float4x4::Identity();
    }

    cp_view_t     View      = cp_drawable_get_view(Drawable, ViewIndex);
    simd_float4x4 Transform = cp_view_get_transform(View);

    // Convert from simd_float4x4 (column-major) to Diligent float4x4 (row-major)
    float4x4 Result;
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            Result[row][col] = Transform.columns[col][row];
        }
    }

    return Result;
}

void PresentCompositorServicesDrawable(IDeviceContext* pContext,
                                       cp_drawable_t   Drawable)
{
    if (pContext == nullptr)
    {
        UNEXPECTED("pContext must not be null");
        return;
    }

    if (Drawable == nullptr)
    {
        UNEXPECTED("Drawable must not be null");
        return;
    }

    RefCntAutoPtr<IDeviceContextMtl> pContextMtl{pContext, IID_DeviceContextMtl};
    if (!pContextMtl)
    {
        UNEXPECTED("Failed to query IDeviceContextMtl interface");
        return;
    }

    id<MTLCommandBuffer> mtlCmdBuffer = pContextMtl->GetMtlCommandBuffer();
    if (mtlCmdBuffer == nil)
    {
        UNEXPECTED("Failed to get Metal command buffer from device context");
        return;
    }

    cp_drawable_encode_present(Drawable, mtlCmdBuffer);
}

} // namespace Diligent


extern "C"
{
    Diligent::Uint32 Diligent_GetCompositorServicesViewCount(cp_drawable_t Drawable)
    {
        return Diligent::GetCompositorServicesViewCount(Drawable);
    }

    void Diligent_GetCompositorServicesSwapchainImage(Diligent::IRenderDevice*                   pDevice,
                                                      cp_drawable_t                               Drawable,
                                                      Diligent::Uint32                            ViewIndex,
                                                      Diligent::COMPOSITOR_SERVICES_TEXTURE_TYPE  TexType,
                                                      Diligent::ITexture**                        ppTexture)
    {
        Diligent::GetCompositorServicesSwapchainImage(pDevice, Drawable, ViewIndex, TexType, ppTexture);
    }

    Diligent::float4x4 Diligent_GetCompositorServicesProjectionMatrix(cp_drawable_t    Drawable,
                                                                      Diligent::Uint32 ViewIndex,
                                                                      float            NearZ,
                                                                      float            FarZ)
    {
        return Diligent::GetCompositorServicesProjectionMatrix(Drawable, ViewIndex, NearZ, FarZ);
    }

    Diligent::float4x4 Diligent_GetCompositorServicesViewTransform(cp_drawable_t    Drawable,
                                                                   Diligent::Uint32 ViewIndex)
    {
        return Diligent::GetCompositorServicesViewTransform(Drawable, ViewIndex);
    }

    void Diligent_PresentCompositorServicesDrawable(Diligent::IDeviceContext* pContext,
                                                    cp_drawable_t             Drawable)
    {
        Diligent::PresentCompositorServicesDrawable(pContext, Drawable);
    }
}
