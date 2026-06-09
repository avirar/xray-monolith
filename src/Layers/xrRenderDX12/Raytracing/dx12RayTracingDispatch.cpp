#include "../dx12stdafx.h"
#include "dx12RayTracingDispatch.h"

#ifdef USE_DX12

dx12RayTracingDispatch::dx12RayTracingDispatch()
    : m_pPipeline(nullptr),
      m_pRootSignature(nullptr),
      m_pipelineDirty(true),
      m_rootSignatureDirty(true)
{
}

dx12RayTracingDispatch::~dx12RayTracingDispatch()
{
}

void dx12RayTracingDispatch::Init()
{
}

void dx12RayTracingDispatch::SetPipeline(ID3D12StateObject* pPipeline)
{
    if (m_pPipeline != pPipeline)
    {
        m_pPipeline = pPipeline;
        m_pipelineDirty = true;
    }
}

void dx12RayTracingDispatch::SetRootSignature(ID3D12RootSignature* pRootSignature)
{
    if (m_pRootSignature != pRootSignature)
    {
        m_pRootSignature = pRootSignature;
        m_rootSignatureDirty = true;
    }
}

void dx12RayTracingDispatch::DispatchRays(const DX12_DISPATCH_RAYS_DESC& Desc)
{
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    dispatchDesc.RayGenerationShaderRecord = Desc.RayGenShaderTableVirtualAddress;
    dispatchDesc.RayGenerationShaderRecordStrideInBytes = Desc.RayGenShaderTableStride;
    dispatchDesc.RayGenerationShaderRecordSizeInBytes = Desc.RayGenShaderTableSize;
    dispatchDesc.MissShaderTable.StartAddress = Desc.MissShaderTableVirtualAddress;
    dispatchDesc.MissShaderTable.StrideInBytes = Desc.MissShaderTableStride;
    dispatchDesc.MissShaderTable.SizeInBytes = Desc.MissShaderTableSize;
    dispatchDesc.HitGroupTable.StartAddress = Desc.HitGroupTableVirtualAddress;
    dispatchDesc.HitGroupTable.StrideInBytes = Desc.HitGroupTableStride;
    dispatchDesc.HitGroupTable.SizeInBytes = Desc.HitGroupTableSize;
    dispatchDesc.CallableShaderTable.StartAddress = Desc.CallableShaderTableVirtualAddress;
    dispatchDesc.CallableShaderTable.StrideInBytes = Desc.CallableShaderTableStride;
    dispatchDesc.CallableShaderTable.SizeInBytes = Desc.CallableShaderTableSize;
    dispatchDesc.Width = Desc.Width;
    dispatchDesc.Height = Desc.Height;
    dispatchDesc.Depth = Desc.Depth;

    DispatchRaysDesc(dispatchDesc);
}

void dx12RayTracingDispatch::DispatchRaysDesc(const D3D12_DISPATCH_RAYS_DESC& Desc)
{
    if (!m_pPipeline)
    {
        LOG("*ERROR* dx12RayTracingDispatch: No pipeline set");
        return;
    }

    ApplyDescriptorHeaps();

    if (m_pipelineDirty)
    {
        Backend12.GetCommandList()->SetPipelineState1(m_pPipeline);
        m_pipelineDirty = false;
    }

    if (m_rootSignatureDirty)
    {
        Backend12.GetCommandList()->SetGraphicsRootSignature(m_pRootSignature);
        m_rootSignatureDirty = false;
    }

    Backend12.GetCommandList()->DispatchRays(&Desc);
}

void dx12RayTracingDispatch::DispatchGI(u32 Width, u32 Height)
{
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    dispatchDesc.RayGenerationShaderRecord = RayTracingPipeline12.GetRayGenSBTGPUVirtualAddress();
    dispatchDesc.RayGenerationShaderRecordStrideInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.RayGenerationShaderRecordSizeInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.MissShaderTable.StartAddress = RayTracingPipeline12.GetMissSBTGPUVirtualAddress();
    dispatchDesc.MissShaderTable.StrideInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.MissShaderTable.SizeInBytes = RayTracingPipeline12.GetShaderTableStride() * RayTracingPipeline12.GetMissRecordCount();
    dispatchDesc.HitGroupTable.StartAddress = RayTracingPipeline12.GetHitGroupSBTGPUVirtualAddress();
    dispatchDesc.HitGroupTable.StrideInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.HitGroupTable.SizeInBytes = RayTracingPipeline12.GetShaderTableStride() * RayTracingPipeline12.GetHitGroupRecordCount();
    dispatchDesc.CallableShaderTable.StartAddress = 0;
    dispatchDesc.CallableShaderTable.StrideInBytes = 0;
    dispatchDesc.CallableShaderTable.SizeInBytes = 0;
    dispatchDesc.Width = Width;
    dispatchDesc.Height = Height;
    dispatchDesc.Depth = 1;

    DispatchRaysDesc(dispatchDesc);
}

