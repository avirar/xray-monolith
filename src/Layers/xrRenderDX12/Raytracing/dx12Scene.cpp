#include "../dx12stdafx.h"
#include "dx12Scene.h"

#ifdef USE_DX12

dx12Scene::dx12Scene()
    : m_pTLAS(nullptr),
      m_staticBLASCount(0),
      m_dynamicBLASCount(0),
      m_bBuilt(false),
      m_bNeedsUpdate(false),
      m_bUseIncrementalTLAS(false)
{
    ZeroMemory(&m_desc, sizeof(m_desc));
    ZeroMemory(m_staticBLAS, sizeof(m_staticBLAS));
    ZeroMemory(m_dynamicBLAS, sizeof(m_dynamicBLAS));
}

dx12Scene::~dx12Scene()
{
    Destroy();
}

void dx12Scene::Init(const DX12_SCENE_DESC& Desc)
{
    m_desc = Desc;
    m_bUseIncrementalTLAS = Desc.bUseIncrementalTLAS;

    DX12_TLAS_DESC tlasDesc;
    tlasDesc.InstanceCount = Desc.MaxStaticInstances + Desc.MaxDynamicInstances;
    tlasDesc.BuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    m_pTLAS = new dx12TLAS();
    m_pTLAS->Init(tlasDesc);
}

void dx12Scene::Destroy()
{
    for (UINT i = 0; i < m_staticBLASCount; i++)
    {
        delete m_staticBLAS[i];
    }
    for (UINT i = 0; i < m_dynamicBLASCount; i++)
    {
        delete m_dynamicBLAS[i];
    }

    if (m_pTLAS)
    {
        delete m_pTLAS;
        m_pTLAS = nullptr;
    }

    m_staticBLASCount = 0;
    m_dynamicBLASCount = 0;
    m_bBuilt = false;
    m_bNeedsUpdate = false;
}

void dx12Scene::AddStaticMesh(const DX12_GEOMETRY_DESC& Geometry)
{
    if (m_staticBLASCount >= 2048)
    {
        LOG("*ERROR* dx12Scene: Maximum static BLAS count reached");
        return;
    }

    DX12_BLAS_DESC blasDesc;
    blasDesc.GeometryCount = 1;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_DESCRIPTOR geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Triangles.VertexBufferLocation = Geometry.pVertexBuffer->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexFormat = Geometry.VertexFormat;
    geometryDesc.Triangles.VertexCount = Geometry.VertexCount;
    geometryDesc.Triangles.VertexBufferStride = Geometry.VertexStride;
    geometryDesc.Triangles.IndexBufferLocation = Geometry.pIndexBuffer->GetGPUVirtualAddress();
    geometryDesc.Triangles.IndexFormat = Geometry.IndexFormat;
    geometryDesc.Triangles.IndexCount = Geometry.IndexCount;
    geometryDesc.Triangles.TransformCount = 0;
    geometryDesc.Triangles.Flags = static_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS>(Geometry.Flags);

    blasDesc.pGeometries = &geometryDesc;
    blasDesc.BuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    blasDesc.MaxVertexCount = Geometry.VertexCount;
    blasDesc.MaxPrimitiveCount = Geometry.IndexCount / 3;

    m_staticBLAS[m_staticBLASCount] = BLASManager12.CreateBLAS(blasDesc);

    RaytracingGeometry12.AddGeometry(Geometry);
    m_staticBLASCount++;
    m_bNeedsUpdate = true;
}

void dx12Scene::AddDynamicMesh(const DX12_GEOMETRY_DESC& Geometry)
{
    if (m_dynamicBLASCount >= 2048)
    {
        LOG("*ERROR* dx12Scene: Maximum dynamic BLAS count reached");
        return;
    }

    DX12_BLAS_DESC blasDesc;
    blasDesc.GeometryCount = 1;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_DESCRIPTOR geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Triangles.VertexBufferLocation = Geometry.pVertexBuffer->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexFormat = Geometry.VertexFormat;
    geometryDesc.Triangles.VertexCount = Geometry.VertexCount;
    geometryDesc.Triangles.VertexBufferStride = Geometry.VertexStride;
    geometryDesc.Triangles.IndexBufferLocation = Geometry.pIndexBuffer->GetGPUVirtualAddress();
    geometryDesc.Triangles.IndexFormat = Geometry.IndexFormat;
    geometryDesc.Triangles.IndexCount = Geometry.IndexCount;
    geometryDesc.Triangles.TransformCount = 0;
    geometryDesc.Triangles.Flags = static_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS>(Geometry.Flags);

    blasDesc.pGeometries = &geometryDesc;
    blasDesc.BuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
    blasDesc.MaxVertexCount = Geometry.VertexCount;
    blasDesc.MaxPrimitiveCount = Geometry.IndexCount / 3;

    m_dynamicBLAS[m_dynamicBLASCount] = BLASManager12.CreateBLAS(blasDesc);

    RaytracingGeometry12.AddGeometry(Geometry);
    m_dynamicBLASCount++;
    m_bNeedsUpdate = true;
}

