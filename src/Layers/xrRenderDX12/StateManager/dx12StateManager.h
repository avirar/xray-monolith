#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"

class dx12StateManager
{
public:
    dx12StateManager();
    ~dx12StateManager();

    void Apply();

    void SetDepthEnable(bool Enable);
    void SetDepthFunc(u32 Func);
    void SetStencil(bool Enable, u32 Func, u32 Ref, u32 Mask, u32 WriteMask,
                    u32 Fail, u32 Pass, u32 ZFail);
    void SetColorWriteEnable(u32 Mask);
    void SetFillMode(u32 Mode);
    void SetCullMode(u32 Mode);
    void EnableScissoring(bool Enable = true);
    void UnmapConstants();

    ID3D12PipelineState* GetOrCreatePSO(const D3D12_PIPELINE_STATE_DESC& Desc);

    void SetSamplers(UINT StartSlot, UINT Count, ID3D12SamplerState* const* Samplers);
    void ApplySamplers();

    struct SamplerState
    {
        ID3D12SamplerState* Samplers[16];
        UINT Count;
        UINT StartSlot;

        bool operator==(const SamplerState& other) const
        {
            if (Count != other.Count || StartSlot != other.StartSlot)
                return false;
            for (UINT i = 0; i < Count; i++)
            {
                if (Samplers[i] != other.Samplers[i])
                    return false;
            }
            return true;
        }
    };

    SamplerState m_currentSamplers;
    SamplerState m_pendingSamplers;
    bool m_samplersDirty;

private:
    struct StateKey
    {
        bool DepthEnable;
        D3D12_COMPARISON_FUNC DepthFunc;
        bool StencilEnable;
        D3D12_STENCIL_OP FrontFail, FrontPass, FrontZFail;
        D3D12_STENCIL_OP BackFail, BackPass, BackZFail;
        D3D12_COMPARISON_FUNC StencilFunc;
        u8 StencilRef;
        u8 StencilReadMask;
        u8 StencilWriteMask;
        u8 ColorWriteMask;
        D3D12_FILL_MODE FillMode;
        D3D12_CULL_MODE CullMode;
        bool ScissorEnable;

        bool operator==(const StateKey& other) const;
        bool operator<(const StateKey& other) const;
    };

    StateKey m_currentState;
    StateKey m_pendingState;
    bool m_stateDirty;

    ComPtr<ID3D12DepthStencilState> m_currentDSS;
    ComPtr<ID3D12RasterizerState> m_currentRS;
    ComPtr<ID3D12BlendState> m_currentBS;

    xr_map<u64, ComPtr<ID3D12PipelineState>> m_psoCache;
};

extern dx12StateManager StateManager12;

#endif // USE_DX12
