#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12Texture.h"

class dx12ReflectionResources
{
public:
    dx12ReflectionResources();
    ~dx12ReflectionResources();

    void Init();
    void Resize(u32 Width, u32 Height);

    // Reflection color texture - stores per-pixel reflection color
    ID3D12Resource* GetReflectionResource() const { return m_pReflectionResource; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetReflectionSRV() const { return m_ReflectionSRV; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetReflectionUAV() const { return m_ReflectionUAV; }

    // Roughness mask texture - stores per-pixel roughness for quality scaling
    ID3D12Resource* GetRoughnessMaskResource() const { return m_pRoughnessMaskResource; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRoughnessMaskSRV() const { return m_RoughnessMaskSRV; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRoughnessMaskUAV() const { return m_RoughnessMaskUAV; }

    u32 GetWidth() const { return m_Width; }
    u32 GetHeight() const { return m_Height; }

private:
    void CreateReflectionTexture(u32 Width, u32 Height);
    void CreateRoughnessMaskTexture(u32 Width, u32 Height);

    ID3D12Resource* m_pReflectionResource;
    D3D12_CPU_DESCRIPTOR_HANDLE m_ReflectionSRV;
    D3D12_CPU_DESCRIPTOR_HANDLE m_ReflectionUAV;

    ID3D12Resource* m_pRoughnessMaskResource;
    D3D12_CPU_DESCRIPTOR_HANDLE m_RoughnessMaskSRV;
    D3D12_CPU_DESCRIPTOR_HANDLE m_RoughnessMaskUAV;

    u32 m_Width;
    u32 m_Height;
};

extern dx12ReflectionResources ReflectionResources12;

#endif // USE_DX12
