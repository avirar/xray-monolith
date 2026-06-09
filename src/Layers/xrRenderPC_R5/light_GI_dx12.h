#pragma once

#ifdef USE_DX12

#include "../xrRender/HW.h"

class R5;

namespace R5GI
{
    // DXR GI generation
    void GenerateGI();
    void UpdateGI();

    // GI configuration
    void EnableGI(bool Enable);
    bool IsGIEnabled();

    // Quality settings
    void SetGISamples(u32 Samples);
    void SetGIDistance(float Distance);
    void SetGIBounces(u32 Bounces);
    void SetGIAttenuation(float Attenuation);

    // Fallback to CPU GI
    void GenerateCPU_GI();
}

#endif // USE_DX12
