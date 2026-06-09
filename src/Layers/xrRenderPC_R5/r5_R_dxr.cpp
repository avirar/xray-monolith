#include "r5_R_dxr.h"

#ifdef USE_DX12

#include "../xrRenderDX12/dx12HW.h"
#include "../xrRenderDX12/Raytracing/dx12Scene.h"
#include "../xrRenderDX12/Raytracing/dx12TLAS.h"
#include "../xrRenderDX12/GI/dx12GI.h"
#include "../xrRenderDX12/Reflections/dx12Reflections.h"
#include "../xrRenderDX12/Shadows/dx12Shadows.h"
#include "../xrRenderDX12/dx12RendererSelector.h"

namespace R5DXR
{
    bool IsDXREnabled()
    {
        return HW12.GetDXRTier() >= D3D12_RAYTRACING_TIER_1_1;
    }

    bool IsDXRSupported()
    {
        return HW12.GetDXRTier() >= D3D12_RAYTRACING_TIER_1_1;
    }

    void DXRUpdate()
    {
        if (!IsDXREnabled())
            return;

        // Update scene acceleration structures
        Scene12.Update();

        // Update GI
        GI12.UpdateGI();

        // Update reflections
        Reflections12.UpdateReflections();

        // Update shadows
        Shadows12.UpdateShadows();
    }

    void DXRClean()
    {
        if (!IsDXREnabled())
            return;

        // Clean up DXR resources
        Scene12.Clean();
    }

    void DXRBuildScene()
    {
        if (!IsDXREnabled())
            return;

        Scene12.Build();
    }

    void DXRUpdateInstances()
    {
        if (!IsDXREnabled())
            return;

        Scene12.UpdateInstances();
    }

    void DXRGIUpdate()
    {
        if (!IsDXREnabled())
            return;

        GI12.UpdateGI();
    }

    void DXRGIEnable(bool Enable)
    {
        GI12.Enable(Enable);
    }

    void DXRReflectionsUpdate()
    {
        if (!IsDXREnabled())
            return;

        Reflections12.UpdateReflections();
    }

    void DXRReflectionsEnable(bool Enable)
    {
        Reflections12.Enable(Enable);
    }

    void DXRShadowsUpdate()
    {
        if (!IsDXREnabled())
            return;

        Shadows12.UpdateShadows();
    }

    void DXRShadowsEnable(bool Enable)
    {
        Shadows12.Enable(Enable);
    }

    void SetDXRQuality(u32 Quality)
    {
        DX12_DXR_QUALITY dxrQuality = (DX12_DXR_QUALITY)Quality;
        RendererSelector12.SetDXRQuality(dxrQuality);
    }

    u32 GetDXRQuality()
    {
        return (u32)RendererSelector12.GetDXRQuality();
    }
}

#endif // USE_DX12
