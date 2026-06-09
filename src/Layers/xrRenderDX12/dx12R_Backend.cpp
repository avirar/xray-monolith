#include "dx12stdafx.h"
#include "dx12R_Backend.h"

#ifdef USE_DX12

dx12Backend::dx12Backend()
    : m_currentPSO(nullptr),
      m_currentRootSignature(nullptr),
      m_currentTopology(D3D12_PRIMITIVE_TOPOLOGY_UNDEFINED),
      m_vbDirty(true),
      m_ibDirty(true),
      m_viewportDirty(true),
      m_scissorDirty(true)
{
    ZeroMemory(&m_currentVBView, sizeof(m_currentVBView));
    ZeroMemory(&m_currentIBView, sizeof(m_currentIBView));
    ZeroMemory(&m_currentViewport, sizeof(m_currentViewport));
    ZeroMemory(&m_currentScissor, sizeof(m_currentScissor));
}

dx12Backend::~dx12Backend()
{
}

void dx12Backend::Init()
{
    CommandManager12.Init(HW12.m_frameCount);
    DescriptorManager12.Init();
}

void dx12Backend::Destroy()
{
    CommandManager12.WaitForGPU();
    CommandManager12.Destroy();
}

void dx12Backend::BeginFrame()
{
    CommandManager12.BeginFrame();
    DescriptorManager12.ResetFrame();
}

void dx12Backend::EndFrame()
{
    CommandManager12.EndFrame();
}

void dx12Backend::SetRenderTarget(ID3D12Resource* RT, ID3D12Resource* DS)
{
    UINT frameIndex = CommandManager12.GetFrameIndex();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = HW12.m_RtvHandles[frameIndex];
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = HW12.m_DsvHandle;

    ID3D12DescriptorHeap* pHeaps[] = {
        HW12.m_pRtvHeap.Get(),
        HW12.m_pDsvHeap.Get(),
        DescriptorManager12.GetSRVUAVHeap(),
        DescriptorManager12.GetCBVHeap()
    };

    GetCommandList()->SetDescriptorHeaps(_countof(pHeaps), pHeaps);
    GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    D3D12_VIEWPORT vp = {};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = static_cast<float>(HW12.m_width);
    vp.Height = static_cast<float>(HW12.m_height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    GetCommandList()->RSSetViewports(1, &vp);

    D3D12_RECT sc = {};
    sc.left = 0;
    sc.top = 0;
    sc.right = HW12.m_width;
    sc.bottom = HW12.m_height;
    GetCommandList()->RSSetScissorRects(1, &sc);
}

void dx12Backend::ClearRenderTarget(ID3D12Resource* RT, const FLOAT Color[4])
{
    UINT frameIndex = CommandManager12.GetFrameIndex();
    D3D12_CPU_DESCRIPTOR_HANDLE handle = HW12.m_RtvHandles[frameIndex];
    GetCommandList()->ClearRenderTargetView(handle, Color, 0, nullptr);
}

void dx12Backend::ClearDepthStencil(ID3D12Resource* DS, D3D12_CLEAR_FLAGS Flags, FLOAT Depth, UINT8 Stencil)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = HW12.m_DsvHandle;
    GetCommandList()->ClearDepthStencilView(handle, Flags, Depth, Stencil, 0, nullptr);
}

void dx12Backend::ResourceBarrier(ID3D12Resource* Resource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
{
    GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(Resource, Before, After));
}

void dx12Backend::ResourceBarriers(UINT Count, const D3D12_RESOURCE_BARRIER* Barriers)
{
    GetCommandList()->ResourceBarrier(Count, Barriers);
}

void dx12Backend::BeginRenderPass()
{
    BeginFrame();
    SetRenderTarget(HW12.m_pBackBuffer[CommandManager12.GetFrameIndex()].Get(),
                    HW12.m_pDepthStencil.Get());
}

void dx12Backend::EndRenderPass()
{
    UINT frameIndex = CommandManager12.GetFrameIndex();

    ResourceBarrier(HW12.m_pBackBuffer[frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    CommandManager12.EndFrame();

    HW12.Present(HW12.selectPresentInterval(), 0);

    UINT64 fenceToWait = HW12.m_FenceValues[(frameIndex + 1) % HW12.m_frameCount];
    if (HW12.m_pFence->GetCompletedValue() < fenceToWait)
    {
        HW12.m_pFence->SetEventOnCompletion(fenceToWait, HW12.m_fenceEvent);
        WaitForSingleObject(HW12.m_fenceEvent, INFINITE);
    }
}

void dx12Backend::SetPipelineState(ID3D12PipelineState* PSO)
{
    if (m_currentPSO != PSO)
    {
        m_currentPSO = PSO;
        GetCommandList()->SetPipelineState(PSO);
    }
}

void dx12Backend::SetRootSignature(ID3D12RootSignature* RootSig)
{
    if (m_currentRootSignature != RootSig)
    {
        m_currentRootSignature = RootSig;
        GetCommandList()->SetGraphicsRootSignature(RootSig);
    }
}

void dx12Backend::SetSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers)
{
    StateManager12.SetSamplers(StartSlot, Count, Samplers);
}

void dx12Backend::SetGraphicsSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers)
{
    GetCommandList()->RSSetSamplers(StartSlot, Count, Samplers);
}

