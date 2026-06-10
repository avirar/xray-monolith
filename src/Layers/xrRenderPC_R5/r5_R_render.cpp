#include "stdafx.h"
#include "r5_R_render.h"

#if 0 // USE_DX12 - disabled until pipeline implementations are complete

#include "../xrRenderDX12/dx12HW.h"
#include "../xrRenderDX12/Hybrid/dx12HybridRenderer.h"
#include "../xrRenderDX12/RenderPipeline/dx12RenderPipeline.h"
#include "../xrRenderDX12/RenderPipeline/dx12LightingPipeline.h"
#include "../xrRenderDX12/RenderPipeline/dx12PostProcessPipeline.h"

namespace R5Render
{
    void RenderFrame()
    {
        HybridRenderer12.RenderFrame();
    }

    void RenderGBuffer()
    {
        RenderPipeline12.RenderGBuffer();
    }

    void RenderLighting()
    {
        LightingPipeline12.RenderLighting();
    }

    void RenderPostProcess()
    {
        PostProcessPipeline12.RenderPostProcess();
    }

    void BeginFrame()
    {
        CommandManager12.BeginFrame();
    }

    void EndFrame()
    {
        CommandManager12.EndFrame();
    }

    u32 GetDrawCallCount()
    {
        return 0; // TODO: Track draw calls
    }

    u32 GetTriangleCount()
    {
        return 0; // TODO: Track triangles
    }
}

#endif
