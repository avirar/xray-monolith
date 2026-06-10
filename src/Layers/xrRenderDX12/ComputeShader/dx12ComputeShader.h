#pragma once

#ifdef USE_DX12

#include "../DX12CommonTypes.h"

class dx12ComputeShader
{
public:
    dx12ComputeShader();
    ~dx12ComputeShader();

    void Init(LPCSTR Name, ID3D12RootSignature* RootSignature);
    void Destroy();

    void SetConstantBuffer(u32 Register, ID3D12Resource* Buffer);
    void SetTexture(u32 Register, ID3D12Resource* Texture, const D3D12_SHADER_RESOURCE_VIEW_DESC* SRVDesc = nullptr);
    void SetSampler(u32 Register, ID3D12SamplerState* Sampler);
    void SetUAV(u32 Register, ID3D12Resource* Resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* UAVDesc = nullptr);

    void Dispatch(u32 ThreadGroupCountX, u32 ThreadGroupCountY, u32 ThreadGroupCountZ);

    ID3D12PipelineState* GetPSO() const { return m_pPSO.Get(); }

    template<typename T>
    dx12ComputeShader& SetConstant(shared_str Name, const T& Value)
    {
        return *this;
    }

private:
    void BuildPSO();
    void FlushConstants();
    void BindResources();

    ComPtr<ID3DBlob> m_pCS;
    ComPtr<ID3D12RootSignature> m_pRootSignature;
    ComPtr<ID3D12PipelineState> m_pPSO;

    ComPtr<ID3D12Resource> m_constantBuffers[16];
    ComPtr<ID3D12Resource> m_textures[32];
    ComPtr<ID3D12Resource> m_uavs[16];
    ID3D12SamplerState* m_samplers[16];

    u32 m_cbRegister[16];
    u32 m_textureRegister[32];
    u32 m_uavRegister[16];
    u32 m_samplerRegister[16];

    UINT m_cbCount;
    UINT m_textureCount;
    UINT m_uavCount;
    UINT m_samplerCount;

    bool m_dirty;

    char m_name[256];
};

class dx12CSCompiler
{
public:
    dx12CSCompiler(dx12ComputeShader& Target);

    dx12CSCompiler& Begin(LPCSTR Name);
    dx12CSCompiler& DefSampler(LPCSTR ResourceName);
    dx12CSCompiler& DefSampler(LPCSTR ResourceName, const D3D12_SAMPLER_DESC& Desc);
    dx12CSCompiler& DefOutput(LPCSTR ResourceName, ID3D12Resource* Resource);
    dx12CSCompiler& DefTexture(LPCSTR ResourceName, ID3D12Resource* Resource);
    void End();

private:
    void Compile(LPCSTR Name);

    dx12ComputeShader& m_target;
    xr_vector<D3D12_SHADER_RESOURCE_VIEW_DESC> m_srvDescs;
    xr_vector<D3D12_UNORDERED_ACCESS_VIEW_DESC> m_uavDescs;
    xr_vector<D3D12_SAMPLER_DESC> m_samplerDescs;
};

#endif // USE_DX12
