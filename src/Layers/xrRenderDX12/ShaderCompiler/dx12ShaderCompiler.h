#pragma once

#ifdef USE_DX12

#include "../DX12CommonTypes.h"

struct DX12_SHADER_COMPILE_DESC
{
    LPCSTR Name;
    LPCSTR Source;
    UINT SourceLength;
    LPCSTR Entry;
    LPCSTR Target;
    LPCSTR Defines;
    LPCSTR IncludePath;
    u32 Flags;
};

class dx12ShaderCompiler
{
public:
    dx12ShaderCompiler();
    ~dx12ShaderCompiler();

    void Init();
    void Destroy();

    HRESULT CompileShader(const DX12_SHADER_COMPILE_DESC& Desc, ComPtr<ID3DBlob>& Result, ComPtr<ID3DBlob>& Errors);
    HRESULT CompileDXC(const DX12_SHADER_COMPILE_DESC& Desc, ComPtr<ID3DBlob>& Result, ComPtr<ID3DBlob>& Errors);
    HRESULT CompileFXC(const DX12_SHADER_COMPILE_DESC& Desc, ComPtr<ID3DBlob>& Result, ComPtr<ID3DBlob>& Errors);

    bool IsDXCAvailable() const { return m_pDXCCompiler != nullptr; }

    ComPtr<ID3DBlob> CompileFromFile(LPCSTR FilePath, LPCSTR Entry, LPCSTR Target, LPCSTR Defines = nullptr);
    ComPtr<ID3DBlob> CompileFromMemory(const void* Source, UINT Length, LPCSTR Entry, LPCSTR Target, LPCSTR Defines = nullptr);

    static LPCSTR GetShaderTarget(LPCSTR Type);

private:
    HRESULT LoadDXC();
    void BuildDefines(const DX12_SHADER_COMPILE_DESC& Desc, xr_vector<LPCSTR>& Defines);

    HMODULE m_dxcModule;

    ComPtr<IDxcLibrary3> m_pDXCLibrary;
    ComPtr<IDxcCompiler3> m_pDXCCompiler;

    xr_map<u64, ComPtr<ID3DBlob>> m_shaderCache;

    static const UINT MaxCacheSize = 2048;
};

extern dx12ShaderCompiler ShaderCompiler12;

#endif // USE_DX12
