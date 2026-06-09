#include "dx12stdafx.h"
#include "dx12ConstantBuffer.h"

#ifdef USE_DX12

dx12ConstantBuffer::dx12ConstantBuffer(const D3D12_SHADER_DESC& ShaderDesc, u32 ConstantBufferIndex)
    : m_pBuffer(nullptr),
      m_pStagingBuffer(nullptr),
      m_uiBufferSize(0),
      m_pBufferData(nullptr),
      m_bChanged(false)
{
    const D3D12_SHADER_BUFFER_DESC& bufDesc = ShaderDesc.pConstantBuffers[ConstantBufferIndex];

    m_strBufferName = bufDesc.Name;
    m_eBufferType = bufDesc.Type;
    m_uiBufferSize = align_up(bufDesc.Size, cbAlignment);

    m_MembersList.resize(bufDesc.Variables);
    m_MembersNames.resize(bufDesc.Variables);

    m_uiMembersCRC = 0;

    HRESULT hr = HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(m_uiBufferSize),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        nullptr,
        IID_PPV_ARGS(&m_pBuffer));
    R_CHK(hr);

    hr = HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(m_uiBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pStagingBuffer));
    R_CHK(hr);

    hr = m_pStagingBuffer->Map(0, nullptr, &m_pBufferData);
    R_CHK(hr);
    ZeroMemory(m_pBufferData, m_uiBufferSize);
}

dx12ConstantBuffer::~dx12ConstantBuffer()
{
    if (m_pStagingBuffer && m_pBufferData)
    {
        m_pStagingBuffer->Unmap(0, nullptr);
        m_pBufferData = nullptr;
    }
}

bool dx12ConstantBuffer::Similar(dx12ConstantBuffer& _in)
{
    return (m_uiMembersCRC == _in.m_uiMembersCRC) &&
           (m_uiBufferSize == _in.m_uiBufferSize);
}

void dx12ConstantBuffer::Flush()
{
    if (m_bChanged && m_pBuffer && m_pStagingBuffer)
    {
        HW12.m_pCommandList->CopyResource(m_pBuffer.Get(), m_pStagingBuffer.Get());
        m_bChanged = false;
    }
}

Fvector4* dx12ConstantBuffer::Access(u16 offset)
{
    return reinterpret_cast<Fvector4*>(reinterpret_cast<u8*>(m_pBufferData) + offset);
}

void dx12ConstantBuffer::set(R_constant* C, R_constant_load& L, const Fmatrix& A)
{
    Fvector4* p = Access(L.Offset);
    p[0].set(A.m[0][0], A.m[0][1], A.m[0][2], A.m[0][3]);
    p[1].set(A.m[1][0], A.m[1][1], A.m[1][2], A.m[1][3]);
    p[2].set(A.m[2][0], A.m[2][1], A.m[2][2], A.m[2][3]);
    p[3].set(A.m[3][0], A.m[3][1], A.m[3][2], A.m[3][3]);
    m_bChanged = true;
}

void dx12ConstantBuffer::set(R_constant* C, R_constant_load& L, const Fvector4& A)
{
    Fvector4* p = Access(L.Offset);
    *p = A;
    m_bChanged = true;
}

void dx12ConstantBuffer::set(R_constant* C, R_constant_load& L, float A)
{
    Fvector4* p = Access(L.Offset);
    p->set(A, 0, 0, 0);
    m_bChanged = true;
}

void dx12ConstantBuffer::set(R_constant* C, R_constant_load& L, int A)
{
    Fvector4* p = Access(L.Offset);
    p->set(float(A), 0, 0, 0);
    m_bChanged = true;
}

void dx12ConstantBuffer::seta(R_constant* C, R_constant_load& L, u32 e, const Fmatrix& A)
{
    Fvector4* p = Access(L.Offset + L.Stride * e);
    p[0].set(A.m[0][0], A.m[0][1], A.m[0][2], A.m[0][3]);
    p[1].set(A.m[1][0], A.m[1][1], A.m[1][2], A.m[1][3]);
    p[2].set(A.m[2][0], A.m[2][1], A.m[2][2], A.m[2][3]);
    p[3].set(A.m[3][0], A.m[3][1], A.m[3][2], A.m[3][3]);
    m_bChanged = true;
}

void dx12ConstantBuffer::seta(R_constant* C, R_constant_load& L, u32 e, const Fvector4& A)
{
    Fvector4* p = Access(L.Offset + L.Stride * e);
    *p = A;
    m_bChanged = true;
}

void* dx12ConstantBuffer::AccessDirect(R_constant_load& L, u32 DataSize)
{
    return reinterpret_cast<u8*>(m_pBufferData) + L.Offset;
}

#endif // USE_DX12
