#pragma once

#ifdef USE_DX12

#include "../DX12CommonTypes.h"

struct DX12_PSO_BUILD_DESC
{
    ID3DBlob* pVS;
    ID3DBlob* pPS;
    ID3DBlob* pGS;
    ID3DBlob* pHS;
    ID3DBlob* pDS;
    ID3DBlob* pCS;

    ID3D12RootSignature* pRootSignature;

    const D3D12_INPUT_ELEMENT_DESC* pInputLayout;
    UINT InputLayoutElementCount;

    D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
    UINT IBStripCutValue;

    DXGI_FORMAT RTFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
    UINT NumRenderTargets;

    DXGI_FORMAT DepthStencilFormat;
    UINT SampleMask;
    UINT NumSamples;
    BOOL ClassicRenderTargets;

    bool bComputeShader;
    bool bGraphicsPipeline;
};

class dx12PSOBuilder
{
public:
    dx12PSOBuilder();
    ~dx12PSOBuilder();

    void Init();

    ID3D12PipelineState* CreateGraphicsPSO(const DX12_PSO_BUILD_DESC& Desc);
    ID3D12PipelineState* CreateComputePSO(const DX12_PSO_BUILD_DESC& Desc);

    ID3D12PipelineState* BuildPSO(
        ID3DBlob* pVS, ID3DBlob* pPS,
        ID3D12RootSignature* pRootSignature,
        const D3D12_INPUT_ELEMENT_DESC* pInputLayout, UINT InputLayoutElementCount,
        DXGI_FORMAT RTFormat, DXGI_FORMAT DSFormat,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    ID3D12PipelineState* BuildComputePSO(
        ID3DBlob* pCS,
        ID3D12RootSignature* pRootSignature);

    void ClearCache();
    UINT GetCacheSize() const { return static_cast<UINT>(m_psoCache.size()); }
    UINT GetCacheHits() const { return m_cacheHits; }
    UINT GetCacheMisses() const { return m_cacheMisses; }

private:
    u64 ComputePSOKey(const D3D12_PIPELINE_STATE_DESC& Desc) const;

    xr_map<u64, ComPtr<ID3D12PipelineState>> m_psoCache;
    UINT m_cacheHits;
    UINT m_cacheMisses;

    static const UINT MaxCacheSize = 4096;
};

extern dx12PSOBuilder PSOBuilder12;

#endif // USE_DX12
