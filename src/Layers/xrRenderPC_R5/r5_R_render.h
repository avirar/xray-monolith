#pragma once

#ifdef USE_DX12

#include "../xrRender/HW.h"

class R5;

namespace R5Render
{
    // Main render entry
    void RenderFrame();

    // Individual passes
    void RenderGBuffer();
    void RenderLighting();
    void RenderPostProcess();

    // Render state
    void BeginFrame();
    void EndFrame();

    // Statistics
    u32 GetDrawCallCount();
    u32 GetTriangleCount();
}

#endif // USE_DX12
