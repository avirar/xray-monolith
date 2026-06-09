#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12Texture.h"

class dx12GIResources
{
public:
    dx12GIResources();
    ~dx12GIResources();

    void Init();
    void Resize(u32 Width, u32 Height);

    // Irradiance texture - stores per-pixel GI color
    ID3D12Resource* GetIrradianceResource() const { return m_pIrradianceResource; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetIrradianceSRV() const { return m_IrradianceSRV; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetIrradianceRTV() const { return m_IrradianceRTV; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetIrradianceUAV() const { return m_IrradianceUAV; }

    // AO texture - stores per-pixel ambient occlusion
    ID3D12Resource* GetAOResource() const { return m_pAOResource; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAOSRV() const { return m_AOSRV; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAORTV() const { return m_AORTV; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetAOUAV() const { return m_AOUAV; }

    u32 GetWidth() const { return m_Width; }
    u32 GetHeight() const { return m_Height; }

private:
    void CreateIrradianceTexture(u32 Width, u32 Height);
    void CreateAOTexture(u32 Width, u32 Height);

    ID3D12Resource* m_pIrradianceResource;
    D3D12_CPU_DESCRIPTOR_HANDLE m_IrradianceSRV;
    D3D12_CPU_DESCRIPTOR_HANDLE m_IrradianceRTV;
    D3D12_CPU_DESCRIPTOR_HANDLE m_IrradianceUAV;

    ID3D12Resource* m_pAOResource;
    D3D12_CPU_DESCRIPTOR_HANDLE m_AOSRV;
    D3D12_CPU_DESCRIPTOR_HANDLE m_AORTV;
    D3D12_CPU_DESCRIPTOR_HANDLE m_AOUAV;

    u32 m_Width;
    u32 m_Height;
    bool m_Initialized;
};

extern dx12GIResources GIResources12;

#endif // USE_DX12
