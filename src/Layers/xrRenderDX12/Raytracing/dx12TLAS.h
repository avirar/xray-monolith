#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12BLAS.h"

struct DX12_TLAS_DESC
{
    UINT InstanceCount;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS BuildFlags;
};

class dx12TLAS
{
public:
    dx12TLAS();
    ~dx12TLAS();

    void Init(const DX12_TLAS_DESC& Desc);
    void Build(ID3D12GraphicsCommandList4* pCommandList, ID3D12Resource* pInstanceBuffer);
    void BuildIncremental(ID3D12GraphicsCommandList4* pCommandList, ID3D12Resource* pInstanceBuffer);
    void Destroy();

    UINT64 GetASSize() const { return m_asSize; }
    UINT64 GetScratchSize() const { return m_scratchSize; }
    UINT64 GetUpdateScratchSize() const { return m_updateScratchSize; }

    ID3D12Resource* GetASBuffer() const { return m_pASBuffer.Get(); }
    ID3D12Resource* GetScratchBuffer() const { return m_pScratchBuffer.Get(); }
    ID3D12Resource* GetUpdateScratchBuffer() const { return m_pUpdateScratchBuffer.Get(); }
    D3D12_GPU_VIRTUAL_ADDRESS GetASGPUVirtualAddress() const { return m_asGPUVirtualAddress; }

    void QuerySizes(const DX12_TLAS_DESC& Desc);

private:
    ComPtr<ID3D12Resource> m_pASBuffer;
    ComPtr<ID3D12Resource> m_pScratchBuffer;
    ComPtr<ID3D12Resource> m_pUpdateScratchBuffer;

    D3D12_GPU_VIRTUAL_ADDRESS m_asGPUVirtualAddress;

    UINT64 m_asSize;
    UINT64 m_scratchSize;
    UINT64 m_updateScratchSize;

    UINT m_instanceCount;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_buildFlags;
};

#endif // USE_DX12
