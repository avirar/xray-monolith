#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12Shadows.h"

#ifdef USE_DX12

dx12Shadows::dx12Shadows()
    : m_Enabled(false),
      m_Initialized(false),
      m_Width(0),
      m_Height(0),
      m_pShadowResource(nullptr),
      m_SamplesPerPixel(4),
      m_LightAreaSize(0.1f),
      m_MaxRayDistance(100.0f),
      m_ShadowOpacity(0.8f)
{
    ZeroMemory(&m_FrameData, sizeof(m_FrameData));
}

dx12Shadows::~dx12Shadows()
{
    if (m_pShadowResource)
        m_pShadowResource->Release();
}

void dx12Shadows::Init()
{
    m_Initialized = true;
    CreateConstantBuffer();
}

void dx12Shadows::CreateConstantBuffer()
{
    m_ConstantBuffer.Create("Shadow_ConstantBuffer");
}

void dx12Shadows::Enable(bool Enable)
{
    m_Enabled = Enable;
}

void dx12Shadows::SetSamplesPerPixel(u32 Samples)
{
    m_SamplesPerPixel = Samples;
}

void dx12Shadows::SetLightAreaSize(float Size)
{
    m_LightAreaSize = Size;
}

void dx12Shadows::SetMaxRayDistance(float Distance)
{
    m_MaxRayDistance = Distance;
}

void dx12Shadows::SetShadowOpacity(float Opacity)
{
    m_ShadowOpacity = Opacity;
}

void dx12Shadows::Resize(u32 Width, u32 Height)
{
    if (m_Width == Width && m_Height == Height)
        return;

    m_Width = Width;
    m_Height = Height;

    CreateShadowTexture(Width, Height);
}

void dx12Shadows::CreateShadowTexture(u32 Width, u32 Height)
{
    if (m_pShadowResource)
    {
        m_pShadowResource->Release();
        m_pShadowResource = nullptr;
    }

    D3D12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R16_FLOAT,
        Width, Height, 1, 1,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );

    R_CHK(HW12.GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_pShadowResource)
    ));

    if (!m_pShadowResource)
    {
        LOG("*ERROR* dx12Shadows: Failed to create shadow texture");
        return;
    }

    m_pShadowResource->SetName(L"Shadow_Map");

    HW12.GetDevice()->CreateUnorderedAccessView(
        m_pShadowResource, nullptr, &m_ShadowUAV);

    HW12.GetDevice()->CreateShaderResourceView(
        m_pShadowResource, nullptr, &m_ShadowSRV);
}

void dx12Shadows::BeginFrame()
{
    if (!m_Enabled || !m_Initialized)
        return;

    UpdateConstantBuffer();
}

void dx12Shadows::UpdateConstantBuffer()
{
    Fmatrix& vp = m_FrameData.ViewProjection;

    vp.set(RImplementation.Cameras.oMatrix);

    m_FrameData.LightPosition.set(RImplementation.Cameras.oPosition);
    m_FrameData.LightType = 0.0f; // Directional (default)
    m_FrameData.LightDirection.set(0, -1, 0);
    m_FrameData.LightRange = m_MaxRayDistance;
    m_FrameData.ScreenWidth = m_Width;
    m_FrameData.ScreenHeight = m_Height;
    m_FrameData.SamplesPerPixel = m_SamplesPerPixel;
    m_FrameData.LightAreaSize = m_LightAreaSize;
    m_FrameData.RayBias = 0.001f;
    m_FrameData.ShadowOpacity = m_ShadowOpacity;
    m_FrameData.MaxRayDistance = m_MaxRayDistance;

    m_ConstantBuffer.Update(&m_FrameData);
}

void dx12Shadows::UpdateShadows()
{
    if (!m_Enabled || !m_Initialized)
        return;

    DispatchShadows();
}

void dx12Shadows::DispatchShadows()
{
    ApplyResourceBarriers();

    // Bind constant buffer
    RayTracingDispatch12.SetRootConstantBufferView(0, m_ConstantBuffer.GetGPUVirtualAddress());

    // Bind G-buffer SRVs (position, normal)
    D3D12_GPU_DESCRIPTOR_HANDLE gbufferSRVs = DescriptorManager12.GetSRVUAVHeapHandle();
    RayTracingDispatch12.SetRootDescriptorTable(1, gbufferSRVs);

    // Bind shadow output UAV
    D3D12_GPU_DESCRIPTOR_HANDLE shadowUAV = DescriptorManager12.GetSRVUAVHeapHandle();
    RayTracingDispatch12.SetRootDescriptorTable(2, shadowUAV);

    // Bind TLAS
    D3D12_GPU_DESCRIPTOR_HANDLE tlasHandle = DescriptorManager12.GetSRVUAVHeapHandle();
    RayTracingDispatch12.SetRootDescriptorTable(3, tlasHandle);

    // Dispatch rays
    RayTracingDispatch12.DispatchShadow(m_Width, m_Height);
}

void dx12Shadows::ApplyResourceBarriers()
{
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pShadowResource,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    Backend12.GetCommandList()->ResourceBarrier(1, &barrier);
}

void dx12Shadows::EndFrame()
{
    if (!m_Enabled || !m_Initialized)
        return;

    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pShadowResource,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON);

    Backend12.GetCommandList()->ResourceBarrier(1, &barrier);
}

D3D12_CPU_DESCRIPTOR_HANDLE dx12Shadows::GetShadowSRV() const
{
    return m_ShadowSRV;
}

dx12Shadows Shadows12;

#endif // USE_DX12
