// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#pragma warning(disable:4995)
#include "../../xrEngine/stdafx.h"
#pragma warning(disable:4995)
#include <d3dx9.h>
#pragma warning(default:4995)
#pragma warning(disable:4714)
#pragma warning( 4 : 4018 )
#pragma warning( 4 : 4244 )
#pragma warning(disable:4237)

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>

#include "../xrRender/xrD3DDefs.h"

#define		R_R1	1
#define		R_R2	2
#define		R_R3	3
#define		R_R4	4
#define		R_R5	5
#define		RENDER	R_R5

#include "../../xrParticles/psystem.h"

#include "../xrRender/HW.h"
#include "../xrRender/Shader.h"
#include "../xrRender/R_Backend.h"
#include "../xrRender/R_Backend_Runtime.h"

#include "../xrRender/resourcemanager.h"

#include "../../xrEngine/vis_common.h"
#include "../../xrEngine/render.h"
#include "../../xrEngine/_d3d_extensions.h"
#include "../../xrEngine/igame_level.h"
#include "../xrRender/blenders\blender.h"
#include "../xrRender/blenders\blender_clsid.h"
#include "../xrRender/xrRender_console.h"
#include "r5.h"
