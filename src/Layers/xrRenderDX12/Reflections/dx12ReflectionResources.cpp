#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12ReflectionResources.h"

#ifdef USE_DX12

dx12ReflectionResources::dx12ReflectionResources()
    : m_pReflectionResource(nullptr),
      m_pRoughnessMaskResource(nullptr),
      m_Width(0),
      m_Height(0)
{
}

dx12ReflectionResources::~dx12ReflectionResources()
{
    if (m_pReflectionResource)
        m_pReflectionResource->Release();
    if (m_pRoughnessMaskResource)
        m_pRoughnessMaskResource->Release();
}

void dx12ReflectionResources::Init()
{
}

void dx12ReflectionResources::Resize(u32 Width, u32 Height)
{
    if (m_Width == Width && m_Height == Height)
        return;

    m_Width = Width;
    m_Height = Height;

    CreateReflectionTexture(Width, Height);
    CreateRoughnessMaskTexture(Width, Height);
}

void dx12ReflectionResources::CreateReflectionTexture(u32 Width, u32 Height)
{
    if (m_pReflectionResource)
    {
        m_pReflectionResource->Release();
        m_pReflectionResource = nullptr;
    }

    D3D12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        Width, Height, 1, 1,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );

    R_CHK(HW12.GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_pReflectionResource)
    ));

    if (!m_pReflectionResource)
    {
        LOG("*ERROR* dx12ReflectionResources: Failed to create reflection texture");
        return;
    }

    m_pReflectionResource->SetName(L"Reflection_Color");

    HW12.GetDevice()->CreateUnorderedAccessView(
        m_pReflectionResource, nullptr, &m_ReflectionUAV);

    HW12.GetDevice()->CreateShaderResourceView(
        m_pReflectionResource, nullptr, &m_ReflectionSRV);
}

void dx12ReflectionResources::CreateRoughnessMaskTexture(u32 Width, u32 Height)
{
    if (m_pRoughnessMaskResource)
    {
        m_pRoughnessMaskResource->Release();
        m_pRoughnessMaskResource = nullptr;
    }

    D3D12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R16_FLOAT,
        Width, Height, 1, 1,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );

    R_CHK(HW12.GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_pRoughnessMaskResource)
    ));

    if (!m_pRoughnessMaskResource)
    {
        LOG("*ERROR* dx12ReflectionResources: Failed to create roughness mask texture");
        return;
    }

    m_pRoughnessMaskResource->SetName(L"Reflection_RoughnessMask");

    HW12.GetDevice()->CreateUnorderedAccessView(
        m_pRoughnessMaskResource, nullptr, &m_RoughnessMaskUAV);

    HW12.GetDevice()->CreateShaderResourceView(
        m_pRoughnessMaskResource, nullptr, &m_RoughnessMaskSRV);
}

dx12ReflectionResources ReflectionResources12;

#endif // USE_DX12
