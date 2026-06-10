#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12RaytracingGeometry.h"
#include <algorithm>

#ifdef USE_DX12

dx12RaytracingGeometry::dx12RaytracingGeometry()
    : m_geometryCount(0),
      m_instanceCount(0),
      m_pTransformMapping(nullptr),
      m_transformsDirty(false)
{
    ZeroMemory(&m_geometries, sizeof(m_geometries));
    ZeroMemory(&m_instances, sizeof(m_instances));
}

dx12RaytracingGeometry::~dx12RaytracingGeometry()
{
    if (m_pTransformMapping)
    {
        m_pInstanceTransformBuffer->Unmap(0, nullptr);
        m_pTransformMapping = nullptr;
    }
}

void dx12RaytracingGeometry::AddGeometry(const DX12_GEOMETRY_DESC& Desc)
{
    if (m_geometryCount >= MaxGeometries)
    {
        LOG("*ERROR* dx12RaytracingGeometry: Maximum geometry count reached");
        return;
    }

    m_geometries[m_geometryCount++] = Desc;
}

void dx12RaytracingGeometry::RemoveGeometry(UINT GeometryIndex)
{
    if (GeometryIndex >= m_geometryCount) return;

    m_geometries[GeometryIndex] = m_geometries[m_geometryCount - 1];
    m_geometryCount--;
}

void dx12RaytracingGeometry::AddInstance(const DX12_GEOMETRY_INSTANCE& Instance)
{
    if (m_instanceCount >= MaxInstances)
    {
        LOG("*ERROR* dx12RaytracingGeometry: Maximum instance count reached");
        return;
    }

    m_instances[m_instanceCount++] = Instance;
    m_transformsDirty = true;
}

void dx12RaytracingGeometry::RemoveInstance(UINT InstanceIndex)
{
    if (InstanceIndex >= m_instanceCount) return;

    m_instances[InstanceIndex] = m_instances[m_instanceCount - 1];
    m_instanceCount--;
    m_transformsDirty = true;
}

void dx12RaytracingGeometry::UpdateInstanceTransform(UINT InstanceIndex, const Fmatrix& Transform)
{
    if (InstanceIndex >= m_instanceCount) return;

    D3D12_RAYTRACING_INSTANCE_DESC& instance = m_instances[InstanceIndex].InstanceDesc;
    // DX12 uses Transform[3][4] (row-major 3x4), not InstanceMatrix[16]
    // Map column-major Fmatrix to row-major Transform: Transform[row][col] = m[col][row]
    instance.Transform[0][0] = Transform.m[0][0];
    instance.Transform[0][1] = Transform.m[1][0];
    instance.Transform[0][2] = Transform.m[2][0];
    instance.Transform[0][3] = Transform.m[3][0];
    instance.Transform[1][0] = Transform.m[0][1];
    instance.Transform[1][1] = Transform.m[1][1];
    instance.Transform[1][2] = Transform.m[2][1];
    instance.Transform[1][3] = Transform.m[3][1];
    instance.Transform[2][0] = Transform.m[0][2];
    instance.Transform[2][1] = Transform.m[1][2];
    instance.Transform[2][2] = Transform.m[2][2];
    instance.Transform[2][3] = Transform.m[3][2];

    m_instances[InstanceIndex].bDirty = true;
    m_transformsDirty = true;
}

void dx12RaytracingGeometry::MarkInstanceDirty(UINT InstanceIndex)
{
    if (InstanceIndex < m_instanceCount)
    {
        m_instances[InstanceIndex].bDirty = true;
        m_transformsDirty = true;
    }
}

void dx12RaytracingGeometry::MarkAllInstancesDirty()
{
    for (UINT i = 0; i < m_instanceCount; i++)
    {
        m_instances[i].bDirty = true;
    }
    m_transformsDirty = true;
}

bool dx12RaytracingGeometry::AnyInstanceDirty() const
{
    return m_transformsDirty;
}

void dx12RaytracingGeometry::UploadInstanceTransforms()
{
    if (!m_transformsDirty || m_instanceCount == 0) return;

    UINT requiredSize = m_instanceCount * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
    if (!m_pInstanceTransformBuffer || requiredSize > InitialTransformBufferSize)
    {
        ResizeInstanceBuffer();
    }

    if (!m_pTransformMapping)
    {
        HRESULT hr = m_pInstanceTransformBuffer->Map(0, nullptr, (void**)&m_pTransformMapping);
        if (FAILED(hr))
        {
            LOG("*ERROR* dx12RaytracingGeometry: Failed to map transform buffer");
            return;
        }
    }

    for (UINT i = 0; i < m_instanceCount; i++)
    {
        m_pTransformMapping[i] = m_instances[i].InstanceDesc;
    }

    m_pInstanceTransformBuffer->Unmap(0, nullptr);
    m_pTransformMapping = nullptr;
    m_transformsDirty = false;

    for (UINT i = 0; i < m_instanceCount; i++)
    {
        m_instances[i].bDirty = false;
    }
}

void dx12RaytracingGeometry::ResizeInstanceBuffer()
{
    m_pInstanceTransformBuffer.Reset();
    m_pTransformMapping = nullptr;

    UINT bufferSize = std::max(InitialTransformBufferSize, m_instanceCount * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_pInstanceTransformBuffer)));
}

dx12RaytracingGeometry RaytracingGeometry12;

#endif // USE_DX12
