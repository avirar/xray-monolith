#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

struct R_constant;
struct R_constant_load;

class dx12ConstantBuffer : public xr_resource_named
{
public:
    dx12ConstantBuffer(const D3D12_SHADER_DESC& ShaderDesc, u32 ConstantBufferIndex);
    ~dx12ConstantBuffer();

    bool Similar(dx12ConstantBuffer& _in);
    ID3D12Resource* GetBuffer() { return m_pBuffer.Get(); }
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() { return m_pBuffer->GetGPUVirtualAddress(); }

    void Flush();

    void set(R_constant* C, R_constant_load& L, const Fmatrix& A);
    void set(R_constant* C, R_constant_load& L, const Fvector4& A);
    void set(R_constant* C, R_constant_load& L, float A);
    void set(R_constant* C, R_constant_load& L, int A);

    void seta(R_constant* C, R_constant_load& L, u32 e, const Fmatrix& A);
    void seta(R_constant* C, R_constant_load& L, u32 e, const Fvector4& A);

    void* AccessDirect(R_constant_load& L, u32 DataSize);

private:
    Fvector4* Access(u16 offset);

private:
    shared_str m_strBufferName;
    D3D12_CBUFFER_TYPE m_eBufferType;

    u32 m_uiMembersCRC;
    xr_vector<D3D12_SHADER_TYPE_DESC> m_MembersList;
    xr_vector<shared_str> m_MembersNames;

    ComPtr<ID3D12Resource> m_pBuffer;
    ComPtr<ID3D12Resource> m_pStagingBuffer;
    u32 m_uiBufferSize;
    void* m_pBufferData;
    bool m_bChanged;

    static const u32 lineSize = sizeof(Fvector4);
    static const u32 cbAlignment = 256;

    dx12ConstantBuffer(const dx12ConstantBuffer&);
    dx12ConstantBuffer& operator=(dx12ConstantBuffer&);
};

typedef resptr_core<dx12ConstantBuffer, resptr_base<dx12ConstantBuffer>> ref_cbuffer12;

#endif // USE_DX12
