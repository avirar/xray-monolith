#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

// Render pass flags
enum DX12_RENDER_PASS
{
    DX12_PASS_NONE = 0,
    DX12_PASS_GBUFFER = (1 << 0),       // Raster G-buffer
    DX12_PASS_GI = (1 << 1),            // DXR GI
    DX12_PASS_SHADOWS = (1 << 2),       // DXR Shadows
    DX12_PASS_REFLECTIONS = (1 << 3),   // DXR Reflections
    DX12_PASS_LIGHTING = (1 << 4),      // Raster light accumulation
    DX12_PASS_POSTPROCESS = (1 << 5),   // Post-processing
    DX12_PASS_PRESENT = (1 << 6),       // Present to swapchain
    DX12_PASS_ALL = 0xFF
};

struct DX12_HYBRID_RENDER_DESC
{
    // Render pass configuration
    u32 EnabledPasses;                 // Bitmask of DX12_RENDER_PASS flags

    // Quality settings
    bool EnableGI;                     // Enable ray-traced GI
    bool EnableShadows;                // Enable ray-traced shadows
    bool EnableReflections;            // Enable ray-traced reflections
    bool EnablePostProcess;            // Enable post-processing

    // Resolution scaling
    float RenderScale;                 // 1.0 = native, 0.5 = half res
    bool EnableUpscaling;              // Enable DLSS/FSR upscaling
};

class dx12HybridRenderer
{
public:
    dx12HybridRenderer();
    ~dx12HybridRenderer();

    void Init();

    // Main render loop
    void RenderFrame();

    // Individual pass execution
    void RenderGBuffer();
    void RenderGI();
    void RenderShadows();
    void RenderReflections();
    void RenderLighting();
    void RenderPostProcess();
    void Present();

    // Configuration
    void SetRenderDesc(const DX12_HYBRID_RENDER_DESC& Desc);
    const DX12_HYBRID_RENDER_DESC& GetRenderDesc() const { return m_Desc; }

    // Frame management
    void BeginFrame();
    void EndFrame();

    // Resource management
    void Resize(u32 Width, u32 Height);

    // Statistics
    u32 GetPassCount() const { return m_PassCount; }
    float GetFrameTime() const { return m_FrameTime; }

private:
    void ExecutePass(DX12_RENDER_PASS Pass);
    void ApplyPassBarriers(DX12_RENDER_PASS Pass);

    DX12_HYBRID_RENDER_DESC m_Desc;

    u32 m_Width;
    u32 m_Height;
    u32 m_RenderWidth;
    u32 m_RenderHeight;

    u32 m_PassCount;
    float m_FrameTime;

    bool m_Initialized;
};

extern dx12HybridRenderer HybridRenderer12;

#endif // USE_DX12
