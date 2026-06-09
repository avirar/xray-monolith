#include "../dx12stdafx.h"
#include "dx12ShadowPipeline.h"

#ifdef USE_DX12

dx12ShadowPipeline::dx12ShadowPipeline()
    : m_pShadowPipeline(nullptr),
      m_pSBTResource(nullptr),
      m_RayGenAddress(0),
      m_HitGroupAddress(0),
      m_MissAddress(0),
      m_ShaderTableStride(0),
      m_pRayGenBlob(nullptr),
      m_pClosestHitBlob(nullptr),
      m_pMissBlob(nullptr)
{
    m_Desc.SamplesPerPixel = 4;
    m_Desc.LightAreaSize = 0.1f;
    m_Desc.MaxRayDistance = 100.0f;
    m_Desc.RayBias = 0.001f;
    m_Desc.ShadowOpacity = 0.8f;
}

dx12ShadowPipeline::~dx12ShadowPipeline()
{
    if (m_pShadowPipeline)
        m_pShadowPipeline->Release();
    if (m_pSBTResource)
        m_pSBTResource->Release();
    if (m_pRayGenBlob)
        m_pRayGenBlob->Release();
    if (m_pClosestHitBlob)
        m_pClosestHitBlob->Release();
    if (m_pMissBlob)
        m_pMissBlob->Release();
}

void dx12ShadowPipeline::Init()
{
}

bool dx12ShadowPipeline::CompileShadowShader(
    const char* ShaderName,
    const char* EntryPoint,
    const char* Target,
    ID3DBlob*& ShaderBlob)
{
    const char* Args[] = {
        "-T", Target,
        "-E", EntryPoint,
        "-enable-16bit-types",
        "-O3"
    };

    ID3DBlob* pErrorBlob = nullptr;

    HRESULT hr = ShaderCompiler12.Compile(
        ShaderName, Args, _countof(Args),
        &ShaderBlob, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            LOG("*ERROR* dx12ShadowPipeline: Shader compilation failed for %s: %s",
                ShaderName, (const char*)pErrorBlob->GetBufferPointer());
            pErrorBlob->Release();
        }
        return false;
    }

    if (pErrorBlob)
        pErrorBlob->Release();

    return true;
}

bool dx12ShadowPipeline::CompileRayGenShadow()
{
    return CompileShadowShader(
        "shaders/raygen_shadow.rhs",
        "RayGenShadow",
        "lib_6_3",
        m_pRayGenBlob);
}

bool dx12ShadowPipeline::CompileClosestHitShadow()
{
    return CompileShadowShader(
        "shaders/closesthit_shadow.rhs",
        "ClosestHitShadow",
        "lib_6_3",
        m_pClosestHitBlob);
}

bool dx12ShadowPipeline::CompileMissShadow()
{
    return CompileShadowShader(
        "shaders/miss_shadow.rhs",
        "MissShadow",
        "lib_6_3",
        m_pMissBlob);
}

