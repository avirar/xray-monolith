#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12CommandManager.h"

#ifdef USE_DX12

dx12CommandManager::dx12CommandManager()
    : m_pGraphicsCommandList(nullptr),
      m_pComputeCommandList(nullptr),
      m_frameIndex(0),
      m_frameCount(3),
      m_fenceValue(0),
      m_graphicsClosed(false),
      m_computeClosed(false)
{
    ZeroMemory(m_pGraphicsAllocators, sizeof(m_pGraphicsAllocators));
    ZeroMemory(m_pComputeAllocators, sizeof(m_pComputeAllocators));
}

dx12CommandManager::~dx12CommandManager()
{
    Destroy();
}

void dx12CommandManager::Init(UINT FrameCount)
{
    m_frameCount = std::min(FrameCount, UINT(_countof(m_pGraphicsAllocators)));
    m_frameIndex = 0;

    for (UINT i = 0; i < m_frameCount; i++)
    {
        R_CHK(HW12.m_pDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_pGraphicsAllocators[i])));

        R_CHK(HW12.m_pDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_pComputeAllocators[i])));
    }

    R_CHK(HW12.m_pDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_pGraphicsAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(&m_pGraphicsCommandList)));

    R_CHK(m_pGraphicsCommandList->Close());
    m_graphicsClosed = true;

    R_CHK(HW12.m_pDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_pComputeAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(&m_pComputeCommandList)));

    R_CHK(m_pComputeCommandList->Close());
    m_computeClosed = true;

    m_frameIndex = HW12.m_pSwapChain->GetCurrentBackBufferIndex();
}

void dx12CommandManager::Destroy()
{
    WaitForGPU();

    m_pGraphicsCommandList.Reset();
    m_pComputeCommandList.Reset();

    for (UINT i = 0; i < m_frameCount; i++)
    {
        m_pGraphicsAllocators[i].Reset();
        m_pComputeAllocators[i].Reset();
    }
}

void dx12CommandManager::BeginFrame()
{
    m_frameIndex = HW12.m_pSwapChain->GetCurrentBackBufferIndex();

    UINT64 fenceValue = HW12.m_FenceValues[m_frameIndex];
    if (HW12.m_pFence->GetCompletedValue() < fenceValue)
    {
        HW12.m_pFence->SetEventOnCompletion(fenceValue, HW12.m_fenceEvent);
        WaitForSingleObject(HW12.m_fenceEvent, INFINITE);
    }

    R_CHK(m_pGraphicsAllocators[m_frameIndex]->Reset());
    R_CHK(m_pGraphicsCommandList->Reset(m_pGraphicsAllocators[m_frameIndex].Get(), nullptr));
    m_graphicsClosed = false;

    R_CHK(m_pComputeAllocators[m_frameIndex]->Reset());
    R_CHK(m_pComputeCommandList->Reset(m_pComputeAllocators[m_frameIndex].Get(), nullptr));
    m_computeClosed = false;
}

void dx12CommandManager::EndFrame()
{
    if (!m_computeClosed)
    {
        CloseComputeCommandList();
    }

    if (!m_graphicsClosed)
    {
        CloseGraphicsCommandList();
    }

    ExecuteCommandLists();
    Signal();
}

void dx12CommandManager::WaitForGPU()
{
    if (!HW12.m_pFence || !HW12.m_pCommandQueue)
        return;

    UINT64 fenceValue = ++m_fenceValue;
    HRESULT hr = HW12.m_pCommandQueue->Signal(HW12.m_pFence.Get(), fenceValue);
    if (FAILED(hr))
        return;

    if (HW12.m_pFence->GetCompletedValue() >= fenceValue)
        return;

    hr = HW12.m_pFence->SetEventOnCompletion(fenceValue, HW12.m_fenceEvent);
    if (SUCCEEDED(hr))
    {
        WaitForSingleObject(HW12.m_fenceEvent, INFINITE);
    }
}

void dx12CommandManager::CloseGraphicsCommandList()
{
    if (!m_graphicsClosed)
    {
        R_CHK(m_pGraphicsCommandList->Close());
        m_graphicsClosed = true;
    }
}

void dx12CommandManager::CloseComputeCommandList()
{
    if (!m_computeClosed)
    {
        R_CHK(m_pComputeCommandList->Close());
        m_computeClosed = true;
    }
}

void dx12CommandManager::ExecuteCommandLists()
{
    ID3D12CommandList* ppCommandLists[2];
    UINT count = 0;

    if (m_computeClosed)
    {
        ppCommandLists[count++] = m_pComputeCommandList.Get();
    }

    if (m_graphicsClosed)
    {
        ppCommandLists[count++] = m_pGraphicsCommandList.Get();
    }

    if (count > 0)
    {
        HW12.m_pCommandQueue->ExecuteCommandLists(count, ppCommandLists);
    }
}

void dx12CommandManager::Signal()
{
    UINT64 fenceValue = ++m_fenceValue;
    HW12.m_FenceValues[m_frameIndex] = fenceValue;
    HW12.m_pCommandQueue->Signal(HW12.m_pFence.Get(), fenceValue);
}

void dx12CommandManager::WaitForFence(UINT64 FenceValue)
{
    if (HW12.m_pFence->GetCompletedValue() >= FenceValue)
        return;

    HRESULT hr = HW12.m_pFence->SetEventOnCompletion(FenceValue, HW12.m_fenceEvent);
    if (SUCCEEDED(hr))
    {
        WaitForSingleObject(HW12.m_fenceEvent, INFINITE);
    }
}

void dx12CommandManager::ResetAllocator(UINT FrameIndex, bool Graphics, bool Compute)
{
    if (Graphics)
    {
        UINT64 fenceValue = HW12.m_FenceValues[FrameIndex];
        if (HW12.m_pFence->GetCompletedValue() >= fenceValue)
        {
            R_CHK(m_pGraphicsAllocators[FrameIndex]->Reset());
        }
    }

    if (Compute)
    {
        UINT64 fenceValue = HW12.m_FenceValues[FrameIndex];
        if (HW12.m_pFence->GetCompletedValue() >= fenceValue)
        {
            R_CHK(m_pComputeAllocators[FrameIndex]->Reset());
        }
    }
}

void dx12CommandManager::Present(IDXGISwapChain3* pSwapChain)
{
    if (pSwapChain)
    {
        pSwapChain->Present(1, 0);
    }
}

dx12CommandManager CommandManager12;

#endif // USE_DX12
