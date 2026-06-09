#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

class dx12CommandManager
{
public:
    dx12CommandManager();
    ~dx12CommandManager();

    void Init(UINT FrameCount = 3);
    void Destroy();

    void BeginFrame();
    void EndFrame();
    void WaitForGPU();

    ID3D12GraphicsCommandList4* GetGraphicsCommandList() const { return m_pGraphicsCommandList.Get(); }
    ID3D12GraphicsCommandList4* GetComputeCommandList() const { return m_pComputeCommandList.Get(); }
    ID3D12CommandAllocator* GetGraphicsAllocator() const { return m_pGraphicsAllocators[m_frameIndex].Get(); }
    ID3D12CommandAllocator* GetComputeAllocator() const { return m_pComputeAllocators[m_frameIndex].Get(); }
    UINT GetFrameIndex() const { return m_frameIndex; }
    UINT GetFrameCount() const { return m_frameCount; }

    void CloseGraphicsCommandList();
    void CloseComputeCommandList();
    void ExecuteCommandLists();

    UINT64 GetFenceValue() const { return m_fenceValue; }
    void Signal();
    void WaitForFence(UINT64 FenceValue);

    void ResetAllocator(UINT FrameIndex, bool Graphics = true, bool Compute = false);

    bool IsGraphicsClosed() const { return m_graphicsClosed; }
    bool IsComputeClosed() const { return m_computeClosed; }

private:
    ComPtr<ID3D12CommandAllocator> m_pGraphicsAllocators[8];
    ComPtr<ID3D12CommandAllocator> m_pComputeAllocators[8];
    ComPtr<ID3D12GraphicsCommandList4> m_pGraphicsCommandList;
    ComPtr<ID3D12GraphicsCommandList4> m_pComputeCommandList;

    UINT m_frameIndex;
    UINT m_frameCount;

    UINT64 m_fenceValue;

    bool m_graphicsClosed;
    bool m_computeClosed;
};

extern dx12CommandManager CommandManager12;

#endif // USE_DX12
