#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12RaytracingGeometry.h"

struct DX12_BLAS_DESC
{
    UINT GeometryCount;
    const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_DESCRIPTOR* pGeometries;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS BuildFlags;
    UINT MaxVertexCount;
    UINT MaxPrimitiveCount;
};

class dx12BLAS
{
public:
    dx12BLAS();
    ~dx12BLAS();

    void Init(const DX12_BLAS_DESC& Desc);
    void Build(ID3D12GraphicsCommandList4* pCommandList);
    void BuildCompacted(ID3D12GraphicsCommandList4* pCommandList, UINT64 CompactedSize);
    void Destroy();

    UINT64 GetASSize() const { return m_asSize; }
    UINT64 GetCompactedSize() const { return m_compactedSize; }
    UINT64 GetScratchSize() const { return m_scratchSize; }

    ID3D12Resource* GetASBuffer() const { return m_pASBuffer.Get(); }
    ID3D12Resource* GetScratchBuffer() const { return m_pScratchBuffer.Get(); }
    D3D12_GPU_VIRTUAL_ADDRESS GetASGPUVirtualAddress() const { return m_asGPUVirtualAddress; }

    void QuerySizes(const DX12_BLAS_DESC& Desc);

private:
    void CreateBLASFromGeometry(const DX12_GEOMETRY_DESC& Geometry,
                                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_DESCRIPTOR& GeometryDesc,
                                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TRIANGLES& Triangles);

    ComPtr<ID3D12Resource> m_pASBuffer;
    ComPtr<ID3D12Resource> m_pScratchBuffer;

    D3D12_GPU_VIRTUAL_ADDRESS m_asGPUVirtualAddress;

    UINT64 m_asSize;
    UINT64 m_compactedSize;
    UINT64 m_scratchSize;

    UINT m_geometryCount;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_INPUT_GEOMETRY_DESCRIPTOR m_geometryDescriptors[2048];
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_buildFlags;

    DX12_BLAS_DESC m_desc;
};

class dx12BLASManager
{
public:
    dx12BLASManager();
    ~dx12BLASManager();

    void Init();

    dx12BLAS* CreateBLAS(const DX12_BLAS_DESC& Desc);
    void DestroyBLAS(dx12BLAS* pBLAS);

    UINT GetBLASCount() const { return m_blasCount; }
    dx12BLAS* GetBLAS(UINT Index) const { return m_blases[Index]; }

    void BuildAll(ID3D12GraphicsCommandList4* pCommandList);

private:
    dx12BLAS* m_blases[2048];
    UINT m_blasCount;

    static const UINT MaxBLAS = 2048;
};

extern dx12BLASManager BLASManager12;

#endif // USE_DX12
