#include "../dx12stdafx.h"
#include "dx12RenderPipeline.h"

#ifdef USE_DX12

dx12RenderPipeline::dx12RenderPipeline()
{
    ZeroMemory(&m_desc, sizeof(m_desc));
    ZeroMemory(&m_PositionRTVs, sizeof(m_PositionRTVs));
    ZeroMemory(&m_ColorRTVs, sizeof(m_ColorRTVs));
    ZeroMemory(&m_NormalRTVs, sizeof(m_NormalRTVs));
    ZeroMemory(&m_RoughnessRTVs, sizeof(m_RoughnessRTVs));
    ZeroMemory(&m_MotionVectorRTVs, sizeof(m_MotionVectorRTVs));
    m_DepthDSV.ptr = 0;
}

dx12RenderPipeline::~dx12RenderPipeline()
{
    Destroy();
}

void dx12RenderPipeline::Init(const DX12_GBUFFER_DESC& Desc)
{
    m_desc = Desc;
    CreateResources(Desc);
    CreateRTVs();
    CreateSRVs();
    CreateSRVDescriptors();
}

void dx12RenderPipeline::Destroy()
{
    m_pPositionBuffer.Reset();
    m_pColorBuffer.Reset();
    m_pNormalBuffer.Reset();
    m_pRoughnessBuffer.Reset();
    m_pMotionVectorBuffer.Reset();
    m_pDepthBuffer.Reset();
    m_pRtvHeap.Reset();
    m_pDsvHeap.Reset();
}

void dx12RenderPipeline::Resize(u32 Width, u32 Height)
{
    m_desc.Width = Width;
    m_desc.Height = Height;
    Destroy();
    CreateResources(m_desc);
    CreateRTVs();
    CreateSRVs();
    CreateSRVDescriptors();
}