void dx12RayTracingDispatch::DispatchShadow(u32 Width, u32 Height)
{
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    dispatchDesc.RayGenerationShaderRecord = RayTracingPipeline12.GetRayGenSBTGPUVirtualAddress();
    dispatchDesc.RayGenerationShaderRecordStrideInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.RayGenerationShaderRecordSizeInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.MissShaderTable.StartAddress = RayTracingPipeline12.GetMissSBTGPUVirtualAddress();
    dispatchDesc.MissShaderTable.StrideInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.MissShaderTable.SizeInBytes = RayTracingPipeline12.GetShaderTableStride() * RayTracingPipeline12.GetMissRecordCount();
    dispatchDesc.HitGroupTable.StartAddress = RayTracingPipeline12.GetHitGroupSBTGPUVirtualAddress();
    dispatchDesc.HitGroupTable.StrideInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.HitGroupTable.SizeInBytes = RayTracingPipeline12.GetShaderTableStride() * RayTracingPipeline12.GetHitGroupRecordCount();
    dispatchDesc.CallableShaderTable.StartAddress = 0;
    dispatchDesc.CallableShaderTable.StrideInBytes = 0;
    dispatchDesc.CallableShaderTable.SizeInBytes = 0;
    dispatchDesc.Width = Width;
    dispatchDesc.Height = Height;
    dispatchDesc.Depth = 1;

    DispatchRaysDesc(dispatchDesc);
}

void dx12RayTracingDispatch::DispatchReflection(u32 Width, u32 Height, UINT SamplesPerPixel)
{
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    dispatchDesc.RayGenerationShaderRecord = RayTracingPipeline12.GetRayGenSBTGPUVirtualAddress();
    dispatchDesc.RayGenerationShaderRecordStrideInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.RayGenerationShaderRecordSizeInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.MissShaderTable.StartAddress = RayTracingPipeline12.GetMissSBTGPUVirtualAddress();
    dispatchDesc.MissShaderTable.StrideInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.MissShaderTable.SizeInBytes = RayTracingPipeline12.GetShaderTableStride() * RayTracingPipeline12.GetMissRecordCount();
    dispatchDesc.HitGroupTable.StartAddress = RayTracingPipeline12.GetHitGroupSBTGPUVirtualAddress();
    dispatchDesc.HitGroupTable.StrideInBytes = RayTracingPipeline12.GetShaderTableStride();
    dispatchDesc.HitGroupTable.SizeInBytes = RayTracingPipeline12.GetShaderTableStride() * RayTracingPipeline12.GetHitGroupRecordCount();
    dispatchDesc.CallableShaderTable.StartAddress = 0;
    dispatchDesc.CallableShaderTable.StrideInBytes = 0;
    dispatchDesc.CallableShaderTable.SizeInBytes = 0;
    dispatchDesc.Width = Width;
    dispatchDesc.Height = Height * SamplesPerPixel;
    dispatchDesc.Depth = 1;

    DispatchRaysDesc(dispatchDesc);
}

void dx12RayTracingDispatch::SetRootConstantBufferView(u32 ParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    Backend12.GetCommandList()->SetGraphicsRootConstantBufferView(ParameterIndex, BufferLocation);
}

void dx12RayTracingDispatch::SetRootDescriptorTable(u32 ParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    Backend12.GetCommandList()->SetGraphicsRootDescriptorTable(ParameterIndex, BaseDescriptor.ptr);
}

void dx12RayTracingDispatch::SetRoot32BitConstant(u32 ParameterIndex, u32 Data, u32 AddressOffset)
{
    Backend12.GetCommandList()->SetGraphicsRoot32BitConstant(ParameterIndex, Data, AddressOffset);
}

void dx12RayTracingDispatch::ApplyDescriptorHeaps()
{
    ID3D12DescriptorHeap* pHeaps[] = {
        DescriptorManager12.GetSRVUAVHeap(),
        DescriptorManager12.GetCBVHeap(),
        SamplerManager12.GetSamplerHeap()
    };
    Backend12.GetCommandList()->SetDescriptorHeaps(_countof(pHeaps), pHeaps);
}

dx12RayTracingDispatch RayTracingDispatch12;

#endif // USE_DX12
