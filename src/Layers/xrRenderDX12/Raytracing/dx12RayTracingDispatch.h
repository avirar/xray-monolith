#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "dx12RayTracingPipeline.h"
#include "dx12Scene.h"

struct DX12_DISPATCH_RAYS_DESC
{
    ID3D12Resource* pRayGenShaderTable;
    UINT RayGenShaderTableStride;
    D3D12_GPU_VIRTUAL_ADDRESS RayGenShaderTableVirtualAddress;
    UINT RayGenShaderTableSize;

    ID3D12Resource* pMissShaderTable;
    UINT MissShaderTableStride;
    D3D12_GPU_VIRTUAL_ADDRESS MissShaderTableVirtualAddress;
    UINT MissShaderTableSize;

    ID3D12Resource* pHitGroupTable;
    UINT HitGroupTableStride;
    D3D12_GPU_VIRTUAL_ADDRESS HitGroupTableVirtualAddress;
    UINT HitGroupTableSize;

    ID3D12Resource* pCallableShaderTable;
    UINT CallableShaderTableStride;
    D3D12_GPU_VIRTUAL_ADDRESS CallableShaderTableVirtualAddress;
    UINT CallableShaderTableSize;

    UINT Width;
    UINT Height;
    UINT Depth;
};

class dx12RayTracingDispatch
{
public:
    dx12RayTracingDispatch();
    ~dx12RayTracingDispatch();

    void Init();

    void SetPipeline(ID3D12StateObject* pPipeline);
    void SetRootSignature(ID3D12RootSignature* pRootSignature);

    void DispatchRays(const DX12_DISPATCH_RAYS_DESC& Desc);
    void DispatchRaysDesc(const D3D12_DISPATCH_RAYS_DESC& Desc);

    void DispatchGI(u32 Width, u32 Height);
    void DispatchShadow(u32 Width, u32 Height);
    void DispatchReflection(u32 Width, u32 Height, UINT SamplesPerPixel);

    void SetRootConstantBufferView(u32 ParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
    void SetRootDescriptorTable(u32 ParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
    void SetRoot32BitConstant(u32 ParameterIndex, u32 Data, u32 AddressOffset = 0);

    ID3D12StateObject* GetPipeline() const { return m_pPipeline; }

private:
    void ApplyDescriptorHeaps();

    ID3D12StateObject* m_pPipeline;
    ID3D12RootSignature* m_pRootSignature;

    bool m_pipelineDirty;
    bool m_rootSignatureDirty;
};

extern dx12RayTracingDispatch RayTracingDispatch12;

#endif // USE_DX12
