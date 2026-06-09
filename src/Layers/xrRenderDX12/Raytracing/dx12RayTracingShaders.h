#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

enum DX12_RAYTRACING_SHADER_TYPE
{
    DRTS_RAYGEN,
    DRTS_CLOSEST_HIT,
    DRTS_ANY_HIT,
    DRTS_MISS,
    DRTS_INTERSECTION,
    DRTS_CALLABLE
};

struct DX12_RAYTRACING_SHADER_DESC
{
    LPCSTR Name;
    LPCSTR Source;
    UINT SourceLength;
    LPCSTR Entry;
    DX12_RAYTRACING_SHADER_TYPE Type;
};

struct DX12_HIT_GROUP_DESC
{
    LPCSTR Name;
    ID3DBlob* pClosestHitShader;
    ID3DBlob* pAnyHitShader;
    ID3DBlob* pIntersectionShader;
    LPCSTR ClosestHitEntry;
    LPCSTR AnyHitEntry;
    LPCSTR IntersectionEntry;
    UINT HitGroupIndex;
};

class dx12RayTracingShaders
{
public:
    dx12RayTracingShaders();
    ~dx12RayTracingShaders();

    void Init();

    ID3DBlob* CompileRayGenShader(LPCSTR Name, LPCSTR Entry = "main");
    ID3DBlob* CompileClosestHitShader(LPCSTR Name, LPCSTR Entry = "main");
    ID3DBlob* CompileAnyHitShader(LPCSTR Name, LPCSTR Entry = "main");
    ID3DBlob* CompileMissShader(LPCSTR Name, LPCSTR Entry = "main");

    void AddHitGroup(const DX12_HIT_GROUP_DESC& Desc);
    UINT GetHitGroupCount() const { return m_hitGroupCount; }
    const DX12_HIT_GROUP_DESC* GetHitGroup(UINT Index) const { return &m_hitGroups[Index]; }

    ID3D12Resource* GetLocalRootSignatureBuffer() const { return m_pLocalRootSignatureBuffer.Get(); }
    D3D12_GPU_VIRTUAL_ADDRESS GetLocalRootSignatureGPUVirtualAddress() const { return m_localRootSignatureGPUAddress; }

    UINT GetShaderTableStride() const { return m_shaderTableStride; }

private:
    HRESULT CreateLocalRootSignature();
    ID3DBlob* CompileShader(const DX12_RAYTRACING_SHADER_DESC& Desc);

    ComPtr<ID3DBlob> m_rayGenShaders[64];
    ComPtr<ID3DBlob> m_missShaders[64];
    ComPtr<ID3DBlob> m_localRootSignature;

    ComPtr<ID3D12Resource> m_pLocalRootSignatureBuffer;
    D3D12_GPU_VIRTUAL_ADDRESS m_localRootSignatureGPUAddress;

    DX12_HIT_GROUP_DESC m_hitGroups[256];
    UINT m_hitGroupCount;

    UINT m_shaderTableStride;

    static const UINT MaxShaders = 64;
    static const UINT MaxHitGroups = 256;
};

extern dx12RayTracingShaders RayTracingShaders12;

#endif // USE_DX12
