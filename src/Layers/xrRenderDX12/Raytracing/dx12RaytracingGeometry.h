#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

enum DX12_GEOMETRY_TYPE
{
    DGT_STATIC_MESH,
    DGT_DYNAMIC_MESH,
    DGT_TERRAIN,
    DGT_INSTANCED
};

enum DX12_GEOMETRY_FLAGS
{
    DGF_OPAQUE = 1 << 0,
    DGF_TRANSPARENT = 1 << 1,
    DGF_NO_DUPLICATE_ANYHIT_INVOCATION = 1 << 2,
    DGF_OPAQUE_AND_NO_DUPLICATE_ANYHIT_INVOCATION = 1 << 3,
};

struct DX12_GEOMETRY_INSTANCE
{
    D3D12_RAYTRACING_INSTANCE_DESC InstanceDesc;
    UINT MeshID;
    UINT InstanceID;
    DX12_GEOMETRY_TYPE Type;
    bool bDirty;
};

struct DX12_GEOMETRY_DESC
{
    ID3D12Resource* pVertexBuffer;
    ID3D12Resource* pIndexBuffer;
    ID3D12Resource* pTransformBuffer;

    DXGI_FORMAT VertexFormat;
    UINT VertexCount;
    UINT VertexStride;

    DXGI_FORMAT IndexFormat;
    UINT IndexCount;

    UINT TransformCount;
    DX12_GEOMETRY_TYPE Type;
    u32 Flags;
};

class dx12RaytracingGeometry
{
public:
    dx12RaytracingGeometry();
    ~dx12RaytracingGeometry();

    void AddGeometry(const DX12_GEOMETRY_DESC& Desc);
    void RemoveGeometry(UINT GeometryIndex);

    UINT GetGeometryCount() const { return m_geometryCount; }
    const DX12_GEOMETRY_DESC* GetGeometry(UINT Index) const { return &m_geometries[Index]; }

    void AddInstance(const DX12_GEOMETRY_INSTANCE& Instance);
    void RemoveInstance(UINT InstanceIndex);
    void UpdateInstanceTransform(UINT InstanceIndex, const Fmatrix& Transform);

    UINT GetInstanceCount() const { return m_instanceCount; }
    DX12_GEOMETRY_INSTANCE* GetInstance(UINT Index) { return &m_instances[Instance]; }
    const DX12_GEOMETRY_INSTANCE* GetInstance(UINT Index) const { return &m_instances[Instance]; }

    void MarkInstanceDirty(UINT InstanceIndex);
    void MarkAllInstancesDirty();
    bool AnyInstanceDirty() const;

    void UploadInstanceTransforms();
    ID3D12Resource* GetInstanceTransformBuffer() const { return m_pInstanceTransformBuffer.Get(); }

private:
    void ResizeInstanceBuffer();

    DX12_GEOMETRY_DESC m_geometries[2048];
    UINT m_geometryCount;

    DX12_GEOMETRY_INSTANCE m_instances[65536];
    UINT m_instanceCount;

    ComPtr<ID3D12Resource> m_pInstanceTransformBuffer;
    ComPtr<ID3D12Resource> m_pInstanceTransformStaging;

    D3D12_RAYTRACING_INSTANCE_DESC* m_pTransformMapping;
    bool m_transformsDirty;

    static const UINT MaxGeometries = 2048;
    static const UINT MaxInstances = 65536;
    static const UINT InitialTransformBufferSize = 65536 * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
};

extern dx12RaytracingGeometry RaytracingGeometry12;

#endif // USE_DX12
