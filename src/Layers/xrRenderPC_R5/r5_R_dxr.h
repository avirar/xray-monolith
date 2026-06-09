#pragma once

#ifdef USE_DX12

#include "../xrRender/HW.h"

class R5;

namespace R5DXR
{
    // DXR capability checks
    bool IsDXREnabled();
    bool IsDXRSupported();

    // Per-frame DXR updates
    void DXRUpdate();
    void DXRClean();

    // Scene management
    void DXRBuildScene();
    void DXRUpdateInstances();

    // GI
    void DXRGIUpdate();
    void DXRGIEnable(bool Enable);

    // Reflections
    void DXRReflectionsUpdate();
    void DXRReflectionsEnable(bool Enable);

    // Shadows
    void DXRShadowsUpdate();
    void DXRShadowsEnable(bool Enable);

    // Quality settings
    void SetDXRQuality(u32 Quality);
    u32 GetDXRQuality();
}

#endif // USE_DX12
