#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12RayTracingShaders.h"

struct DX12_SHADOW_PIPELINE_DESC
{
    // Shadow quality
    u32 SamplesPerPixel;         // Samples per light for soft shadows (default: 4)
    float LightAreaSize;         // Area light size for soft penumbra (default: 0.1)
    float MaxRayDistance;        // Maximum ray travel distance (default: 100)

    // Bias
    float RayBias;               // Self-intersection avoidance bias (default: 0.001)
    float ShadowOpacity;         // Shadow darkness factor (default: 0.8)
};

class dx12ShadowPipeline
{
public:
    dx12ShadowPipeline();
    ~dx12ShadowPipeline();

    void Init();

    // Create shadow ray tracing pipeline
    ID3D12StateObject* CreateShadowPipeline(const DX12_SHADOW_PIPELINE_DESC& Desc);

    // Compile shadow shaders
    bool CompileRayGenShadow();
    bool CompileClosestHitShadow();
    bool CompileMissShadow();

    // Build shadow SBT
    void BuildShadowSBT();

    // Get SBT info for dispatch
    ID3D12Resource* GetSBTResource() const { return m_pSBTResource; }
    D3D12_GPU_VIRTUAL_ADDRESS GetRayGenAddress() const { return m_RayGenAddress; }
    D3D12_GPU_VIRTUAL_ADDRESS GetHitGroupAddress() const { return m_HitGroupAddress; }
    D3D12_GPU_VIRTUAL_ADDRESS GetMissAddress() const { return m_MissAddress; }
    UINT GetShaderTableStride() const { return m_ShaderTableStride; }

    const DX12_SHADOW_PIPELINE_DESC& GetDesc() const { return m_Desc; }

private:
    bool CompileShadowShader(
        const char* ShaderName,
        const char* EntryPoint,
        const char* Target,
        ID3DBlob*& ShaderBlob);

    ID3D12StateObject* m_pShadowPipeline;

    ID3D12Resource* m_pSBTResource;
    D3D12_GPU_VIRTUAL_ADDRESS m_RayGenAddress;
    D3D12_GPU_VIRTUAL_ADDRESS m_HitGroupAddress;
    D3D12_GPU_VIRTUAL_ADDRESS m_MissAddress;
    UINT m_ShaderTableStride;

    DX12_SHADOW_PIPELINE_DESC m_Desc;

    ID3DBlob* m_pRayGenBlob;
    ID3DBlob* m_pClosestHitBlob;
    ID3DBlob* m_pMissBlob;
};

extern dx12ShadowPipeline ShadowPipeline12;

#endif // USE_DX12
