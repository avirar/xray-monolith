// xrRender_R5.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../xrRender/dxRenderFactory.h"
#include "../xrRender/dxUIRender.h"
#include "../xrRender/dxDebugRender.h"
#include "../../xrEngine/Render.h"

//#pragma comment(lib,"xrEngine.lib")

//BOOL APIENTRY DllMain( HANDLE hModule,
BOOL DllMainXrRenderR5(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		::Render = reinterpret_cast<IRender_interface*>(&RImplementation);
		::RenderFactory = &RenderFactoryImpl;
		::DU = &DUImpl;
		UIRender = &UIRenderImpl;
		DRender	= &DebugRenderImpl;
		xrRender_initconsole();
		break ;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


extern "C" {
bool /*_declspec(dllexport)*/ SupportsDX12Rendering();
};

bool /*_declspec(dllexport)*/ SupportsDX12Rendering()
{
	return true;
}
