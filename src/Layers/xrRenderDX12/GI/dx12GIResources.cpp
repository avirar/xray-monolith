#include "../dx12stdafx.h"
#include "dx12GIResources.h"

#ifdef USE_DX12

dx12GIResources::dx12GIResources()
    : m_pIrradianceResource(nullptr),
      m_pAOResource(nullptr),
      m_Width(0),
      m_Height(0),
      m_Initialized(false)
{
}

dx12GIResources::~dx12GIResources()
{
    if (m_pIrradianceResource)
        m_pIrradianceResource->Release();
    if (m_pAOResource)
        m_pAOResource->Release();
}

void dx12GIResources::Init()
{
    m_Initialized = true;
}

void dx12GIResources::Resize(u32 Width, u32 Height)
{
    if (m_Width == Width && m_Height == Height)
        return;

    m_Width = Width;
    m_Height = Height;

    CreateIrradianceTexture(Width, Height);
    CreateAOTexture(Width, Height);
}

void dx12GIResources::CreateIrradianceTexture(u32 Width, u32 Height)
{
    if (m_pIrradianceResource)
    {
        m_pIrradianceResource->Release();
        m_pIrradianceResource = nullptr;
    }

    D3D12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        Width, Height, 1, 1,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    memcpy(clearValue.Color, clearColor, sizeof(clearColor));

    R_CHK(HW12.GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &clearValue,
        IID_PPV_ARGS(&m_pIrradianceResource)
    ));

    if (!m_pIrradianceResource)
    {
        LOG("*ERROR* dx12GIResources: Failed to create irradiance texture");
        return;
    }

    m_pIrradianceResource->SetName(L"GI_Irradiance");

    HW12.GetDevice()->CreateUnorderedAccessView(
        m_pIrradianceResource, nullptr, &m_IrradianceUAV);

    HW12.GetDevice()->CreateShaderResourceView(
        m_pIrradianceResource, nullptr, &m_IrradianceSRV);

    HW12.GetDevice()->CreateRenderTargetView(
        m_pIrradianceResource, nullptr, &m_IrradianceRTV);
}

void dx12GIResources::CreateAOTexture(u32 Width, u32 Height)
{
    if (m_pAOResource)
    {
        m_pAOResource->Release();
        m_pAOResource = nullptr;
    }

    D3D12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R16_FLOAT,
        Width, Height, 1, 1,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R16_FLOAT;
    float clearColor[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
    memcpy(clearValue.Color, clearColor, sizeof(clearColor));

    R_CHK(HW12.GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &clearValue,
        IID_PPV_ARGS(&m_pAOResource)
    ));

    if (!m_pAOResource)
    {
        LOG("*ERROR* dx12GIResources: Failed to create AO texture");
        return;
    }

    m_pAOResource->SetName(L"GI_AO");

    HW12.GetDevice()->CreateUnorderedAccessView(
        m_pAOResource, nullptr, &m_AOUAV);

    HW12.GetDevice()->CreateShaderResourceView(
        m_pAOResource, nullptr, &m_AOSRV);

    HW12.GetDevice()->CreateRenderTargetView(
        m_pAOResource, nullptr, &m_AORTV);
}

dx12GIResources GIResources12;

#endif // USE_DX12
