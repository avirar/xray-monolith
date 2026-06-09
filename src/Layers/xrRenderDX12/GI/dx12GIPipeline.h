#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12RayTracingShaders.h"
#include "dx12RayTracingPipeline.h"

struct DX12_GI_PIPELINE_DESC
{
    // GI quality settings
    u32 HemisphereSamples;      // Number of hemisphere samples per pixel (default: 8)
    u32 MaxRayDistance;         // Maximum ray travel distance (default: 100)
    float RayBias;              // Self-intersection avoidance bias (default: 0.001)
    float SkyContribution;      // Sky ambient contribution factor (default: 0.3)

    // Bounce settings
    u32 BounceCount;            // Number of bounces (0 = direct only, 1+ = indirect)
    float BounceAttenuation;    // Energy loss per bounce (default: 0.8)
};

class dx12GIPipeline
{
public:
    dx12GIPipeline();
    ~dx12GIPipeline();

    void Init();

    // Create GI ray tracing pipeline
    ID3D12StateObject* CreateGIPipeline(const DX12_GI_PIPELINE_DESC& Desc);

    // Compile GI shaders
    bool CompileRayGenGI();
    bool CompileClosestHitGI();
    bool CompileMissGI();

    // Build GI SBT
    void BuildGISBT();

    // Get SBT info for dispatch
    ID3D12Resource* GetGISBTResource() const { return m_pSBTResource; }
    D3D12_GPU_VIRTUAL_ADDRESS GetRayGenAddress() const { return m_RayGenAddress; }
    D3D12_GPU_VIRTUAL_ADDRESS GetHitGroupAddress() const { return m_HitGroupAddress; }
    D3D12_GPU_VIRTUAL_ADDRESS GetMissAddress() const { return m_MissAddress; }
    UINT GetShaderTableStride() const { return m_ShaderTableStride; }

    const DX12_GI_PIPELINE_DESC& GetDesc() const { return m_Desc; }

private:
    bool CompileGIShader(
        const char* ShaderName,
        const char* EntryPoint,
        const char* Target,
        ID3DBlob*& ShaderBlob);

    ID3D12StateObject* m_pGIPipeline;

    ID3D12Resource* m_pSBTResource;
    D3D12_GPU_VIRTUAL_ADDRESS m_RayGenAddress;
    D3D12_GPU_VIRTUAL_ADDRESS m_HitGroupAddress;
    D3D12_GPU_VIRTUAL_ADDRESS m_MissAddress;
    UINT m_ShaderTableStride;

    DX12_GI_PIPELINE_DESC m_Desc;

    ID3DBlob* m_pRayGenBlob;
    ID3DBlob* m_pClosestHitBlob;
    ID3DBlob* m_pMissBlob;
};

extern dx12GIPipeline GIPipeline12;

#endif // USE_DX12
