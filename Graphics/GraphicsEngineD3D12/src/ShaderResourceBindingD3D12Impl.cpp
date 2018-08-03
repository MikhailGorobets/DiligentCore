/*     Copyright 2015-2018 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
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

#include "pch.h"
#include "ShaderResourceBindingD3D12Impl.h"
#include "PipelineStateD3D12Impl.h"
#include "ShaderD3D12Impl.h"
#include "RenderDeviceD3D12Impl.h"

namespace Diligent
{

ShaderResourceBindingD3D12Impl::ShaderResourceBindingD3D12Impl(IReferenceCounters*     pRefCounters,
                                                               PipelineStateD3D12Impl* pPSO,
                                                               bool                    IsPSOInternal) :
    TBase( pRefCounters, pPSO, IsPSOInternal ),
    m_ShaderResourceCache(ShaderResourceCacheD3D12::DbgCacheContentType::SRBResources),
    m_NumShaders(pPSO->GetNumShaders())
{
    auto* ppShaders = pPSO->GetShaders();

    auto* pRenderDeviceD3D12Impl = ValidatedCast<RenderDeviceD3D12Impl>(pPSO->GetDevice());
    auto& ResCacheDataAllocator = pPSO->GetSRBMemoryAllocator().GetResourceCacheDataAllocator(0);
    pPSO->GetRootSignature().InitResourceCache(pRenderDeviceD3D12Impl, m_ShaderResourceCache, ResCacheDataAllocator);
    
    auto* pResLayoutRawMem = ALLOCATE(GetRawAllocator(), "Raw memory for ShaderResourceLayoutD3D12", m_NumShaders * sizeof(ShaderResourceLayoutD3D12));
    m_pResourceLayouts = reinterpret_cast<ShaderResourceLayoutD3D12*>(pResLayoutRawMem);

    for (Uint32 s = 0; s < m_NumShaders; ++s)
    {
        auto* pShader = ppShaders[s];
        auto ShaderType = pShader->GetDesc().ShaderType;
        auto ShaderInd = GetShaderTypeIndex(ShaderType);

        auto& ShaderResLayoutDataAllocator = pPSO->GetSRBMemoryAllocator().GetShaderVariableDataAllocator(s);

        // http://diligentgraphics.com/diligent-engine/architecture/d3d12/shader-resource-layout#Initializing-Resource-Layouts-in-a-Shader-Resource-Binding-Object
        std::array<SHADER_VARIABLE_TYPE, 2> AllowedVarTypes = { SHADER_VARIABLE_TYPE_MUTABLE, SHADER_VARIABLE_TYPE_DYNAMIC };
        const auto& SrcLayout = pPSO->GetShaderResLayout(s);
        new (m_pResourceLayouts + s) ShaderResourceLayoutD3D12(*this, SrcLayout, ShaderResLayoutDataAllocator, AllowedVarTypes.data(), static_cast<Uint32>(AllowedVarTypes.size()), m_ShaderResourceCache);

        m_ResourceLayoutIndex[ShaderInd] = static_cast<Int8>(s);
    }
}

ShaderResourceBindingD3D12Impl::~ShaderResourceBindingD3D12Impl()
{
    for(Uint32 l = 0; l < m_NumShaders; ++l)
        m_pResourceLayouts[l].~ShaderResourceLayoutD3D12();

    GetRawAllocator().Free(m_pResourceLayouts);
}

IMPLEMENT_QUERY_INTERFACE( ShaderResourceBindingD3D12Impl, IID_ShaderResourceBindingD3D12, TBase )

void ShaderResourceBindingD3D12Impl::BindResources(Uint32 ShaderFlags, IResourceMapping* pResMapping, Uint32 Flags)
{
    for (Uint32 s = 0; s < m_NumShaders; ++s )
    {
        const auto& ShaderRes = m_pResourceLayouts[s].GetShaderResources();
        if (ShaderFlags & ShaderRes.GetShaderType())
        {
            m_pResourceLayouts[s].BindResources(pResMapping, Flags, &m_ShaderResourceCache);
        }
    }
}

IShaderVariable *ShaderResourceBindingD3D12Impl::GetVariable(SHADER_TYPE ShaderType, const char *Name)
{
    auto ShaderInd = GetShaderTypeIndex(ShaderType);
    auto ResLayoutInd = m_ResourceLayoutIndex[ShaderInd];
    if (ResLayoutInd < 0)
    {
        LOG_ERROR("Failed to find shader variable \"", Name,"\" in shader resource binding: shader type ", GetShaderTypeLiteralName(ShaderType), " is not initialized");
        return ValidatedCast<PipelineStateD3D12Impl>(GetPipelineState())->GetDummyShaderVar();
    }
    auto* pVar = m_pResourceLayouts[ResLayoutInd].GetShaderVariable(Name);
    if(pVar == nullptr)
        pVar = ValidatedCast<PipelineStateD3D12Impl>(GetPipelineState())->GetDummyShaderVar();

    return pVar;
}

Uint32 ShaderResourceBindingD3D12Impl::GetVariableCount(SHADER_TYPE ShaderType) const 
{
    auto ShaderInd = GetShaderTypeIndex(ShaderType);
    auto ResLayoutInd = m_ResourceLayoutIndex[ShaderInd];
    if (ResLayoutInd < 0)
    {
        LOG_ERROR("Shader type ", GetShaderTypeLiteralName(ShaderType), " is not initialized");
        return 0;
    }

    return m_pResourceLayouts[ResLayoutInd].GetVariableCount();
}

IShaderVariable* ShaderResourceBindingD3D12Impl::GetVariable(SHADER_TYPE ShaderType, Uint32 Index)
{
    auto ShaderInd = GetShaderTypeIndex(ShaderType);
    auto ResLayoutInd = m_ResourceLayoutIndex[ShaderInd];
    if (ResLayoutInd < 0)
    {
        LOG_ERROR("Failed to find shader variable at index ", Index, " in shader resource binding: shader type ", GetShaderTypeLiteralName(ShaderType), " is not initialized");
        return ValidatedCast<PipelineStateD3D12Impl>(GetPipelineState())->GetDummyShaderVar();
    }

    return m_pResourceLayouts[ResLayoutInd].GetShaderVariable(Index);
}


#ifdef VERIFY_SHADER_BINDINGS
void ShaderResourceBindingD3D12Impl::dbgVerifyResourceBindings(const PipelineStateD3D12Impl* pPSO)
{
    auto* pRefPSO = ValidatedCast<PipelineStateD3D12Impl>(GetPipelineState());
    if (pPSO->IsIncompatibleWith(pRefPSO))
    {
        LOG_ERROR("Shader resource binding is incompatible with the pipeline state \"", pPSO->GetDesc().Name, '\"');
        return;
    }
    for(Uint32 l = 0; l < m_NumShaders; ++l)
    {
        // Use reference layout from pipeline state that contains all shader resource types
        const auto& ShaderResLayout = pRefPSO->GetShaderResLayout(l);
        ShaderResLayout.dbgVerifyBindings(m_ShaderResourceCache);
    }
}
#endif


void ShaderResourceBindingD3D12Impl::InitializeStaticResources(const PipelineStateD3D12Impl* pPSO)
{
    VERIFY(!StaticResourcesInitialized(), "Static resources have already been initialized");
    VERIFY(pPSO == GetPipelineState(), "Invalid pipeline state provided");

    auto NumShaders = pPSO->GetNumShaders();
    auto ppShaders = pPSO->GetShaders();
    // Copy static resources
    for (Uint32 s = 0; s < NumShaders; ++s)
    {
        auto* pShader = ValidatedCast<ShaderD3D12Impl>( ppShaders[s] );
#ifdef VERIFY_SHADER_BINDINGS
        pShader->DbgVerifyStaticResourceBindings();
#endif
        const auto& ShaderResLayout = pPSO->GetShaderResLayout(s);
        auto& StaticResLayout = pShader->GetStaticResLayout();
        StaticResLayout.CopyStaticResourceDesriptorHandles(ShaderResLayout, m_ShaderResourceCache);
    }

    m_bStaticResourcesInitialized = true;
}

}
