#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

// DX12 console variables
class dx12ConsoleVars
{
public:
    dx12ConsoleVars();
    ~dx12ConsoleVars();

    void Init();

    // Renderer selection
    int r_dxr_renderer;            // 0=auto, 1=dx12, 2=dx11, 3=dx10, 4=dx9

    // DXR quality
    int r_dxr_quality;             // 0=low, 1=medium, 2=high, 3=ultra

    // Feature toggles
    int r_dxr_gi;                  // 0=off, 1=on
    int r_dxr_shadows;             // 0=off, 1=on
    int r_dxr_reflections;         // 0=off, 1=on
    int r_dxr_postprocess;         // 0=off, 1=on

    // GI settings
    int r_dxr_gi_samples;          // 1-16 hemisphere samples
    float r_dxr_gi_distance;       // max ray distance
    int r_dxr_gi_bounces;          // 0-2 bounce count
    float r_dxr_gi_attenuation;    // energy loss per bounce

    // Shadow settings
    int r_dxr_shadow_samples;      // 1-16 samples per pixel
    float r_dxr_shadow_area;       // area light size
    float r_dxr_shadow_opacity;    // shadow darkness

    // Reflection settings
    int r_dxr_reflection_samples;  // 1-8 samples per pixel
    int r_dxr_reflection_bounces;  // 1-4 max bounces
    float r_dxr_reflection_distance; // max ray distance
    float r_dxr_reflection_roughness; // min roughness threshold

    // Resolution scaling
    float r_dxr_render_scale;      // 0.5-1.0 render scale
    int r_dxr_upscaling;           // 0=off, 1=on

    // HDR
    int r_dxr_hdr10;               // 0=off, 1=on

    // VRR
    int r_dxr_vrr;                 // 0=off, 1=on
    int r_dxr_tearing;             // 0=off, 1=on

    // Apply console variables to engine
    void ApplySettings();

private:
    bool m_Initialized;
};

extern dx12ConsoleVars DX12CV;

#endif // USE_DX12