void dx12RenderPipeline::BeginGBuffer()
{
    UINT frameIndex = CommandManager12.GetFrameIndex();

    D3D12_RESOURCE_BARRIER barriers[5] = {};
    barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_pPositionBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_pColorBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    barriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(m_pNormalBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    barriers[3] = CD3DX12_RESOURCE_BARRIER::Transition(m_pRoughnessBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    barriers[4] = CD3DX12_RESOURCE_BARRIER::Transition(m_pMotionVectorBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

    Backend12.ResourceBarriers(5, barriers);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[5] = {};
    rtvs[0] = m_PositionRTVs[frameIndex];
    rtvs[1] = m_ColorRTVs[frameIndex];
    rtvs[2] = m_NormalRTVs[frameIndex];
    rtvs[3] = m_RoughnessRTVs[frameIndex];
    rtvs[4] = m_MotionVectorRTVs[frameIndex];

    ID3D12DescriptorHeap* pHeaps[] = {
        m_pRtvHeap.Get(),
        m_pDsvHeap.Get(),
        DescriptorManager12.GetSRVUAVHeap(),
        DescriptorManager12.GetCBVHeap()
    };
    Backend12.GetCommandList()->SetDescriptorHeaps(_countof(pHeaps), pHeaps);
    Backend12.GetCommandList()->OMSetRenderTargets(5, rtvs, FALSE, &m_DepthDSV);

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

void dx12RenderPipeline::EndGBuffer()
{
    D3D12_RESOURCE_BARRIER barriers[5] = {};
    barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_pPositionBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_pColorBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(m_pNormalBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barriers[3] = CD3DX12_RESOURCE_BARRIER::Transition(m_pRoughnessBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barriers[4] = CD3DX12_RESOURCE_BARRIER::Transition(m_pMotionVectorBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    Backend12.ResourceBarriers(5, barriers);
}

void dx12RenderPipeline::ClearGBuffer(const FLOAT ClearColor[4], FLOAT Depth, UINT8 Stencil)
{
    UINT frameIndex = CommandManager12.GetFrameIndex();

    FLOAT positionClear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Backend12.GetCommandList()->ClearRenderTargetView(m_PositionRTVs[frameIndex], positionClear, 0, nullptr);
    Backend12.GetCommandList()->ClearRenderTargetView(m_ColorRTVs[frameIndex], ClearColor, 0, nullptr);
    Backend12.GetCommandList()->ClearRenderTargetView(m_NormalRTVs[frameIndex], ClearColor, 0, nullptr);
    Backend12.GetCommandList()->ClearRenderTargetView(m_RoughnessRTVs[frameIndex], ClearColor, 0, nullptr);
    FLOAT mvClear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Backend12.GetCommandList()->ClearRenderTargetView(m_MotionVectorRTVs[frameIndex], mvClear, 0, nullptr);

    Backend12.GetCommandList()->ClearDepthStencilView(m_DepthDSV,
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, Depth, Stencil, 0, nullptr);
}

void dx12RenderPipeline::TransitionGBufferToShaderResource()
{
    D3D12_RESOURCE_BARRIER barriers[5] = {};
    barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_pPositionBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_pColorBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(m_pNormalBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barriers[3] = CD3DX12_RESOURCE_BARRIER::Transition(m_pRoughnessBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barriers[4] = CD3DX12_RESOURCE_BARRIER::Transition(m_pMotionVectorBuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    Backend12.ResourceBarriers(5, barriers);
}

void dx12RenderPipeline::TransitionGBufferToRenderTarget()
{
    D3D12_RESOURCE_BARRIER barriers[5] = {};
    barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_pPositionBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_pColorBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    barriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(m_pNormalBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    barriers[3] = CD3DX12_RESOURCE_BARRIER::Transition(m_pRoughnessBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    barriers[4] = CD3DX12_RESOURCE_BARRIER::Transition(m_pMotionVectorBuffer.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

    Backend12.ResourceBarriers(5, barriers);
}

void dx12RenderPipeline::CreateResources(const DX12_GBUFFER_DESC& Desc)
{
    D3D12_RESOURCE_DESC rtDesc = {};
    rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rtDesc.Alignment = 0;
    rtDesc.Width = Desc.Width;
    rtDesc.Height = Desc.Height;
    rtDesc.DepthOrArraySize = 1;
    rtDesc.MipLevels = 1;
    rtDesc.SampleDesc.Count = Desc.SampleCount;
    rtDesc.SampleDesc.Quality = Desc.SampleCount > 1 ? Desc.SampleCount - 1 : 0;
    rtDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    rtDesc.Format = Desc.PositionFormat;
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &rtDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pPositionBuffer)));

    rtDesc.Format = Desc.ColorFormat;
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &rtDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pColorBuffer)));

    rtDesc.Format = Desc.NormalFormat;
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &rtDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pNormalBuffer)));

    rtDesc.Format = Desc.RoughnessFormat;
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &rtDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pRoughnessBuffer)));

    rtDesc.Format = Desc.MotionVectorFormat;
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &rtDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pMotionVectorBuffer)));

    D3D12_RESOURCE_DESC dsDesc = {};
    dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    dsDesc.Alignment = 0;
    dsDesc.Width = Desc.Width;
    dsDesc.Height = Desc.Height;
    dsDesc.DepthOrArraySize = 1;
    dsDesc.MipLevels = 1;
    dsDesc.Format = Desc.DepthFormat;
    dsDesc.SampleDesc.Count = Desc.SampleCount;
    dsDesc.SampleDesc.Quality = Desc.SampleCount > 1 ? Desc.SampleCount - 1 : 0;
    dsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear = {};
    optClear.Format = Desc.DepthFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &dsDesc, D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(&m_pDepthBuffer)));
}

