#include "stdafx.h"
#include "dx12stdafx.h"
#include "dx12RendererSelector.h"

#ifdef USE_DX12

dx12RendererSelector::dx12RendererSelector()
    : m_SelectedRenderer(DX12_RENDERER_AUTO),
      m_DXRQuality(DX12_DXR_QUALITY_MEDIUM),
      m_EnableGI(true),
      m_EnableShadows(true),
      m_EnableReflections(true),
      m_EnablePostProcess(true),
      m_Initialized(false)
{
    ZeroMemory(&m_Capabilities, sizeof(m_Capabilities));
}

dx12RendererSelector::~dx12RendererSelector()
{
}

void dx12RendererSelector::Init()
{
    m_Initialized = true;

    DetectCapabilities();

    if (m_Capabilities.SupportsDX12)
    {
        m_SelectedRenderer = DX12_RENDERER_DX12;
        Msg("*     Renderer selected: DirectX 12");
    }
    else
    {
        m_SelectedRenderer = DX12_RENDERER_DX11;
        Msg("*     Renderer selected: DirectX 11 (DX12 not supported)");
    }

    ApplyDXRQualitySettings();
}

DX12_RENDERER_CAPABILITIES dx12RendererSelector::DetectCapabilities()
{
    m_Capabilities.SupportsDX12 = CheckDX12Support();
    m_Capabilities.SupportsDXR = CheckDXRSupport();

    if (m_Capabilities.SupportsDX12)
    {
        m_Capabilities.RaytracingTier = (u32)HW12.GetDXRTier();
        m_Capabilities.MaxRayRecursionDepth = 31; // DX12 maximum

        // Check HDR10 support
        m_Capabilities.SupportsHDR10 = HW12.GetDXGISupportsHDR10();

        // Check VRR support
        m_Capabilities.SupportsVRR = HW12.GetSupportsVRR();
        m_Capabilities.SupportsTearing = HW12.GetSupportsTearing();

        Msg("*     DX12 capabilities:");
        Msg("*       DXR: %s", m_Capabilities.SupportsDXR ? "Yes" : "No");
        Msg("*       Raytracing Tier: %d", m_Capabilities.RaytracingTier);
        Msg("*       HDR10: %s", m_Capabilities.SupportsHDR10 ? "Yes" : "No");
        Msg("*       VRR: %s", m_Capabilities.SupportsVRR ? "Yes" : "No");
    }

    return m_Capabilities;
}

bool dx12RendererSelector::CheckDX12Support()
{
    return HW12.GetDevice() != nullptr;
}

bool dx12RendererSelector::CheckDXRSupport()
{
    return HW12.GetDXRTier() >= D3D12_RAYTRACING_TIER_1_1;
}

DX12_RENDERER_TYPE dx12RendererSelector::SelectRenderer(DX12_RENDERER_TYPE RequestedType)
{
    if (RequestedType == DX12_RENDERER_AUTO)
    {
        // Auto-detect
        if (m_Capabilities.SupportsDX12)
            return DX12_RENDERER_DX12;
        else
            return DX12_RENDERER_DX11;
    }

    // Validate requested type
    switch (RequestedType)
    {
    case DX12_RENDERER_DX12:
        if (!m_Capabilities.SupportsDX12)
        {
            Msg("*     DX12 not supported, falling back to DX11");
            return DX12_RENDERER_DX11;
        }
        break;
    case DX12_RENDERER_DX11:
    case DX12_RENDERER_DX10:
    case DX12_RENDERER_DX9:
        break;
    default:
        return DX12_RENDERER_AUTO;
    }

    m_SelectedRenderer = RequestedType;
    return m_SelectedRenderer;
}

void dx12RendererSelector::SetDXRQuality(DX12_DXR_QUALITY Quality)
{
    m_DXRQuality = Quality;
    ApplyDXRQualitySettings();
}

void dx12RendererSelector::ApplyDXRQualitySettings()
{
    switch (m_DXRQuality)
    {
    case DX12_DXR_QUALITY_LOW:
        GI12.SetHemisphereSamples(1);
        Shadows12.SetSamplesPerPixel(1);
        Reflections12.SetSamplesPerPixel(1);
        Msg("*     DXR Quality: Low");
        break;
    case DX12_DXR_QUALITY_MEDIUM:
        GI12.SetHemisphereSamples(4);
        Shadows12.SetSamplesPerPixel(4);
        Reflections12.SetSamplesPerPixel(2);
        Msg("*     DXR Quality: Medium");
        break;
    case DX12_DXR_QUALITY_HIGH:
        GI12.SetHemisphereSamples(8);
        Shadows12.SetSamplesPerPixel(8);
        Reflections12.SetSamplesPerPixel(4);
        Msg("*     DXR Quality: High");
        break;
    case DX12_DXR_QUALITY_ULTRA:
        GI12.SetHemisphereSamples(16);
        Shadows12.SetSamplesPerPixel(16);
        Reflections12.SetSamplesPerPixel(8);
        Msg("*     DXR Quality: Ultra");
        break;
    }
}

void dx12RendererSelector::EnableGI(bool Enable)
{
    m_EnableGI = Enable;
    GI12.Enable(Enable);
}

void dx12RendererSelector::EnableShadows(bool Enable)
{
    m_EnableShadows = Enable;
    Shadows12.Enable(Enable);
}

void dx12RendererSelector::EnableReflections(bool Enable)
{
    m_EnableReflections = Enable;
    Reflections12.Enable(Enable);
}

void dx12RendererSelector::EnablePostProcess(bool Enable)
{
    m_EnablePostProcess = Enable;
}

dx12RendererSelector RendererSelector12;

#endif // USE_DX12
