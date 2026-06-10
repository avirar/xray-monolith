#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12ShaderCompiler.h"

#ifdef USE_DX12

#include <d3dcompiler.h>
#include <dxcapi.h>

dx12ShaderCompiler::dx12ShaderCompiler()
    : m_dxcModule(nullptr),
      m_pDXCLibrary(nullptr),
      m_pDXCCompiler(nullptr)
{
}

dx12ShaderCompiler::~dx12ShaderCompiler()
{
    Destroy();
}

void dx12ShaderCompiler::Init()
{
    LoadDXC();
}

void dx12ShaderCompiler::Destroy()
{
    m_pDXCCompiler.Reset();
    m_pDXCLibrary.Reset();

    if (m_dxcModule)
    {
        FreeLibrary(m_dxcModule);
        m_dxcModule = nullptr;
    }

    m_shaderCache.clear();
}

HRESULT dx12ShaderCompiler::CompileShader(const DX12_SHADER_COMPILE_DESC& Desc, ComPtr<ID3DBlob>& Result, ComPtr<ID3DBlob>& Errors)
{
    u64 key = xxhash64(Desc.Source, Desc.SourceLength, 0);
    key ^= xxhash64(Desc.Target, strlen(Desc.Target), key);
    key ^= xxhash64(Desc.Defines, Desc.Defines ? strlen(Desc.Defines) : 0, key);

    xr_map<u64, ComPtr<ID3DBlob>>::iterator it = m_shaderCache.find(key);
    if (it != m_shaderCache.end())
    {
        Result = it->second;
        return S_OK;
    }

    HRESULT hr = S_OK;

    if (m_pDXCCompiler)
    {
        hr = CompileDXC(Desc, Result, Errors);
    }
    else
    {
        hr = CompileFXC(Desc, Result, Errors);
    }

    if (SUCCEEDED(hr) && Result)
    {
        if (m_shaderCache.size() >= MaxCacheSize)
        {
            m_shaderCache.erase(m_shaderCache.begin());
        }
        m_shaderCache[key] = Result;
    }

    return hr;
}

HRESULT dx12ShaderCompiler::CompileDXC(const DX12_SHADER_COMPILE_DESC& Desc, ComPtr<ID3DBlob>& Result, ComPtr<ID3DBlob>& Errors)
{
    ComPtr<IDxcBlobEncoding> sourceBlob;
    HRESULT hr = m_pDXCLibrary->CreateBlobWithEncodingFromPinned(
        Desc.Source, Desc.SourceLength, CP_UTF8, &sourceBlob);
    if (FAILED(hr))
        return hr;

    xr_vector<LPCSTR> defines;
    BuildDefines(Desc, defines);

    ComPtr<IDxcOperationResult> result;
    hr = m_pDXCCompiler->Compile(
        sourceBlob.Get(),
        Desc.Name,
        defines.size() > 0 ? defines.data() : nullptr,
        static_cast<UINT>(defines.size()),
        Desc.Defines,
        Desc.Entry,
        Desc.Target,
        nullptr, 0,
        nullptr, 0,
        &result);

    if (SUCCEEDED(hr))
    {
        hr = result->GetStatus(&hr);
        if (SUCCEEDED(hr))
        {
            result->GetResult(&Result);
        }
        else
        {
            result->GetErrorBuffer(&Errors);
        }
    }

    return hr;
}

HRESULT dx12ShaderCompiler::CompileFXC(const DX12_SHADER_COMPILE_DESC& Desc, ComPtr<ID3DBlob>& Result, ComPtr<ID3DBlob>& Errors)
{
    ComPtr<IDxcIncludeHandler> includeHandler;

    HRESULT hr = D3DCompile(
        Desc.Source,
        Desc.SourceLength,
        Desc.Name,
        Desc.Defines ? &CD3DX12_SHADER_MACRO(Desc.Defines) : nullptr,
        includeHandler.Get(),
        Desc.Entry,
        Desc.Target,
        Desc.Flags,
        0,
        &Result,
        &Errors);

    return hr;
}

bool dx12ShaderCompiler::IsDXCAvailable() const
{
    return m_pDXCCompiler != nullptr;
}

