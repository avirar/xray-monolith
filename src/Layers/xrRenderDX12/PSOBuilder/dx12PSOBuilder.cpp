#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12PSOBuilder.h"

#ifdef USE_DX12

dx12PSOBuilder::dx12PSOBuilder()
    : m_cacheHits(0),
      m_cacheMisses(0)
{
}

dx12PSOBuilder::~dx12PSOBuilder()
{
}

void dx12PSOBuilder::Init()
{
    m_psoCache.clear();
    m_cacheHits = 0;
    m_cacheMisses = 0;
}

u64 dx12PSOBuilder::ComputePSOKey(const D3D12_PIPELINE_STATE_DESC& Desc) const
{
    struct KeyData
    {
        const void* pVS;
        const void* pPS;
        const void* pGS;
        const void* pHS;
        const void* pDS;
        const void* pRootSignature;
        D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
        DXGI_FORMAT RTFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
        DXGI_FORMAT DepthStencilFormat;
        UINT NumRenderTargets;
        UINT SampleMask;
        UINT NumSamples;
        BOOL ClassicRenderTargets;
        UINT IBStripCutValue;
    };

    KeyData key;
    ZeroMemory(&key, sizeof(key));

    key.pVS = Desc.VS.pShaderBytecode;
    key.pPS = Desc.PS.pShaderBytecode;
    key.pGS = Desc.GS.pShaderBytecode;
    key.pHS = Desc.HS.pShaderBytecode;
    key.pDS = Desc.DS.pShaderBytecode;
    key.pRootSignature = Desc.pRootSignature;
    key.PrimitiveTopologyType = Desc.PrimitiveTopologyType;
    CopyMemory(key.RTFormats, Desc.RTVFormats, sizeof(key.RTFormats));
    key.DepthStencilFormat = Desc.DSVFormat;
    key.NumRenderTargets = Desc.NumRenderTargets;
    key.SampleMask = Desc.SampleMask;
    key.NumSamples = Desc.SampleDesc.Count;
    key.ClassicRenderTargets = Desc.Flags & D3D12_PIPELINE_STATE_FLAG_CLASSIC_RENDER_TARGETS;
    key.IBStripCutValue = (UINT)Desc.IBStripCutValue;

    return xxhash64(&key, sizeof(key), 0);
}

ID3D12PipelineState* dx12PSOBuilder::CreateGraphicsPSO(const DX12_PSO_BUILD_DESC& Desc)
{
    D3D12_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.VS = Desc.pVS ? CD3DX12_SHADER_BYTECODE(Desc.pVS) : CD3DX12_SHADER_BYTECODE();
    psoDesc.PS = Desc.pPS ? CD3DX12_SHADER_BYTECODE(Desc.pPS) : CD3DX12_SHADER_BYTECODE();
    psoDesc.GS = Desc.pGS ? CD3DX12_SHADER_BYTECODE(Desc.pGS) : CD3DX12_SHADER_BYTECODE();
    psoDesc.HS = Desc.pHS ? CD3DX12_SHADER_BYTECODE(Desc.pHS) : CD3DX12_SHADER_BYTECODE();
    psoDesc.DS = Desc.pDS ? CD3DX12_SHADER_BYTECODE(Desc.pDS) : CD3DX12_SHADER_BYTECODE();

    psoDesc.pRootSignature = Desc.pRootSignature;

    psoDesc.InputLayout = CD3DX12_INPUT_LAYOUT_DESC(Desc.pInputLayout, Desc.InputLayoutElementCount);
    psoDesc.PrimitiveTopologyType = Desc.PrimitiveTopologyType;
  psoDesc.IBStripCutValue = (D3D12_INDEX_BUFFER_STRIP_CUT_VALUE)(Desc.IBStripCutValue ? Desc.IBStripCutValue :
                                (UINT)D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF);

    CopyMemory(psoDesc.RTVFormats, Desc.RTFormats, sizeof(Desc.RTFormats));
    psoDesc.NumRenderTargets = Desc.NumRenderTargets;

    psoDesc.DSVFormat = Desc.DepthStencilFormat;
    psoDesc.SampleMask = Desc.SampleMask;
    psoDesc.SampleDesc.Count = Desc.NumSamples ? Desc.NumSamples : 1;
    psoDesc.SampleDesc.Quality = 0;

    if (Desc.ClassicRenderTargets)
        psoDesc.Flags = static_cast<D3D12_PIPELINE_STATE_FLAGS>(D3D12_PIPELINE_STATE_FLAG_CLASSIC_RENDER_TARGETS);

    u64 key = ComputePSOKey(psoDesc);

    xr_map<u64, ComPtr<ID3D12PipelineState>>::iterator it = m_psoCache.find(key);
    if (it != m_psoCache.end())
    {
        m_cacheHits++;
        return it->second.Get();
    }

    m_cacheMisses++;

    ComPtr<ID3D12PipelineState> pso;
    HRESULT hr = HW12.m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
    if (SUCCEEDED(hr))
    {
        if (m_psoCache.size() >= MaxCacheSize)
        {
            m_psoCache.erase(m_psoCache.begin());
        }
        m_psoCache[key] = pso;
        return pso.Get();
    }

    LOG("*ERROR* dx12PSOBuilder: Failed to create graphics PSO (0x%08X)", hr);
    return nullptr;
}

