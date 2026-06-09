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
#include <d3d12_raytracing.h>
#include <d3d12sdklayers.h>
#include <d3d12video.h>
#include <dxgi1_6.h>
#include <D3DCompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "DX12CommonTypes.h"
#include "dx12HW.h"

#include "../xrRender/xrD3DDefs.h"
#include "../xrRender/HW.h"
#include "../xrRender/Shader.h"
#include "../xrRender/R_Backend.h"
#include "../xrRender/R_Backend_Runtime.h"
#include "../xrRender/resourcemanager.h"

#include "../../xrEngine/vis_common.h"
#include "../../xrEngine/render.h"
#include "../../xrEngine/_d3d_extensions.h"
#include "../../xrEngine/igame_level.h"
#include "../xrRender/blenders/blender.h"
#include "../xrRender/blenders/blender_clsid.h"
#include "../xrRender/xrRender_console.h"

#define R_R5 5
#define RENDER R_R5
