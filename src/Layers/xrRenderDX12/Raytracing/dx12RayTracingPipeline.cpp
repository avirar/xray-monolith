#include "../dx12stdafx.h"
#include "dx12RayTracingPipeline.h"

#ifdef USE_DX12

dx12RayTracingPipeline::dx12RayTracingPipeline()
    : m_rayGenSBTGPUAddress(0),
      m_hitGroupSBTGPUAddress(0),
      m_missSBTGPUAddress(0),
      m_rayGenRecordCount(0),
      m_hitGroupRecordCount(0),
      m_missRecordCount(0)
{
}

dx12RayTracingPipeline::~dx12RayTracingPipeline()
{
}

void dx12RayTracingPipeline::Init()
{
}

ID3D12StateObject* dx12RayTracingPipeline::CreatePipeline(const DX12_RAYTRACING_PIPELINE_DESC& Desc)
{
    D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
    stateObjectDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxPayloadSizeInBytes = Desc.MaxPayloadSize ? Desc.MaxPayloadSize : 32;
    pipelineConfig.MaxAttributeSizeInBytes = Desc.MaxAttributeSize ? Desc.MaxAttributeSize : 8;
    pipelineConfig.MissingHitActionOnMiss = D3D12_MISSING_HIT_ACTION_ON_MISS_INVOCATION_MISS_SHADER;

    stateObjectDesc.ConfigLookup = D3D12_STATE_OBJECT_CONFIG_LOOKUP_TYPE_EXPORTED;
    stateObjectDesc.pRaytracingStateSubobject = &pipelineConfig;

    D3D12_SUBOBJECT_DESC subobjects[128];
    UINT subobjectCount = 0;

    D3D12_STATE_OBJECT_CONFIGURATION configuration = {};
    configuration.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1;
    configuration.pDescription = &pipelineConfig;
    subobjects[subobjectCount++] = configuration;

    D3D12_RAYTRACING_PIPELINE_CONFIG1 config1 = {};
    config1.MaxPayloadSizeInBytes = pipelineConfig.MaxPayloadSizeInBytes;
    config1.MaxAttributeSizeInBytes = pipelineConfig.MaxAttributeSizeInBytes;
    config1.MissingHitActionOnMiss = pipelineConfig.MissingHitActionOnMiss;

    D3D12_SUBOBJECT_DESC configSubobject = {};
    configSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1;
    configSubobject.pDescription = &config1;
    subobjects[subobjectCount++] = configSubobject;

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSigSubobject = {};
    globalRootSigSubobject.pGlobalRootSignature = Desc.pRootSignature;

    D3D12_SUBOBJECT_DESC globalRootSigDesc = {};
    globalRootSigDesc.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    globalRootSigDesc.pDescription = &globalRootSigSubobject;
    subobjects[subobjectCount++] = globalRootSigDesc;

    D3D12_LOCAL_ROOT_SIGNATURE localRootSigSubobject = {};
    localRootSigSubobject.pLocalRootSignature = RayTracingShaders12.GetLocalRootSignatureBuffer()
        ? nullptr : Desc.pRootSignature;

    D3D12_SUBOBJECT_DESC localRootSigDesc = {};
    localRootSigDesc.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
    localRootSigDesc.pDescription = &localRootSigSubobject;
    subobjects[subobjectCount++] = localRootSigDesc;

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    shaderConfig.MaxPayloadSizeInBytes = pipelineConfig.MaxPayloadSizeInBytes;
    shaderConfig.MaxAttributeSizeInBytes = pipelineConfig.MaxAttributeSizeInBytes;

    D3D12_SUBOBJECT_DESC shaderConfigDesc = {};
    shaderConfigDesc.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    shaderConfigDesc.pDescription = &shaderConfig;
    subobjects[subobjectCount++] = shaderConfigDesc;

    D3D12_RAYTRACING_PIPELINE_CONFIG1 pipelineConfig1 = {};
    pipelineConfig1.MaxPayloadSizeInBytes = pipelineConfig.MaxPayloadSizeInBytes;
    pipelineConfig1.MaxAttributeSizeInBytes = pipelineConfig.MaxAttributeSizeInBytes;
    pipelineConfig1.MissingHitActionOnMiss = D3D12_MISSING_HIT_ACTION_ON_MISS_INVOCATION_MISS_SHADER;

    D3D12_SUBOBJECT_DESC pipelineConfigDesc = {};
    pipelineConfigDesc.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1;
    pipelineConfigDesc.pDescription = &pipelineConfig1;
    subobjects[subobjectCount++] = pipelineConfigDesc;

    D3D12_SHADER_IDENTIFIER rayGenShaderID = {};
    rayGenShaderID.IdentifierSize = 16;
    rayGenShaderID.pIdentifierData = nullptr;

    D3D12_DXIL_SUBOBJECT_DXIL_LIBRARY rayGenLib = {};
    rayGenLib.pDXILLibrary = CD3DX12_SHADER_BYTECODE(Desc.pRayGenShader);
    rayGenLib.NumExports = 1;
    rayGenLib.pExports = &CD3DX12_EXPORT_DESC(Desc.pRayGenShader->GetBufferPointer(), "main");

    D3D12_SUBOBJECT_DESC rayGenLibDesc = {};
    rayGenLibDesc.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    rayGenLibDesc.pDescription = &rayGenLib;
    subobjects[subobjectCount++] = rayGenLibDesc;

    D3D12_SUBOBJECT_DESC rayGenAssocDesc = {};
    rayGenAssocDesc.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYGENERATION_SHADER;
    rayGenAssocDesc.pDescription = &CD3DX12_SHADER_ASSOCIATION_DESC("main");
    subobjects[subobjectCount++] = rayGenAssocDesc;

    if (Desc.pMissShader)
    {
        D3D12_DXIL_SUBOBJECT_DXIL_LIBRARY missLib = {};
        missLib.pDXILLibrary = CD3DX12_SHADER_BYTECODE(Desc.pMissShader);
        missLib.NumExports = 1;
        missLib.pExports = &CD3DX12_EXPORT_DESC(Desc.pMissShader->GetBufferPointer(), "main");

        D3D12_SUBOBJECT_DESC missLibDesc = {};
        missLibDesc.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
        missLibDesc.pDescription = &missLib;
        subobjects[subobjectCount++] = missLibDesc;

        D3D12_SUBOBJECT_DESC missAssocDesc = {};
        missAssocDesc.Type = D3D12_STATE_SUBOBJECT_TYPE_MISS_SHADER;
        missAssocDesc.pDescription = &CD3DX12_SHADER_ASSOCIATION_DESC("main");
        subobjects[subobjectCount++] = missAssocDesc;
    }

    for (UINT i = 0; i < Desc.HitGroupCount; i++)
    {
        const DX12_HIT_GROUP_DESC& hg = Desc.pHitGroups[i];

        D3D12_HIT_GROUP1 hitGroup = {};
        hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
        hitGroup.HitGroupExport = hg.Name;
        hitGroup.AnnotatedShaderIndex = 0;

        if (hg.pClosestHitShader)
        {
            hitGroup.ClosestHitShader = CD3DX12_SHADER_IDENTIFIER(CD3DX12_EXPORT_DESC(hg.pClosestHitShader->GetBufferPointer(), hg.ClosestHitEntry ? hg.ClosestHitEntry : "main"));
        }
        else
        {
            hitGroup.ClosestHitShader = D3D12_SENTINEL_HIT_GROUP_SHADER;
        }

        if (hg.pAnyHitShader)
        {
            hitGroup.AnyHitShader = CD3DX12_SHADER_IDENTIFIER(CD3DX12_EXPORT_DESC(hg.pAnyHitShader->GetBufferPointer(), hg.AnyHitEntry ? hg.AnyHitEntry : "main"));
        }
        else
        {
            hitGroup.AnyHitShader = D3D12_SENTINEL_HIT_GROUP_SHADER;
        }

        hitGroup.IntersectionShader = D3D12_SENTINEL_HIT_GROUP_SHADER;

        D3D12_SUBOBJECT_DESC hitGroupDesc = {};
        hitGroupDesc.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
        hitGroupDesc.pDescription = &hitGroup;
        subobjects[subobjectCount++] = hitGroupDesc;
    }

    stateObjectDesc.NumSubobjects = subobjectCount;
    stateObjectDesc.pSubobjects = subobjects;

    ComPtr<ID3D12StateObject> stateObject;
    HRESULT hr = HW12.m_pDevice->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&stateObject));
    if (FAILED(hr))
    {
        LOG("*ERROR* dx12RayTracingPipeline: Failed to create raytracing pipeline (0x%08X)", hr);
        return nullptr;
    }

    BuildShaderBindingTable(stateObject.Get(), Desc.pRayGenShader, Desc.pMissShader,
                            Desc.HitGroupCount, Desc.pHitGroups);

    return stateObject.Detach();
}

