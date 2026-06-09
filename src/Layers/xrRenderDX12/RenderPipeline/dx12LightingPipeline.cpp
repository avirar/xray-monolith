#include "../dx12stdafx.h"
#include "dx12LightingPipeline.h"

#ifdef USE_DX12

dx12LightingPipeline::dx12LightingPipeline()
    : m_lightCount(0)
{
    ZeroMemory(&m_desc, sizeof(m_desc));
    ZeroMemory(&m_lights, sizeof(m_lights));
    ZeroMemory(&m_AccumulatorRTVs, sizeof(m_AccumulatorRTVs));
}

dx12LightingPipeline::~dx12LightingPipeline()
{
    Destroy();
}

void dx12LightingPipeline::Init(const DX12_ACCUMULATOR_DESC& Desc)
{
    m_desc = Desc;
    CreateResources(Desc);
    CreateRTVs();
    CreateSRV();
}

void dx12LightingPipeline::Destroy()
{
    m_pAccumulatorBuffer.Reset();
    m_pRtvHeap.Reset();
    m_lightCount = 0;
}

void dx12LightingPipeline::Resize(u32 Width, u32 Height)
{
    m_desc.Width = Width;
    m_desc.Height = Height;
    Destroy();
    CreateResources(m_desc);
    CreateRTVs();
    CreateSRV();
}

void dx12LightingPipeline::BeginAccumulator()
{
    UINT frameIndex = CommandManager12.GetFrameIndex();

    Backend12.ResourceBarrier(m_pAccumulatorBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

    ID3D12DescriptorHeap* pHeaps[] = {
        m_pRtvHeap.Get(),
        HW12.m_pDsvHeap.Get(),
        DescriptorManager12.GetSRVUAVHeap(),
        DescriptorManager12.GetCBVHeap()
    };
    Backend12.GetCommandList()->SetDescriptorHeaps(_countof(pHeaps), pHeaps);
    Backend12.GetCommandList()->OMSetRenderTargets(1, &m_AccumulatorRTVs[frameIndex], FALSE, &HW12.m_DsvHandle);

    D3D12_VIEWPORT vp = {};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = static_cast<float>(m_desc.Width);
    vp.Height = static_cast<float>(m_desc.Height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    Backend12.GetCommandList()->RSSetViewports(1, &vp);

    D3D12_RECT sc = {};
    sc.left = 0;
    sc.top = 0;
    sc.right = m_desc.Width;
    sc.bottom = m_desc.Height;
    Backend12.GetCommandList()->RSSetScissorRects(1, &sc);
}

void dx12LightingPipeline::EndAccumulator()
{
    Backend12.ResourceBarrier(m_pAccumulatorBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void dx12LightingPipeline::ClearAccumulator(const FLOAT ClearColor[4])
{
    UINT frameIndex = CommandManager12.GetFrameIndex();
    Backend12.GetCommandList()->ClearRenderTargetView(m_AccumulatorRTVs[frameIndex], ClearColor, 0, nullptr);
}

void dx12LightingPipeline::TransitionAccumulatorToShaderResource()
{
    Backend12.ResourceBarrier(m_pAccumulatorBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void dx12LightingPipeline::TransitionAccumulatorToRenderTarget()
{
    Backend12.ResourceBarrier(m_pAccumulatorBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void dx12LightingPipeline::AddLight(const DX12_LIGHT_DATA& Light)
{
    if (m_lightCount < _countof(m_lights))
    {
        m_lights[m_lightCount++] = Light;
    }
}

void dx12LightingPipeline::CreateResources(const DX12_ACCUMULATOR_DESC& Desc)
{
    D3D12_RESOURCE_DESC rtDesc = {};
    rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rtDesc.Alignment = 0;
    rtDesc.Width = Desc.Width;
    rtDesc.Height = Desc.Height;
    rtDesc.DepthOrArraySize = 1;
    rtDesc.MipLevels = 1;
    rtDesc.Format = Desc.AccumulatorFormat;
    rtDesc.SampleDesc.Count = Desc.SampleCount;
    rtDesc.SampleDesc.Quality = Desc.SampleCount > 1 ? Desc.SampleCount - 1 : 0;
    rtDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &rtDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pAccumulatorBuffer)));
}

void dx12LightingPipeline::CreateRTVs()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = HW12.m_frameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    R_CHK(HW12.m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap)));

    D3D12_CPU_DESCRIPTOR_HANDLE rtvStart = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    UINT rtvIncrement = HW12.m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    for (UINT i = 0; i < HW12.m_frameCount; i++)
    {
        m_AccumulatorRTVs[i].ptr = rtvStart.ptr + i * rtvIncrement;
        HW12.m_pDevice->CreateRenderTargetView(m_pAccumulatorBuffer.Get(), nullptr, m_AccumulatorRTVs[i]);
    }
}

void dx12LightingPipeline::CreateSRV()
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_desc.AccumulatorFormat;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    m_AccumulatorSRVDesc = srvDesc;

    D3D12_CPU_DESCRIPTOR_HANDLE handle = DescriptorManager12.AllocateSRV();
    HW12.m_pDevice->CreateShaderResourceView(m_pAccumulatorBuffer.Get(), &m_AccumulatorSRVDesc, handle);
}

dx12LightingPipeline LightingPipeline12;

#endif // USE_DX12
