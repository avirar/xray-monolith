#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

// Renderer type enumeration
enum DX12_RENDERER_TYPE
{
    DX12_RENDERER_AUTO = 0,     // Auto-detect best available
    DX12_RENDERER_DX12 = 1,     // Force DX12
    DX12_RENDERER_DX11 = 2,     // Force DX11
    DX12_RENDERER_DX10 = 3,     // Force DX10
    DX12_RENDERER_DX9 = 4       // Force DX9
};

// DXR quality presets
enum DX12_DXR_QUALITY
{
    DX12_DXR_QUALITY_LOW = 0,       // 1 GI sample, 1 shadow sample, 1 reflection sample
    DX12_DXR_QUALITY_MEDIUM = 1,    // 4 GI samples, 4 shadow samples, 2 reflection samples
    DX12_DXR_QUALITY_HIGH = 2,      // 8 GI samples, 8 shadow samples, 4 reflection samples
    DX12_DXR_QUALITY_ULTRA = 3      // 16 GI samples, 16 shadow samples, 8 reflection samples
};

struct DX12_RENDERER_CAPABILITIES
{
    bool SupportsDX12;              // DX12 supported
    bool SupportsDXR;               // DXR 1.1+ supported
    bool SupportsHDR10;             // HDR10 output supported
    bool SupportsVRR;               // Variable refresh rate supported
    bool SupportsTearing;           // Allow tearing supported

    u32 RaytracingTier;             // DXR tier (0, 1.1, 1.2, 1.3)
    u32 MaxRayRecursionDepth;       // Maximum ray recursion depth
};

class dx12RendererSelector
{
public:
    dx12RendererSelector();
    ~dx12RendererSelector();

    void Init();

    // Renderer selection
    DX12_RENDERER_TYPE SelectRenderer(DX12_RENDERER_TYPE RequestedType);
    bool IsDX12Selected() const { return m_SelectedRenderer == DX12_RENDERER_DX12; }

    // Capability detection
    DX12_RENDERER_CAPABILITIES DetectCapabilities();
    bool CheckDXRSupport() const { return m_Capabilities.SupportsDXR; }
    u32 GetRaytracingTier() const { return m_Capabilities.RaytracingTier; }

    // DXR quality
    void SetDXRQuality(DX12_DXR_QUALITY Quality);
    DX12_DXR_QUALITY GetDXRQuality() const { return m_DXRQuality; }

    // Feature toggles
    void EnableGI(bool Enable);
    void EnableShadows(bool Enable);
    void EnableReflections(bool Enable);
    void EnablePostProcess(bool Enable);

    bool IsGIEnabled() const { return m_EnableGI; }
    bool IsShadowsEnabled() const { return m_EnableShadows; }
    bool IsReflectionsEnabled() const { return m_EnableReflections; }
    bool IsPostProcessEnabled() const { return m_EnablePostProcess; }

    // Get selected renderer type
    DX12_RENDERER_TYPE GetSelectedRenderer() const { return m_SelectedRenderer; }

private:
    bool CheckDX12Support();
    bool CheckDXRSupport();
    void ApplyDXRQualitySettings();

    DX12_RENDERER_TYPE m_SelectedRenderer;
    DX12_RENDERER_CAPABILITIES m_Capabilities;
    DX12_DXR_QUALITY m_DXRQuality;

    bool m_EnableGI;
    bool m_EnableShadows;
    bool m_EnableReflections;
    bool m_EnablePostProcess;

    bool m_Initialized;
};

extern dx12RendererSelector RendererSelector12;

#endif // USE_DX12
