#pragma once

#ifdef USE_DX12

#include "dx12R_Backend.h"

// Bridge Windows SDK enum names to DirectX-Headers names
#ifndef D3D12_PRIMITIVE_TOPOLOGY_UNDEFINED
#define D3D12_PRIMITIVE_TOPOLOGY_UNDEFINED D3D_PRIMITIVE_TOPOLOGY_UNDEFINED
#define D3D12_PRIMITIVE_TOPOLOGY_POINT_LIST D3D_PRIMITIVE_TOPOLOGY_POINTLIST
#define D3D12_PRIMITIVE_TOPOLOGY_LINE_LIST D3D_PRIMITIVE_TOPOLOGY_LINELIST
#define D3D12_PRIMITIVE_TOPOLOGY_LINE_STRIP D3D_PRIMITIVE_TOPOLOGY_LINESTRIP
#define D3D12_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
#define D3D12_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
#define D3D12_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST
#endif

IC void CBackend::set_xform(u32 ID, const Fmatrix& M)
{
    stat.xforms++;
}

IC void CBackend::set_RT(ID3D12Resource* RT, u32 ID)
{
    if (RT != pRT[ID])
    {
        PGO(Msg("PGO:setRT"));
        stat.target_rt++;
        pRT[ID] = RT;
        m_bChangedRTorZB = true;
    }
}

IC void CBackend::set_ZB(ID3D12Resource* ZB)
{
    if (ZB != pZB)
    {
        PGO(Msg("PGO:setZB"));
        stat.target_zb++;
        pZB = ZB;
        m_bChangedRTorZB = true;
    }
}

ICF void CBackend::set_Format(SDeclaration* _decl)
{
    if (decl != _decl)
    {
        PGO(Msg("PGO:v_format:%x", _decl));
#ifdef DEBUG
        stat.decl++;
#endif
        decl = _decl;
    }
}

ICF void CBackend::set_PS(ID3DPipelineState* _ps, LPCSTR _n)
{
    if (ps12 != _ps)
    {
        PGO(Msg("PGO:Pshader:%x", _ps));
        stat.ps++;
        ps12 = _ps;
    }
}

ICF void CBackend::set_VS(ID3DPipelineState* _vs, LPCSTR _n)
{
    if (vs12 != _vs)
    {
        PGO(Msg("PGO:Vshader:%x", _vs));
        stat.vs++;
        vs12 = _vs;
    }
}

ICF void CBackend::set_GS(ID3D12GeometryShader* _gs, LPCSTR _n)
{
    if (gs != _gs)
    {
        PGO(Msg("PGO:Gshader:%x", _gs));
        gs = _gs;
    }
}

ICF void CBackend::set_HS(ID3D12HullShader* _hs, LPCSTR _n)
{
    if (hs != _hs)
    {
        PGO(Msg("PGO:Hshader:%x", _hs));
        hs = _hs;
    }
}

ICF void CBackend::set_DS(ID3D12DomainShader* _ds, LPCSTR _n)
{
    if (ds != _ds)
    {
        PGO(Msg("PGO:Dshader:%x", _ds));
        ds = _ds;
    }
}

ICF void CBackend::set_CS(ID3D12ComputeShader* _cs, LPCSTR _n)
{
    if (cs != _cs)
    {
        PGO(Msg("PGO:Cshader:%x", _cs));
        cs = _cs;
    }
}

ICF void CBackend::set_Vertices(ID3D12Resource* _vb, u32 _vb_stride)
{
    if ((vb != _vb) || (vb_stride != _vb_stride))
    {
        PGO(Msg("PGO:VB:%x,%d", _vb, _vb_stride));
#ifdef DEBUG
        stat.vb++;
#endif
        Backend12.SetVertexBuffer(_vb, _vb_stride);
        vb = _vb;
        vb_stride = _vb_stride;
    }
}

ICF void CBackend::set_Indices(ID3D12Resource* _ib)
{
    if (ib != _ib)
    {
        PGO(Msg("PGO:IB:%x", _ib));
#ifdef DEBUG
        stat.ib++;
#endif
        Backend12.SetIndexBuffer(_ib);
        ib = _ib;
    }
}

IC D3D12_PRIMITIVE_TOPOLOGY TranslateTopology(D3DPRIMITIVETYPE T)
{
    static D3D12_PRIMITIVE_TOPOLOGY translateTable[] =
    {
        D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
        D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
        D3D_PRIMITIVE_TOPOLOGY_LINELIST,
        D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
        D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
    };

    VERIFY(T < sizeof(translateTable) / sizeof(translateTable[0]));
    VERIFY(T >= 0);

    D3D12_PRIMITIVE_TOPOLOGY result = translateTable[T];
    VERIFY(result != D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);

    return result;
}

IC u32 GetIndexCount(D3DPRIMITIVETYPE T, u32 iPrimitiveCount)
{
    switch (T)
    {
    case D3DPT_POINTLIST:
        return iPrimitiveCount;
    case D3DPT_LINELIST:
        return iPrimitiveCount * 2;
    case D3DPT_LINESTRIP:
        return iPrimitiveCount + 1;
    case D3DPT_TRIANGLELIST:
        return iPrimitiveCount * 3;
    case D3DPT_TRIANGLESTRIP:
        return iPrimitiveCount + 2;
    default: NODEFAULT;
#ifdef DEBUG
        return 0;
#endif
    }
}

IC void CBackend::ApplyPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology)
{
    if (m_PrimitiveTopology != Topology)
    {
        m_PrimitiveTopology = Topology;
        Backend12.IASetPrimitiveTopology(Topology);
    }
}