void dx12RenderPipeline::CreateRTVs()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 5 * HW12.m_frameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    R_CHK(HW12.m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap)));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    R_CHK(HW12.m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDsvHeap)));

    D3D12_CPU_DESCRIPTOR_HANDLE rtvStart = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    UINT rtvIncrement = HW12.m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    for (UINT i = 0; i < HW12.m_frameCount; i++)
    {
        m_PositionRTVs[i].ptr = rtvStart.ptr + i * rtvIncrement;
        HW12.m_pDevice->CreateRenderTargetView(m_pPositionBuffer.Get(), nullptr, m_PositionRTVs[i]);

        m_ColorRTVs[i].ptr = rtvStart.ptr + (HW12.m_frameCount + i) * rtvIncrement;
        HW12.m_pDevice->CreateRenderTargetView(m_pColorBuffer.Get(), nullptr, m_ColorRTVs[i]);

        m_NormalRTVs[i].ptr = rtvStart.ptr + (HW12.m_frameCount * 2 + i) * rtvIncrement;
        HW12.m_pDevice->CreateRenderTargetView(m_pNormalBuffer.Get(), nullptr, m_NormalRTVs[i]);

        m_RoughnessRTVs[i].ptr = rtvStart.ptr + (HW12.m_frameCount * 3 + i) * rtvIncrement;
        HW12.m_pDevice->CreateRenderTargetView(m_pRoughnessBuffer.Get(), nullptr, m_RoughnessRTVs[i]);

        m_MotionVectorRTVs[i].ptr = rtvStart.ptr + (HW12.m_frameCount * 4 + i) * rtvIncrement;
        HW12.m_pDevice->CreateRenderTargetView(m_pMotionVectorBuffer.Get(), nullptr, m_MotionVectorRTVs[i]);
    }

    HW12.m_pDevice->CreateDepthStencilView(m_pDepthBuffer.Get(), nullptr, m_pDsvHeap->GetCPUDescriptorHandleForHeapStart());
    m_DepthDSV = m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void dx12RenderPipeline::CreateSRVs()
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    srvDesc.Format = m_desc.PositionFormat;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    m_PositionSRVDesc = srvDesc;

    srvDesc.Format = m_desc.ColorFormat;
    m_ColorSRVDesc = srvDesc;

    srvDesc.Format = m_desc.NormalFormat;
    m_NormalSRVDesc = srvDesc;

    srvDesc.Format = m_desc.RoughnessFormat;
    m_RoughnessSRVDesc = srvDesc;

    srvDesc.Format = m_desc.MotionVectorFormat;
    m_MotionVectorSRVDesc = srvDesc;
}

void dx12RenderPipeline::CreateSRVDescriptors()
{
    D3D12_CPU_DESCRIPTOR_HANDLE posHandle = DescriptorManager12.AllocateSRV();
    HW12.m_pDevice->CreateShaderResourceView(m_pPositionBuffer.Get(), &m_PositionSRVDesc, posHandle);

    D3D12_CPU_DESCRIPTOR_HANDLE colHandle = DescriptorManager12.AllocateSRV();
    HW12.m_pDevice->CreateShaderResourceView(m_pColorBuffer.Get(), &m_ColorSRVDesc, colHandle);

    D3D12_CPU_DESCRIPTOR_HANDLE normHandle = DescriptorManager12.AllocateSRV();
    HW12.m_pDevice->CreateShaderResourceView(m_pNormalBuffer.Get(), &m_NormalSRVDesc, normHandle);

    D3D12_CPU_DESCRIPTOR_HANDLE roughHandle = DescriptorManager12.AllocateSRV();
    HW12.m_pDevice->CreateShaderResourceView(m_pRoughnessBuffer.Get(), &m_RoughnessSRVDesc, roughHandle);

    D3D12_CPU_DESCRIPTOR_HANDLE mvHandle = DescriptorManager12.AllocateSRV();
    HW12.m_pDevice->CreateShaderResourceView(m_pMotionVectorBuffer.Get(), &m_MotionVectorSRVDesc, mvHandle);
}

dx12RenderPipeline RenderPipeline12;

#endif // USE_DX12
