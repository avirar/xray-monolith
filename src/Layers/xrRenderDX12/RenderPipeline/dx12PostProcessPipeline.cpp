#include "../dx12stdafx.h"
#include "dx12PostProcessPipeline.h"

#ifdef USE_DX12

dx12PostProcessPipeline::dx12PostProcessPipeline()
{
    ZeroMemory(&m_desc, sizeof(m_desc));
    ZeroMemory(&m_GenericRTVs0, sizeof(m_GenericRTVs0));
    ZeroMemory(&m_GenericRTVs1, sizeof(m_GenericRTVs1));
    ZeroMemory(&m_BloomRTVs, sizeof(m_BloomRTVs));
    ZeroMemory(&m_BloomRTVs1, sizeof(m_BloomRTVs1));
    ZeroMemory(&m_SSAORTVs, sizeof(m_SSAORTVs));
}

dx12PostProcessPipeline::~dx12PostProcessPipeline()
{
    Destroy();
}

void dx12PostProcessPipeline::Init(const DX12_POSTPROCESS_DESC& Desc)
{
    m_desc = Desc;
    CreateResources(Desc);
    CreateRTVs();
    CreateSRVs();
}

void dx12PostProcessPipeline::Destroy()
{
    m_pGenericBuffer0.Reset();
    m_pGenericBuffer1.Reset();
    m_pBloomBuffer.Reset();
    m_pBloomBuffer1.Reset();
    m_pSSAOBuffer.Reset();
    m_pRtvHeap.Reset();
}

void dx12PostProcessPipeline::Resize(u32 Width, u32 Height)
{
    m_desc.Width = Width;
    m_desc.Height = Height;
    Destroy();
    CreateResources(m_desc);
    CreateRTVs();
    CreateSRVs();
}

void dx12PostProcessPipeline::BeginPass(DX12_POSTPROCESS_PASS Pass)
{
    UINT frameIndex = CommandManager12.GetFrameIndex();

    D3D12_CPU_DESCRIPTOR_HANDLE rtv;
    ID3D12Resource* target = nullptr;

    switch (Pass)
    {
    case DPP_BLOOM_EXTRACT:
    case DPP_BLOOM_BLUR_H:
    case DPP_BLOOM_BLUR_V:
        target = m_pBloomBuffer.Get();
        rtv = m_BloomRTVs[frameIndex];
        break;
    case DPP_SSAO_BLUR:
        target = m_pSSAOBuffer.Get();
        rtv = m_SSAORTVs[frameIndex];
        break;
    case DPP_SSS:
    case DPP_SUNSHAFTS:
        target = m_pGenericBuffer0.Get();
        rtv = m_GenericRTVs0[frameIndex];
        break;
    case DPP_TAA:
    case DPP_FINAL_COMBINE:
        target = m_pGenericBuffer1.Get();
        rtv = m_GenericRTVs1[frameIndex];
        break;
    default:
        return;
    }

    Backend12.ResourceBarrier(target,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

    ID3D12DescriptorHeap* pHeaps[] = {
        m_pRtvHeap.Get(),
        HW12.m_pDsvHeap.Get(),
        DescriptorManager12.GetSRVUAVHeap(),
        DescriptorManager12.GetCBVHeap()
    };
    Backend12.GetCommandList()->SetDescriptorHeaps(_countof(pHeaps), pHeaps);
    Backend12.GetCommandList()->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

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

void dx12PostProcessPipeline::EndPass(DX12_POSTPROCESS_PASS Pass)
{
    ID3D12Resource* target = nullptr;

    switch (Pass)
    {
    case DPP_BLOOM_EXTRACT:
    case DPP_BLOOM_BLUR_H:
    case DPP_BLOOM_BLUR_V:
        target = m_pBloomBuffer.Get();
        break;
    case DPP_SSAO_BLUR:
        target = m_pSSAOBuffer.Get();
        break;
    case DPP_SSS:
    case DPP_SUNSHAFTS:
        target = m_pGenericBuffer0.Get();
        break;
    case DPP_TAA:
    case DPP_FINAL_COMBINE:
        target = m_pGenericBuffer1.Get();
        break;
    default:
        return;
    }

    Backend12.ResourceBarrier(target,
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void dx12PostProcessPipeline::TransitionToShaderResource(DX12_POSTPROCESS_PASS Pass)
{
    ID3D12Resource* target = nullptr;

    switch (Pass)
    {
    case DPP_BLOOM_EXTRACT:
    case DPP_BLOOM_BLUR_H:
    case DPP_BLOOM_BLUR_V:
        target = m_pBloomBuffer.Get();
        break;
    case DPP_SSAO_BLUR:
        target = m_pSSAOBuffer.Get();
        break;
    case DPP_SSS:
    case DPP_SUNSHAFTS:
        target = m_pGenericBuffer0.Get();
        break;
    case DPP_TAA:
    case DPP_FINAL_COMBINE:
        target = m_pGenericBuffer1.Get();
        break;
    default:
        return;
    }

    Backend12.ResourceBarrier(target,
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void dx12PostProcessPipeline::TransitionToRenderTarget(DX12_POSTPROCESS_PASS Pass)
{
    ID3D12Resource* target = nullptr;

    switch (Pass)
    {
    case DPP_BLOOM_EXTRACT:
    case DPP_BLOOM_BLUR_H:
    case DPP_BLOOM_BLUR_V:
        target = m_pBloomBuffer.Get();
        break;
    case DPP_SSAO_BLUR:
        target = m_pSSAOBuffer.Get();
        break;
    case DPP_SSS:
    case DPP_SUNSHAFTS:
        target = m_pGenericBuffer0.Get();
        break;
    case DPP_TAA:
    case DPP_FINAL_COMBINE:
        target = m_pGenericBuffer1.Get();
        break;
    default:
        return;
    }

    Backend12.ResourceBarrier(target,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void dx12PostProcessPipeline::ResolveMSAA(ID3D12Resource* Src, ID3D12Resource* Dst, DXGI_FORMAT Format)
{
    Backend12.GetCommandList()->ResolveSubresource(Dst, 0, Src, 0, Format);
}

void dx12PostProcessPipeline::CreateResources(const DX12_POSTPROCESS_DESC& Desc)
{
    D3D12_RESOURCE_DESC rtDesc = {};
    rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rtDesc.Alignment = 0;
    rtDesc.DepthOrArraySize = 1;
    rtDesc.MipLevels = 1;
    rtDesc.SampleDesc.Count = 1;
    rtDesc.SampleDesc.Quality = 0;
    rtDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    rtDesc.Width = Desc.Width;
    rtDesc.Height = Desc.Height;
    rtDesc.Format = Desc.GenericFormat;
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &rtDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pGenericBuffer0)));

    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &rtDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pGenericBuffer1)));

    rtDesc.Width = Desc.Width / 2;
    rtDesc.Height = Desc.Height / 2;
    rtDesc.Format = Desc.BloomFormat;
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &rtDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pBloomBuffer)));

    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &rtDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pBloomBuffer1)));

    rtDesc.Width = Desc.Width;
    rtDesc.Height = Desc.Height;
    rtDesc.Format = Desc.GenericFormat;
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &rtDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pSSAOBuffer)));
}

