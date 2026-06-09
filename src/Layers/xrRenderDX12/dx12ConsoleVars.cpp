#include "dx12stdafx.h"
#include "dx12ConsoleVars.h"

#ifdef USE_DX12

dx12ConsoleVars::dx12ConsoleVars()
    : r_dxr_renderer(0),
      r_dxr_quality(1),
      r_dxr_gi(1),
      r_dxr_shadows(1),
      r_dxr_reflections(1),
      r_dxr_postprocess(1),
      r_dxr_gi_samples(8),
      r_dxr_gi_distance(100.0f),
      r_dxr_gi_bounces(1),
      r_dxr_gi_attenuation(0.8f),
      r_dxr_shadow_samples(4),
      r_dxr_shadow_area(0.1f),
      r_dxr_shadow_opacity(0.8f),
      r_dxr_reflection_samples(4),
      r_dxr_reflection_bounces(2),
      r_dxr_reflection_distance(200.0f),
      r_dxr_reflection_roughness(0.5f),
      r_dxr_render_scale(1.0f),
      r_dxr_upscaling(0),
      r_dxr_hdr10(0),
      r_dxr_vrr(0),
      r_dxr_tearing(0),
      m_Initialized(false)
{
}

dx12ConsoleVars::~dx12ConsoleVars()
{
}

void dx12ConsoleVars::Init()
{
    m_Initialized = true;

    // Register console variables
    CMD(CCC_Integer, "r_dxr_renderer", &r_dxr_renderer, 0, 4);
    CMD(CCC_Integer, "r_dxr_quality", &r_dxr_quality, 0, 3);

    CMD(CCC_Integer, "r_dxr_gi", &r_dxr_gi, 0, 1);
    CMD(CCC_Integer, "r_dxr_shadows", &r_dxr_shadows, 0, 1);
    CMD(CCC_Integer, "r_dxr_reflections", &r_dxr_reflections, 0, 1);
    CMD(CCC_Integer, "r_dxr_postprocess", &r_dxr_postprocess, 0, 1);

    CMD(CCC_Integer, "r_dxr_gi_samples", &r_dxr_gi_samples, 1, 16);
    CMD(CCC_Float, "r_dxr_gi_distance", &r_dxr_gi_distance, 1.0f, 500.0f);
    CMD(CCC_Integer, "r_dxr_gi_bounces", &r_dxr_gi_bounces, 0, 2);
    CMD(CCC_Float, "r_dxr_gi_attenuation", &r_dxr_gi_attenuation, 0.1f, 1.0f);

    CMD(CCC_Integer, "r_dxr_shadow_samples", &r_dxr_shadow_samples, 1, 16);
    CMD(CCC_Float, "r_dxr_shadow_area", &r_dxr_shadow_area, 0.01f, 1.0f);
    CMD(CCC_Float, "r_dxr_shadow_opacity", &r_dxr_shadow_opacity, 0.1f, 1.0f);

    CMD(CCC_Integer, "r_dxr_reflection_samples", &r_dxr_reflection_samples, 1, 8);
    CMD(CCC_Integer, "r_dxr_reflection_bounces", &r_dxr_reflection_bounces, 1, 4);
    CMD(CCC_Float, "r_dxr_reflection_distance", &r_dxr_reflection_distance, 1.0f, 500.0f);
    CMD(CCC_Float, "r_dxr_reflection_roughness", &r_dxr_reflection_roughness, 0.0f, 1.0f);

    CMD(CCC_Float, "r_dxr_render_scale", &r_dxr_render_scale, 0.5f, 1.0f);
    CMD(CCC_Integer, "r_dxr_upscaling", &r_dxr_upscaling, 0, 1);

    CMD(CCC_Integer, "r_dxr_hdr10", &r_dxr_hdr10, 0, 1);
    CMD(CCC_Integer, "r_dxr_vrr", &r_dxr_vrr, 0, 1);
    CMD(CCC_Integer, "r_dxr_tearing", &r_dxr_tearing, 0, 1);

    ApplySettings();
}

void dx12ConsoleVars::ApplySettings()
{
    if (!m_Initialized)
        return;

    // Renderer selection
    DX12_RENDERER_TYPE rendererType = (DX12_RENDERER_TYPE)r_dxr_renderer;
    RendererSelector12.SelectRenderer(rendererType);

    // DXR quality
    DX12_DXR_QUALITY quality = (DX12_DXR_QUALITY)r_dxr_quality;
    RendererSelector12.SetDXRQuality(quality);

    // Feature toggles
    RendererSelector12.EnableGI(r_dxr_gi != 0);
    RendererSelector12.EnableShadows(r_dxr_shadows != 0);
    RendererSelector12.EnableReflections(r_dxr_reflections != 0);
    RendererSelector12.EnablePostProcess(r_dxr_postprocess != 0);

    // GI settings
    GI12.SetHemisphereSamples(r_dxr_gi_samples);
    GI12.SetMaxRayDistance(r_dxr_gi_distance);
    GI12.SetBounceCount(r_dxr_gi_bounces);
    GI12.SetBounceAttenuation(r_dxr_gi_attenuation);

    // Shadow settings
    Shadows12.SetSamplesPerPixel(r_dxr_shadow_samples);
    Shadows12.SetLightAreaSize(r_dxr_shadow_area);
    Shadows12.SetShadowOpacity(r_dxr_shadow_opacity);

    // Reflection settings
    Reflections12.SetSamplesPerPixel(r_dxr_reflection_samples);
    Reflections12.SetMaxBounces(r_dxr_reflection_bounces);
    Reflections12.SetMaxRayDistance(r_dxr_reflection_distance);
    Reflections12.SetMinRoughness(r_dxr_reflection_roughness);

    // Hybrid render descriptor
    DX12_HYBRID_RENDER_DESC desc;
    desc.RenderScale = r_dxr_render_scale;
    desc.EnableUpscaling = (r_dxr_upscaling != 0);
    HybridRenderer12.SetRenderDesc(desc);

    Msg("*     DX12 settings applied");
}

dx12ConsoleVars DX12CV;

#endif // USE_DX12