IC void CBackend::Compute(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
    stat.calls++;

    ApplyDescriptorHeaps();
    StateManager12.Apply();
    constants.flush();
    Backend12.Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

IC void CBackend::Render(D3DPRIMITIVETYPE T, u32 baseV, u32 startV, u32 countV, u32 startI, u32 PC)
{
    D3D12_PRIMITIVE_TOPOLOGY Topology = TranslateTopology(T);
    u32 iIndexCount = GetIndexCount(T, PC);

    if (hs != 0 || ds != 0)
    {
        R_ASSERT(Topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        Topology = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    }

    stat.calls++;
    stat.verts += countV;
    stat.polys += PC;

    ApplyPrimitiveTopology(Topology);
    ApplyDescriptorHeaps();
    ApplyRTandZB();
    ApplyVertexLayout();
    StateManager12.Apply();
    constants.flush();

    Backend12.DrawIndexedInstanced(iIndexCount, 1, startI, baseV, 0);

    PGO(Msg("PGO:DIP:%dv/%df", countV, PC));
}

IC void CBackend::Render(D3DPRIMITIVETYPE T, u32 startV, u32 PC)
{
    if (T == D3DPT_TRIANGLEFAN)
        return;

    D3D12_PRIMITIVE_TOPOLOGY Topology = TranslateTopology(T);
    u32 iVertexCount = GetIndexCount(T, PC);

    stat.calls++;
    stat.verts += 3 * PC;
    stat.polys += PC;

    ApplyPrimitiveTopology(Topology);
    ApplyDescriptorHeaps();
    ApplyRTandZB();
    ApplyVertexLayout();
    StateManager12.Apply();
    constants.flush();

    Backend12.DrawInstanced(iVertexCount, 1, startV, 0);

    PGO(Msg("PGO:DIP:%dv/%df", 3 * PC, PC));
}

IC void CBackend::set_Geometry(SGeometry* _geom)
{
    set_Format(&*_geom->dcl);
    set_Vertices(_geom->vb, _geom->vb_stride);
    set_Indices(_geom->ib);
}

IC void CBackend::set_Scissor(Irect* R)
{
    if (R)
    {
        StateManager12.EnableScissoring();
        RECT* clip = (RECT*)R;
        Backend12.RSSetScissorRects(1, clip);
    }
    else
    {
        StateManager12.EnableScissoring(FALSE);
        Backend12.RSSetScissorRects(0, nullptr);
    }
}

IC void CBackend::set_Stencil(u32 _enable, u32 _func, u32 _ref, u32 _mask, u32 _writemask, u32 _fail, u32 _pass,
                              u32 _zfail)
{
    StateManager12.SetStencil(_enable, _func, _ref, _mask, _writemask, _fail, _pass, _zfail);
}

IC void CBackend::set_Z(u32 _enable)
{
    StateManager12.SetDepthEnable(_enable);
}

IC void CBackend::set_ZFunc(u32 _func)
{
    StateManager12.SetDepthFunc(_func);
}

IC void CBackend::set_AlphaRef(u32 _value)
{
    VERIFY(!"Not implemented.");
}

IC void CBackend::set_ColorWriteEnable(u32 _mask)
{
    StateManager12.SetColorWriteEnable(_mask);
}

ICF void CBackend::set_FillMode(u32 _mode)
{
    StateManager12.SetFillMode(_mode);
}

ICF void CBackend::set_CullMode(u32 _mode)
{
    StateManager12.SetCullMode(_mode);
}

IC void CBackend::ApplyVertexLayout()
{
    VERIFY(vs12);
    VERIFY(decl);
}

ICF void CBackend::set_VS(ref_vs& _vs)
{
    set_VS(_vs->vs, _vs->cName.c_str());
}

ICF void CBackend::set_VS(SVS* _vs)
{
    set_VS(_vs->vs, _vs->cName.c_str());
}

IC void CBackend::set_Constants(R_constant_table* C)
{
    if (ctable == C) return;
    ctable = C;
    xforms.unmap();
    hemi.unmap();
    tree.unmap();
#ifdef USE_DX11
    LOD.unmap();
#endif
    StateManager12.UnmapConstants();
    if (!C) return;

    PGO(Msg("PGO:c-table"));

    R_constant_table::c_table::iterator it = C->table.begin();
    R_constant_table::c_table::iterator end = C->table.end();
    for (; it != end; ++it)
    {
        R_constant* Cs = &**it;
        VERIFY(Cs);
        if (Cs && Cs->handler)
            Cs->handler->setup(Cs);
    }
}

ICF void CBackend::ApplyRTandZB()
{
    if (m_bChangedRTorZB)
    {
        m_bChangedRTorZB = false;
        Backend12.SetRenderTarget(pRT[0], pZB);
    }
}

IC void CBackend::ApplyDescriptorHeaps()
{
}

IC void CBackend::ResourceBarrier(ID3D12Resource* Resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{
    Backend12.ResourceBarrier(Resource, StateBefore, StateAfter);
}

IC void CBackend::ResourceBarriers(UINT NumBarriers, const D3D12_RESOURCE_BARRIER* Barriers)
{
    Backend12.ResourceBarriers(NumBarriers, Barriers);
}

IC void CBackend::BeginRenderPass()
{
    Backend12.BeginRenderPass();
}

IC void CBackend::EndRenderPass()
{
    Backend12.EndRenderPass();
}

IC void CBackend::Flush()
{
    Backend12.Flush();
}

#endif // USE_DX12
