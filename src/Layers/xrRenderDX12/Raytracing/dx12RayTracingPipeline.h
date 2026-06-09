#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12RayTracingShaders.h"

struct DX12_RAYTRACING_PIPELINE_DESC
{
    ID3DBlob* pRayGenShader;
    ID3DBlob* pMissShader;
    UINT HitGroupCount;
    const DX12_HIT_GROUP_DESC* pHitGroups;

    ID3D12RootSignature* pRootSignature;

    UINT MaxPayloadSize;
    UINT MaxAttributeSize;

    LPCSTR Name;
};

class dx12RayTracingPipeline
{
public:
    dx12RayTracingPipeline();
    ~dx12RayTracingPipeline();

    void Init();

    ID3D12StateObject* CreatePipeline(const DX12_RAYTRACING_PIPELINE_DESC& Desc);
    ID3D12StateObject* GetOrCreatePipeline(LPCSTR Name);

    void BuildShaderBindingTable(ID3D12StateObject* pPipeline,
                                 ID3DBlob* pRayGenShader,
                                 ID3DBlob* pMissShader,
                                 UINT HitGroupCount,
                                 const DX12_HIT_GROUP_DESC* pHitGroups);

    ID3D12Resource* GetRayGenSBT() const { return m_pRayGenSBT.Get(); }
    ID3D12Resource* GetHitGroupSBT() const { return m_pHitGroupSBT.Get(); }
    ID3D12Resource* GetMissSBT() const { return m_pMissSBT.Get(); }

    D3D12_GPU_VIRTUAL_ADDRESS GetRayGenSBTGPUVirtualAddress() const { return m_rayGenSBTGPUAddress; }
    D3D12_GPU_VIRTUAL_ADDRESS GetHitGroupSBTGPUVirtualAddress() const { return m_hitGroupSBTGPUAddress; }
    D3D12_GPU_VIRTUAL_ADDRESS GetMissSBTGPUVirtualAddress() const { return m_missSBTGPUAddress; }

    UINT GetRayGenRecordCount() const { return m_rayGenRecordCount; }
    UINT GetHitGroupRecordCount() const { return m_hitGroupRecordCount; }
    UINT GetMissRecordCount() const { return m_missRecordCount; }

    UINT GetShaderTableStride() const;

private:
    HRESULT CreateSBTBuffers();
    void BuildRayGenRecord(ID3DBlob* pShader);
    void BuildHitGroupRecords(UINT Count, const DX12_HIT_GROUP_DESC* pHitGroups);
    void BuildMissRecord(ID3DBlob* pShader);

    ComPtr<ID3D12Resource> m_pRayGenSBT;
    ComPtr<ID3D12Resource> m_pHitGroupSBT;
    ComPtr<ID3D12Resource> m_pMissSBT;

    D3D12_GPU_VIRTUAL_ADDRESS m_rayGenSBTGPUAddress;
    D3D12_GPU_VIRTUAL_ADDRESS m_hitGroupSBTGPUAddress;
    D3D12_GPU_VIRTUAL_ADDRESS m_missSBTGPUAddress;

    UINT m_rayGenRecordCount;
    UINT m_hitGroupRecordCount;
    UINT m_missRecordCount;

    xr_map<u64, ComPtr<ID3D12StateObject>> m_pipelineCache;

    static const UINT MaxSBTRecords = 1024;
};

extern dx12RayTracingPipeline RayTracingPipeline12;

#endif // USE_DX12
