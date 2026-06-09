#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

class dx12DescriptorManager
{
public:
    dx12DescriptorManager();
    ~dx12DescriptorManager();

    void Init();
    void ResetFrame();

    D3D12_CPU_DESCRIPTOR_HANDLE AllocateSRV();
    D3D12_CPU_DESCRIPTOR_HANDLE AllocateUAV();
    D3D12_CPU_DESCRIPTOR_HANDLE AllocateCBV();
    D3D12_GPU_DESCRIPTOR_HANDLE AllocateGPUSRV();
    D3D12_GPU_DESCRIPTOR_HANDLE AllocateGPUUAV();
    D3D12_GPU_DESCRIPTOR_HANDLE AllocateGPUCBV();

    void CreateSRV(ID3D12Resource* Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* Desc,
                   D3D12_CPU_DESCRIPTOR_HANDLE Handle);
    void CreateUAV(ID3D12Resource* Resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* Desc,
                   D3D12_CPU_DESCRIPTOR_HANDLE Handle);
    void CreateCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC* Desc, D3D12_CPU_DESCRIPTOR_HANDLE Handle);

    void SetDescriptorHeaps(ID3D12DescriptorHeap** Heaps, UINT Count);

    ID3D12DescriptorHeap* GetSRVUAVHeap() const { return m_pSRVUAVHeap.Get(); }
    ID3D12DescriptorHeap* GetCBVHeap() const { return m_pCBVHeap.Get(); }

    UINT GetDescriptorSize() const { return m_descriptorSize; }

private:
    void GrowHeap(ComPtr<ID3D12DescriptorHeap>& Heap, UINT& Capacity,
                  D3D12_DESCRIPTOR_HEAP_TYPE Type, D3D12_DESCRIPTOR_HEAP_FLAGS Flags);

    ComPtr<ID3D12DescriptorHeap> m_pSRVUAVHeap;
    ComPtr<ID3D12DescriptorHeap> m_pCBVHeap;

    D3D12_CPU_DESCRIPTOR_HANDLE m_srvUAVHeapStart;
    D3D12_CPU_DESCRIPTOR_HANDLE m_cbvHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE m_srvUAVGPUHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE m_cbvGPUHeapStart;

    UINT m_srvUAVCapacity;
    UINT m_srvUAVOffset;
    UINT m_cbvCapacity;
    UINT m_cbvOffset;

    UINT m_descriptorSize;

    static const UINT InitialHeapSize = 4096;
    static const UINT HeapGrowthSize = 2048;
};

extern dx12DescriptorManager DescriptorManager12;

#endif // USE_DX12