void dx12Backend::SetComputeSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers)
{
    GetComputeCommandList()->CSSetSamplers(StartSlot, Count, Samplers);
}

ID3D12RootSignature* dx12Backend::GetRootSignature(DX12_ROOT_SIGNATURE_TYPE Type)
{
    return RootSignature12.GetRootSignature(Type);
}

ID3D12PipelineState* dx12Backend::BuildPSO(ID3DBlob* pVS, ID3DBlob* pPS, ID3D12RootSignature* pRootSignature,
                                            const D3D12_INPUT_ELEMENT_DESC* pInputLayout, UINT InputLayoutElementCount,
                                            DXGI_FORMAT RTFormat, DXGI_FORMAT DSFormat,
                                            D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType)
{
    return PSOBuilder12.BuildPSO(pVS, pPS, pRootSignature, pInputLayout, InputLayoutElementCount,
                                  RTFormat, DSFormat, TopologyType);
}

ID3D12PipelineState* dx12Backend::BuildComputePSO(ID3DBlob* pCS, ID3D12RootSignature* pRootSignature)
{
    return PSOBuilder12.BuildComputePSO(pCS, pRootSignature);
}

void dx12Backend::SetVertexBuffer(ID3D12Resource* VB, u32 Stride, u32 Offset)
{
    D3D12_VERTEX_BUFFER_VIEW vbView = {};
    vbView.BufferLocation = VB->GetGPUVirtualAddress() + Offset;
    vbView.StrideInBytes = Stride;
    vbView.SizeInBytes = 0;

    if (vbView.BufferLocation != m_currentVBView.BufferLocation ||
        vbView.StrideInBytes != m_currentVBView.StrideInBytes)
    {
        m_currentVBView = vbView;
        m_vbDirty = true;
    }
}

void dx12Backend::SetIndexBuffer(ID3D12Resource* IB, DXGI_FORMAT Format, u32 Offset)
{
    D3D12_INDEX_BUFFER_VIEW ibView = {};
    ibView.BufferLocation = IB->GetGPUVirtualAddress() + Offset;
    ibView.Format = Format;
    ibView.SizeInBytes = 0;

    if (ibView.BufferLocation != m_currentIBView.BufferLocation ||
        ibView.Format != m_currentIBView.Format)
    {
        m_currentIBView = ibView;
        m_ibDirty = true;
    }
}

void dx12Backend::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology)
{
    if (m_currentTopology != Topology)
    {
        m_currentTopology = Topology;
        GetCommandList()->IASetPrimitiveTopology(Topology);
    }
}

void dx12Backend::DrawIndexedInstanced(u32 IndexCountPerInstance, u32 InstanceCount, u32 StartIndexLocation,
                                       INT BaseVertexLocation, u32 StartInstanceLocation)
{
    if (m_vbDirty)
    {
        GetCommandList()->IASetVertexBuffers(0, 1, &m_currentVBView);
        m_vbDirty = false;
    }

    if (m_ibDirty)
    {
        GetCommandList()->IASetIndexBuffer(&m_currentIBView);
        m_ibDirty = false;
    }

    GetCommandList()->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount,
                                           StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void dx12Backend::DrawInstanced(u32 VertexCountPerInstance, u32 InstanceCount, u32 StartVertexLocation,
                                u32 StartInstanceLocation)
{
    if (m_vbDirty)
    {
        GetCommandList()->IASetVertexBuffers(0, 1, &m_currentVBView);
        m_vbDirty = false;
    }

    GetCommandList()->DrawInstanced(VertexCountPerInstance, InstanceCount,
                                    StartVertexLocation, StartInstanceLocation);
}

void dx12Backend::Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
    GetComputeCommandList()->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void dx12Backend::SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* DescriptorHeaps)
{
    GetCommandList()->SetDescriptorHeaps(NumDescriptorHeaps, DescriptorHeaps);
}

void dx12Backend::SetGraphicsRootSignature(ID3D12RootSignature* RootSignature)
{
    GetCommandList()->SetGraphicsRootSignature(RootSignature);
}

