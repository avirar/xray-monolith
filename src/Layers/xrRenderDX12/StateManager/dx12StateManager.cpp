#include "stdafx.h"
#include "../dx12stdafx.h"
#include "dx12StateManager.h"

#ifdef USE_DX12

#if 0

dx12StateManager::dx12StateManager()
    : m_stateDirty(true),
      m_samplersDirty(false)
{
    ZeroMemory(&m_currentState, sizeof(m_currentState));
    ZeroMemory(&m_pendingState, sizeof(m_pendingState));
    ZeroMemory(&m_currentSamplers, sizeof(m_currentSamplers));
    ZeroMemory(&m_pendingSamplers, sizeof(m_pendingSamplers));
}

dx12StateManager::~dx12StateManager()
{
}

bool dx12StateManager::StateKey::operator==(const StateKey& other) const
{
    return DepthEnable == other.DepthEnable &&
           DepthFunc == other.DepthFunc &&
           StencilEnable == other.StencilEnable &&
           FrontFail == other.FrontFail &&
           FrontPass == other.FrontPass &&
           FrontZFail == other.FrontZFail &&
           BackFail == other.BackFail &&
           BackPass == other.BackPass &&
           BackZFail == other.BackZFail &&
           StencilFunc == other.StencilFunc &&
           StencilRef == other.StencilRef &&
           StencilReadMask == other.StencilReadMask &&
           StencilWriteMask == other.StencilWriteMask &&
           ColorWriteMask == other.ColorWriteMask &&
           FillMode == other.FillMode &&
           CullMode == other.CullMode &&
           ScissorEnable == other.ScissorEnable;
}

bool dx12StateManager::StateKey::operator<(const StateKey& other) const
{
    return memcmp(this, &other, sizeof(StateKey)) < 0;
}

void dx12StateManager::Apply()
{
    ApplySamplers();

    if (!m_stateDirty)
        return;

    if (m_pendingState.DepthEnable != m_currentState.DepthEnable ||
        m_pendingState.DepthFunc != m_currentState.DepthFunc ||
        m_pendingState.StencilEnable != m_currentState.StencilEnable ||
        m_pendingState.FrontFail != m_currentState.FrontFail ||
        m_pendingState.FrontPass != m_currentState.FrontPass ||
        m_pendingState.FrontZFail != m_currentState.FrontZFail ||
        m_pendingState.BackFail != m_currentState.BackFail ||
        m_pendingState.BackPass != m_currentState.BackPass ||
        m_pendingState.BackZFail != m_currentState.BackZFail ||
        m_pendingState.StencilFunc != m_currentState.StencilFunc ||
        m_pendingState.StencilRef != m_currentState.StencilRef ||
        m_pendingState.StencilReadMask != m_currentState.StencilReadMask)
    {
        D3D12_DEPTH_STENCIL_DESC dssDesc = {};
        dssDesc.DepthEnable = m_pendingState.DepthEnable;
        dssDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        dssDesc.DepthFunc = m_pendingState.DepthFunc;
        dssDesc.StencilEnable = m_pendingState.StencilEnable;
        dssDesc.FrontFace.StencilFailOp = m_pendingState.FrontFail;
        dssDesc.FrontFace.StencilDepthFailOp = m_pendingState.FrontZFail;
        dssDesc.FrontFace.StencilPassOp = m_pendingState.FrontPass;
        dssDesc.FrontFace.StencilFunc = m_pendingState.StencilFunc;
        dssDesc.BackFace = dssDesc.FrontFace;
        dssDesc.StencilReadMask = m_pendingState.StencilReadMask;
        dssDesc.StencilWriteMask = m_pendingState.StencilWriteMask;

        HRESULT hr = HW12.m_pDevice->CreateDepthStencilState(&dssDesc, IID_PPV_ARGS(&m_currentDSS));
        if (SUCCEEDED(hr))
        {
            HW12.m_pCommandList->OMSetDepthStencilState(m_currentDSS.Get(), m_pendingState.StencilRef);
        }
    }

    if (m_pendingState.FillMode != m_currentState.FillMode ||
        m_pendingState.CullMode != m_currentState.CullMode ||
        m_pendingState.ScissorEnable != m_currentState.ScissorEnable)
    {
        D3D12_RASTERIZER_DESC rsDesc = {};
        rsDesc.FillMode = m_pendingState.FillMode;
        rsDesc.CullMode = m_pendingState.CullMode;
        rsDesc.FrontCounterClockwise = FALSE;
        rsDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rsDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rsDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rsDesc.DepthClipEnable = TRUE;
        rsDesc.MultisampleEnable = FALSE;
        rsDesc.AntialiasedLineEnable = FALSE;
        rsDesc.ForcedSampleCount = 0;

        HRESULT hr = HW12.m_pDevice->CreateRasterizerState(&rsDesc, IID_PPV_ARGS(&m_currentRS));
        if (SUCCEEDED(hr))
        {
            HW12.m_pCommandList->RSSetState(m_currentRS.Get());
        }
    }

    if (m_pendingState.ColorWriteMask != m_currentState.ColorWriteMask)
    {
        D3D12_BLEND_DESC bsDesc = {};
        bsDesc.RenderTarget[0].BlendEnable = TRUE;
        bsDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        bsDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
        bsDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        bsDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        bsDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        bsDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        bsDesc.RenderTarget[0].RenderTargetWriteMask = m_pendingState.ColorWriteMask;

        HRESULT hr = HW12.m_pDevice->CreateBlendState(&bsDesc, IID_PPV_ARGS(&m_currentBS));
        if (SUCCEEDED(hr))
        {
            FLOAT blendFactor[4] = { 0, 0, 0, 0 };
            HW12.m_pCommandList->OMSetBlendState(m_currentBS.Get(), blendFactor, 0xFFFFFFFF);
        }
    }

    m_currentState = m_pendingState;
    m_stateDirty = false;
}

