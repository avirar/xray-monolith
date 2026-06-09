#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

enum DX12_LIGHT_TYPE
{
    DLT_DIRECTIONAL,
    DLT_POINT,
    DLT_SPOT,
    DLT_OMNI
};

struct DX12_LIGHT_DATA
{
    Fvector Position;
    Fvector Direction;
    Fvector Color;
    float Intensity;
    float Range;
    float InnerConeAngle;
    float OuterConeAngle;
    float Attenuation0;
    float Attenuation1;
    float Attenuation2;
    DX12_LIGHT_TYPE Type;
};

struct DX12_ACCUMULATOR_DESC
{
    u32 Width;
    u32 Height;
    DXGI_FORMAT AccumulatorFormat;
    DXGI_FORMAT DepthFormat;
    UINT SampleCount;
};

class dx12LightingPipeline
{
public:
    dx12LightingPipeline();
    ~dx12LightingPipeline();

    void Init(const DX12_ACCUMULATOR_DESC& Desc);
    void Destroy();

    void Resize(u32 Width, u32 Height);

    void BeginAccumulator();
    void EndAccumulator();

    void ClearAccumulator(const FLOAT ClearColor[4]);

    ID3D12Resource* GetAccumulatorBuffer() const { return m_pAccumulatorBuffer.Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAccumulatorRTV(UINT FrameIndex) const { return m_AccumulatorRTVs[FrameIndex]; }

    void TransitionAccumulatorToShaderResource();
    void TransitionAccumulatorToRenderTarget();

    void AddLight(const DX12_LIGHT_DATA& Light);
    UINT GetLightCount() const { return m_lightCount; }
    const DX12_LIGHT_DATA* GetLights() const { return m_lights; }

private:
    void CreateResources(const DX12_ACCUMULATOR_DESC& Desc);
    void CreateRTVs();
    void CreateSRV();

    ComPtr<ID3D12Resource> m_pAccumulatorBuffer;
    ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;

    D3D12_CPU_DESCRIPTOR_HANDLE m_AccumulatorRTVs[3];

    D3D12_SHADER_RESOURCE_VIEW_DESC m_AccumulatorSRVDesc;

    DX12_LIGHT_DATA m_lights[512];
    UINT m_lightCount;

    DX12_ACCUMULATOR_DESC m_desc;
};

extern dx12LightingPipeline LightingPipeline12;

#endif // USE_DX12