ID3D12StateObject* dx12RayTracingPipeline::GetOrCreatePipeline(LPCSTR Name)
{
    u64 key = xxhash64(Name, strlen(Name), 0);

    xr_map<u64, ComPtr<ID3D12StateObject>>::iterator it = m_pipelineCache.find(key);
    if (it != m_pipelineCache.end())
    {
        return it->second.Get();
    }

    return nullptr;
}

void dx12RayTracingPipeline::BuildShaderBindingTable(ID3D12StateObject* pPipeline,
                                                      ID3DBlob* pRayGenShader,
                                                      ID3DBlob* pMissShader,
                                                      UINT HitGroupCount,
                                                      const DX12_HIT_GROUP_DESC* pHitGroups)
{
    UINT stride = GetShaderTableStride();

    CreateSBTBuffers();

    BuildRayGenRecord(pRayGenShader);
    BuildMissRecord(pMissShader);
    BuildHitGroupRecords(HitGroupCount, pHitGroups);
}

ID3D12Resource* dx12RayTracingPipeline::GetRayGenSBT() const
{
    return m_pRayGenSBT.Get();
}

ID3D12Resource* dx12RayTracingPipeline::GetHitGroupSBT() const
{
    return m_pHitGroupSBT.Get();
}

ID3D12Resource* dx12RayTracingPipeline::GetMissSBT() const
{
    return m_pMissSBT.Get();
}