ID3D12PipelineState* dx12PSOBuilder::CreateComputePSO(const DX12_PSO_BUILD_DESC& Desc)
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.CS = Desc.pCS ? CD3DX12_SHADER_BYTECODE(Desc.pCS) : CD3DX12_SHADER_BYTECODE();
    psoDesc.pRootSignature = Desc.pRootSignature;
    psoDesc.NodeMask = 0;

    u64 key = xxhash64(&psoDesc.CS, sizeof(psoDesc.CS), 0) ^ xxhash64(&psoDesc.pRootSignature, sizeof(psoDesc.pRootSignature), 0);

    xr_map<u64, ComPtr<ID3D12PipelineState>>::iterator it = m_psoCache.find(key);
    if (it != m_psoCache.end())
    {
        m_cacheHits++;
        return it->second.Get();
    }

    m_cacheMisses++;

    ComPtr<ID3D12PipelineState> pso;
    HRESULT hr = HW12.m_pDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pso));
    if (SUCCEEDED(hr))
    {
        if (m_psoCache.size() >= MaxCacheSize)
        {
            m_psoCache.erase(m_psoCache.begin());
        }
        m_psoCache[key] = pso;
        return pso.Get();
    }

    LOG("*ERROR* dx12PSOBuilder: Failed to create compute PSO (0x%08X)", hr);
    return nullptr;
}

ID3D12PipelineState* dx12PSOBuilder::BuildPSO(
    ID3DBlob* pVS, ID3DBlob* pPS,
    ID3D12RootSignature* pRootSignature,
    const D3D12_INPUT_ELEMENT_DESC* pInputLayout, UINT InputLayoutElementCount,
    DXGI_FORMAT RTFormat, DXGI_FORMAT DSFormat,
    D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType)
{
    DX12_PSO_BUILD_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.pVS = pVS;
    desc.pPS = pPS;
    desc.pRootSignature = pRootSignature;
    desc.pInputLayout = pInputLayout;
    desc.InputLayoutElementCount = InputLayoutElementCount;
    desc.PrimitiveTopologyType = TopologyType;
    desc.RTFormats[0] = RTFormat;
    desc.NumRenderTargets = 1;
    desc.DepthStencilFormat = DSFormat;
    desc.SampleMask = UINT_MAX;
    desc.NumSamples = 1;
    desc.ClassicRenderTargets = FALSE;

    return CreateGraphicsPSO(desc);
}

ID3D12PipelineState* dx12PSOBuilder::BuildComputePSO(
    ID3DBlob* pCS,
    ID3D12RootSignature* pRootSignature)
{
    DX12_PSO_BUILD_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.pCS = pCS;
    desc.pRootSignature = pRootSignature;

    return CreateComputePSO(desc);
}

void dx12PSOBuilder::ClearCache()
{
    m_psoCache.clear();
    m_cacheHits = 0;
    m_cacheMisses = 0;
}

dx12PSOBuilder PSOBuilder12;

#endif // USE_DX12
