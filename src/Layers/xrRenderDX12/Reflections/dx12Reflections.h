#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12ReflectionResources.h"
#include "dx12ReflectionPipeline.h"
#include "dx12ConstantBuffer.h"

struct DX12_REFLECTION_FRAME_DATA
{
    Fmatrix ViewProjection;      // 64 bytes
    Fmatrix View;                // 64 bytes
    Fvector CameraPosition;      // 16 bytes
    float Padding1;              // 4 bytes

    u32 ScreenWidth;             // 4 bytes
    u32 ScreenHeight;            // 4 bytes
    u32 SamplesPerPixel;         // 4 bytes
    float MaxRayDistance;        // 4 bytes

    float RayBias;               // 4 bytes
    float SkyContribution;       // 4 bytes
    u32 MaxBounces;              // 4 bytes
    float MinRoughness;          // 4 bytes

    u32 Padding2[4];            // 16 bytes (total: 256 bytes)
};

class dx12Reflections
{
public:
    dx12Reflections();
    ~dx12Reflections();

    void Init();

    // Per-frame reflection update
    void UpdateReflections();
    void BeginFrame();
    void EndFrame();

    // Reflection configuration
    void SetSamplesPerPixel(u32 Samples);
    void SetMaxBounces(u32 Bounces);
    void SetMaxRayDistance(float Distance);
    void SetMinRoughness(float Roughness);
    void SetSkyContribution(float Contribution);

    // Enable/disable reflections
    void Enable(bool Enable);
    bool IsEnabled() const { return m_Enabled; }

    // Get reflection output for lighting pass
    D3D12_CPU_DESCRIPTOR_HANDLE GetReflectionSRV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetRoughnessMaskSRV() const;

    // Resource management
    void Resize(u32 Width, u32 Height);

private:
    void CreateConstantBuffer();
    void UpdateConstantBuffer();
    void DispatchReflections();
    void ApplyResourceBarriers();

    bool m_Enabled;
    bool m_Initialized;

    u32 m_Width;
    u32 m_Height;

    dx12ConstantBuffer<DX12_REFLECTION_FRAME_DATA> m_ConstantBuffer;
    DX12_REFLECTION_FRAME_DATA m_FrameData;

    // Reflection quality settings
    u32 m_SamplesPerPixel;
    u32 m_MaxBounces;
    float m_MaxRayDistance;
    float m_MinRoughness;
    float m_SkyContribution;
};

extern dx12Reflections Reflections12;

#endif // USE_DX12
