#pragma once

#ifdef USE_DX12

#include "../DX12CommonTypes.h"

struct DX12_GBUFFER_DESC
{
    u32 Width;
    u32 Height;
    DXGI_FORMAT PositionFormat;
    DXGI_FORMAT ColorFormat;
    DXGI_FORMAT NormalFormat;
    DXGI_FORMAT RoughnessFormat;
    DXGI_FORMAT MotionVectorFormat;
    DXGI_FORMAT DepthFormat;
    UINT SampleCount;
};

class dx12RenderPipeline
{
public:
    dx12RenderPipeline();
    ~dx12RenderPipeline();

    void Init(const DX12_GBUFFER_DESC& Desc);
    void Destroy();

    void Resize(u32 Width, u32 Height);

    void BeginGBuffer();
    void EndGBuffer();

    void ClearGBuffer(const FLOAT ClearColor[4], FLOAT Depth, UINT8 Stencil);

    ID3D12Resource* GetPositionBuffer() const { return m_pPositionBuffer.Get(); }
    ID3D12Resource* GetColorBuffer() const { return m_pColorBuffer.Get(); }
    ID3D12Resource* GetNormalBuffer() const { return m_pNormalBuffer.Get(); }
    ID3D12Resource* GetRoughnessBuffer() const { return m_pRoughnessBuffer.Get(); }
    ID3D12Resource* GetMotionVectorBuffer() const { return m_pMotionVectorBuffer.Get(); }
    ID3D12Resource* GetDepthBuffer() const { return m_pDepthBuffer.Get(); }

    D3D12_CPU_DESCRIPTOR_HANDLE GetPositionRTV(UINT FrameIndex) const { return m_PositionRTVs[FrameIndex]; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetColorRTV(UINT FrameIndex) const { return m_ColorRTVs[FrameIndex]; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetNormalRTV(UINT FrameIndex) const { return m_NormalRTVs[FrameIndex]; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRoughnessRTV(UINT FrameIndex) const { return m_RoughnessRTVs[FrameIndex]; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetMotionVectorRTV(UINT FrameIndex) const { return m_MotionVectorRTVs[FrameIndex]; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthDSV() const { return m_DepthDSV; }

    void TransitionGBufferToShaderResource();
    void TransitionGBufferToRenderTarget();

private:
    void CreateResources(const DX12_GBUFFER_DESC& Desc);
    void CreateRTVs();
    void CreateSRVs();
    void CreateSRVDescriptors();

    ComPtr<ID3D12Resource> m_pPositionBuffer;
    ComPtr<ID3D12Resource> m_pColorBuffer;
    ComPtr<ID3D12Resource> m_pNormalBuffer;
    ComPtr<ID3D12Resource> m_pRoughnessBuffer;
    ComPtr<ID3D12Resource> m_pMotionVectorBuffer;
    ComPtr<ID3D12Resource> m_pDepthBuffer;

    ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;

    D3D12_CPU_DESCRIPTOR_HANDLE m_PositionRTVs[3];
    D3D12_CPU_DESCRIPTOR_HANDLE m_ColorRTVs[3];
    D3D12_CPU_DESCRIPTOR_HANDLE m_NormalRTVs[3];
    D3D12_CPU_DESCRIPTOR_HANDLE m_RoughnessRTVs[3];
    D3D12_CPU_DESCRIPTOR_HANDLE m_MotionVectorRTVs[3];
    D3D12_CPU_DESCRIPTOR_HANDLE m_DepthDSV;

    D3D12_SHADER_RESOURCE_VIEW_DESC m_PositionSRVDesc;
    D3D12_SHADER_RESOURCE_VIEW_DESC m_ColorSRVDesc;
    D3D12_SHADER_RESOURCE_VIEW_DESC m_NormalSRVDesc;
    D3D12_SHADER_RESOURCE_VIEW_DESC m_RoughnessSRVDesc;
    D3D12_SHADER_RESOURCE_VIEW_DESC m_MotionVectorSRVDesc;

    DX12_GBUFFER_DESC m_desc;
};

extern dx12RenderPipeline RenderPipeline12;

#endif // USE_DX12
