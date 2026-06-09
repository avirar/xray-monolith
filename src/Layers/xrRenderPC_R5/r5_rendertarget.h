#pragma once

#ifdef USE_DX12

#include "../xrRender/HW.h"
#include "../xrRender/resourcemanager.h"

class dx12GIResources;
class dx12ReflectionResources;
class dx12Shadows;

class R5RenderTarget
{
public:
    R5RenderTarget();
    ~R5RenderTarget();

    // Initialize render targets
    void Init();
    void Destroy();

    // Resize render targets
    void Resize(u32 Width, u32 Height);

    // Get render target resources
    ID3D12Resource* GetGBufferPosition() const;
    ID3D12Resource* GetGBufferColor() const;
    ID3D12Resource* GetGBufferNormal() const;
    ID3D12Resource* GetGIResource() const;
    ID3D12Resource* GetShadowResource() const;
    ID3D12Resource* GetReflectionResource() const;

    // Get descriptor handles
    D3D12_CPU_DESCRIPTOR_HANDLE GetGBufferPositionSRV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetGBufferColorSRV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetGBufferNormalSRV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetGISRV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetShadowSRV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetReflectionSRV() const;

    // Resource state management
    void TransitionToShaderResource();
    void TransitionToRenderTarget();
    void TransitionToUnorderedAccess();

private:
    u32 m_Width;
    u32 m_Height;
    bool m_Initialized;
};

extern R5RenderTarget R5RT;

#endif // USE_DX12
