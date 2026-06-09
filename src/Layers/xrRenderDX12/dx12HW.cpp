#include "dx12HW.h"

#ifdef USE_DX12

#include <dxgidebug.h>
#include <d3dcompiler.h>
#include <algorithm>

#include "../../xrEngine/XR_IOConsole.h"
#include "../../Include/xrAPI/xrAPI.h"
#include "../xrRender/xrRender_console.h"
#include "../../xrCore/xrDebugRender/xrDebugRenderProxy.h"

#ifndef _EDITOR
void fill_vid_mode_list(CHW12* _hw);
void free_vid_mode_list();
void fill_render_mode_list();
void free_render_mode_list();
#else
void fill_vid_mode_list(CHW12* _hw) {}
void free_vid_mode_list() {}
void fill_render_mode_list() {}
void free_render_mode_list() {}
#endif

CHW12 HW12;

extern u32 g_screenmode;

static ComPtr<ID3D12Debug1> g_pDebugController;

CHW12::CHW12() :
    m_pFactory(nullptr),
    m_pAdapter(nullptr),
    m_pOutput(nullptr),
    m_pDevice(nullptr),
    m_pCommandQueue(nullptr),
    m_pSwapChain(nullptr),
    m_pDepthStencil(nullptr),
    m_pCommandList(nullptr),
    m_pFence(nullptr),
    m_fenceEvent(nullptr),
    m_fenceValue(0),
    m_frameIndex(0),
    m_frameCount(3),
    m_DriverType(D3D_DRIVER_TYPE_UNKNOWN),
    FeatureLevel(D3D_FEATURE_LEVEL_12_0),
    m_hWnd(nullptr),
    m_bUsePerfhud(false),
    m_SupportsVRR(false),
    m_bDXRSupported(false),
    m_DXRTier(D3D12_RAYTRACING_TIER_NOT_SUPPORTED),
    m_width(0),
    m_height(0),
    m_format(DXGI_FORMAT_R8G8B8A8_UNORM),
    m_rtvDescriptorSize(0),
    m_dsvDescriptorSize(0),
    m_cbvSrvUavDescriptorSize(0),
    m_move_window(true)
{
    ZeroMemory(m_FenceValues, sizeof(m_FenceValues));
    ZeroMemory(m_pBackBuffer, sizeof(m_pBackBuffer));
    ZeroMemory(m_pCommandAllocators, sizeof(m_pCommandAllocators));
    ZeroMemory(m_RtvHandles, sizeof(m_RtvHandles));

    Device.seqAppActivate.Add(this);
    Device.seqAppDeactivate.Add(this);
}

CHW12::~CHW12()
{
    Device.seqAppActivate.Remove(this);
    Device.seqAppDeactivate.Remove(this);
}

void CHW12::Validate()
{
}

void CHW12::AcquireDefaultOutput()
{
    VERIFY(m_pAdapter);
    R_CHK(m_pAdapter->EnumOutputs(0, &m_pOutput));
}

IDXGIOutput1* CHW12::FindOutputOnCurrentAdapter(HMONITOR hMon)
{
    if (!m_pAdapter || !hMon)
        return nullptr;

    UINT oi = 0;
    IDXGIOutput1* pOut = nullptr;
    while (m_pAdapter->EnumOutputs(oi, &pOut) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        if (SUCCEEDED(pOut->GetDesc(&desc)) && desc.Monitor == hMon)
        {
            return pOut;
        }
        pOut->Release();
        ++oi;
    }
    return nullptr;
}

void CHW12::SelectAdapterAndOutput(HMONITOR hTargetMonitor)
{
    m_pAdapter.Reset();
    m_pOutput.Reset();

    for (UINT ai = 0;; ++ai)
    {
        ComPtr<IDXGIAdapter1> adapter;
        if (m_pFactory->EnumAdapters1(ai, &adapter) == DXGI_ERROR_NOT_FOUND)
            break;

        for (UINT oi = 0;; ++oi)
        {
            ComPtr<IDXGIOutput1> output;
            if (adapter->EnumOutputs(oi, &output) == DXGI_ERROR_NOT_FOUND)
                break;

            DXGI_OUTPUT_DESC desc;
            if (SUCCEEDED(output->GetDesc(&desc)) && desc.Monitor == hTargetMonitor)
            {
                m_pAdapter = std::move(adapter);
                m_pOutput = std::move(output);
                return;
            }
        }
    }

    Msg("!HW12: selected monitor not found on any adapter, falling back to default");
    R_CHK(m_pFactory->EnumAdapters1(0, &m_pAdapter));
    AcquireDefaultOutput();
}

