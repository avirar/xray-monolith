#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12SamplerManager.h"

#ifdef USE_DX12

dx12SamplerManager::dx12SamplerManager()
    : m_samplerOffset(0),
      m_samplerCapacity(0)
{
    for (UINT i = 0; i < DST_COUNT; i++)
        m_staticSamplers[i] = nullptr;
}

dx12SamplerManager::~dx12SamplerManager()
{
}

void dx12SamplerManager::Init()
{
    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.MinLOD = 0;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] =
    samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 0.0f;

    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MaxAnisotropy = 1;
    R_CHK(HW12.m_pDevice->CreateSamplerState(&samplerDesc, m_staticSamplers[DST_LINEAR_WRAP].GetAddressOf()));

    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    R_CHK(HW12.m_pDevice->CreateSamplerState(&samplerDesc, m_staticSamplers[DST_POINT_WRAP].GetAddressOf()));

    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    R_CHK(HW12.m_pDevice->CreateSamplerState(&samplerDesc, m_staticSamplers[DST_LINEAR_CLAMP].GetAddressOf()));

    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    R_CHK(HW12.m_pDevice->CreateSamplerState(&samplerDesc, m_staticSamplers[DST_POINT_CLAMP].GetAddressOf()));

    samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MaxAnisotropy = 16;
    R_CHK(HW12.m_pDevice->CreateSamplerState(&samplerDesc, m_staticSamplers[DST_ANISOTROPIC_WRAP].GetAddressOf()));

    samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    R_CHK(HW12.m_pDevice->CreateSamplerState(&samplerDesc, m_staticSamplers[DST_ANISOTROPIC_CLAMP].GetAddressOf()));

    samplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_LESS_EQUAL;
    R_CHK(HW12.m_pDevice->CreateSamplerState(&samplerDesc, m_staticSamplers[DST_COMPARISON_LINEAR].GetAddressOf()));

    samplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
    R_CHK(HW12.m_pDevice->CreateSamplerState(&samplerDesc, m_staticSamplers[DST_COMPARISON_POINT].GetAddressOf()));

    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_NEVER;
    R_CHK(HW12.m_pDevice->CreateSamplerState(&samplerDesc, m_staticSamplers[DST_LINEAR_MIRROR].GetAddressOf()));

    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    R_CHK(HW12.m_pDevice->CreateSamplerState(&samplerDesc, m_staticSamplers[DST_POINT_MIRROR].GetAddressOf()));

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = InitialSamplerHeapSize;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;
    R_CHK(HW12.m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pSamplerHeap)));

    m_samplerCapacity = InitialSamplerHeapSize;
    m_samplerOffset = 0;
    m_samplerHeapStart = m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart();
}

ID3D12SamplerState* dx12SamplerManager::GetSampler(DX12_SAMPLER_TYPE Type)
{
    VERIFY(Type < DST_COUNT);
    return m_staticSamplers[Type].Get();
}

ID3D12SamplerState* dx12SamplerManager::CreateSampler(const DX12_SAMPLER_DESC& Desc)
{
    u64 key = xxhash64(&Desc, sizeof(Desc), 0);

    xr_map<u64, ComPtr<ID3D12SamplerState>>::iterator it = m_customSamplers.find(key);
    if (it != m_customSamplers.end())
        return it->second.Get();

    D3D12_SAMPLER_DESC d3d12Desc = {};
    d3d12Desc.Filter = Desc.Filter;
    d3d12Desc.AddressU = Desc.AddressU;
    d3d12Desc.AddressV = Desc.AddressV;
    d3d12Desc.AddressW = Desc.AddressW;
    d3d12Desc.MipLODBias = Desc.MipLODBias;
    d3d12Desc.MaxAnisotropy = Desc.MaxAnisotropy ? Desc.MaxAnisotropy : 1;
    d3d12Desc.ComparisonFunc = Desc.ComparisonFunc;
    CopyMemory(d3d12Desc.BorderColor, Desc.BorderColor, sizeof(Desc.BorderColor));
    d3d12Desc.MinLOD = Desc.MinLOD;
    d3d12Desc.MaxLOD = Desc.MaxLOD ? Desc.MaxLOD : D3D12_FLOAT32_MAX;

    ComPtr<ID3D12SamplerState> sampler;
    HRESULT hr = HW12.m_pDevice->CreateSamplerState(&d3d12Desc, sampler.GetAddressOf());
    if (SUCCEEDED(hr))
    {
        m_customSamplers[key] = sampler;
        return sampler.Get();
    }

    LOG("*ERROR* dx12SamplerManager: Failed to create custom sampler");
    return nullptr;
}

D3D12_CPU_DESCRIPTOR_HANDLE dx12SamplerManager::AllocateSamplerDescriptor()
{
    if (m_samplerOffset >= m_samplerCapacity)
    {
        LOG("*ERROR* dx12SamplerManager: Sampler heap exhausted");
        D3D12_CPU_DESCRIPTOR_HANDLE nullHandle;
        nullHandle.ptr = 0;
        return nullHandle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE handle;
    UINT descriptorSize = HW12.m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    handle.ptr = m_samplerHeapStart.ptr + m_samplerOffset * descriptorSize;
    m_samplerOffset++;
    return handle;
}

void dx12SamplerManager::CreateSamplerDescriptor(ID3D12SamplerState* Sampler, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
{
    HW12.m_pDevice->CreateSamplerDescriptor(Sampler, Handle);
}

void dx12SamplerManager::SetSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers)
{
    Backend12.GetCommandList()->RSSetSamplers(StartSlot, Count, Samplers);
}

void dx12SamplerManager::SetComputeSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers)
{
    Backend12.GetComputeCommandList()->CSSetSamplers(StartSlot, Count, Samplers);
}

dx12SamplerManager SamplerManager12;

#endif // USE_DX12
