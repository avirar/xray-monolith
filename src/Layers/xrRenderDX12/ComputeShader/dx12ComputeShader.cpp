#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12ComputeShader.h"

#ifdef USE_DX12

dx12ComputeShader::dx12ComputeShader()
    : m_cbCount(0),
      m_textureCount(0),
      m_uavCount(0),
      m_samplerCount(0),
      m_dirty(true)
{
    ZeroMemory(&m_constantBuffers, sizeof(m_constantBuffers));
    ZeroMemory(&m_textures, sizeof(m_textures));
    ZeroMemory(&m_uavs, sizeof(m_uavs));
    ZeroMemory(m_samplers, sizeof(m_samplers));
    ZeroMemory(&m_cbRegister, sizeof(m_cbRegister));
    ZeroMemory(&m_textureRegister, sizeof(m_textureRegister));
    ZeroMemory(&m_uavRegister, sizeof(m_uavRegister));
    ZeroMemory(&m_samplerRegister, sizeof(m_samplerRegister));
    ZeroMemory(m_name, sizeof(m_name));
}

dx12ComputeShader::~dx12ComputeShader()
{
    Destroy();
}

void dx12ComputeShader::Init(LPCSTR Name, ID3D12RootSignature* RootSignature)
{
    strconcat(sizeof(m_name), m_name, Name);

    m_pRootSignature = RootSignature;

    char csPath[512];
    strconcat(sizeof(csPath), csPath, Render->getShaderPath(), Name, ".cs");

    ComPtr<ID3DBlob> shaderBlob = ShaderCompiler12.CompileFromFile(csPath, "main", "cs_6_0");
    if (shaderBlob)
    {
        m_pCS = shaderBlob;
        BuildPSO();
    }
}

void dx12ComputeShader::Destroy()
{
    m_pCS.Reset();
    m_pRootSignature.Reset();
    m_pPSO.Reset();

    for (UINT i = 0; i < _countof(m_constantBuffers); i++)
        m_constantBuffers[i].Reset();
    for (UINT i = 0; i < _countof(m_textures); i++)
        m_textures[i].Reset();
    for (UINT i = 0; i < _countof(m_uavs); i++)
        m_uavs[i].Reset();

    m_cbCount = 0;
    m_textureCount = 0;
    m_uavCount = 0;
    m_samplerCount = 0;
}

void dx12ComputeShader::SetConstantBuffer(u32 Register, ID3D12Resource* Buffer)
{
    m_constantBuffers[Register] = Buffer;
    m_cbRegister[m_cbCount++] = Register;
    m_dirty = true;
}

void dx12ComputeShader::SetTexture(u32 Register, ID3D12Resource* Texture, const D3D12_SHADER_RESOURCE_VIEW_DESC* SRVDesc)
{
    m_textures[Register] = Texture;
    m_textureRegister[m_textureCount++] = Register;
    m_dirty = true;
}

void dx12ComputeShader::SetSampler(u32 Register, ID3D12SamplerState* Sampler)
{
    m_samplers[Register] = Sampler;
    m_samplerRegister[m_samplerCount++] = Register;
    m_dirty = true;
}

void dx12ComputeShader::SetUAV(u32 Register, ID3D12Resource* Resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* UAVDesc)
{
    m_uavs[Register] = Resource;
    m_uavRegister[m_uavCount++] = Register;
    m_dirty = true;
}

void dx12ComputeShader::Dispatch(u32 ThreadGroupCountX, u32 ThreadGroupCountY, u32 ThreadGroupCountZ)
{
    if (!m_pPSO) return;

    FlushConstants();
    BindResources();

    ID3D12DescriptorHeap* pHeaps[] = {
        DescriptorManager12.GetSRVUAVHeap(),
        DescriptorManager12.GetCBVHeap(),
        SamplerManager12.GetSamplerHeap()
    };
    Backend12.GetComputeCommandList()->SetDescriptorHeaps(_countof(pHeaps), pHeaps);

    Backend12.GetComputeCommandList()->SetPipelineState(m_pPSO.Get());
    Backend12.GetComputeCommandList()->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

    for (UINT i = 0; i < m_uavCount; i++)
    {
        Backend12.ResourceBarrier(m_uavs[m_uavRegister[i]].Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_READ);
    }
}

void dx12ComputeShader::BuildPSO()
{
    if (!m_pCS || !m_pRootSignature) return;

    D3D12_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.CS = CD3DX12_SHADER_BYTECODE(m_pCS);
    psoDesc.pRootSignature = m_pRootSignature.Get();
    psoDesc.NodeMask = 0;

    HRESULT hr = HW12.m_pDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pPSO));
    if (FAILED(hr))
    {
        LOG("*ERROR* dx12ComputeShader: Failed to create PSO for %s", m_name);
    }
}

void dx12ComputeShader::FlushConstants()
{
    for (UINT i = 0; i < m_cbCount; i++)
    {
        Backend12.SetComputeRootConstantBufferView(m_cbRegister[i],
            m_constantBuffers[m_cbRegister[i]]->GetGPUVirtualAddress());
    }
}

void dx12ComputeShader::BindResources()
{
    if (!m_dirty) return;

    if (m_pRootSignature)
    {
        Backend12.SetComputeRootSignature(m_pRootSignature.Get());
    }

    if (m_samplerCount > 0)
    {
        ID3D12SamplerState* samplerPtrs[16];
        for (UINT i = 0; i < m_samplerCount && i < 16; i++)
        {
            samplerPtrs[i] = m_samplers[m_samplerRegister[i]];
        }
        Backend12.SetComputeSamplers(0, m_samplerCount, samplerPtrs);
    }

    m_dirty = false;
}

dx12CSCompiler::dx12CSCompiler(dx12ComputeShader& Target)
    : m_target(Target)
{
}

dx12CSCompiler& dx12CSCompiler::Begin(LPCSTR Name)
{
    Compile(Name);
    return *this;
}

dx12CSCompiler& dx12CSCompiler::DefSampler(LPCSTR ResourceName)
{
    D3D12_SAMPLER_DESC desc = {};
    desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    desc.AddressU = desc.AddressV = desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc.MipLODBias = 0.0f;
    desc.MaxAnisotropy = 1;
    desc.ComparisonFunc = D3D12_COMPARISON_NEVER;
    desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] = desc.BorderColor[3] = 0.0f;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D12_FLOAT32_MAX;
    m_samplerDescs.push_back(desc);
    return *this;
}

dx12CSCompiler& dx12CSCompiler::DefSampler(LPCSTR ResourceName, const D3D12_SAMPLER_DESC& Desc)
{
    m_samplerDescs.push_back(Desc);
    return *this;
}

dx12CSCompiler& dx12CSCompiler::DefOutput(LPCSTR ResourceName, ID3D12Resource* Resource)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipSlice = 0;
    m_uavDescs.push_back(desc);
    m_target.SetUAV(m_uavDescs.size() - 1, Resource, &desc);
    return *this;
}

dx12CSCompiler& dx12CSCompiler::DefTexture(LPCSTR ResourceName, ID3D12Resource* Resource)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipLevels = 1;
    desc.Texture2D.MostDetailedMip = 0;
    desc.Texture2D.ResourceMinLODClamp = 0.0f;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    m_srvDescs.push_back(desc);
    m_target.SetTexture(m_srvDescs.size() - 1, Resource, &desc);
    return *this;
}

void dx12CSCompiler::End()
{
}

void dx12CSCompiler::Compile(LPCSTR Name)
{
    m_target.Init(Name, RootSignature12.GetRootSignature(RST_COMPUTE_HDAO));
}

#endif // USE_DX12