void CHW12::CheckDXRCapabilities()
{
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
    HRESULT hr = m_pDevice->CheckFeatureSupport(
        D3D12_FEATURE_D3D12_OPTIONS5,
        &options5,
        sizeof(options5));

    if (SUCCEEDED(hr))
    {
        m_bDXRSupported = options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1;
        m_DXRTier = options5.RaytracingTier;
    }
    else
    {
        m_bDXRSupported = false;
        m_DXRTier = D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    }

    if (m_bDXRSupported)
    {
        Msg("* HW12: DXR supported, tier %d", (u32)m_DXRTier);
    }
    else
    {
        Msg("! HW12: DXR not supported on this hardware");
    }
}

void CHW12::CreateD3D()
{
    HRESULT hr;

#ifdef DEBUG
    {
        ComPtr<ID3D12Debug> debugInterface;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
        if (SUCCEEDED(hr))
        {
            debugInterface->EnableDebugLayer();
            debugInterface.As(&g_pDebugController);
        }
    }
#endif

    hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_pFactory));
    R_CHK(hr);

    m_pAdapter.Reset();
    m_pOutput.Reset();
    m_bUsePerfhud = false;

#ifndef MASTER_GOLD
    UINT i = 0;
    while (m_pFactory->EnumAdapters1(i, &m_pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 desc;
        m_pAdapter->GetDesc1(&desc);
        if (!wcscmp(desc.Description, L"NVIDIA PerfHUD"))
        {
            m_bUsePerfhud = true;
            AcquireDefaultOutput();
            break;
        }
        else
        {
            m_pAdapter.Reset();
        }
        ++i;
    }
#endif

    if (!m_pAdapter)
    {
        SelectAdapterAndOutput(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY));
    }

    {
        ComPtr<IDXGIFactory5> factory5;
        hr = m_pFactory.As(&factory5);
        if (SUCCEEDED(hr) && factory5)
        {
            BOOL supportsVrr = FALSE;
            hr = factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &supportsVrr,
                sizeof(supportsVrr));
            m_SupportsVRR = SUCCEEDED(hr) && supportsVrr;
        }
        else
        {
            m_SupportsVRR = false;
        }
    }
}

void CHW12::DestroyD3D()
{
    WaitForGPU();

    m_pOutput.Reset();
    m_pAdapter.Reset();
    m_pFactory.Reset();

    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }
}

void CHW12::CreateDescriptorHeaps()
{
    HRESULT hr;

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = m_frameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap));
    R_CHK(hr);

    m_rtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDsvHeap));
    R_CHK(hr);

    m_dsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_cbvSrvUavDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_RtvHandles[0] = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 1; i < m_frameCount; i++)
    {
        m_RtvHandles[i].ptr = m_RtvHandles[i - 1].ptr + i * m_rtvDescriptorSize;
    }

    m_DsvHandle = m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void CHW12::CreateDepthStencil()
{
    HRESULT hr;

    D3D12_RESOURCE_DESC1 depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Alignment = 0;
    depthDesc.Width = m_width;
    depthDesc.Height = m_height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear = {};
    optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

    hr = m_pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_READ,
        &optClear,
        IID_PPV_ARGS(&m_pDepthStencil));
    R_CHK(hr);

    m_pDevice->CreateDepthStencilView(m_pDepthStencil.Get(), nullptr, m_DsvHandle);
}

