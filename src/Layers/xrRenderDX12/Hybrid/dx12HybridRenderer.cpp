#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12HybridRenderer.h"
#include "../GI/dx12GI.h"
#include "../Reflections/dx12Reflections.h"
#include "../Shadows/dx12Shadows.h"
#include "../RenderPipeline/dx12RenderPipeline.h"
#include "../RenderPipeline/dx12LightingPipeline.h"
#include "../RenderPipeline/dx12PostProcessPipeline.h"

#ifdef USE_DX12

dx12HybridRenderer::dx12HybridRenderer()
    : m_Width(0),
      m_Height(0),
      m_RenderWidth(0),
      m_RenderHeight(0),
      m_PassCount(0),
      m_FrameTime(0.0f),
      m_Initialized(false)
{
    m_Desc.EnabledPasses = DX12_PASS_ALL;
    m_Desc.EnableGI = true;
    m_Desc.EnableShadows = true;
    m_Desc.EnableReflections = true;
    m_Desc.EnablePostProcess = true;
    m_Desc.RenderScale = 1.0f;
    m_Desc.EnableUpscaling = false;
}

dx12HybridRenderer::~dx12HybridRenderer()
{
}

void dx12HybridRenderer::Init()
{
    m_Initialized = true;
}

void dx12HybridRenderer::SetRenderDesc(const DX12_HYBRID_RENDER_DESC& Desc)
{
    m_Desc = Desc;
}

void dx12HybridRenderer::Resize(u32 Width, u32 Height)
{
    m_Width = Width;
    m_Height = Height;

    // Calculate render resolution based on scale
    m_RenderWidth = (u32)(Width * m_Desc.RenderScale);
    m_RenderHeight = (u32)(Height * m_Desc.RenderScale);

    // Resize all subsystems
    GI12.Resize(m_RenderWidth, m_RenderHeight);
    Reflections12.Resize(m_RenderWidth, m_RenderHeight);
    Shadows12.Resize(m_RenderWidth, m_RenderHeight);
}

void dx12HybridRenderer::BeginFrame()
{
    if (!m_Initialized)
        return;

    m_PassCount = 0;

    // Begin all subsystems
    GI12.BeginFrame();
    Reflections12.BeginFrame();
    Shadows12.BeginFrame();
}

void dx12HybridRenderer::RenderFrame()
{
    if (!m_Initialized)
        return;

    BeginFrame();

    // Execute render passes in order
    if (m_Desc.EnabledPasses & DX12_PASS_GBUFFER)
        RenderGBuffer();

    if (m_Desc.EnabledPasses & DX12_PASS_GI && m_Desc.EnableGI)
        RenderGI();

    if (m_Desc.EnabledPasses & DX12_PASS_SHADOWS && m_Desc.EnableShadows)
        RenderShadows();

    if (m_Desc.EnabledPasses & DX12_PASS_REFLECTIONS && m_Desc.EnableReflections)
        RenderReflections();

    if (m_Desc.EnabledPasses & DX12_PASS_LIGHTING)
        RenderLighting();

    if (m_Desc.EnabledPasses & DX12_PASS_POSTPROCESS && m_Desc.EnablePostProcess)
        RenderPostProcess();

    if (m_Desc.EnabledPasses & DX12_PASS_PRESENT)
        Present();

    EndFrame();
}

void dx12HybridRenderer::RenderGBuffer()
{
    ApplyPassBarriers(DX12_PASS_GBUFFER);
    m_PassCount++;

    // G-buffer rendering is handled by RenderPipeline12
    RenderPipeline12.RenderGBuffer();
}

void dx12HybridRenderer::RenderGI()
{
    ApplyPassBarriers(DX12_PASS_GI);
    m_PassCount++;

    GI12.UpdateGI();
}

void dx12HybridRenderer::RenderShadows()
{
    ApplyPassBarriers(DX12_PASS_SHADOWS);
    m_PassCount++;

    Shadows12.UpdateShadows();
}

void dx12HybridRenderer::RenderReflections()
{
    ApplyPassBarriers(DX12_PASS_REFLECTIONS);
    m_PassCount++;

    Reflections12.UpdateReflections();
}

void dx12HybridRenderer::RenderLighting()
{
    ApplyPassBarriers(DX12_PASS_LIGHTING);
    m_PassCount++;

    LightingPipeline12.RenderLighting();
}

void dx12HybridRenderer::RenderPostProcess()
{
    ApplyPassBarriers(DX12_PASS_POSTPROCESS);
    m_PassCount++;

    PostProcessPipeline12.RenderPostProcess();
}

void dx12HybridRenderer::Present()
{
    m_PassCount++;

    // Present is handled by CommandManager12
    CommandManager12.Present();
}

void dx12HybridRenderer::EndFrame()
{
    // End all subsystems
    GI12.EndFrame();
    Reflections12.EndFrame();
    Shadows12.EndFrame();
}

void dx12HybridRenderer::ExecutePass(DX12_RENDER_PASS Pass)
{
    switch (Pass)
    {
    case DX12_PASS_GBUFFER:
        RenderGBuffer();
        break;
    case DX12_PASS_GI:
        RenderGI();
        break;
    case DX12_PASS_SHADOWS:
        RenderShadows();
        break;
    case DX12_PASS_REFLECTIONS:
        RenderReflections();
        break;
    case DX12_PASS_LIGHTING:
        RenderLighting();
        break;
    case DX12_PASS_POSTPROCESS:
        RenderPostProcess();
        break;
    case DX12_PASS_PRESENT:
        Present();
        break;
    }
}

void dx12HybridRenderer::ApplyPassBarriers(DX12_RENDER_PASS Pass)
{
    // Apply resource barriers based on pass transitions
    // This ensures proper state transitions between raster and ray tracing passes

    switch (Pass)
    {
    case DX12_PASS_GBUFFER:
        // G-buffer outputs: ensure RT state
        break;
    case DX12_PASS_GI:
        // GI needs G-buffer as PS resource, writes to GI UAV
        break;
    case DX12_PASS_SHADOWS:
        // Shadows need G-buffer as PS resource, write to shadow UAV
        break;
    case DX12_PASS_REFLECTIONS:
        // Reflections need G-buffer as PS resource, write to reflection UAV
        break;
    case DX12_PASS_LIGHTING:
        // Lighting reads GI, shadows, reflections as PS resources
        break;
    case DX12_PASS_POSTPROCESS:
        // Post-process reads lighting output
        break;
    }
}

dx12HybridRenderer HybridRenderer12;

#endif // USE_DX12
