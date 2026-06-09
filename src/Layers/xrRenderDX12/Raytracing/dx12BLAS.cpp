#include "../dx12stdafx.h"
#include "dx12BLAS.h"

#ifdef USE_DX12

dx12BLAS::dx12BLAS()
    : m_asGPUVirtualAddress(0),
      m_asSize(0),
      m_compactedSize(0),
      m_scratchSize(0),
      m_geometryCount(0)
{
    ZeroMemory(&m_desc, sizeof(m_desc));
    ZeroMemory(&m_geometryDescriptors, sizeof(m_geometryDescriptors));
}

dx12BLAS::~dx12BLAS()
{
    Destroy();
}

void dx12BLAS::Init(const DX12_BLAS_DESC& Desc)
{
    m_desc = Desc;
    m_geometryCount = Desc.GeometryCount;
    m_buildFlags = Desc.BuildFlags;

    if (m_geometryCount > 0 && m_geometryCount <= 2048)
    {
        CopyMemory(m_geometryDescriptors, Desc.pGeometries,
                   m_geometryCount * sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_DESCRIPTOR));
    }

    QuerySizes(Desc);

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_asSize);

    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_pASBuffer)));

    m_asGPUVirtualAddress = m_pASBuffer->GetGPUVirtualAddress();

    resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_scratchSize);
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_pScratchBuffer)));
}

void dx12BLAS::Build(ID3D12GraphicsCommandList4* pCommandList)
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION resourceLocation =
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION_INLINE;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESC asBuildDesc = {};
    asBuildDesc.DestAS.GPUVirtualAddress = m_asGPUVirtualAddress;
    asBuildDesc.SourceAS.GPUVirtualAddress = 0;
    asBuildDesc.Mode = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_MODE_BUILD;
    asBuildDesc.Flags = m_buildFlags;
    asBuildDesc.InputLayout.NumDescriptorTables = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].NumDescriptorArrays = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.NumDescriptorRanges = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.NumDescriptors = m_geometryCount;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].NumDescriptors = m_geometryCount;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorType = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESCRIPTOR_RANGE_TYPE_TRIANGLES;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.IndexBuffer = 0;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.VertexBufferStride = 0;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.TransformCount = 0;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.pResourceLocation = &resourceLocation;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.pGeometryDescriptors = m_geometryDescriptors;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESC scratchDesc = {};
    scratchDesc.DestAS.GPUVirtualAddress = m_pScratchBuffer->GetGPUVirtualAddress();

    pCommandList->BuildRaytracingAccelerationStructure(&asBuildDesc, 1, &scratchDesc);

    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pASBuffer.Get(),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
    pCommandList->ResourceBarrier(1, &barrier);
}

void dx12BLAS::BuildCompacted(ID3D12GraphicsCommandList4* pCommandList, UINT64 CompactedSize)
{
    m_compactedSize = CompactedSize;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION resourceLocation =
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION_INLINE;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESC asBuildDesc = {};
    asBuildDesc.DestAS.GPUVirtualAddress = m_asGPUVirtualAddress;
    asBuildDesc.SourceAS.GPUVirtualAddress = 0;
    asBuildDesc.Mode = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_MODE_COMPACT;
    asBuildDesc.Flags = m_buildFlags;
    asBuildDesc.InputLayout.NumDescriptorTables = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].NumDescriptorArrays = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.NumDescriptorRanges = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.NumDescriptors = m_geometryCount;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].NumDescriptors = m_geometryCount;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorType = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESCRIPTOR_RANGE_TYPE_TRIANGLES;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.IndexBuffer = 0;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.VertexBufferStride = 0;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.TransformCount = 0;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.pResourceLocation = &resourceLocation;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.pGeometryDescriptors = m_geometryDescriptors;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESC scratchDesc = {};
    scratchDesc.DestAS.GPUVirtualAddress = m_pScratchBuffer->GetGPUVirtualAddress();

    pCommandList->BuildRaytracingAccelerationStructure(&asBuildDesc, 1, &scratchDesc);
}

void dx12BLAS::Destroy()
{
    m_pASBuffer.Reset();
    m_pScratchBuffer.Reset();
    m_asGPUVirtualAddress = 0;
    m_asSize = 0;
    m_compactedSize = 0;
    m_scratchSize = 0;
}

void dx12BLAS::QuerySizes(const DX12_BLAS_DESC& Desc)
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION resourceLocation =
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION_INLINE;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
    HW12.m_pDevice->GetRaytracingAccelerationStructurePrebuildInfo(
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
        &CD3DX12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS(
            Desc.BuildFlags,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_PREFER_FAST_TRACE,
            1,
            &Desc.pGeometries[0],
            &resourceLocation),
        &prebuildInfo);

    m_asSize = prebuildInfo.AccelerationStructurePrebuildSize;
    m_compactedSize = prebuildInfo.CompactedAccelerationStructureSize;
    m_scratchSize = prebuildInfo.UpdateScratchBufferSize;
}

dx12BLASManager::dx12BLASManager()
    : m_blasCount(0)
{
    ZeroMemory(m_blases, sizeof(m_blases));
}

dx12BLASManager::~dx12BLASManager()
{
    for (UINT i = 0; i < m_blasCount; i++)
    {
        delete m_blases[i];
    }
}

void dx12BLASManager::Init()
{
}

dx12BLAS* dx12BLASManager::CreateBLAS(const DX12_BLAS_DESC& Desc)
{
    if (m_blasCount >= MaxBLAS)
    {
        LOG("*ERROR* dx12BLASManager: Maximum BLAS count reached");
        return nullptr;
    }

    dx12BLAS* pBLAS = new dx12BLAS();
    pBLAS->Init(Desc);
    m_blases[m_blasCount++] = pBLAS;
    return pBLAS;
}

void dx12BLASManager::DestroyBLAS(dx12BLAS* pBLAS)
{
    for (UINT i = 0; i < m_blasCount; i++)
    {
        if (m_blases[i] == pBLAS)
        {
            m_blases[i] = m_blases[m_blasCount - 1];
            m_blasCount--;
            delete pBLAS;
            return;
        }
    }
}

void dx12BLASManager::BuildAll(ID3D12GraphicsCommandList4* pCommandList)
{
    for (UINT i = 0; i < m_blasCount; i++)
    {
        m_blases[i]->Build(pCommandList);
    }
}

dx12BLASManager BLASManager12;

#endif // USE_DX12