void dx12Scene::BuildScene()
{
    ID3D12GraphicsCommandList4* pCommandList = CommandManager12.GetGraphicsCommandList();

    BuildStaticBLAS();
    BuildTLAS();

    m_bBuilt = true;
    m_bNeedsUpdate = false;
}

void dx12Scene::UpdateScene()
{
    if (!m_bBuilt || !m_bNeedsUpdate) return;

    ID3D12GraphicsCommandList4* pCommandList = CommandManager12.GetGraphicsCommandList();

    RaytracingGeometry12.UploadInstanceTransforms();
    UploadInstanceData();

    if (m_bUseIncrementalTLAS)
    {
        m_pTLAS->BuildIncremental(pCommandList, RaytracingGeometry12.GetInstanceTransformBuffer());
    }
    else
    {
        m_pTLAS->Build(pCommandList, RaytracingGeometry12.GetInstanceTransformBuffer());
    }

    m_bNeedsUpdate = false;
}

void dx12Scene::BeginFrame()
{
    if (RaytracingGeometry12.AnyInstanceDirty())
    {
        m_bNeedsUpdate = true;
    }
}

void dx12Scene::EndFrame()
{
}

D3D12_GPU_VIRTUAL_ADDRESS dx12Scene::GetTLASGPUVirtualAddress() const
{
    return m_pTLAS->GetASGPUVirtualAddress();
}

void dx12Scene::BuildStaticBLAS()
{
    ID3D12GraphicsCommandList4* pCommandList = CommandManager12.GetGraphicsCommandList();

    for (UINT i = 0; i < m_staticBLASCount; i++)
    {
        m_staticBLAS[i]->Build(pCommandList);
    }
}

void dx12Scene::BuildTLAS()
{
    ID3D12GraphicsCommandList4* pCommandList = CommandManager12.GetGraphicsCommandList();

    UploadInstanceData();

    D3D12_RESOURCE_BARRIER barriers[2] = {};
    barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pTLAS->GetASBuffer(),
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        D3D12_RESOURCE_STATE_GENERIC_READ);
    barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pTLAS->GetScratchBuffer(),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
    pCommandList->ResourceBarrier(_countof(barriers), barriers);

    m_pTLAS->Build(pCommandList, RaytracingGeometry12.GetInstanceTransformBuffer());

    barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pTLAS->GetASBuffer(),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
    barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pTLAS->GetScratchBuffer(),
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        D3D12_RESOURCE_STATE_GENERIC_READ);
    pCommandList->ResourceBarrier(_countof(barriers), barriers);
}

void dx12Scene::UploadInstanceData()
{
    UINT totalInstances = RaytracingGeometry12.GetInstanceCount();
    if (totalInstances == 0) return;

    UINT bufferSize = totalInstances * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    ComPtr<ID3D12Resource> stagingBuffer;
    R_CHK(HW12.m_pDevice->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&stagingBuffer)));

    void* pData;
    R_CHK(stagingBuffer->Map(0, nullptr, &pData));

    for (UINT i = 0; i < totalInstances; i++)
    {
        D3D12_RAYTRACING_INSTANCE_DESC* pInstance = static_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(pData);
        pInstance[i] = RaytracingGeometry12.GetInstance(i)->InstanceDesc;
    }

    stagingBuffer->Unmap(0, nullptr);

    if (!m_pStaticInstanceBuffer || bufferSize > m_pStaticInstanceBuffer->GetDesc().Width)
    {
        m_pStaticInstanceBuffer.Reset();
        R_CHK(HW12.m_pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&m_pStaticInstanceBuffer)));
    }

    Backend12.GetCommandList()->CopyResource(m_pStaticInstanceBuffer.Get(), stagingBuffer.Get());
}

dx12Scene Scene12;

#endif // USE_DX12