D3D12_GPU_VIRTUAL_ADDRESS dx12RayTracingPipeline::GetRayGenSBTGPUVirtualAddress() const
{
    return m_rayGenSBTGPUAddress;
}

D3D12_GPU_VIRTUAL_ADDRESS dx12RayTracingPipeline::GetHitGroupSBTGPUVirtualAddress() const
{
    return m_hitGroupSBTGPUAddress;
}

D3D12_GPU_VIRTUAL_ADDRESS dx12RayTracingPipeline::GetMissSBTGPUVirtualAddress() const
{
    return m_missSBTGPUAddress;
}

UINT dx12RayTracingPipeline::GetShaderTableStride() const
{
    return HW12.m_pDevice->GetRaytracingShaderTableAlignment();
}

HRESULT dx12RayTracingPipeline::CreateSBTBuffers()
{
    UINT stride = GetShaderTableStride();

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC rayGenDesc = CD3DX12_RESOURCE_DESC::Buffer(stride);
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &rayGenDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_pRayGenSBT)));
    m_rayGenSBTGPUAddress = m_pRayGenSBT->GetGPUVirtualAddress();

    D3D12_RESOURCE_DESC hitGroupDesc = CD3DX12_RESOURCE_DESC::Buffer(stride * MaxSBTRecords);
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &hitGroupDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_pHitGroupSBT)));
    m_hitGroupSBTGPUAddress = m_pHitGroupSBT->GetGPUVirtualAddress();

    D3D12_RESOURCE_DESC missDesc = CD3DX12_RESOURCE_DESC::Buffer(stride * MaxSBTRecords);
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &missDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_pMissSBT)));
    m_missSBTGPUAddress = m_pMissSBT->GetGPUVirtualAddress();

    return S_OK;
}