void CHW12::CreateDevice(HWND hwnd, bool move_window)
{
    m_hWnd = hwnd;
    m_move_window = move_window;
    CreateD3D();

    m_DriverType = Caps.bForceGPU_REF ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_HARDWARE;
    if (m_bUsePerfhud)
        m_DriverType = D3D_DRIVER_TYPE_REFERENCE;

    DXGI_ADAPTER_DESC1 Desc;
    R_CHK(m_pAdapter->GetDesc1(&Desc));
    Msg("* GPU [vendor:%X]-[device:%X]: %S", Desc.VendorId, Desc.DeviceId, Desc.Description);

    Caps.id_vendor = Desc.VendorId;
    Caps.id_device = Desc.DeviceId;

    HRESULT hr;
    UINT deviceFlags = D3D12_CREATE_DEVICE_ROOT_SIGNATURE_ALL;

#ifdef DEBUG
    if (strstr(Core.Params, "--dx12-dbg"))
    {
        deviceFlags |= D3D12_CREATE_DEVICE_DEBUG;
    }
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
    };

    hr = D3D12CreateDevice(
        m_pAdapter.Get(),
        D3D_FEATURE_LEVEL_12_0,
        IID_PPV_ARGS(&m_pDevice));
    R_CHK(hr);

    CheckDXRCapabilities();

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));
    R_CHK(hr);

    BOOL bWindowed = (g_screenmode != 2);

    DXGI_SWAP_CHAIN_DESC1& sd = m_ChainDesc;
    ZeroMemory(&sd, sizeof(sd));

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC& sd_fullscreen = m_ChainDescFullscreen;
    ZeroMemory(&sd_fullscreen, sizeof(sd_fullscreen));
    sd_fullscreen.Windowed = bWindowed;
    sd_fullscreen.Scaling = DXGI_MODE_SCALING_ASPECT_RATIO_STRETCH;

    selectResolution(sd.Width, sd.Height, bWindowed);
    m_width = sd.Width;
    m_height = sd.Height;

    sd.Format = ps_r4_hdr10_on ? DXGI_FORMAT_R10G10B10A2_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
    m_format = sd.Format;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = m_frameCount;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    if (m_SupportsVRR && bWindowed)
    {
        sd.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }

    ComPtr<IDXGISwapChain1> swapChain;
    hr = m_pFactory->CreateSwapChainForHwnd(
        m_pCommandQueue.Get(),
        m_hWnd,
        &sd,
        nullptr,
        nullptr,
        &swapChain);
    R_CHK(hr);

    hr = swapChain.As(&m_pSwapChain);
    R_CHK(hr);

    m_pFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

    if (bWindowed)
    {
        sd_fullscreen.RefreshRate.Numerator = 60;
        sd_fullscreen.RefreshRate.Denominator = 1;
    }
    else
    {
        sd_fullscreen.RefreshRate = selectRefresh(sd.Width, sd.Height, sd.Format);
    }

    CreateDescriptorHeaps();

    for (UINT i = 0; i < m_frameCount; i++)
    {
        hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pBackBuffer[i]));
        R_CHK(hr);

        m_pDevice->CreateRenderTargetView(m_pBackBuffer[i].Get(), nullptr, m_RtvHandles[i]);
    }

    CreateDepthStencil();

    for (UINT i = 0; i < m_frameCount; i++)
    {
        hr = m_pDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_pCommandAllocators[i]));
        R_CHK(hr);
    }

    hr = m_pDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_pCommandAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(&m_pCommandList));
    R_CHK(hr);

    hr = m_pCommandList->Close();
    R_CHK(hr);

    hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
    R_CHK(hr);

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    VERIFY(m_fenceEvent);

    m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    RootSignature12.Init();
    PSOBuilder12.Init();
    SamplerManager12.Init();
    ShaderCompiler12.Init();
    BLASManager12.Init();
    RayTracingShaders12.Init();
    RayTracingPipeline12.Init();
    RayTracingDispatch12.Init();
    GIResources12.Init();
    GIPipeline12.Init();
    GI12.Init();
    ReflectionResources12.Init();
    ReflectionPipeline12.Init();
    Reflections12.Init();
    ShadowPipeline12.Init();
    Shadows12.Init();
    HybridRenderer12.Init();
    RenderPassManager12.Init();

    size_t memory = Desc.DedicatedVideoMemory;
    Msg("*     Texture memory: %d M", (u32)(memory / (1024 * 1024)));

    if (m_bDXRSupported)
    {
        Msg("*     DXR Tier: %d", (u32)m_DXRTier);
    }

#ifndef _EDITOR
    updateWindowProps(hwnd);
    fill_vid_mode_list(this);
#endif
}

void CHW12::DestroyDevice()
{
    WaitForGPU();

    m_pCommandList.Reset();
    m_pCommandQueue.Reset();

    for (UINT i = 0; i < m_frameCount; i++)
    {
        m_pCommandAllocators[i].Reset();
        m_pBackBuffer[i].Reset();
    }

    m_pDepthStencil.Reset();
    m_pRtvHeap.Reset();
    m_pDsvHeap.Reset();
    m_pSwapChain.Reset();

    DestroyD3D();

#ifndef _EDITOR
    free_vid_mode_list();
#endif
}