ID3D12StateObject* dx12ShadowPipeline::CreateShadowPipeline(const DX12_SHADOW_PIPELINE_DESC& Desc)
{
    m_Desc = Desc;

    if (!m_pRayGenBlob || !m_pClosestHitBlob || !m_pMissBlob)
    {
        LOG("*ERROR* dx12ShadowPipeline: Shadow shaders not compiled");
        return nullptr;
    }

    m_ShaderTableStride = HW12.GetDevice()->GetRaytracingShaderTableAlignment();

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC localRootSigDesc = {};
    localRootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;

    D3D12_ROOT_SIGNATURE1 localRootSig = {};
    localRootSig.NumParameters = 0;
    localRootSig.NumStaticSamplers = 0;
    localRootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    localRootSigDesc.Desc_1_1 = localRootSig;

    ComPtr<ID3DBlob> localRootSigBlob;
    ComPtr<ID3DBlob> errorBlob;
    R_CHK(D3D12SerializeVersionedRootSignature(&localRootSigDesc, &localRootSigBlob, &errorBlob));

    D3D12_STATE_SUBOBJECT subobjects[6];

    D3D12_RAYTRACING_PIPELINE_CONFIG1 rtConfig = {};
    rtConfig.MaxPayloadSizeInBytes = sizeof(float); // Shadow factor
    rtConfig.MaxAttributeSizeInBytesInAllHitGroups = sizeof(float);
    subobjects[0].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1;
    subobjects[0].pDescription = &rtConfig;

    subobjects[1].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    subobjects[1].pDescription = &localRootSigBlob->GetBufferPointer();

    D3D12_STATE_OBJECT_DESC_V1::CD3DX12_DXIL_LIBRARY_SUBOBJECT raygenLib;
    raygenLib.SetDXILLibrary(m_pRayGenBlob);
    subobjects[2].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    subobjects[2].pDescription = raygenLib;

    D3D12_STATE_OBJECT_DESC_V1::CD3DX12_DXIL_LIBRARY_SUBOBJECT hitLib;
    hitLib.SetDXILLibrary(m_pClosestHitBlob);
    subobjects[3].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    subobjects[3].pDescription = hitLib;

    D3D12_STATE_OBJECT_DESC_V1::CD3DX12_DXIL_LIBRARY_SUBOBJECT missLib;
    missLib.SetDXILLibrary(m_pMissBlob);
    subobjects[4].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    subobjects[4].pDescription = missLib;

    D3D12_STATE_OBJECT_DESC_V1::CD3DX12_HIT_GROUP_SUBOBJECT hitGroup;
    hitGroup.SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
    hitGroup.SetHitGroupExport("ClosestHitShadow");
    subobjects[5].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    subobjects[5].pDescription = hitGroup;

    D3D12_STATE_OBJECT_DESC pipelineDesc = {};
    pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    pipelineDesc.NumSubobjects = _countof(subobjects);
    pipelineDesc.pSubobjects = subobjects;

    ComPtr<ID3D12StateObject> pPipeline;
    R_CHK(HW12.GetDevice()->CreateStateObject(&pipelineDesc, IID_PPV_ARGS(&pPipeline)));

    if (!pPipeline)
    {
        LOG("*ERROR* dx12ShadowPipeline: Failed to create shadow pipeline");
        return nullptr;
    }

    if (m_pShadowPipeline)
        m_pShadowPipeline->Release();

    m_pShadowPipeline = pPipeline.Detach();

    BuildShadowSBT();

    return m_pShadowPipeline;
}

void dx12ShadowPipeline::BuildShadowSBT()
{
    if (!m_pShadowPipeline || !m_pRayGenBlob || !m_pClosestHitBlob || !m_pMissBlob)
        return;

    UINT stride = m_ShaderTableStride;
    UINT totalRecords = 3;
    UINT bufferSize = stride * totalRecords;

    D3D12_RESOURCE_DESC sbtDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    R_CHK(HW12.GetDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &sbtDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pSBTResource)
    ));

    if (!m_pSBTResource)
    {
        LOG("*ERROR* dx12ShadowPipeline: Failed to create SBT buffer");
        return;
    }

    m_pSBTResource->SetName(L"Shadow_SBT");

    ComPtr<ID3D12Resource> pStaging;
    D3D12_RESOURCE_DESC stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    D3D12_HEAP_PROPERTIES stagingProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    R_CHK(HW12.GetDevice()->CreateCommittedResource(
        &stagingProps,
        D3D12_HEAP_FLAG_NONE,
        &stagingDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&pStaging)
    ));

    void* pMapped = nullptr;
    R_CHK(pStaging->Map(0, nullptr, &pMapped));

    D3D12_SHADER_BINDING_TABLE_RECORD* pRecords = (D3D12_SHADER_BINDING_TABLE_RECORD*)pMapped;

    pRecords[0].ShaderID = { 0, 0, 0, 0 };
    pRecords[0].DispatchRPCHints = { 0, 0, 0, 0 };

    pRecords[1].ShaderID = { 0, 0, 0, 0 };
    pRecords[1].DispatchRPCHints = { 0, 0, 0, 0 };

    pRecords[2].ShaderID = { 0, 0, 0, 0 };
    pRecords[2].DispatchRPCHints = { 0, 0, 0, 0 };

    pStaging->Unmap(0, nullptr);

    Backend12.GetCommandList()->CopyResource(m_pSBTResource, pStaging.Get());

    m_RayGenAddress = m_pSBTResource->GetGPUVirtualAddress();
    m_HitGroupAddress = m_RayGenAddress + stride;
    m_MissAddress = m_HitGroupAddress + stride;
}

dx12ShadowPipeline ShadowPipeline12;

#endif // USE_DX12