void dx12Backend::SetComputeRootSignature(ID3D12RootSignature* RootSignature)
{
    GetComputeCommandList()->SetComputeRootSignature(RootSignature);
}

void dx12Backend::SetGraphicsRootDescriptorTable(u32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor.ptr);
}

void dx12Backend::SetComputeRootDescriptorTable(u32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    GetComputeCommandList()->SetComputeRootDescriptorTable(RootParameterIndex, BaseDescriptor.ptr);
}

void dx12Backend::SetGraphicsRootConstantBufferView(u32 RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    GetCommandList()->SetGraphicsRootConstantBufferView(RootParameterIndex, BufferLocation);
}

void dx12Backend::SetComputeRootConstantBufferView(u32 RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    GetComputeCommandList()->SetComputeRootConstantBufferView(RootParameterIndex, BufferLocation);
}

void dx12Backend::CopyResource(ID3D12Resource* Dst, ID3D12Resource* Src)
{
    GetCommandList()->CopyResource(Dst->GetGPUVirtualAddress(), Src->GetGPUVirtualAddress());
}

void dx12Backend::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* Dst, u64 DstX, u64 DstY, u64 DstZ,
                                    const D3D12_TEXTURE_COPY_LOCATION* Src, const D3D12_BOX* SrcBox)
{
    GetCommandList()->CopyTextureRegion(Dst, DstX, DstY, DstZ, Src, SrcBox);
}

void dx12Backend::UpdateSubresource(ID3D12Resource* Dst, UINT DstSubresource, const D3D12_BOX* DstBox,
                                    const void* SrcData, UINT SrcRowPitch, UINT SrcSlicePitch)
{
    GetCommandList()->UpdateSubresource(Dst, DstSubresource, DstBox, SrcData, SrcRowPitch, SrcSlicePitch);
}

void dx12Backend::ResolveSubresource(ID3D12Resource* Dst, UINT DstSubresource, ID3D12Resource* Src,
                                     UINT SrcSubresource, DXGI_FORMAT Format)
{
    GetCommandList()->ResolveSubresource(Dst, DstSubresource, Src, SrcSubresource, Format);
}

void dx12Backend::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const FLOAT Color[4],
                                        UINT NumRects, const D3D12_RECT* Rects)
{
    GetCommandList()->ClearRenderTargetView(RenderTargetView, Color, NumRects, Rects);
}

void dx12Backend::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags,
                                        FLOAT Depth, UINT8 Stencil, UINT NumRects, const D3D12_RECT* Rects)
{
    GetCommandList()->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, Rects);
}

void dx12Backend::IASetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* Views)
{
    GetCommandList()->IASetVertexBuffers(StartSlot, NumViews, Views);
}

void dx12Backend::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* View)
{
    GetCommandList()->IASetIndexBuffer(View);
}

void dx12Backend::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology)
{
    GetCommandList()->IASetPrimitiveTopology(Topology);
}

void dx12Backend::RSSetViewports(UINT NumViewports, const D3D12_VIEWPORT* Viewports)
{
    GetCommandList()->RSSetViewports(NumViewports, Viewports);
}

void dx12Backend::RSSetScissorRects(UINT NumRects, const D3D12_RECT* Rects)
{
    GetCommandList()->RSSetScissorRects(NumRects, Rects);
}

void dx12Backend::OMSetBlendState(ID3D12BlendState* BlendState, const FLOAT BlendFactor[4], UINT SampleMask)
{
    GetCommandList()->OMSetBlendState(BlendState, BlendFactor, SampleMask);
}

void dx12Backend::OMSetDepthStencilState(ID3D12DepthStencilState* DepthStencilState, UINT StencilRef)
{
    GetCommandList()->OMSetDepthStencilState(DepthStencilState, StencilRef);
}

void dx12Backend::OMSetRenderTargets(UINT NumRenderTargetViews, const D3D12_CPU_DESCRIPTOR_HANDLE* RenderTargetViews,
                                     BOOL RTsSingleHandleToDescriptorHeap, UINT NumDepthStencilViews,
                                     const D3D12_CPU_DESCRIPTOR_HANDLE* DepthStencilViews)
{
    GetCommandList()->OMSetRenderTargets(NumRenderTargetViews, RenderTargetViews,
                                         RTsSingleHandleToDescriptorHeap, DepthStencilViews);
}

void dx12Backend::SetPipelineStatistics1(const D3D12_PIPELINE_STATISTICS_QUERY_DATA* Data)
{
    GetCommandList()->SetPipelineStatistics1(Data);
}

void dx12Backend::Flush()
{
    CommandManager12.WaitForGPU();
}

dx12Backend Backend12;

#endif // USE_DX12