void CHW12::WaitForGPU()
{
    if (!m_pFence || !m_pCommandQueue)
        return;

    UINT64 fenceValue = ++m_fenceValue;
    HRESULT hr = m_pCommandQueue->Signal(m_pFence.Get(), fenceValue);
    if (FAILED(hr))
        return;

    if (m_pFence->GetCompletedValue() >= fenceValue)
        return;

    hr = m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
    if (SUCCEEDED(hr))
    {
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void CHW12::UpdateViews()
{
    WaitForGPU();

    for (UINT i = 0; i < m_frameCount; i++)
    {
        m_pBackBuffer[i].Reset();
    }
    ZeroMemory(m_RtvHandles, sizeof(m_RtvHandles));

    m_pRtvHeap.Reset();
    CreateDescriptorHeaps();

    for (UINT i = 0; i < m_frameCount; i++)
    {
        HRESULT hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pBackBuffer[i]));
        R_CHK(hr);
        m_pDevice->CreateRenderTargetView(m_pBackBuffer[i].Get(), nullptr, m_RtvHandles[i]);
    }

    m_pDepthStencil.Reset();
    m_pDsvHeap.Reset();

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    R_CHK(m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDsvHeap)));
    m_DsvHandle = m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();

    CreateDepthStencil();
}

void CHW12::Reset(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC1& cd = m_ChainDesc;
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC& cd_fs = m_ChainDescFullscreen;

    BOOL bWindowed = (g_screenmode != 2);
    cd_fs.Windowed = bWindowed;

    if (!bWindowed)
    {
        ShowWindow(hwnd, SW_SHOWNORMAL);
        SetForegroundWindow(hwnd);
    }

    m_pSwapChain->SetFullscreenState(!bWindowed, bWindowed ? nullptr : m_pOutput.Get());

    selectResolution(cd.Width, cd.Height, bWindowed);
    m_width = cd.Width;
    m_height = cd.Height;

    if (bWindowed)
    {
        cd_fs.RefreshRate.Numerator = 60;
        cd_fs.RefreshRate.Denominator = 1;
    }
    else
    {
        cd_fs.RefreshRate = selectRefresh(cd.Width, cd.Height, cd.Format);
    }

    WaitForGPU();

    for (UINT i = 0; i < m_frameCount; i++)
    {
        m_pCommandAllocators[i]->Reset();
    }

    UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    if (m_SupportsVRR && bWindowed)
    {
        flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }

    HRESULT hr = m_pSwapChain->ResizeBuffers(
        m_frameCount,
        cd.Width,
        cd.Height,
        cd.Format,
        flags);
    R_CHK(hr);

    UpdateViews();
    updateWindowProps(hwnd);
}

void CHW12::Present(u32 SyncInterval, u32 Flags)
{
    UINT syncInterval = static_cast<UINT>(SyncInterval);
    UINT presentFlags = static_cast<UINT>(Flags);

    if (!psDeviceFlags.is(rsVSync))
    {
        if (m_SupportsVRR && (g_screenmode != 2))
        {
            presentFlags |= DXGI_PRESENT_ALLOW_TEARING;
        }
    }

    HRESULT hr = m_pSwapChain->Present(syncInterval, presentFlags);
    if (hr == DXGI_ERROR_DEVICE_REMOVED)
    {
        Msg("! HW12: Device removed: 0x%08X", m_pDevice->GetDeviceRemovedReason());
        return;
    }
}

void CHW12::OnAppActivate()
{
    if (!m_pSwapChain)
        return;

    BOOL is_windowed = m_ChainDescFullscreen.Windowed;

    if (!is_windowed)
    {
        ShowWindow(m_hWnd, SW_RESTORE);
        m_pSwapChain->SetFullscreenState(TRUE, m_pOutput.Get());

        WaitForGPU();

        for (UINT i = 0; i < m_frameCount; i++)
        {
            m_pCommandAllocators[i]->Reset();
        }

        const auto& cd = m_ChainDesc;

        UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        if (m_SupportsVRR)
        {
            flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        }

        m_pSwapChain->ResizeBuffers(m_frameCount, cd.Width, cd.Height, cd.Format, flags);
        UpdateViews();
    }
}

