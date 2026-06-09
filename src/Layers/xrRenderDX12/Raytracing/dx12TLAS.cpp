#include "../dx12stdafx.h"
#include "dx12TLAS.h"

#ifdef USE_DX12

dx12TLAS::dx12TLAS()
    : m_asGPUVirtualAddress(0),
      m_asSize(0),
      m_scratchSize(0),
      m_updateScratchSize(0),
      m_instanceCount(0)
{
}

dx12TLAS::~dx12TLAS()
{
    Destroy();
}

void dx12TLAS::Init(const DX12_TLAS_DESC& Desc)
{
    m_instanceCount = Desc.InstanceCount;
    m_buildFlags = Desc.BuildFlags;

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

    resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_updateScratchSize);
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_pUpdateScratchBuffer)));
}

void dx12TLAS::Build(ID3D12GraphicsCommandList4* pCommandList, ID3D12Resource* pInstanceBuffer)
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION resourceLocation =
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION_BUFFER;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_DESCRIPTOR geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_TYPE_INSTANCES;
    geometryDesc.InstanceDesc.Locations = pInstanceBuffer->GetGPUVirtualAddress();
    geometryDesc.InstanceDesc.DescCount = m_instanceCount;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESC asBuildDesc = {};
    asBuildDesc.DestAS.GPUVirtualAddress = m_asGPUVirtualAddress;
    asBuildDesc.SourceAS.GPUVirtualAddress = 0;
    asBuildDesc.Mode = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_MODE_BUILD;
    asBuildDesc.Flags = m_buildFlags;
    asBuildDesc.InputLayout.NumDescriptorTables = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].NumDescriptorArrays = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.NumDescriptorRanges = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.NumDescriptors = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].NumDescriptors = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorType = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESCRIPTOR_RANGE_TYPE_INSTANCES;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.pResourceLocation = &resourceLocation;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.pGeometryDescriptors = &geometryDesc;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESC scratchDesc = {};
    scratchDesc.DestAS.GPUVirtualAddress = m_pScratchBuffer->GetGPUVirtualAddress();

    pCommandList->BuildRaytracingAccelerationStructure(&asBuildDesc, 1, &scratchDesc);

    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pASBuffer.Get(),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
    pCommandList->ResourceBarrier(1, &barrier);
}

void dx12TLAS::BuildIncremental(ID3D12GraphicsCommandList4* pCommandList, ID3D12Resource* pInstanceBuffer)
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION resourceLocation =
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION_BUFFER;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_DESCRIPTOR geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_TYPE_INSTANCES;
    geometryDesc.InstanceDesc.Locations = pInstanceBuffer->GetGPUVirtualAddress();
    geometryDesc.InstanceDesc.DescCount = m_instanceCount;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESC asBuildDesc = {};
    asBuildDesc.DestAS.GPUVirtualAddress = m_asGPUVirtualAddress;
    asBuildDesc.SourceAS.GPUVirtualAddress = m_asGPUVirtualAddress;
    asBuildDesc.Mode = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_MODE_INCREMENTAL;
    asBuildDesc.Flags = m_buildFlags;
    asBuildDesc.InputLayout.NumDescriptorTables = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].NumDescriptorArrays = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.NumDescriptorRanges = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.NumDescriptors = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].NumDescriptors = 1;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorType = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESCRIPTOR_RANGE_TYPE_INSTANCES;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.pResourceLocation = &resourceLocation;
    asBuildDesc.InputLayout.DescriptorTables[0].DescriptorArray.DescriptorRange.AppendDescriptorRanges[0].DescriptorPoolImmedtiate.pGeometryDescriptors = &geometryDesc;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_DESC scratchDesc = {};
    scratchDesc.DestAS.GPUVirtualAddress = m_pUpdateScratchBuffer->GetGPUVirtualAddress();

    pCommandList->BuildRaytracingAccelerationStructure(&asBuildDesc, 1, &scratchDesc);
}

void dx12TLAS::Destroy()
{
    m_pASBuffer.Reset();
    m_pScratchBuffer.Reset();
    m_pUpdateScratchBuffer.Reset();
    m_asGPUVirtualAddress = 0;
    m_asSize = 0;
    m_scratchSize = 0;
    m_updateScratchSize = 0;
}

void dx12TLAS::QuerySizes(const DX12_TLAS_DESC& Desc)
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION resourceLocation =
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_RESOURCE_LOCATION_BUFFER;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_DESCRIPTOR geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_TYPE_INSTANCES;
    geometryDesc.InstanceDesc.DescCount = Desc.InstanceCount;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
    HW12.m_pDevice->GetRaytracingAccelerationStructurePrebuildInfo(
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
        &CD3DX12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS(
            Desc.BuildFlags,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_PREFER_FAST_TRACE,
            1,
            &geometryDesc,
            &resourceLocation),
        &prebuildInfo);

    m_asSize = prebuildInfo.AccelerationStructurePrebuildSize;
    m_scratchSize = prebuildInfo.BuildScratchBufferSize;
    m_updateScratchSize = prebuildInfo.UpdateScratchBufferSize;
}

#endif // USE_DX12