void dx12StateManager::SetDepthEnable(bool Enable)
{
    m_pendingState.DepthEnable = Enable;
    m_stateDirty = true;
}

void dx12StateManager::SetDepthFunc(u32 Func)
{
    static const D3D12_COMPARISON_FUNC funcs[] = {
        D3D12_COMPARISON_NEVER,
        D3D12_COMPARISON_LESS,
        D3D12_COMPARISON_EQUAL,
        D3D12_COMPARISON_LESS_EQUAL,
        D3D12_COMPARISON_GREATER,
        D3D12_COMPARISON_NOT_EQUAL,
        D3D12_COMPARISON_GREATER_EQUAL,
        D3D12_COMPARISON_ALWAYS,
    };

    if (Func < _countof(funcs))
    {
        m_pendingState.DepthFunc = funcs[Func];
        m_stateDirty = true;
    }
}

void dx12StateManager::SetStencil(bool Enable, u32 Func, u32 Ref, u32 Mask, u32 WriteMask,
                                   u32 Fail, u32 Pass, u32 ZFail)
{
    static const D3D12_STENCIL_OP ops[] = {
        D3D12_STENCIL_OP_KEEP,
        D3D12_STENCIL_OP_ZERO,
        D3D12_STENCIL_OP_REPLACE,
        D3D12_STENCIL_OP_INCR_SAT,
        D3D12_STENCIL_OP_DECR_SAT,
        D3D12_STENCIL_OP_INVERT,
        D3D12_STENCIL_OP_INCR,
        D3D12_STENCIL_OP_DECR,
    };

    static const D3D12_COMPARISON_FUNC funcs[] = {
        D3D12_COMPARISON_NEVER,
        D3D12_COMPARISON_LESS,
        D3D12_COMPARISON_EQUAL,
        D3D12_COMPARISON_LESS_EQUAL,
        D3D12_COMPARISON_GREATER,
        D3D12_COMPARISON_NOT_EQUAL,
        D3D12_COMPARISON_GREATER_EQUAL,
        D3D12_COMPARISON_ALWAYS,
    };

    m_pendingState.StencilEnable = Enable;
    m_pendingState.StencilFunc = Func < _countof(funcs) ? funcs[Func] : D3D12_COMPARISON_ALWAYS;
    m_pendingState.StencilRef = static_cast<u8>(Ref);
    m_pendingState.StencilReadMask = static_cast<u8>(Mask);
    m_pendingState.StencilWriteMask = static_cast<u8>(WriteMask);
    m_pendingState.FrontFail = Fail < _countof(ops) ? ops[Fail] : D3D12_STENCIL_OP_KEEP;
    m_pendingState.FrontPass = Pass < _countof(ops) ? ops[Pass] : D3D12_STENCIL_OP_KEEP;
    m_pendingState.FrontZFail = ZFail < _countof(ops) ? ops[ZFail] : D3D12_STENCIL_OP_KEEP;
    m_pendingState.BackFail = m_pendingState.FrontFail;
    m_pendingState.BackPass = m_pendingState.FrontPass;
    m_pendingState.BackZFail = m_pendingState.FrontZFail;
    m_stateDirty = true;
}