void dx12RayTracingPipeline::BuildRayGenRecord(ID3DBlob* pShader)
{
    UINT stride = GetShaderTableStride();

    D3D12_SHADER_BINDING_TABLE_RECORD record = {};
    record.ShaderID[0] = 0;
    record.ShaderID[1] = 0;
    record.ShaderID[2] = 0;
    record.ShaderID[3] = 0;
    record.DispatchRPCHints[0] = 0;
    record.DispatchRPCHints[1] = 0;
    record.DispatchRPCHints[2] = 0;
    record.DispatchRPCHints[3] = 0;

    ComPtr<ID3D12Resource> stagingBuffer;
    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(stride);
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&stagingBuffer)));

    void* pData;
    R_CHK(stagingBuffer->Map(0, nullptr, &pData));
    CopyMemory(pData, &record, sizeof(record));
    ZeroMemory(static_cast<BYTE*>(pData) + sizeof(record), stride - sizeof(record));
    stagingBuffer->Unmap(0, nullptr);

    Backend12.GetCommandList()->CopyResource(m_pRayGenSBT.Get(), stagingBuffer.Get());
    m_rayGenRecordCount = 1;
}

void dx12RayTracingPipeline::BuildHitGroupRecords(UINT Count, const DX12_HIT_GROUP_DESC* pHitGroups)
{
    UINT stride = GetShaderTableStride();

    ComPtr<ID3D12Resource> stagingBuffer;
    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(stride * Count);
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&stagingBuffer)));

    void* pData;
    R_CHK(stagingBuffer->Map(0, nullptr, &pData));

    for (UINT i = 0; i < Count; i++)
    {
        D3D12_SHADER_BINDING_TABLE_RECORD* record = reinterpret_cast<D3D12_SHADER_BINDING_TABLE_RECORD*>(
            static_cast<BYTE*>(pData) + i * stride);

        record->ShaderID[0] = i;
        record->ShaderID[1] = 0;
        record->ShaderID[2] = 0;
        record->ShaderID[3] = 0;
        record->DispatchRPCHints[0] = 0;
        record->DispatchRPCHints[1] = 0;
        record->DispatchRPCHints[2] = 0;
        record->DispatchRPCHints[3] = 0;
    }

    stagingBuffer->Unmap(0, nullptr);
    Backend12.GetCommandList()->CopyResource(m_pHitGroupSBT.Get(), stagingBuffer.Get());
    m_hitGroupRecordCount = Count;
}

void dx12RayTracingPipeline::BuildMissRecord(ID3DBlob* pShader)
{
    UINT stride = GetShaderTableStride();

    D3D12_SHADER_BINDING_TABLE_RECORD record = {};
    record.ShaderID[0] = 0;
    record.ShaderID[1] = 0;
    record.ShaderID[2] = 0;
    record.ShaderID[3] = 0;
    record.DispatchRPCHints[0] = 0;
    record.DispatchRPCHints[1] = 0;
    record.DispatchRPCHints[2] = 0;
    record.DispatchRPCHints[3] = 0;

    ComPtr<ID3D12Resource> stagingBuffer;
    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(stride);
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&stagingBuffer)));

    void* pData;
    R_CHK(stagingBuffer->Map(0, nullptr, &pData));
    CopyMemory(pData, &record, sizeof(record));
    ZeroMemory(static_cast<BYTE*>(pData) + sizeof(record), stride - sizeof(record));
    stagingBuffer->Unmap(0, nullptr);

    Backend12.GetCommandList()->CopyResource(m_pMissSBT.Get(), stagingBuffer.Get());
    m_missRecordCount = pShader ? 1 : 0;
}

dx12RayTracingPipeline RayTracingPipeline12;

#endif // USE_DX12
