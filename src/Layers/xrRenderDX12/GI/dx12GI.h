#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12GIResources.h"
#include "dx12GIPipeline.h"
#include "dx12ConstantBuffer.h"

struct DX12_GI_FRAME_DATA
{
    Fmatrix ViewProjection;      // 64 bytes
    Fmatrix View;                // 64 bytes
    Fvector CameraPosition;      // 16 bytes
    float Padding1;              // 4 bytes

    u32 ScreenWidth;             // 4 bytes
    u32 ScreenHeight;            // 4 bytes
    u32 HemisphereSamples;       // 4 bytes
    float MaxRayDistance;        // 4 bytes

    float RayBias;               // 4 bytes
    float SkyContribution;       // 4 bytes
    u32 BounceCount;             // 4 bytes
    float BounceAttenuation;     // 4 bytes

    u32 Padding2[4];            // 16 bytes (total: 256 bytes)
};

class dx12GI
{
public:
    dx12GI();
    ~dx12GI();

    void Init();

    // Per-frame GI update
    void UpdateGI();
    void BeginFrame();
    void EndFrame();

    // GI configuration
    void SetHemisphereSamples(u32 Samples);
    void SetMaxRayDistance(float Distance);
    void SetBounceCount(u32 Count);
    void SetBounceAttenuation(float Attenuation);
    void SetSkyContribution(float Contribution);

    // Enable/disable GI
    void Enable(bool Enable);
    bool IsEnabled() const { return m_Enabled; }

    // Get GI output for lighting pass
    D3D12_CPU_DESCRIPTOR_HANDLE GetIrradianceSRV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetAOSRV() const;

    // Resource management
    void Resize(u32 Width, u32 Height);

private:
    void CreateConstantBuffer();
    void UpdateConstantBuffer();
    void DispatchGI();
    void ApplyResourceBarriers();

    bool m_Enabled;
    bool m_Initialized;

    u32 m_Width;
    u32 m_Height;

    dx12ConstantBuffer<DX12_GI_FRAME_DATA> m_ConstantBuffer;
    DX12_GI_FRAME_DATA m_FrameData;

    // GI quality settings
    u32 m_HemisphereSamples;
    float m_MaxRayDistance;
    u32 m_BounceCount;
    float m_BounceAttenuation;
    float m_SkyContribution;
};

extern dx12GI GI12;

#endif // USE_DX12
