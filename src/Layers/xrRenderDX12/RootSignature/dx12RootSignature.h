#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

enum DX12_ROOT_SIGNATURE_TYPE
{
    RST_GRAPHICS_DEFAULT,
    RST_GRAPHICS_LIT,
    RST_GRAPHICS_SIMPLE,
    RST_GRAPHICS_DECAL,
    RST_GRAPHICS_EMISSIVE,
    RST_COMPUTE_HDAO,
    RST_COMPUTE_GI,
    RST_COMPUTE_POSTPROCESS,
    RST_RAYTRACING,
    RST_COUNT
};

struct DX12_ROOT_SIGNATURE_DESC
{
    DX12_ROOT_SIGNATURE_TYPE Type;
    UINT NumCBVs;
    UINT NumSRVs;
    UINT NumUAVs;
    UINT NumSamplers;
    bool bVolatileDescriptors;
    bool bUAVsAllDescriptors;
};

class dx12RootSignature
{
public:
    dx12RootSignature();
    ~dx12RootSignature();

    void Init();

    ID3D12RootSignature* GetRootSignature(DX12_ROOT_SIGNATURE_TYPE Type);
    ID3D12RootSignature* CreateCustomRootSignature(const DX12_ROOT_SIGNATURE_DESC& Desc);

    static void BuildDeferredRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& RootSigDesc);
    static void BuildComputeRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& RootSigDesc);
    static void BuildRayTracingRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& RootSigDesc);

private:
    HRESULT CreateRootSignature(const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& RootSigDesc,
                                ComPtr<ID3D12RootSignature>& RootSig);

    ComPtr<ID3D12RootSignature> m_rootSignatures[RST_COUNT];
    xr_map<u64, ComPtr<ID3D12RootSignature>> m_customRootSignatures;

    static const UINT MaxCBVs = 16;
    static const UINT MaxSRVs = 64;
    static const UINT MaxUAVs = 16;
    static const UINT MaxSamplers = 16;
};

extern dx12RootSignature RootSignature12;

#endif // USE_DX12
