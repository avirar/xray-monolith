#pragma once

#ifdef USE_DX12

#include "DX12CommonTypes.h"
#include "../xrRender/HWCaps.h"

#ifndef _MAYA_EXPORT
#include "../xrRender/stats_manager.h"
#endif

class pureAppActivate;
class pureAppDeactivate;

class CHW12 : public pureAppActivate, public pureAppDeactivate
{
public:
    CHW12();
    ~CHW12();

    void CreateD3D();
    void DestroyD3D();
    void CreateDevice(HWND hwnd, bool move_window);
    void DestroyDevice();
    void Reset(HWND hwnd);

    IDXGIOutput1* FindOutputOnCurrentAdapter(HMONITOR hMon);

    void selectResolution(u32& dwWidth, u32& dwHeight, BOOL bWindowed);
    DXGI_FORMAT selectDepthStencil(DXGI_FORMAT targetFormat);
    u32 selectPresentInterval();
    u32 selectGPU();
    DXGI_RATIONAL selectRefresh(u32 dwWidth, u32 dwHeight, DXGI_FORMAT fmt);
    void updateWindowProps(HWND hwnd);

    void Validate();

    void UpdateViews();
    void Present(u32 SyncInterval, u32 Flags);
    void WaitForGPU();

    void OnAppActivate();
    void OnAppDeactivate();

    ID3D12Device* GetDevice() { return m_pDevice.Get(); }

    DXGI_SWAP_CHAIN_DESC1 m_ChainDesc;
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC m_ChainDescFullscreen;

    ComPtr<IDXGIFactory4> m_pFactory;
    ComPtr<IDXGIAdapter1> m_pAdapter;
    ComPtr<IDXGIOutput1> m_pOutput;

    ComPtr<ID3D12Device5> m_pDevice;
    ComPtr<ID3D12CommandQueue> m_pCommandQueue;

    ComPtr<IDXGISwapChain3> m_pSwapChain;

    ComPtr<ID3D12Resource> m_pBackBuffer[3];
    ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_RtvHandles[3];

    ComPtr<ID3D12Resource> m_pDepthStencil;
    D3D12_CPU_DESCRIPTOR_HANDLE m_DsvHandle;
    ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;

    ComPtr<ID3D12CommandAllocator> m_pCommandAllocators[3];
    ComPtr<ID3D12GraphicsCommandList4> m_pCommandList;

    ComPtr<ID3D12Fence> m_pFence;
    HANDLE m_fenceEvent;
    UINT64 m_fenceValue;
    UINT64 m_FenceValues[3];

    UINT m_frameIndex;
    UINT m_frameCount;

    CHWCaps Caps;

    D3D_DRIVER_TYPE m_DriverType;
    D3D_FEATURE_LEVEL FeatureLevel;
    HWND m_hWnd;
    bool m_bUsePerfhud;
    bool m_SupportsVRR;
    bool m_bDXRSupported;
    D3D12_RAYTRACING_TIER m_DXRTier;

    u32 m_width;
    u32 m_height;
    DXGI_FORMAT m_format;

    UINT m_rtvDescriptorSize;
    UINT m_dsvDescriptorSize;
    UINT m_cbvSrvUavDescriptorSize;

private:
    void AcquireDefaultOutput();
    void SelectAdapterAndOutput(HMONITOR hTargetMonitor);
    void CheckDXRCapabilities();
    void CreateDescriptorHeaps();
    void CreateDepthStencil();

    bool m_move_window;
};

extern ECORE_API CHW12 HW12;

#endif // USE_DX12
