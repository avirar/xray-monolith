#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12RootSignature.h"

#ifdef USE_DX12

dx12RootSignature::dx12RootSignature()
{
    for (UINT i = 0; i < RST_COUNT; i++)
        m_rootSignatures[i] = nullptr;
}

dx12RootSignature::~dx12RootSignature()
{
}

void dx12RootSignature::Init()
{
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc;

    BuildDeferredRootSignature(desc);
    CreateRootSignature(desc, m_rootSignatures[RST_GRAPHICS_DEFAULT]);

    BuildComputeRootSignature(desc);
    CreateRootSignature(desc, m_rootSignatures[RST_COMPUTE_HDAO]);

    BuildRayTracingRootSignature(desc);
    CreateRootSignature(desc, m_rootSignatures[RST_RAYTRACING]);
}

ID3D12RootSignature* dx12RootSignature::GetRootSignature(DX12_ROOT_SIGNATURE_TYPE Type)
{
    VERIFY(Type < RST_COUNT);
    return m_rootSignatures[Type].Get();
}

ID3D12RootSignature* dx12RootSignature::CreateCustomRootSignature(const DX12_ROOT_SIGNATURE_DESC& Desc)
{
    u64 key = xxhash64(&Desc, sizeof(Desc), 0);

    xr_map<u64, ComPtr<ID3D12RootSignature>>::iterator it = m_customRootSignatures.find(key);
    if (it != m_customRootSignatures.end())
        return it->second.Get();

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;

    CD3DX12_ROOT_PARAMETER1 parameters[MaxCBVs + 4];
    UINT paramCount = 0;

    for (UINT i = 0; i < Desc.NumCBVs && paramCount < (MaxCBVs + 4); i++, paramCount++)
    {
        parameters[paramCount].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_ALL);
    }

    if (Desc.NumSRVs > 0 && paramCount < (MaxCBVs + 4))
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Desc.NumSRVs, 0, 0,
                       Desc.bVolatileDescriptors ? D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE : 0);
        parameters[paramCount].InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_ALL);
        paramCount++;
    }

    if (Desc.NumUAVs > 0 && paramCount < (MaxCBVs + 4))
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, Desc.NumUAVs, 0, 0,
                       Desc.bUAVsAllDescriptors ? D3D12_DESCRIPTOR_RANGE_FLAG_UAVs_ALL : 0);
        parameters[paramCount].InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_ALL);
        paramCount++;
    }

    if (Desc.NumSamplers > 0 && paramCount < (MaxCBVs + 4))
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, Desc.NumSamplers, 0, 0,
                       Desc.bVolatileDescriptors ? D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE : 0);
        parameters[paramCount].InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_ALL);
        paramCount++;
    }

    CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
    {
        CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
        CD3DX12_STATIC_SAMPLER_DESC(1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
        CD3DX12_STATIC_SAMPLER_DESC(2, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
    };

    rootSigDesc.Init_1_1(paramCount, parameters, _countof(staticSamplers), staticSamplers,
                         D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3D12RootSignature> rootSig;
    if (SUCCEEDED(CreateRootSignature(rootSigDesc, rootSig)))
    {
        m_customRootSignatures[key] = rootSig;
        return rootSig.Get();
    }

    return nullptr;
}

void dx12RootSignature::BuildDeferredRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& RootSigDesc)
{
    CD3DX12_ROOT_PARAMETER1 parameters[6];
    ZeroMemory(parameters, sizeof(parameters));

    parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
    parameters[1].InitAsConstantBufferView(6, 0, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_DESCRIPTOR_RANGE1 srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MaxSRVs, 0, 0,
                  D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    parameters[2].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
    samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, MaxSamplers, 0, 0,
                      D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    parameters[3].InitAsDescriptorTable(1, &samplerRange, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_DESCRIPTOR_RANGE1 uavRange;
    uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, MaxUAVs, 0, 0,
                  D3D12_DESCRIPTOR_RANGE_FLAG_UAVs_ALL);
    parameters[4].InitAsDescriptorTable(1, &uavRange, D3D12_SHADER_VISIBILITY_ALL);

    parameters[5].InitAsDescriptorTable(1,
        &CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, MaxSRVs, 0,
                                   D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE),
        D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
    {
        CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
        CD3DX12_STATIC_SAMPLER_DESC(1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
        CD3DX12_STATIC_SAMPLER_DESC(2, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
        CD3DX12_STATIC_SAMPLER_DESC(3, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    0.0f, 8),
    };

    RootSigDesc.Init_1_1(6, parameters, _countof(staticSamplers), staticSamplers,
                         D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void dx12RootSignature::BuildComputeRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& RootSigDesc)
{
    CD3DX12_ROOT_PARAMETER1 parameters[4];
    ZeroMemory(parameters, sizeof(parameters));

    parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_DESCRIPTOR_RANGE1 srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MaxSRVs, 0, 0,
                  D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    parameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_DESCRIPTOR_RANGE1 uavRange;
    uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, MaxUAVs, 0, 0,
                  D3D12_DESCRIPTOR_RANGE_FLAG_UAVs_ALL);
    parameters[2].InitAsDescriptorTable(1, &uavRange, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
    samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 4, 0, 0, 0);
    parameters[3].InitAsDescriptorTable(1, &samplerRange, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
    {
        CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
        CD3DX12_STATIC_SAMPLER_DESC(1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
    };

    RootSigDesc.Init_1_1(4, parameters, _countof(staticSamplers), staticSamplers,
                         D3D12_ROOT_SIGNATURE_FLAG_NONE);
}

void dx12RootSignature::BuildRayTracingRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& RootSigDesc)
{
    CD3DX12_ROOT_PARAMETER1 parameters[4];
    ZeroMemory(parameters, sizeof(parameters));

    parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_DESCRIPTOR_RANGE1 srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MaxSRVs, 0, 0,
                  D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    parameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_DESCRIPTOR_RANGE1 uavRange;
    uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 4, 0, 0,
                  D3D12_DESCRIPTOR_RANGE_FLAG_UAVs_ALL);
    parameters[2].InitAsDescriptorTable(1, &uavRange, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
    samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 4, 0, 0, 0);
    parameters[3].InitAsDescriptorTable(1, &samplerRange, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
    {
        CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
    };

    RootSigDesc.Init_1_1(4, parameters, _countof(staticSamplers), staticSamplers,
                         D3D12_ROOT_SIGNATURE_FLAG_NONE);
}

HRESULT dx12RootSignature::CreateRootSignature(const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& RootSigDesc,
                                                ComPtr<ID3D12RootSignature>& RootSig)
{
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3D12SerializeVersionedRootSignature(&RootSigDesc, &signature, &error);
    if (FAILED(hr))
    {
        LOG("*ERROR* dx12RootSignature: Failed to serialize root signature: %ls",
            error ? error->GetBufferPointer() : L"unknown error");
        return hr;
    }

    hr = HW12.m_pDevice->CreateRootSignature(0, signature->GetBufferPointer(),
                                              signature->GetBufferSize(),
                                              IID_PPV_ARGS(&RootSig));
    if (FAILED(hr))
    {
        LOG("*ERROR* dx12RootSignature: Failed to create root signature");
        return hr;
    }

    return S_OK;
}

dx12RootSignature RootSignature12;

#endif // USE_DX12