void dx12StateManager::SetColorWriteEnable(u32 Mask)
{
    m_pendingState.ColorWriteMask = static_cast<u8>(Mask);
    m_stateDirty = true;
}

void dx12StateManager::SetFillMode(u32 Mode)
{
    m_pendingState.FillMode = static_cast<D3D12_FILL_MODE>(Mode);
    m_stateDirty = true;
}

void dx12StateManager::SetCullMode(u32 Mode)
{
    m_pendingState.CullMode = static_cast<D3D12_CULL_MODE>(Mode);
    m_stateDirty = true;
}

void dx12StateManager::EnableScissoring(bool Enable)
{
    m_pendingState.ScissorEnable = Enable;
    m_stateDirty = true;
}

void dx12StateManager::UnmapConstants()
{
}

void dx12StateManager::SetSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers)
{
    m_pendingSamplers.StartSlot = StartSlot;
    m_pendingSamplers.Count = Count;
    for (UINT i = 0; i < Count && i < 16; i++)
    {
        m_pendingSamplers.Samplers[i] = Samplers[i];
    }
    for (UINT i = Count; i < 16; i++)
    {
        m_pendingSamplers.Samplers[i] = nullptr;
    }
    m_samplersDirty = true;
}

void dx12StateManager::ApplySamplers()
{
    if (!m_samplersDirty)
        return;

    if (!(m_pendingSamplers == m_currentSamplers))
    {
        if (m_pendingSamplers.Count > 0)
        {
            HW12.m_pCommandList->RSSetSamplers(m_pendingSamplers.StartSlot,
                                                m_pendingSamplers.Count,
                                                m_pendingSamplers.Samplers);
        }
        m_currentSamplers = m_pendingSamplers;
    }

    m_samplersDirty = false;
}

ID3D12PipelineState* dx12StateManager::GetOrCreatePSO(const D3D12_PIPELINE_STATE_DESC& Desc)
{
    u64 key = xxhash64(&Desc, sizeof(Desc), 0);

    xr_map<u64, ComPtr<ID3D12PipelineState>>::iterator it = m_psoCache.find(key);
    if (it != m_psoCache.end())
        return it->second.Get();

    ComPtr<ID3D12PipelineState> pso;
    HRESULT hr = HW12.m_pDevice->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&pso));
    if (SUCCEEDED(hr))
    {
        m_psoCache[key] = std::move(pso);
        return m_psoCache[key].Get();
    }

    return nullptr;
}

dx12StateManager StateManager12;

#endif // if 0

// Stub to satisfy linker - DX12 uses PSO instead of individual states
dx12StateManager StateManager12;

#endif // USE_DX12
