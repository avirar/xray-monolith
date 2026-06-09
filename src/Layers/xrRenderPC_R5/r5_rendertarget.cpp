#include "r5_rendertarget.h"

#ifdef USE_DX12

#include "../xrRenderDX12/dx12HW.h"
#include "../xrRenderDX12/GI/dx12GIResources.h"
#include "../xrRenderDX12/Reflections/dx12ReflectionResources.h"
#include "../xrRenderDX12/Shadows/dx12Shadows.h"
#include "../xrRenderDX12/RenderPipeline/dx12RenderPipeline.h"

R5RenderTarget R5RT;

R5RenderTarget::R5RenderTarget()
    : m_Width(0),
      m_Height(0),
      m_Initialized(false)
{
}

R5RenderTarget::~R5RenderTarget()
{
}

void R5RenderTarget::Init()
{
    m_Initialized = true;
}

void R5RenderTarget::Destroy()
{
    m_Initialized = false;
}

void R5RenderTarget::Resize(u32 Width, u32 Height)
{
    m_Width = Width;
    m_Height = Height;

    // Resize subsystem render targets
    GIResources12.Resize(Width, Height);
    ReflectionResources12.Resize(Width, Height);
}

ID3D12Resource* R5RenderTarget::GetGBufferPosition() const
{
    return nullptr; // Will be provided by RenderPipeline12
}

ID3D12Resource* R5RenderTarget::GetGBufferColor() const
{
    return nullptr; // Will be provided by RenderPipeline12
}

ID3D12Resource* R5RenderTarget::GetGBufferNormal() const
{
    return nullptr; // Will be provided by RenderPipeline12
}

ID3D12Resource* R5RenderTarget::GetGIResource() const
{
    return GIResources12.GetIrradianceResource();
}

ID3D12Resource* R5RenderTarget::GetShadowResource() const
{
    return nullptr; // Will be provided by Shadows12
}

ID3D12Resource* R5RenderTarget::GetReflectionResource() const
{
    return ReflectionResources12.GetReflectionResource();
}

D3D12_CPU_DESCRIPTOR_HANDLE R5RenderTarget::GetGBufferPositionSRV() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE::NullHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE R5RenderTarget::GetGBufferColorSRV() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE::NullHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE R5RenderTarget::GetGBufferNormalSRV() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE::NullHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE R5RenderTarget::GetGISRV() const
{
    return GIResources12.GetIrradianceSRV();
}

D3D12_CPU_DESCRIPTOR_HANDLE R5RenderTarget::GetShadowSRV() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE::NullHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE R5RenderTarget::GetReflectionSRV() const
{
    return ReflectionResources12.GetReflectionSRV();
}

void R5RenderTarget::TransitionToShaderResource()
{
    // Resource barriers handled by individual subsystems
}

void R5RenderTarget::TransitionToRenderTarget()
{
    // Resource barriers handled by individual subsystems
}

void R5RenderTarget::TransitionToUnorderedAccess()
{
    // Resource barriers handled by individual subsystems
}

#endif // USE_DX12
