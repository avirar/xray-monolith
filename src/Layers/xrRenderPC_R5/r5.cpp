#include "r5.h"

#ifdef USE_DX12

#include "../xrRenderDX12/dx12HW.h"
#include "../xrRenderDX12/Hybrid/dx12HybridRenderer.h"
#include "../xrRenderDX12/Hybrid/dx12RenderPassManager.h"
#include "../xrRenderDX12/GI/dx12GI.h"
#include "../xrRenderDX12/Reflections/dx12Reflections.h"
#include "../xrRenderDX12/Shadows/dx12Shadows.h"
#include "../xrRenderDX12/dx12RendererSelector.h"

R5* R5Instance = nullptr;

R5::R5()
    : m_Initialized(false),
      m_DXRSuported(false),
      m_DX12Supported(false)
{
}

R5::~R5()
{
}

void R5::Init()
{
    if (m_Initialized)
        return;

    R5Instance = this;

    // Initialize DX12 hardware
    HW12.CreateDevice(nullptr, false);

    // Check capabilities
    m_DX12Supported = HW12.GetDevice() != nullptr;
    m_DXRSuported = HW12.GetDXRTier() >= D3D12_RAYTRACING_TIER_1_1;

    if (!m_DX12Supported)
    {
        LOG("*ERROR* R5: DX12 not supported, cannot initialize renderer");
        return;
    }

    Msg("*     R5 (DX12/DXR) renderer initialized");
    Msg("*     DXR Support: %s", m_DXRSuported ? "Yes" : "No");
    Msg("*     Raytracing Tier: %d", (u32)HW12.GetDXRTier());

    m_Initialized = true;
}

void R5::Destroy()
{
    if (!m_Initialized)
        return;

    HW12.DestroyDevice();

    R5Instance = nullptr;
    m_Initialized = false;
}

void R5::Render()
{
    if (!m_Initialized)
        return;

    BeginFrame();

    // Execute render passes
    HybridRenderer12.RenderFrame();

    EndFrame();
}

void R5::Present()
{
    if (!m_Initialized)
        return;

    CommandManager12.Present();
}

bool R5::IsDXRSupported() const
{
    return m_DXRSuported;
}

bool R5::IsDX12Supported() const
{
    return m_DX12Supported;
}

void R5::BeginFrame()
{
    if (!m_Initialized)
        return;

    CommandManager12.BeginFrame();
}

void R5::EndFrame()
{
    if (!m_Initialized)
        return;

    CommandManager12.EndFrame();
}

void R5::Resize(u32 Width, u32 Height)
{
    if (!m_Initialized)
        return;

    HybridRenderer12.Resize(Width, Height);
}

u32 R5::GetPassCount() const
{
    return HybridRenderer12.GetPassCount();
}

float R5::GetFrameTime() const
{
    return HybridRenderer12.GetFrameTime();
}

#endif // USE_DX12
