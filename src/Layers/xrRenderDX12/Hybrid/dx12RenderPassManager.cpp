#include "../dx12stdafx.h"
#include "dx12RenderPassManager.h"

#ifdef USE_DX12

dx12RenderPassManager::dx12RenderPassManager()
    : m_RenderWidth(0),
      m_RenderHeight(0),
      m_Initialized(false)
{
}

dx12RenderPassManager::~dx12RenderPassManager()
{
}

void dx12RenderPassManager::Init()
{
    m_Initialized = true;

    // Default pass order
    ClearPasses();
    AddPass(DX12_PASS_GBUFFER);
    AddPass(DX12_PASS_GI);
    AddPass(DX12_PASS_SHADOWS);
    AddPass(DX12_PASS_REFLECTIONS);
    AddPass(DX12_PASS_LIGHTING);
    AddPass(DX12_PASS_POSTPROCESS);
    AddPass(DX12_PASS_PRESENT);
}

void dx12RenderPassManager::AddPass(DX12_RENDER_PASS Pass)
{
    if (!HasPass(Pass))
    {
        m_Passes.push_back(Pass);
    }
}

void dx12RenderPassManager::RemovePass(DX12_RENDER_PASS Pass)
{
    for (u32 i = 0; i < m_Passes.size(); i++)
    {
        if (m_Passes[i] == Pass)
        {
            m_Passes.erase(m_Passes.begin() + i);
            break;
        }
    }
}

void dx12RenderPassManager::ClearPasses()
{
    m_Passes.clear();
}

void dx12RenderPassManager::ExecuteAllPasses()
{
    for (u32 i = 0; i < m_Passes.size(); i++)
    {
        HybridRenderer12.ExecutePass(m_Passes[i]);
    }
}

bool dx12RenderPassManager::HasPass(DX12_RENDER_PASS Pass) const
{
    for (u32 i = 0; i < m_Passes.size(); i++)
    {
        if (m_Passes[i] == Pass)
            return true;
    }
    return false;
}

DX12_RENDER_PASS dx12RenderPassManager::GetPass(u32 Index) const
{
    if (Index < m_Passes.size())
        return m_Passes[Index];
    return DX12_PASS_NONE;
}

void dx12RenderPassManager::SetRenderResolution(u32 Width, u32 Height)
{
    m_RenderWidth = Width;
    m_RenderHeight = Height;
}

dx12RenderPassManager RenderPassManager12;

#endif // USE_DX12