void CHW12::OnAppDeactivate()
{
    if (!m_pSwapChain)
        return;

    BOOL is_windowed = m_ChainDescFullscreen.Windowed;

    if (!is_windowed)
    {
        m_pSwapChain->SetFullscreenState(FALSE, nullptr);

        WaitForGPU();

        for (UINT i = 0; i < m_frameCount; i++)
        {
            m_pCommandAllocators[i]->Reset();
        }

        const auto& cd = m_ChainDesc;

        UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        if (m_SupportsVRR)
        {
            flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        }

        m_pSwapChain->ResizeBuffers(m_frameCount, cd.Width, cd.Height, cd.Format, flags);
        UpdateViews();

        ShowWindow(m_hWnd, SW_MINIMIZE);
    }
}

DXGI_FORMAT CHW12::selectDepthStencil(DXGI_FORMAT /*targetFormat*/)
{
    return DXGI_FORMAT_D24_UNORM_S8_UINT;
}

void CHW12::selectResolution(u32& dwWidth, u32& dwHeight, BOOL bWindowed)
{
    fill_vid_mode_list(this);

    if (psCurrentVidMode[0] == 0 || psCurrentVidMode[1] == 0)
    {
        extern void GetMonitorResolution(u32& h, u32& v);
        GetMonitorResolution(psCurrentVidMode[0], psCurrentVidMode[1]);
    }

    if (g_screenmode == 0)
    {
        RECT clientRect;
        GetClientRect(Device.m_hWnd, &clientRect);
        dwWidth = clientRect.right;
        dwHeight = clientRect.bottom;
    }
    else if (g_screenmode == 1)
    {
        dwWidth = psCurrentVidMode[0];
        dwHeight = psCurrentVidMode[1];
    }
    else
    {
        string64 buff;
        xr_sprintf(buff, sizeof(buff), "%dx%d", psCurrentVidMode[0], psCurrentVidMode[1]);

        if (_ParseItem(buff, vid_mode_token) == u32(-1))
        {
            xr_sprintf(buff, sizeof(buff), "vid_mode %s", vid_mode_token[0].name);
            Console->Execute(buff);
        }

        dwWidth = psCurrentVidMode[0];
        dwHeight = psCurrentVidMode[1];
    }
}

u32 CHW12::selectPresentInterval()
{
    if (!psDeviceFlags.is(rsVSync))
        return DXGI_PRESENT_INTERVAL_IMMEDIATELY;
    return DXGI_PRESENT_INTERVAL_ONE;
}

u32 CHW12::selectGPU()
{
    if (Caps.bForceGPU_SW)
        return 0;
    if (Caps.bForceGPU_REF)
        return 1;
    return 2;
}

DXGI_RATIONAL CHW12::selectRefresh(u32 dwWidth, u32 dwHeight, DXGI_FORMAT fmt)
{
    DXGI_RATIONAL res = { 60, 1 };

    if (psDeviceFlags.is(rsRefresh60hz) || strstr(Core.Params, "-60hz"))
    {
        refresh_rate = 1.f / 60.f;
        return res;
    }

    float CurrentFreq = 60.0f;

    VERIFY(m_pOutput);

    UINT num = 0;
    UINT flags = 0;

    m_pOutput->GetDisplayModeList(fmt, flags, &num, nullptr);
    if (num == 0)
    {
        refresh_rate = 1.f / CurrentFreq;
        return res;
    }

    xr_vector<DXGI_MODE_DESC1> modes;
    modes.resize(num);
    m_pOutput->GetDisplayModeList(fmt, flags, &num, &modes.front());

    for (u32 i = 0; i < num; ++i)
    {
        const DXGI_MODE_DESC1& desc = modes[i];

        if (desc.Width == dwWidth && desc.Height == dwHeight)
        {
            VERIFY(desc.RefreshRate.Denominator);
            float TempFreq = static_cast<float>(desc.RefreshRate.Numerator) /
                static_cast<float>(desc.RefreshRate.Denominator);
            if (TempFreq > CurrentFreq)
            {
                CurrentFreq = TempFreq;
                res = desc.RefreshRate;
            }
        }
    }

    refresh_rate = 1.f / CurrentFreq;
    return res;
}

