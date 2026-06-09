#include "../dx12stdafx.h"
#include "dx12DescriptorManager.h"

#ifdef USE_DX12

dx12DescriptorManager::dx12DescriptorManager()
    : m_pSRVUAVHeap(nullptr),
      m_pCBVHeap(nullptr),
      m_srvUAVCapacity(0),
      m_srvUAVOffset(0),
      m_cbvCapacity(0),
      m_cbvOffset(0),
      m_descriptorSize(0)
{
}

dx12DescriptorManager::~dx12DescriptorManager()
{
}

void dx12DescriptorManager::Init()
{
    m_descriptorSize = HW12.m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_DESCRIPTOR_HEAP_DESC srvUAVDesc = {};
    srvUAVDesc.NumDescriptors = InitialHeapSize;
    srvUAVDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvUAVDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    srvUAVDesc.NodeMask = 0;
    R_CHK(HW12.m_pDevice->CreateDescriptorHeap(&srvUAVDesc, IID_PPV_ARGS(&m_pSRVUAVHeap)));

    m_srvUAVCapacity = InitialHeapSize;
    m_srvUAVOffset = 0;
    m_srvUAVHeapStart = m_pSRVUAVHeap->GetCPUDescriptorHandleForHeapStart();
    m_srvUAVGPUHeapStart = m_pSRVUAVHeap->GetGPUDescriptorHandleForHeapStart();

    D3D12_DESCRIPTOR_HEAP_DESC cbvDesc = {};
    cbvDesc.NumDescriptors = InitialHeapSize / 4;
    cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvDesc.NodeMask = 0;
    R_CHK(HW12.m_pDevice->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&m_pCBVHeap)));

    m_cbvCapacity = InitialHeapSize / 4;
    m_cbvOffset = 0;
    m_cbvHeapStart = m_pCBVHeap->GetCPUDescriptorHandleForHeapStart();
    m_cbvGPUHeapStart = m_pCBVHeap->GetGPUDescriptorHandleForHeapStart();
}

void dx12DescriptorManager::ResetFrame()
{
    m_srvUAVOffset = 0;
    m_cbvOffset = 0;
}

void dx12DescriptorManager::GrowHeap(ComPtr<ID3D12DescriptorHeap>& Heap, UINT& Capacity,
                                      D3D12_DESCRIPTOR_HEAP_TYPE Type, D3D12_DESCRIPTOR_HEAP_FLAGS Flags)
{
    Capacity += HeapGrowthSize;

    ComPtr<ID3D12DescriptorHeap> newHeap;
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = Capacity;
    desc.Type = Type;
    desc.Flags = Flags;
    desc.NodeMask = 0;
    R_CHK(HW12.m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap)));

    D3D12_CPU_DESCRIPTOR_HANDLE oldStart = Heap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE newStart = newHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < Capacity - HeapGrowthSize; i++)
    {
        HW12.m_pDevice->CopyDescriptor(
            newStart.ptr + i * m_descriptorSize,
            oldStart.ptr + i * m_descriptorSize);
    }

    Heap = std::move(newHeap);
}

D3D12_CPU_DESCRIPTOR_HANDLE dx12DescriptorManager::AllocateSRV()
{
    if (m_srvUAVOffset >= m_srvUAVCapacity)
    {
        GrowHeap(m_pSRVUAVHeap, m_srvUAVCapacity,
                 D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                 D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        m_srvUAVHeapStart = m_pSRVUAVHeap->GetCPUDescriptorHandleForHeapStart();
        m_srvUAVGPUHeapStart = m_pSRVUAVHeap->GetGPUDescriptorHandleForHeapStart();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE handle;
    handle.ptr = m_srvUAVHeapStart.ptr + m_srvUAVOffset * m_descriptorSize;
    m_srvUAVOffset++;
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE dx12DescriptorManager::AllocateUAV()
{
    return AllocateSRV();
}

D3D12_CPU_DESCRIPTOR_HANDLE dx12DescriptorManager::AllocateCBV()
{
    if (m_cbvOffset >= m_cbvCapacity)
    {
        GrowHeap(m_pCBVHeap, m_cbvCapacity,
                 D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                 D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        m_cbvHeapStart = m_pCBVHeap->GetCPUDescriptorHandleForHeapStart();
        m_cbvGPUHeapStart = m_pCBVHeap->GetGPUDescriptorHandleForHeapStart();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE handle;
    handle.ptr = m_cbvHeapStart.ptr + m_cbvOffset * m_descriptorSize;
    m_cbvOffset++;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE dx12DescriptorManager::AllocateGPUSRV()
{
    if (m_srvUAVOffset >= m_srvUAVCapacity)
    {
        GrowHeap(m_pSRVUAVHeap, m_srvUAVCapacity,
                 D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                 D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        m_srvUAVGPUHeapStart = m_pSRVUAVHeap->GetGPUDescriptorHandleForHeapStart();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE handle;
    handle.ptr = m_srvUAVGPUHeapStart.ptr + m_srvUAVOffset * m_descriptorSize;
    m_srvUAVOffset++;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE dx12DescriptorManager::AllocateGPUUAV()
{
    return AllocateGPUSRV();
}

D3D12_GPU_DESCRIPTOR_HANDLE dx12DescriptorManager::AllocateGPUCBV()
{
    if (m_cbvOffset >= m_cbvCapacity)
    {
        GrowHeap(m_pCBVHeap, m_cbvCapacity,
                 D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                 D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        m_cbvGPUHeapStart = m_pCBVHeap->GetGPUDescriptorHandleForHeapStart();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE handle;
    handle.ptr = m_cbvGPUHeapStart.ptr + m_cbvOffset * m_descriptorSize;
    m_cbvOffset++;
    return handle;
}

void dx12DescriptorManager::CreateSRV(ID3D12Resource* Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* Desc,
                                       D3D12_CPU_DESCRIPTOR_HANDLE Handle)
{
    HW12.m_pDevice->CreateShaderResourceView(Resource, Desc, Handle);
}

void dx12DescriptorManager::CreateUAV(ID3D12Resource* Resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* Desc,
                                       D3D12_CPU_DESCRIPTOR_HANDLE Handle)
{
    HW12.m_pDevice->CreateUnorderedAccessView(Resource, nullptr, Desc, Handle);
}

void dx12DescriptorManager::CreateCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC* Desc, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
{
    HW12.m_pDevice->CreateConstantBufferView(Desc, Handle);
}

void dx12DescriptorManager::SetDescriptorHeaps(ID3D12DescriptorHeap** Heaps, UINT Count)
{
    HW12.m_pCommandList->SetDescriptorHeaps(Count, Heaps);
}

dx12DescriptorManager DescriptorManager12;

#endif // USE_DX12
