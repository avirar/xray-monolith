#include "../dx12stdafx.h"
#include "dx12GI.h"

#ifdef USE_DX12

dx12GI::dx12GI()
    : m_Enabled(false),
      m_Initialized(false),
      m_Width(0),
      m_Height(0),
      m_HemisphereSamples(8),
      m_MaxRayDistance(100.0f),
      m_BounceCount(1),
      m_BounceAttenuation(0.8f),
      m_SkyContribution(0.3f)
{
    ZeroMemory(&m_FrameData, sizeof(m_FrameData));
}

dx12GI::~dx12GI()
{
}

void dx12GI::Init()
{
    m_Initialized = true;
    CreateConstantBuffer();
}

void dx12GI::CreateConstantBuffer()
{
    m_ConstantBuffer.Create("GI_ConstantBuffer");
}

void dx12GI::Enable(bool Enable)
{
    m_Enabled = Enable;
}

void dx12GI::SetHemisphereSamples(u32 Samples)
{
    m_HemisphereSamples = Samples;
}

void dx12GI::SetMaxRayDistance(float Distance)
{
    m_MaxRayDistance = Distance;
}

void dx12GI::SetBounceCount(u32 Count)
{
    m_BounceCount = Count;
}

void dx12GI::SetBounceAttenuation(float Attenuation)
{
    m_BounceAttenuation = Attenuation;
}

void dx12GI::SetSkyContribution(float Contribution)
{
    m_SkyContribution = Contribution;
}

void dx12GI::Resize(u32 Width, u32 Height)
{
    m_Width = Width;
    m_Height = Height;
    GIResources12.Resize(Width, Height);
}

void dx12GI::BeginFrame()
{
    if (!m_Enabled || !m_Initialized)
        return;

    UpdateConstantBuffer();
}

void dx12GI::UpdateConstantBuffer()
{
    Fmatrix& vp = m_FrameData.ViewProjection;
    Fmatrix& view = m_FrameData.View;

    // Get current view/projection matrices from render state
    view.set(RImplementation.Cameras.mvMatrix);
    vp.mulA_B_C(RImplementation.Cameras.oMatrix, RImplementation.Cameras.mvMatrix);

    m_FrameData.CameraPosition.set(RImplementation.Cameras.oPosition);
    m_FrameData.ScreenWidth = m_Width;
    m_FrameData.ScreenHeight = m_Height;
    m_FrameData.HemisphereSamples = m_HemisphereSamples;
    m_FrameData.MaxRayDistance = m_MaxRayDistance;
    m_FrameData.RayBias = 0.001f;
    m_FrameData.SkyContribution = m_SkyContribution;
    m_FrameData.BounceCount = m_BounceCount;
    m_FrameData.BounceAttenuation = m_BounceAttenuation;

    m_ConstantBuffer.Update(&m_FrameData);
}

void dx12GI::UpdateGI()
{
    if (!m_Enabled || !m_Initialized)
        return;

    DispatchGI();
}

void dx12GI::DispatchGI()
{
    ApplyResourceBarriers();

    // Bind constant buffer
    RayTracingDispatch12.SetRootConstantBufferView(0, m_ConstantBuffer.GetGPUVirtualAddress());

    // Bind G-buffer SRVs (position, normal)
    D3D12_GPU_DESCRIPTOR_HANDLE gbufferSRVs = DescriptorManager12.GetSRVUAVHeapHandle();
    RayTracingDispatch12.SetRootDescriptorTable(1, gbufferSRVs);

    // Bind GI output UAVs
    D3D12_GPU_DESCRIPTOR_HANDLE giUAVs = DescriptorManager12.GetSRVUAVHeapHandle();
    RayTracingDispatch12.SetRootDescriptorTable(2, giUAVs);

    // Bind TLAS
    D3D12_GPU_DESCRIPTOR_HANDLE tlasHandle = DescriptorManager12.GetSRVUAVHeapHandle();
    RayTracingDispatch12.SetRootDescriptorTable(3, tlasHandle);

    // Dispatch rays
    RayTracingDispatch12.DispatchGI(m_Width, m_Height);
}

void dx12GI::ApplyResourceBarriers()
{
    // Transition GI resources to appropriate states
    D3D12_RESOURCE_BARRIER barriers[2];
    UINT numBarriers = 0;

    // Irradiance: COMMON -> UAV
    barriers[numBarriers] = CD3DX12_RESOURCE_BARRIER::Transition(
        GIResources12.GetIrradianceResource(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    numBarriers++;

    // AO: COMMON -> UAV
    barriers[numBarriers] = CD3DX12_RESOURCE_BARRIER::Transition(
        GIResources12.GetAOResource(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    numBarriers++;

    Backend12.GetCommandList()->ResourceBarrier(numBarriers, barriers);
}

void dx12GI::EndFrame()
{
    if (!m_Enabled || !m_Initialized)
        return;

    // Transition GI resources back to COMMON state
    D3D12_RESOURCE_BARRIER barriers[2];
    UINT numBarriers = 0;

    barriers[numBarriers] = CD3DX12_RESOURCE_BARRIER::Transition(
        GIResources12.GetIrradianceResource(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON);
    numBarriers++;

    barriers[numBarriers] = CD3DX12_RESOURCE_BARRIER::Transition(
        GIResources12.GetAOResource(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON);
    numBarriers++;

    Backend12.GetCommandList()->ResourceBarrier(numBarriers, barriers);
}

D3D12_CPU_DESCRIPTOR_HANDLE dx12GI::GetIrradianceSRV() const
{
    return GIResources12.GetIrradianceSRV();
}

D3D12_CPU_DESCRIPTOR_HANDLE dx12GI::GetAOSRV() const
{
    return GIResources12.GetAOSRV();
}

dx12GI GI12;

#endif // USE_DX12