void CHW12::updateWindowProps(HWND m_hWnd)
{
    BOOL bWindowed = (g_screenmode != 2);

    if (bWindowed)
    {
        if (m_move_window)
        {
            u32 dwWindowStyle = 0;
            if (g_screenmode == 1)
            {
                dwWindowStyle |= WS_POPUP;
            }
            else
            {
                dwWindowStyle |= WS_BORDER | WS_OVERLAPPEDWINDOW;
                if (!strstr(Core.Params, "-no_dialog_header"))
                    dwWindowStyle |= WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX;
            }

            SetWindowLong(m_hWnd, GWL_STYLE, dwWindowStyle);

            extern void GetMonitorResolution(u32& h, u32& v);
            extern void GetMonitorPosition(int& x, int& y);

            u32 monW, monH;
            GetMonitorResolution(monW, monH);
            int monX, monY;
            GetMonitorPosition(monX, monY);

            if (psCurrentVidMode[0] == 0 || psCurrentVidMode[1] == 0)
                GetMonitorResolution(psCurrentVidMode[0], psCurrentVidMode[1]);

            LONG res_width = g_screenmode == 0 ? psCurrentVidMode[0] : monW;
            LONG res_height = g_screenmode == 0 ? psCurrentVidMode[1] : monH;

            RECT m_rcWindowBounds;
            SetRect(&m_rcWindowBounds,
                (LONG(monW) - res_width) / 2,
                (LONG(monH) - res_height) / 2,
                (monW + res_width) / 2,
                (monH + res_height) / 2);

            SetWindowPos(m_hWnd,
                HWND_NOTOPMOST,
                monX + m_rcWindowBounds.left,
                monY + m_rcWindowBounds.top,
                (m_rcWindowBounds.right - m_rcWindowBounds.left),
                (m_rcWindowBounds.bottom - m_rcWindowBounds.top),
                SWP_SHOWWINDOW | SWP_NOCOPYBITS | SWP_DRAWFRAME);
        }
    }
    else
    {
        SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    }

    ShowCursor(FALSE);
    SetForegroundWindow(m_hWnd);
    RECT winRect;
    GetClientRect(m_hWnd, &winRect);
    MapWindowPoints(m_hWnd, nullptr, reinterpret_cast<LPPOINT>(&winRect), 2);
    ClipCursor(&winRect);
}

#ifndef _EDITOR

void free_vid_mode_list()
{
    for (int i = 0; vid_mode_token[i].name; i++)
    {
        xr_free(vid_mode_token[i].name);
    }
    xr_free(vid_mode_token);
    vid_mode_token = nullptr;
}

struct _uniq_mode
{
    _uniq_mode(LPCSTR v) : _val(v) {}
    LPCSTR _val;
    bool operator()(LPCSTR _other) { return !stricmp(_val, _other); }
};

void fill_vid_mode_list(CHW12* _hw)
{
    if (vid_mode_token != nullptr) return;

    xr_vector<LPCSTR> _tmp;
    xr_vector<DXGI_MODE_DESC1> modes;

    VERIFY(_hw->m_pOutput);

    UINT num = 0;
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    UINT flags = 0;

    _hw->m_pOutput->GetDisplayModeList(format, flags, &num, nullptr);
    modes.resize(num);
    _hw->m_pOutput->GetDisplayModeList(format, flags, &num, &modes.front());

    for (u32 i = 0; i < num; ++i)
    {
        const DXGI_MODE_DESC1& desc = modes[i];

        if (desc.Width < 800)
            continue;

        string32 str;
        xr_sprintf(str, sizeof(str), "%dx%d", desc.Width, desc.Height);

        if (_tmp.end() != std::find_if(_tmp.begin(), _tmp.end(), _uniq_mode(str)))
            continue;

        _tmp.push_back(nullptr);
        _tmp.back() = xr_strdup(str);
    }

    u32 _cnt = _tmp.size() + 1;
    vid_mode_token = xr_alloc<xr_token>(_cnt);

    vid_mode_token[_cnt - 1].id = -1;
    vid_mode_token[_cnt - 1].name = nullptr;

#ifdef DEBUG
    Msg("Available video modes[%d]:", _tmp.size());
#endif
    for (u32 i = 0; i < _tmp.size(); ++i)
    {
        vid_mode_token[i].id = i;
        vid_mode_token[i].name = _tmp[i];
#ifdef DEBUG
        Msg("[%s]", _tmp[i]);
#endif
    }
}

#endif // _EDITOR

#endif // USE_DX12
