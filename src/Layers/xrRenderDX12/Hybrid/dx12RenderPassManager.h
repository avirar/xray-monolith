#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12HybridRenderer.h"

class dx12RenderPassManager
{
public:
    dx12RenderPassManager();
    ~dx12RenderPassManager();

    void Init();

    // Pass ordering
    void AddPass(DX12_RENDER_PASS Pass);
    void RemovePass(DX12_RENDER_PASS Pass);
    void ClearPasses();

    // Pass execution
    void ExecuteAllPasses();
    bool HasPass(DX12_RENDER_PASS Pass) const;

    // Pass count
    u32 GetPassCount() const { return m_Passes.size(); }

    // Get pass at index
    DX12_RENDER_PASS GetPass(u32 Index) const;

    // Render configuration
    void SetRenderResolution(u32 Width, u32 Height);
    u32 GetRenderWidth() const { return m_RenderWidth; }
    u32 GetRenderHeight() const { return m_RenderHeight; }

private:
    xr_vector<DX12_RENDER_PASS> m_Passes;

    u32 m_RenderWidth;
    u32 m_RenderHeight;

    bool m_Initialized;
};

extern dx12RenderPassManager RenderPassManager12;

#endif // USE_DX12
