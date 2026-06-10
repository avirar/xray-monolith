#pragma once

#ifdef USE_DX12

#include "../DX12CommonTypes.h"

enum DX12_POSTPROCESS_PASS
{
    DPP_BLOOM_EXTRACT,
    DPP_BLOOM_BLUR_H,
    DPP_BLOOM_BLUR_V,
    DPP_SSAO_BLUR,
    DPP_SSS,
    DPP_SUNSHAFTS,
    DPP_TAA,
    DPP_FINAL_COMBINE,
    DPP_COUNT
};

struct DX12_POSTPROCESS_DESC
{
    u32 Width;
    u32 Height;
    DXGI_FORMAT GenericFormat;
    DXGI_FORMAT BloomFormat;
    DXGI_FORMAT DepthFormat;
};

class dx12PostProcessPipeline
{
public:
    dx12PostProcessPipeline();
    ~dx12PostProcessPipeline();

    void Init(const DX12_POSTPROCESS_DESC& Desc);
    void Destroy();

    void Resize(u32 Width, u32 Height);

    void BeginPass(DX12_POSTPROCESS_PASS Pass);
    void EndPass(DX12_POSTPROCESS_PASS Pass);

    ID3D12Resource* GetGenericBuffer0() const { return m_pGenericBuffer0.Get(); }
    ID3D12Resource* GetGenericBuffer1() const { return m_pGenericBuffer1.Get(); }
    ID3D12Resource* GetBloomBuffer() const { return m_pBloomBuffer.Get(); }
    ID3D12Resource* GetBloomBuffer1() const { return m_pBloomBuffer1.Get(); }
    ID3D12Resource* GetSSAOBuffer() const { return m_pSSAOBuffer.Get(); }

    D3D12_CPU_DESCRIPTOR_HANDLE GetGenericRTV0(UINT FrameIndex) const { return m_GenericRTVs0[FrameIndex]; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetGenericRTV1(UINT FrameIndex) const { return m_GenericRTVs1[FrameIndex]; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetBloomRTV(UINT FrameIndex) const { return m_BloomRTVs[FrameIndex]; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetBloomRTV1(UINT FrameIndex) const { return m_BloomRTVs1[FrameIndex]; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSSAORTV(UINT FrameIndex) const { return m_SSAORTVs[FrameIndex]; }

    void TransitionToShaderResource(DX12_POSTPROCESS_PASS Pass);
    void TransitionToRenderTarget(DX12_POSTPROCESS_PASS Pass);

    void ResolveMSAA(ID3D12Resource* Src, ID3D12Resource* Dst, DXGI_FORMAT Format);

private:
    void CreateResources(const DX12_POSTPROCESS_DESC& Desc);
    void CreateRTVs();
    void CreateSRVs();

    ComPtr<ID3D12Resource> m_pGenericBuffer0;
    ComPtr<ID3D12Resource> m_pGenericBuffer1;
    ComPtr<ID3D12Resource> m_pBloomBuffer;
    ComPtr<ID3D12Resource> m_pBloomBuffer1;
    ComPtr<ID3D12Resource> m_pSSAOBuffer;

    ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;

    D3D12_CPU_DESCRIPTOR_HANDLE m_GenericRTVs0[3];
    D3D12_CPU_DESCRIPTOR_HANDLE m_GenericRTVs1[3];
    D3D12_CPU_DESCRIPTOR_HANDLE m_BloomRTVs[3];
    D3D12_CPU_DESCRIPTOR_HANDLE m_BloomRTVs1[3];
    D3D12_CPU_DESCRIPTOR_HANDLE m_SSAORTVs[3];

    DX12_POSTPROCESS_DESC m_desc;
};

extern dx12PostProcessPipeline PostProcessPipeline12;

#endif // USE_DX12
