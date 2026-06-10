#pragma once

#ifdef USE_DX12

#include "../DX12CommonTypes.h"
#include "../Raytracing/dx12RayTracingShaders.h"

struct DX12_REFLECTION_PIPELINE_DESC
{
    // Reflection quality
    u32 SamplesPerPixel;         // Samples per pixel for anti-aliasing (default: 4)
    u32 MaxBounces;              // Maximum reflection bounces (default: 2)
    float MinRoughness;          // Minimum roughness to skip ray tracing (default: 0.5)
    float MaxRayDistance;        // Maximum ray travel distance (default: 200)

    // Clamping
    float RayBias;               // Self-intersection avoidance bias (default: 0.001)
    float SkyContribution;       // Sky contribution when ray misses (default: 0.2)
};

class dx12ReflectionPipeline
{
public:
    dx12ReflectionPipeline();
    ~dx12ReflectionPipeline();

    void Init();

    // Create reflection ray tracing pipeline
    ID3D12StateObject* CreateReflectionPipeline(const DX12_REFLECTION_PIPELINE_DESC& Desc);

    // Compile reflection shaders
    bool CompileRayGenReflection();
    bool CompileClosestHitReflection();
    bool CompileMissReflection();

    // Build reflection SBT
    void BuildReflectionSBT();

    // Get SBT info for dispatch
    ID3D12Resource* GetSBTResource() const { return m_pSBTResource; }
    D3D12_GPU_VIRTUAL_ADDRESS GetRayGenAddress() const { return m_RayGenAddress; }
    D3D12_GPU_VIRTUAL_ADDRESS GetHitGroupAddress() const { return m_HitGroupAddress; }
    D3D12_GPU_VIRTUAL_ADDRESS GetMissAddress() const { return m_MissAddress; }
    UINT GetShaderTableStride() const { return m_ShaderTableStride; }

    const DX12_REFLECTION_PIPELINE_DESC& GetDesc() const { return m_Desc; }

private:
    bool CompileReflectionShader(
        const char* ShaderName,
        const char* EntryPoint,
        const char* Target,
        ID3DBlob*& ShaderBlob);

    ID3D12StateObject* m_pReflectionPipeline;

    ID3D12Resource* m_pSBTResource;
    D3D12_GPU_VIRTUAL_ADDRESS m_RayGenAddress;
    D3D12_GPU_VIRTUAL_ADDRESS m_HitGroupAddress;
    D3D12_GPU_VIRTUAL_ADDRESS m_MissAddress;
    UINT m_ShaderTableStride;

    DX12_REFLECTION_PIPELINE_DESC m_Desc;

    ID3DBlob* m_pRayGenBlob;
    ID3DBlob* m_pClosestHitBlob;
    ID3DBlob* m_pMissBlob;
};

extern dx12ReflectionPipeline ReflectionPipeline12;

#endif // USE_DX12
