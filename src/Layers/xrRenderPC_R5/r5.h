#pragma once

#ifdef USE_DX12

#include "../xrRender/HW.h"
#include "../xrRender/R_Backend.h"

class dx12HybridRenderer;
class dx12RenderPassManager;
class dx12GI;
class dx12Reflections;
class dx12Shadows;

class R5 : public CBackend
{
public:
    R5();
    ~R5();

    virtual void Init();
    virtual void Destroy();
    virtual void Render();
    virtual void Present();

    // Renderer capabilities
    bool IsDXRSupported() const;
    bool IsDX12Supported() const;

    // Frame management
    void BeginFrame();
    void EndFrame();

    // Resource management
    void Resize(u32 Width, u32 Height);

    // Statistics
    u32 GetPassCount() const;
    float GetFrameTime() const;

private:
    bool m_Initialized;
    bool m_DXRSuported;
    bool m_DX12Supported;
};

extern R5* R5Instance;

#endif // USE_DX12