void dx12PostProcessPipeline::CreateRTVs()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 5 * HW12.m_frameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    R_CHK(HW12.m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap)));

    D3D12_CPU_DESCRIPTOR_HANDLE rtvStart = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    UINT rtvIncrement = HW12.m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    for (UINT i = 0; i < HW12.m_frameCount; i++)
    {
        m_GenericRTVs0[i].ptr = rtvStart.ptr + i * rtvIncrement;
        HW12.m_pDevice->CreateRenderTargetView(m_pGenericBuffer0.Get(), nullptr, m_GenericRTVs0[i]);

        m_GenericRTVs1[i].ptr = rtvStart.ptr + (HW12.m_frameCount + i) * rtvIncrement;
        HW12.m_pDevice->CreateRenderTargetView(m_pGenericBuffer1.Get(), nullptr, m_GenericRTVs1[i]);

        m_BloomRTVs[i].ptr = rtvStart.ptr + (HW12.m_frameCount * 2 + i) * rtvIncrement;
        HW12.m_pDevice->CreateRenderTargetView(m_pBloomBuffer.Get(), nullptr, m_BloomRTVs[i]);

        m_BloomRTVs1[i].ptr = rtvStart.ptr + (HW12.m_frameCount * 3 + i) * rtvIncrement;
        HW12.m_pDevice->CreateRenderTargetView(m_pBloomBuffer1.Get(), nullptr, m_BloomRTVs1[i]);

        m_SSAORTVs[i].ptr = rtvStart.ptr + (HW12.m_frameCount * 4 + i) * rtvIncrement;
        HW12.m_pDevice->CreateRenderTargetView(m_pSSAOBuffer.Get(), nullptr, m_SSAORTVs[i]);
    }
}

void dx12PostProcessPipeline::CreateSRVs()
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    srvDesc.Format = m_desc.GenericFormat;
    D3D12_CPU_DESCRIPTOR_HANDLE handle = DescriptorManager12.AllocateSRV();
    HW12.m_pDevice->CreateShaderResourceView(m_pGenericBuffer0.Get(), &srvDesc, handle);

    handle = DescriptorManager12.AllocateSRV();
    HW12.m_pDevice->CreateShaderResourceView(m_pGenericBuffer1.Get(), &srvDesc, handle);

    srvDesc.Format = m_desc.BloomFormat;
    handle = DescriptorManager12.AllocateSRV();
    HW12.m_pDevice->CreateShaderResourceView(m_pBloomBuffer.Get(), &srvDesc, handle);

    handle = DescriptorManager12.AllocateSRV();
    HW12.m_pDevice->CreateShaderResourceView(m_pBloomBuffer1.Get(), &srvDesc, handle);

    srvDesc.Format = m_desc.GenericFormat;
    handle = DescriptorManager12.AllocateSRV();
    HW12.m_pDevice->CreateShaderResourceView(m_pSSAOBuffer.Get(), &srvDesc, handle);
}

dx12PostProcessPipeline PostProcessPipeline12;

#endif // USE_DX12