ComPtr<ID3DBlob> dx12ShaderCompiler::CompileFromFile(LPCSTR FilePath, LPCSTR Entry, LPCSTR Target, LPCSTR Defines)
{
    DX12_SHADER_COMPILE_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.Name = FilePath;
    desc.Entry = Entry ? Entry : "main";
    desc.Target = Target;
    desc.Defines = Defines;

    ComPtr<ID3DBlob> result;
    ComPtr<ID3DBlob> errors;

    HRESULT hr = CompileShader(desc, result, errors);
    if (FAILED(hr))
    {
        if (errors)
        {
            LOG("*ERROR* dx12ShaderCompiler: %ls", (wchar_t*)errors->GetBufferPointer());
        }
        return nullptr;
    }

    return result;
}

ComPtr<ID3DBlob> dx12ShaderCompiler::CompileFromMemory(const void* Source, UINT Length, LPCSTR Entry, LPCSTR Target, LPCSTR Defines)
{
    DX12_SHADER_COMPILE_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.Name = nullptr;
    desc.Source = static_cast<LPCSTR>(Source);
    desc.SourceLength = Length;
    desc.Entry = Entry ? Entry : "main";
    desc.Target = Target;
    desc.Defines = Defines;

    ComPtr<ID3DBlob> result;
    ComPtr<ID3DBlob> errors;

    HRESULT hr = CompileShader(desc, result, errors);
    if (FAILED(hr))
    {
        if (errors)
        {
            LOG("*ERROR* dx12ShaderCompiler: %ls", (wchar_t*)errors->GetBufferPointer());
        }
        return nullptr;
    }

    return result;
}

LPCSTR dx12ShaderCompiler::GetShaderTarget(LPCSTR Type)
{
    if (!Type) return "ps_6_0";

    switch (Type[0])
    {
    case 'v': return "vs_6_0";
    case 'p': return "ps_6_0";
    case 'g': return "gs_6_0";
    case 'h': return "hs_6_0";
    case 'd': return "ds_6_0";
    case 'c': return "cs_6_0";
    default: return "ps_6_0";
    }
}

HRESULT dx12ShaderCompiler::LoadDXC()
{
    m_dxcModule = LoadLibraryA("dxc.dll");
    if (!m_dxcModule)
    {
        LOG("*WARNING* dx12ShaderCompiler: DXC not available, falling back to FXC");
        return E_FAIL;
    }

    auto DxcCreateInstance = (decltype(&::DxcCreateInstance)*)GetProcAddress(m_dxcModule, "DxcCreateInstance");
    if (!DxcCreateInstance)
    {
        FreeLibrary(m_dxcModule);
        m_dxcModule = nullptr;
        return E_FAIL;
    }

    ComPtr<IUnknown> pLibUnknown;
    HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pLibUnknown));
    if (FAILED(hr))
    {
        FreeLibrary(m_dxcModule);
        m_dxcModule = nullptr;
        return hr;
    }

    hr = pLibUnknown.As(&m_pDXCLibrary);
    if (FAILED(hr))
    {
        FreeLibrary(m_dxcModule);
        m_dxcModule = nullptr;
        return hr;
    }

    ComPtr<IUnknown> pCompUnknown;
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompUnknown));
    if (FAILED(hr))
    {
        m_pDXCLibrary.Reset();
        FreeLibrary(m_dxcModule);
        m_dxcModule = nullptr;
        return hr;
    }

    hr = pCompUnknown.As(&m_pDXCCompiler);
    if (FAILED(hr))
    {
        m_pDXCLibrary.Reset();
        FreeLibrary(m_dxcModule);
        m_dxcModule = nullptr;
        return hr;
    }

    LOG("*INFO* dx12ShaderCompiler: DXC loaded successfully");
    return S_OK;
}

void dx12ShaderCompiler::BuildDefines(const DX12_SHADER_COMPILE_DESC& Desc, xr_vector<LPCSTR>& Defines)
{
    if (!Desc.Defines) return;

    char definesBuf[4096];
    strconcat(sizeof(definesBuf), definesBuf, Desc.Defines);

    char* token = strtok(definesBuf, " \t,;");
    while (token)
    {
        Defines.push_back(token);
        token = strtok(nullptr, " \t,;");
    }
}

dx12ShaderCompiler ShaderCompiler12;

#endif // USE_DX12
