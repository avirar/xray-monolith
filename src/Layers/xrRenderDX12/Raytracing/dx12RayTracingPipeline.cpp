#include "stdafx.h"
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
    // TODO: Implement proper DX12 raytracing pipeline creation
    return nullptr;
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
