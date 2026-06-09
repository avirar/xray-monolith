#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12ShadowPipeline.h"
#include "dx12ConstantBuffer.h"
#include "dx12Texture.h"

struct DX12_SHADOW_FRAME_DATA
{
    Fmatrix ViewProjection;      // 64 bytes
    Fvector LightPosition;       // 16 bytes
    float LightType;             // 4 bytes (0=directional, 1=point, 2=spot)

    Fvector LightDirection;      // 16 bytes
    float LightRange;            // 4 bytes

    u32 ScreenWidth;             // 4 bytes
    u32 ScreenHeight;            // 4 bytes
    u32 SamplesPerPixel;         // 4 bytes
    float LightAreaSize;         // 4 bytes

    float RayBias;               // 4 bytes
    float ShadowOpacity;         // 4 bytes
    float MaxRayDistance;        // 4 bytes
    float Padding1;              // 4 bytes

    u32 Padding2[4];            // 16 bytes (total: 256 bytes)
};

class dx12Shadows
{
public:
    dx12Shadows();
    ~dx12Shadows();

    void Init();

    // Per-frame shadow update
    void UpdateShadows();
    void BeginFrame();
    void EndFrame();

    // Shadow configuration
    void SetSamplesPerPixel(u32 Samples);
    void SetLightAreaSize(float Size);
    void SetMaxRayDistance(float Distance);
    void SetShadowOpacity(float Opacity);

    // Enable/disable shadows
    void Enable(bool Enable);
    bool IsEnabled() const { return m_Enabled; }

    // Get shadow output for lighting pass
    D3D12_CPU_DESCRIPTOR_HANDLE GetShadowSRV() const;

    // Resource management
    void Resize(u32 Width, u32 Height);

private:
    void CreateConstantBuffer();
    void CreateShadowTexture(u32 Width, u32 Height);
    void UpdateConstantBuffer();
    void DispatchShadows();
    void ApplyResourceBarriers();

    bool m_Enabled;
    bool m_Initialized;

    u32 m_Width;
    u32 m_Height;

    dx12ConstantBuffer<DX12_SHADOW_FRAME_DATA> m_ConstantBuffer;
    DX12_SHADOW_FRAME_DATA m_FrameData;

    // Shadow output texture
    ID3D12Resource* m_pShadowResource;
    D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowSRV;
    D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowUAV;

    // Shadow quality settings
    u32 m_SamplesPerPixel;
    float m_LightAreaSize;
    float m_MaxRayDistance;
    float m_ShadowOpacity;
};

extern dx12Shadows Shadows12;

#endif // USE_DX12
