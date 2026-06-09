#pragma once

#ifdef USE_DX12

#include "dx12HW.h"
#include "CommandManager/dx12CommandManager.h"
#include "StateManager/dx12StateManager.h"
#include "DescriptorManager/dx12DescriptorManager.h"
#include "RootSignature/dx12RootSignature.h"
#include "PSOBuilder/dx12PSOBuilder.h"
#include "SamplerManager/dx12SamplerManager.h"

class dx12Backend
{
public:
    dx12Backend();
    ~dx12Backend();

    void Init();
    void Destroy();

    void BeginFrame();
    void EndFrame();

    void SetRenderTarget(ID3D12Resource* RT, ID3D12Resource* DS);
    void ClearRenderTarget(ID3D12Resource* RT, const FLOAT Color[4]);
    void ClearDepthStencil(ID3D12Resource* DS, D3D12_CLEAR_FLAGS Flags, FLOAT Depth, UINT8 Stencil);

    void ResourceBarrier(ID3D12Resource* Resource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After);
    void ResourceBarriers(UINT Count, const D3D12_RESOURCE_BARRIER* Barriers);
    void BeginRenderPass();
    void EndRenderPass();

    void SetPipelineState(ID3D12PipelineState* PSO);
    void SetRootSignature(ID3D12RootSignature* RootSig);

    void SetSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers);
    void SetGraphicsSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers);
    void SetComputeSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers);

    ID3D12RootSignature* GetRootSignature(DX12_ROOT_SIGNATURE_TYPE Type);
    ID3D12PipelineState* BuildPSO(ID3DBlob* pVS, ID3DBlob* pPS, ID3D12RootSignature* pRootSignature,
                                  const D3D12_INPUT_ELEMENT_DESC* pInputLayout, UINT InputLayoutElementCount,
                                  DXGI_FORMAT RTFormat, DXGI_FORMAT DSFormat,
                                  D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    ID3D12PipelineState* BuildComputePSO(ID3DBlob* pCS, ID3D12RootSignature* pRootSignature);

    void SetVertexBuffer(ID3D12Resource* VB, u32 Stride, u32 Offset = 0);
    void SetIndexBuffer(ID3D12Resource* IB, DXGI_FORMAT Format = DXGI_FORMAT_R16_UINT, u32 Offset = 0);
    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology);

    void DrawIndexedInstanced(u32 IndexCountPerInstance, u32 InstanceCount, u32 StartIndexLocation,
                              INT BaseVertexLocation, u32 StartInstanceLocation);
    void DrawInstanced(u32 VertexCountPerInstance, u32 InstanceCount, u32 StartVertexLocation,
                       u32 StartInstanceLocation);
    void Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);

    void SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* DescriptorHeaps);
    void SetGraphicsRootSignature(ID3D12RootSignature* RootSignature);
    void SetComputeRootSignature(ID3D12RootSignature* RootSignature);
    void SetGraphicsRootDescriptorTable(u32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
    void SetComputeRootDescriptorTable(u32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
    void SetGraphicsRootConstantBufferView(u32 RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
    void SetComputeRootConstantBufferView(u32 RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);

    void CopyResource(ID3D12Resource* Dst, ID3D12Resource* Src);
    void CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* Dst, u64 DstX, u64 DstY, u64 DstZ,
                           const D3D12_TEXTURE_COPY_LOCATION* Src, const D3D12_BOX* SrcBox);
    void UpdateSubresource(ID3D12Resource* Dst, UINT DstSubresource, const D3D12_BOX* DstBox,
                           const void* SrcData, UINT SrcRowPitch, UINT SrcSlicePitch);
    void ResolveSubresource(ID3D12Resource* Dst, UINT DstSubresource, ID3D12Resource* Src,
                            UINT SrcSubresource, DXGI_FORMAT Format);
    void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const FLOAT Color[4],
                               UINT NumRects, const D3D12_RECT* Rects);
    void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags,
                               FLOAT Depth, UINT8 Stencil, UINT NumRects, const D3D12_RECT* Rects);

    void IASetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* Views);
    void IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* View);
    void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology);

    void RSSetViewports(UINT NumViewports, const D3D12_VIEWPORT* Viewports);
    void RSSetScissorRects(UINT NumRects, const D3D12_RECT* Rects);

    void OMSetBlendState(ID3D12BlendState* BlendState, const FLOAT BlendFactor[4], UINT SampleMask);
    void OMSetDepthStencilState(ID3D12DepthStencilState* DepthStencilState, UINT StencilRef);
    void OMSetRenderTargets(UINT NumRenderTargetViews, const D3D12_CPU_DESCRIPTOR_HANDLE* RenderTargetViews,
                            BOOL RTsSingleHandleToDescriptorHeap, UINT NumDepthStencilViews,
                            const D3D12_CPU_DESCRIPTOR_HANDLE* DepthStencilViews);

    void SetPipelineStatistics1(const D3D12_PIPELINE_STATISTICS_QUERY_DATA* Data);

    ID3D12GraphicsCommandList4* GetCommandList() const { return CommandManager12.GetGraphicsCommandList(); }
    ID3D12GraphicsCommandList4* GetComputeCommandList() const { return CommandManager12.GetComputeCommandList(); }

    void Flush();

private:
    ID3D12PipelineState* m_currentPSO;
    ID3D12RootSignature* m_currentRootSignature;
    D3D12_PRIMITIVE_TOPOLOGY m_currentTopology;

    D3D12_VERTEX_BUFFER_VIEW m_currentVBView;
    D3D12_INDEX_BUFFER_VIEW m_currentIBView;
    bool m_vbDirty;
    bool m_ibDirty;

    D3D12_VIEWPORT m_currentViewport;
    D3D12_RECT m_currentScissor;
    bool m_viewportDirty;
    bool m_scissorDirty;
};

extern dx12Backend Backend12;

#endif // USE_DX12
