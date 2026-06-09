#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12RaytracingGeometry.h"
#include "dx12BLAS.h"
#include "dx12TLAS.h"

struct DX12_SCENE_DESC
{
    UINT MaxStaticInstances;
    UINT MaxDynamicInstances;
    bool bUseIncrementalTLAS;
};

class dx12Scene
{
public:
    dx12Scene();
    ~dx12Scene();

    void Init(const DX12_SCENE_DESC& Desc);
    void Destroy();

    void BuildScene();
    void UpdateScene();

    void AddStaticMesh(const DX12_GEOMETRY_DESC& Geometry);
    void AddDynamicMesh(const DX12_GEOMETRY_DESC& Geometry);

    void BeginFrame();
    void EndFrame();

    D3D12_GPU_VIRTUAL_ADDRESS GetTLASGPUVirtualAddress() const { return m_pTLAS->GetASGPUVirtualAddress(); }
    dx12TLAS* GetTLAS() const { return m_pTLAS; }

    bool IsBuilt() const { return m_bBuilt; }
    bool NeedsUpdate() const { return m_bNeedsUpdate; }

    UINT GetStaticBLASCount() const { return m_staticBLASCount; }
    UINT GetDynamicBLASCount() const { return m_dynamicBLASCount; }

private:
    void BuildStaticBLAS();
    void BuildTLAS();
    void UploadInstanceData();

    dx12TLAS* m_pTLAS;

    ComPtr<ID3D12Resource> m_pStaticInstanceBuffer;
    ComPtr<ID3D12Resource> m_pDynamicInstanceBuffer;

    UINT m_staticBLASCount;
    UINT m_dynamicBLASCount;

    dx12BLAS* m_staticBLAS[2048];
    dx12BLAS* m_dynamicBLAS[2048];

    bool m_bBuilt;
    bool m_bNeedsUpdate;
    bool m_bUseIncrementalTLAS;

    DX12_SCENE_DESC m_desc;
};

extern dx12Scene Scene12;

#endif // USE_DX12
