#include "light_GI_dx12.h"

#ifdef USE_DX12

#include "../xrRenderDX12/dx12HW.h"
#include "../xrRenderDX12/GI/dx12GI.h"
#include "../xrRenderDX12/dx12RendererSelector.h"

namespace R5GI
{
    void GenerateGI()
    {
        if (RendererSelector12.CheckDXRSupport())
        {
            GI12.UpdateGI();
        }
        else
        {
            GenerateCPU_GI();
        }
    }

    void UpdateGI()
    {
        if (RendererSelector12.CheckDXRSupport())
        {
            GI12.UpdateGI();
        }
    }

    void EnableGI(bool Enable)
    {
        GI12.Enable(Enable);
    }

    bool IsGIEnabled()
    {
        return GI12.IsEnabled();
    }

    void SetGISamples(u32 Samples)
    {
        GI12.SetHemisphereSamples(Samples);
    }

    void SetGIDistance(float Distance)
    {
        GI12.SetMaxRayDistance(Distance);
    }

    void SetGIBounces(u32 Bounces)
    {
        GI12.SetBounceCount(Bounces);
    }

    void SetGIAttenuation(float Attenuation)
    {
        GI12.SetBounceAttenuation(Attenuation);
    }

    void GenerateCPU_GI()
    {
        // Fallback to CPU-based GI (existing light::gi_generate())
        // This is a placeholder - the actual implementation would call
        // the existing CPU GI code from light_GI.cpp
        LOG("*WARN* R5GI: Using CPU GI fallback");
    }
}

#endif // USE_DX12
