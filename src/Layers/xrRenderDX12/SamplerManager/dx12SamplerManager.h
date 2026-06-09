#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

enum DX12_SAMPLER_TYPE
{
    DST_LINEAR_WRAP,
    DST_POINT_WRAP,
    DST_LINEAR_CLAMP,
    DST_POINT_CLAMP,
    DST_ANISOTROPIC_WRAP,
    DST_ANISOTROPIC_CLAMP,
    DST_COMPARISON_LINEAR,
    DST_COMPARISON_POINT,
    DST_LINEAR_MIRROR,
    DST_POINT_MIRROR,
    DST_COUNT
};

struct DX12_SAMPLER_DESC
{
    D3D12_FILTER Filter;
    D3D12_TEXTURE_ADDRESS_MODE AddressU;
    D3D12_TEXTURE_ADDRESS_MODE AddressV;
    D3D12_TEXTURE_ADDRESS_MODE AddressW;
    FLOAT MipLODBias;
    UINT MaxAnisotropy;
    D3D12_COMPARISON_FUNC ComparisonFunc;
    FLOAT BorderColor[4];
    UINT MinLOD;
    UINT MaxLOD;
};

class dx12SamplerManager
{
public:
    dx12SamplerManager();
    ~dx12SamplerManager();

    void Init();

    ID3D12SamplerState* GetSampler(DX12_SAMPLER_TYPE Type);
    ID3D12SamplerState* CreateSampler(const DX12_SAMPLER_DESC& Desc);

    D3D12_CPU_DESCRIPTOR_HANDLE AllocateSamplerDescriptor();
    void CreateSamplerDescriptor(ID3D12SamplerState* Sampler, D3D12_CPU_DESCRIPTOR_HANDLE Handle);

    ID3D12DescriptorHeap* GetSamplerHeap() const { return m_pSamplerHeap.Get(); }

    void SetSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers);
    void SetComputeSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers);

private:
    ComPtr<ID3D12SamplerState> m_staticSamplers[DST_COUNT];
    ComPtr<ID3D12DescriptorHeap> m_pSamplerHeap;

    D3D12_CPU_DESCRIPTOR_HANDLE m_samplerHeapStart;
    UINT m_samplerOffset;
    UINT m_samplerCapacity;

    xr_map<u64, ComPtr<ID3D12SamplerState>> m_customSamplers;

    static const UINT InitialSamplerHeapSize = 64;
};

extern dx12SamplerManager SamplerManager12;

#endif // USE_DX12
