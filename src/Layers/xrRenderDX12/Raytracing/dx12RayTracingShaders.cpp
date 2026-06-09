#include "../dx12stdafx.h"
#include "dx12RayTracingShaders.h"
#include "../ShaderCompiler/dx12ShaderCompiler.h"

#ifdef USE_DX12

dx12RayTracingShaders::dx12RayTracingShaders()
    : m_hitGroupCount(0),
      m_localRootSignatureGPUAddress(0),
      m_shaderTableStride(0)
{
    ZeroMemory(&m_rayGenShaders, sizeof(m_rayGenShaders));
    ZeroMemory(&m_missShaders, sizeof(m_missShaders));
    ZeroMemory(&m_hitGroups, sizeof(m_hitGroups));
}

dx12RayTracingShaders::~dx12RayTracingShaders()
{
}

void dx12RayTracingShaders::Init()
{
    m_shaderTableStride = HW12.m_pDevice->GetRaytracingShaderTableAlignment();
    CreateLocalRootSignature();
}

ID3DBlob* dx12RayTracingShaders::CompileRayGenShader(LPCSTR Name, LPCSTR Entry)
{
    DX12_RAYTRACING_SHADER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Name = Name;
    desc.Entry = Entry;
    desc.Type = DRTS_RAYGEN;

    return CompileShader(desc);
}

ID3DBlob* dx12RayTracingShaders::CompileClosestHitShader(LPCSTR Name, LPCSTR Entry)
{
    DX12_RAYTRACING_SHADER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Name = Name;
    desc.Entry = Entry;
    desc.Type = DRTS_CLOSEST_HIT;

    return CompileShader(desc);
}

ID3DBlob* dx12RayTracingShaders::CompileAnyHitShader(LPCSTR Name, LPCSTR Entry)
{
    DX12_RAYTRACING_SHADER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Name = Name;
    desc.Entry = Entry;
    desc.Type = DRTS_ANY_HIT;

    return CompileShader(desc);
}

ID3DBlob* dx12RayTracingShaders::CompileMissShader(LPCSTR Name, LPCSTR Entry)
{
    DX12_RAYTRACING_SHADER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Name = Name;
    desc.Entry = Entry;
    desc.Type = DRTS_MISS;

    return CompileShader(desc);
}

void dx12RayTracingShaders::AddHitGroup(const DX12_HIT_GROUP_DESC& Desc)
{
    if (m_hitGroupCount >= MaxHitGroups)
    {
        LOG("*ERROR* dx12RayTracingShaders: Maximum hit group count reached");
        return;
    }

    m_hitGroups[m_hitGroupCount++] = Desc;
}

HRESULT dx12RayTracingShaders::CreateLocalRootSignature()
{
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init_1_1(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSigDesc, &signature, &error);
    if (FAILED(hr))
    {
        LOG("*ERROR* dx12RayTracingShaders: Failed to serialize local root signature");
        return hr;
    }

    hr = HW12.m_pDevice->CreateRootSignature(0, signature->GetBufferPointer(),
                                              signature->GetBufferSize(),
                                              IID_PPV_ARGS(&m_localRootSignature));
    if (FAILED(hr))
    {
        LOG("*ERROR* dx12RayTracingShaders: Failed to create local root signature");
        return hr;
    }

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(signature->GetBufferSize());

    hr = HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_pLocalRootSignatureBuffer));
    if (FAILED(hr))
    {
        LOG("*ERROR* dx12RayTracingShaders: Failed to create local root signature buffer");
        return hr;
    }

    m_localRootSignatureGPUAddress = m_pLocalRootSignatureBuffer->GetGPUVirtualAddress();

    return S_OK;
}

ID3DBlob* dx12RayTracingShaders::CompileShader(const DX12_RAYTRACING_SHADER_DESC& Desc)
{
    LPCSTR target = nullptr;
    switch (Desc.Type)
    {
    case DRTS_RAYGEN: target = "lib_6_3"; break;
    case DRTS_CLOSEST_HIT: target = "lib_6_3"; break;
    case DRTS_ANY_HIT: target = "lib_6_3"; break;
    case DRTS_MISS: target = "lib_6_3"; break;
    default: target = "lib_6_3"; break;
    }

    DX12_SHADER_COMPILE_DESC compileDesc;
    ZeroMemory(&compileDesc, sizeof(compileDesc));
    compileDesc.Name = Desc.Name;
    compileDesc.Source = Desc.Source;
    compileDesc.SourceLength = Desc.SourceLength;
    compileDesc.Entry = Desc.Entry;
    compileDesc.Target = target;

    ComPtr<ID3DBlob> result;
    ComPtr<ID3DBlob> errors;

    HRESULT hr = ShaderCompiler12.CompileShader(compileDesc, result, errors);
    if (FAILED(hr))
    {
        if (errors)
        {
            LOG("*ERROR* dx12RayTracingShaders: Failed to compile %s: %ls",
                Desc.Name, (wchar_t*)errors->GetBufferPointer());
        }
        return nullptr;
    }

    return result.Detach();
}

dx12RayTracingShaders RayTracingShaders12;

#endif // USE_DX12
