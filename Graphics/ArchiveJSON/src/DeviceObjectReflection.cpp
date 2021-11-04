/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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


#include "pch.h"

namespace Diligent
{

void DeviceObjectReflection::Serialize(nlohmann::json& Json, const IRenderPass* pDeviceObject)
{
    auto& ResourceDesc = pDeviceObject->GetDesc();
    VERIFY_EXPR(ResourceDesc.Name != nullptr);
    Json = pDeviceObject->GetDesc();
}

void DeviceObjectReflection::Deserialize(const nlohmann::json& Json, IRenderPass** pDeviceObject)
{
    auto pArchiveFactory = EngineEnvironment::GetInstance()->GetArchiveFactory();
    auto ResourceDesc    = Json.get<RenderPassDesc>();
    pArchiveFactory->CreateRenderPass(ResourceDesc, pDeviceObject);

    if (!pDeviceObject)
        LOG_ERROR_AND_THROW("Failed to create RenderPass -> '", ResourceDesc.Name, "'.");
}

void DeviceObjectReflection::Serialize(nlohmann::json& Json, const IShader* pDeviceObject)
{
    auto& ResourceDesc = pDeviceObject->GetDesc();
    VERIFY_EXPR(ResourceDesc.Name != nullptr);
    Json = pDeviceObject->GetDesc();
}

void DeviceObjectReflection::Deserialize(const nlohmann::json& Json, IShader** pDeviceObject)
{
    auto pEnvironment    = EngineEnvironment::GetInstance();
    auto pArchiveFactory = pEnvironment->GetArchiveFactory();
    auto ResourceDesc    = Json.get<ShaderCreateInfo>();
    pArchiveFactory->CreateShader(ResourceDesc, pEnvironment->GetDeviceBits(), pDeviceObject);
    if (!pDeviceObject)
        LOG_ERROR_AND_THROW("Failed to create Shader -> '", ResourceDesc.Desc.Name, "'.");
}

void DeviceObjectReflection::Serialize(nlohmann::json& Json, IPipelineResourceSignature** pDeviceObjects, size_t Size)
{
    for (size_t i = 0; i < Size; i++)
    {
        auto& ResourceDesc = pDeviceObjects[i]->GetDesc();
        VERIFY_EXPR(ResourceDesc.Name != nullptr);
        Json.push_back(ResourceDesc);
    }
}

void DeviceObjectReflection::Deserialize(const nlohmann::json& Json, IPipelineResourceSignature** pDeviceObjects, size_t Size)
{
    auto pEnvironment    = EngineEnvironment::GetInstance();
    auto pArchiveFactory = pEnvironment->GetArchiveFactory();

    *pDeviceObjects = reinterpret_cast<IPipelineResourceSignature*>(EngineEnvironment::GetInstance()->GetMemoryAllocator()->Allocate<IPipelineResourceSignature*>(Size));
    for (size_t i = 0; i < Size; i++)
    {
        IPipelineResourceSignature* pDeviceObject;
        auto                        ResourceDesc = Json[i].get<PipelineResourceSignatureDesc>();
        pArchiveFactory->CreatePipelineResourceSignature(ResourceDesc, pEnvironment->GetDeviceBits(), &pDeviceObject);
        if (!pDeviceObject)
            LOG_ERROR_AND_THROW("Failed to create Resource Signature -> '", ResourceDesc.Name, "'.");
        pDeviceObjects[i] = pDeviceObject;
    }
}

} // namespace Diligent
