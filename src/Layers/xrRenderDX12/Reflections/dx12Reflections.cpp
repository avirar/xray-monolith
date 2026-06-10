#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12Reflections.h"

#ifdef USE_DX12

dx12Reflections::dx12Reflections()
    : m_Enabled(false),
      m_Initialized(false),
      m_Width(0),
      m_Height(0),
      m_SamplesPerPixel(4),
      m_MaxBounces(2),
      m_MaxRayDistance(200.0f),
      m_MinRoughness(0.5f),
      m_SkyContribution(0.2f)
{
    ZeroMemory(&m_FrameData, sizeof(m_FrameData));
}

dx12Reflections::~dx12Reflections()
{
}

void dx12Reflections::Init()
{
    m_Initialized = true;
    CreateConstantBuffer();
}

void dx12Reflections::CreateConstantBuffer()
{
    m_ConstantBuffer.Create("Reflection_ConstantBuffer");
}

void dx12Reflections::Enable(bool Enable)
{
    m_Enabled = Enable;
}

void dx12Reflections::SetSamplesPerPixel(u32 Samples)
{
    m_SamplesPerPixel = Samples;
}

void dx12Reflections::SetMaxBounces(u32 Bounces)
{
    m_MaxBounces = Bounces;
}

void dx12Reflections::SetMaxRayDistance(float Distance)
{
    m_MaxRayDistance = Distance;
}

void dx12Reflections::SetMinRoughness(float Roughness)
{
    m_MinRoughness = Roughness;
}

void dx12Reflections::SetSkyContribution(float Contribution)
{
    m_SkyContribution = Contribution;
}

void dx12Reflections::Resize(u32 Width, u32 Height)
{
    m_Width = Width;
    m_Height = Height;
    ReflectionResources12.Resize(Width, Height);
}

void dx12Reflections::BeginFrame()
{
    if (!m_Enabled || !m_Initialized)
        return;

    UpdateConstantBuffer();
}

void dx12Reflections::UpdateConstantBuffer()
{
    Fmatrix& vp = m_FrameData.ViewProjection;
    Fmatrix& view = m_FrameData.View;

    view.set(RImplementation.Cameras.mvMatrix);
    vp.mulA_B_C(RImplementation.Cameras.oMatrix, RImplementation.Cameras.mvMatrix);

    m_FrameData.CameraPosition.set(RImplementation.Cameras.oPosition);
    m_FrameData.ScreenWidth = m_Width;
    m_FrameData.ScreenHeight = m_Height;
    m_FrameData.SamplesPerPixel = m_SamplesPerPixel;
    m_FrameData.MaxRayDistance = m_MaxRayDistance;
    m_FrameData.RayBias = 0.001f;
    m_FrameData.SkyContribution = m_SkyContribution;
    m_FrameData.MaxBounces = m_MaxBounces;
    m_FrameData.MinRoughness = m_MinRoughness;

    m_ConstantBuffer.Update(&m_FrameData);
}

void dx12Reflections::UpdateReflections()
{
    if (!m_Enabled || !m_Initialized)
        return;

    DispatchReflections();
}

void dx12Reflections::DispatchReflections()
{
    ApplyResourceBarriers();

    // Bind constant buffer
    RayTracingDispatch12.SetRootConstantBufferView(0, m_ConstantBuffer.GetGPUVirtualAddress());

    // Bind G-buffer SRVs (position, normal, roughness)
    D3D12_GPU_DESCRIPTOR_HANDLE gbufferSRVs = DescriptorManager12.GetSRVUAVHeapHandle();
    RayTracingDispatch12.SetRootDescriptorTable(1, gbufferSRVs);

    // Bind reflection output UAVs
    D3D12_GPU_DESCRIPTOR_HANDLE reflectionUAVs = DescriptorManager12.GetSRVUAVHeapHandle();
    RayTracingDispatch12.SetRootDescriptorTable(2, reflectionUAVs);

    // Bind TLAS
    D3D12_GPU_DESCRIPTOR_HANDLE tlasHandle = DescriptorManager12.GetSRVUAVHeapHandle();
    RayTracingDispatch12.SetRootDescriptorTable(3, tlasHandle);

    // Dispatch rays
    RayTracingDispatch12.DispatchReflection(m_Width, m_Height, m_SamplesPerPixel);
}

void dx12Reflections::ApplyResourceBarriers()
{
    D3D12_RESOURCE_BARRIER barriers[2];
    UINT numBarriers = 0;

    barriers[numBarriers] = CD3DX12_RESOURCE_BARRIER::Transition(
        ReflectionResources12.GetReflectionResource(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    numBarriers++;

    barriers[numBarriers] = CD3DX12_RESOURCE_BARRIER::Transition(
        ReflectionResources12.GetRoughnessMaskResource(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    numBarriers++;

    Backend12.GetCommandList()->ResourceBarrier(numBarriers, barriers);
}

void dx12Reflections::EndFrame()
{
    if (!m_Enabled || !m_Initialized)
        return;

    D3D12_RESOURCE_BARRIER barriers[2];
    UINT numBarriers = 0;

    barriers[numBarriers] = CD3DX12_RESOURCE_BARRIER::Transition(
        ReflectionResources12.GetReflectionResource(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON);
    numBarriers++;

    barriers[numBarriers] = CD3DX12_RESOURCE_BARRIER::Transition(
        ReflectionResources12.GetRoughnessMaskResource(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON);
    numBarriers++;

    Backend12.GetCommandList()->ResourceBarrier(numBarriers, barriers);
}

D3D12_CPU_DESCRIPTOR_HANDLE dx12Reflections::GetReflectionSRV() const
{
    return ReflectionResources12.GetReflectionSRV();
}

D3D12_CPU_DESCRIPTOR_HANDLE dx12Reflections::GetRoughnessMaskSRV() const
{
    return ReflectionResources12.GetRoughnessMaskSRV();
}

dx12Reflections Reflections12;

#endif // USE_DX12
