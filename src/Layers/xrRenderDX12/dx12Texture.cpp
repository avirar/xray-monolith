#include "stdafx.h"
#include "dx12stdafx.h"
#include "dx12Texture.h"

#ifdef USE_DX12

dx12Texture::dx12Texture()
    : m_pResource(nullptr),
      m_width(0),
      m_height(0),
      m_depth(0),
      m_mipLevels(1),
      m_format(DXGI_FORMAT_UNKNOWN),
      m_dimension(D3D12_RESOURCE_DIMENSION_UNKNOWN),
      m_size(0)
{
}

dx12Texture::~dx12Texture()
{
    Destroy();
}

bool dx12Texture::Create2D(u32 Width, u32 Height, DXGI_FORMAT Format, u32 MipLevels,
                           D3D12_RESOURCE_FLAGS Flags, const D3D12_SUBRESOURCE_DATA* InitialData)
{
    m_width = Width;
    m_height = Height;
    m_depth = 1;
    m_mipLevels = MipLevels;
    m_format = Format;
    m_dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    if (m_mipLevels == 0)
    {
        m_mipLevels = static_cast<u32>(std::floor(std::log2(std::max(Width, Height)))) + 1;
    }

    D3D12_RESOURCE_DESC1 desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = Width;
    desc.Height = Height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = m_mipLevels;
    desc.Format = Format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = Flags;

    HRESULT hr = HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_pResource));

    if (FAILED(hr))
    {
        Msg("! dx12Texture::Create2D failed: 0x%08X", hr);
        return false;
    }

    if (InitialData)
    {
        UpdateSubresources(m_pResource.Get(), InitialData, 0, 0, m_mipLevels * m_depth);
        m_pResource->Barrier(CD3DX12_RESOURCE_BARRIER::Transition(
            m_pResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }

    D3D12_RESOURCE_DESC rdesc = m_pResource->GetDesc();
    m_size = rdesc.Width * rdesc.Height * GetFormatSize(Format);

    return true;
}

bool dx12Texture::CreateBuffer(u64 Size, DXGI_FORMAT Format, const D3D12_SUBRESOURCE_DATA* InitialData)
{
    m_format = Format;
    m_dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    m_size = Size;

    D3D12_RESOURCE_DESC1 desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = Size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = Format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAGS_NONE;

    HRESULT hr = HW12.m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_pResource));

    if (FAILED(hr))
    {
        Msg("! dx12Texture::CreateBuffer failed: 0x%08X", hr);
        return false;
    }

    if (InitialData)
    {
        UpdateSubresources(m_pResource.Get(), InitialData, 0, 0, 1);
    }

    return true;
}

void dx12Texture::Destroy()
{
    m_pResource.Reset();
    m_width = 0;
    m_height = 0;
    m_depth = 0;
    m_mipLevels = 1;
    m_format = DXGI_FORMAT_UNKNOWN;
    m_dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;
    m_size = 0;
}

void dx12Texture::CreateSRV(ID3D12DescriptorHeap* Heap, D3D12_CPU_DESCRIPTOR_HANDLE Handle, DXGI_FORMAT ViewFormat)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if (m_dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
    {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Format = ViewFormat != DXGI_FORMAT_UNKNOWN ? ViewFormat : m_format;
        srvDesc.Texture2D.MipLevels = m_mipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    }
    else if (m_dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = ViewFormat != DXGI_FORMAT_UNKNOWN ? ViewFormat : m_format;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = static_cast<u32>(m_size / GetFormatSize(srvDesc.Format));
        srvDesc.Buffer.StructureByteStride = 0;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    }

    HW12.m_pDevice->CreateShaderResourceView(m_pResource.Get(), &srvDesc, Handle);
}

void dx12Texture::CreateUAV(ID3D12DescriptorHeap* Heap, D3D12_CPU_DESCRIPTOR_HANDLE Handle, DXGI_FORMAT ViewFormat)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

    if (m_dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
    {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Format = ViewFormat != DXGI_FORMAT_UNKNOWN ? ViewFormat : m_format;
        uavDesc.Texture2D.MipSlice = 0;
        uavDesc.Texture2D.PlaneSlice = 0;
    }
    else if (m_dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Format = ViewFormat != DXGI_FORMAT_UNKNOWN ? ViewFormat : m_format;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = static_cast<u32>(m_size / GetFormatSize(uavDesc.Format));
        uavDesc.Buffer.StructureByteStride = 0;
        uavDesc.Buffer.CounterOffsetInBytes = 0;
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    }

    HW12.m_pDevice->CreateUnorderedAccessView(m_pResource.Get(), nullptr, &uavDesc, Handle);
}

void dx12Texture::GenerateMips()
{
    if (m_mipLevels <= 1)
        return;

    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pResource.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_GENERATE_MIPS);

    HW12.m_pCommandList->ResourceBarrier(1, &barrier);
    HW12.m_pCommandList->GenerateMips(m_pResource.Get());

    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pResource.Get(),
        D3D12_RESOURCE_STATE_GENERATE_MIPS,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    HW12.m_pCommandList->ResourceBarrier(1, &barrier);
}

void dx12Texture::Map(u32 Subresource, D3D12_MAP MapType, D3D12_RANGE* ReadRange, void** Data)
{
    HRESULT hr = m_pResource->Map(Subresource, MapType, ReadRange, Data);
    R_CHK(hr);
}

void dx12Texture::Unmap(u32 Subresource, const D3D12_RANGE* WrittenRange)
{
    m_pResource->Unmap(Subresource, WrittenRange);
}

u32 GetFormatSize(DXGI_FORMAT Format)
{
    switch (Format)
    {
    case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
    case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
    case DXGI_FORMAT_R32G32_FLOAT: return 8;
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT: return 4;
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_SNORM: return 8;
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SINT:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SRGB: return 2;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_TYPELESS: return 4;
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_TYPELESS: return 1;
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_Y416:
    case DXGI_FORMAT_YUY2: return 4;
    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216: return 8;
    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016: return 2;
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_D32_FLOAT: return 4;
    case DXGI_FORMAT_D24_UNORM_S8_UINT: return 4;
    case DXGI_FORMAT_D16_UNORM: return 2;
    default: return 4;
    }
}

#endif // USE_DX12
