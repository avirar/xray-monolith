#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

class dx12Texture
{
public:
    dx12Texture();
    ~dx12Texture();

    bool Create2D(u32 Width, u32 Height, DXGI_FORMAT Format, u32 MipLevels,
                  D3D12_RESOURCE_FLAGS Flags, const D3D12_SUBRESOURCE_DATA* InitialData);

    bool CreateBuffer(u64 Size, DXGI_FORMAT Format, const D3D12_SUBRESOURCE_DATA* InitialData);

    void Destroy();

    ID3D12Resource* GetResource() const { return m_pResource.Get(); }
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return m_pResource->GetGPUVirtualAddress(); }

    void CreateSRV(ID3D12DescriptorHeap* Heap, D3D12_CPU_DESCRIPTOR_HANDLE Handle, DXGI_FORMAT ViewFormat = DXGI_FORMAT_UNKNOWN);
    void CreateUAV(ID3D12DescriptorHeap* Heap, D3D12_CPU_DESCRIPTOR_HANDLE Handle, DXGI_FORMAT ViewFormat = DXGI_FORMAT_UNKNOWN);

    u32 GetWidth() const { return m_width; }
    u32 GetHeight() const { return m_height; }
    u32 GetDepth() const { return m_depth; }
    u32 GetMipLevels() const { return m_mipLevels; }
    DXGI_FORMAT GetFormat() const { return m_format; }
    D3D12_RESOURCE_DIMENSION GetDimension() const { return m_dimension; }
    u64 GetSize() const { return m_size; }

    void GenerateMips();

    void Map(u32 Subresource, D3D12_MAP MapType, D3D12_RANGE* ReadRange, void** Data);
    void Unmap(u32 Subresource, const D3D12_RANGE* WrittenRange);

private:
    ComPtr<ID3D12Resource> m_pResource;

    u32 m_width;
    u32 m_height;
    u32 m_depth;
    u32 m_mipLevels;
    DXGI_FORMAT m_format;
    D3D12_RESOURCE_DIMENSION m_dimension;
    u64 m_size;
};

typedef resptr_core<dx12Texture, resptr_base<dx12Texture>> ref_texture12;

#endif // USE_DX12
