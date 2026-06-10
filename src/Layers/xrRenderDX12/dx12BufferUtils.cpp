#include "stdafx.h"

#ifdef USE_DX12

#include "../xrRenderDX10/dx10BufferUtils.h"
#include "dx12HW.h"
#include "dx12R_Backend.h"
#include "Buffer/dx12BufferManager.h"

namespace dx10BufferUtils
{
	HRESULT CreateVertexBuffer(ID3DVertexBuffer** ppBuffer, const void* pData, UINT DataSize, bool bImmutable)
	{
		ID3D12Resource** ppRes = reinterpret_cast<ID3D12Resource**>(ppBuffer);
		*ppRes = nullptr;

#if 0 // DX12 API issues: D3D12_HEAP_PROPERTIES has no Flags member, UpdateSubresource is not a command list method
		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.Flags = D3D12_HEAP_FLAG_NONE;

		D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(DataSize);
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		HRESULT hr = HW12.m_pDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(ppRes)
		);
		if (FAILED(hr)) return hr;

		// Upload data via upload heap
		ID3D12Resource* pUploadHeap = nullptr;
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		hr = HW12.m_pDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pUploadHeap)
		);
		if (FAILED(hr)) { (*ppRes)->Release(); *ppRes = nullptr; return hr; }

		Backend12.GetCommandList()->UpdateSubresource(*ppRes, 0, nullptr, pData, 0, 0);
		pUploadHeap->Release();
#endif
		return S_OK;
	}

	HRESULT CreateIndexBuffer(ID3DIndexBuffer** ppBuffer, const void* pData, UINT DataSize, bool bImmutable)
	{
		return CreateVertexBuffer(ppBuffer, pData, DataSize, bImmutable);
	}

	HRESULT CreateConstantBuffer(ID3DBuffer** ppBuffer, UINT DataSize)
	{
		ID3D12Resource** ppRes = reinterpret_cast<ID3D12Resource**>(ppBuffer);
		*ppRes = nullptr;

		// Align to 256 bytes
		DataSize = (DataSize + 255) & ~255;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(DataSize);
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		HRESULT hr = HW12.m_pDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(ppRes)
		);
		return hr;
	}

	struct VertexFormatPairs
	{
		D3DDECLTYPE m_dx9FMT;
		DXGI_FORMAT m_dx10FMT;
	};

	VertexFormatPairs VertexFormatList[] =
	{
		{D3DDECLTYPE_FLOAT1, DXGI_FORMAT_R32_FLOAT},
		{D3DDECLTYPE_FLOAT2, DXGI_FORMAT_R32G32_FLOAT},
		{D3DDECLTYPE_FLOAT3, DXGI_FORMAT_R32G32B32_FLOAT},
		{D3DDECLTYPE_FLOAT4, DXGI_FORMAT_R32G32B32A32_FLOAT},
		{D3DDECLTYPE_D3DCOLOR, DXGI_FORMAT_R8G8B8A8_UNORM},
		{D3DDECLTYPE_UBYTE4, DXGI_FORMAT_R8G8B8A8_UINT},
		{D3DDECLTYPE_SHORT2, DXGI_FORMAT_R16G16_SINT},
		{D3DDECLTYPE_SHORT4, DXGI_FORMAT_R16G16B16A16_SINT},
		{D3DDECLTYPE_UBYTE4N, DXGI_FORMAT_R8G8B8A8_UNORM},
		{D3DDECLTYPE_SHORT2N, DXGI_FORMAT_R16G16_SNORM},
		{D3DDECLTYPE_SHORT4N, DXGI_FORMAT_R16G16B16A16_SNORM},
		{D3DDECLTYPE_USHORT2N, DXGI_FORMAT_R16G16_UNORM},
		{D3DDECLTYPE_USHORT4N, DXGI_FORMAT_R16G16B16A16_UNORM},
		{D3DDECLTYPE_FLOAT16_2, DXGI_FORMAT_R16G16_FLOAT},
		{D3DDECLTYPE_FLOAT16_4, DXGI_FORMAT_R16G16B16A16_FLOAT}
	};

	DXGI_FORMAT ConvertVertexFormat(D3DDECLTYPE dx9FMT)
	{
		int arrayLength = sizeof(VertexFormatList) / sizeof(VertexFormatList[0]);
		for (int i = 0; i < arrayLength; ++i)
		{
			if (VertexFormatList[i].m_dx9FMT == dx9FMT)
				return VertexFormatList[i].m_dx10FMT;
		}
		return DXGI_FORMAT_UNKNOWN;
	}

	struct VertexSemanticPairs
	{
		D3DDECLUSAGE m_dx9Semantic;
		LPCSTR m_dx10Semantic;
	};

	VertexSemanticPairs VertexSemanticList[] =
	{
		{D3DDECLUSAGE_POSITION, "POSITION"},
		{D3DDECLUSAGE_BLENDWEIGHT, "BLENDWEIGHT"},
		{D3DDECLUSAGE_BLENDINDICES, "BLENDINDICES"},
		{D3DDECLUSAGE_NORMAL, "NORMAL"},
		{D3DDECLUSAGE_PSIZE, "PSIZE"},
		{D3DDECLUSAGE_TEXCOORD, "TEXCOORD"},
		{D3DDECLUSAGE_TANGENT, "TANGENT"},
		{D3DDECLUSAGE_BINORMAL, "BINORMAL"},
		{D3DDECLUSAGE_POSITIONT, "POSITIONT"},
		{D3DDECLUSAGE_COLOR, "COLOR"},
	};

	LPCSTR ConvertSemantic(D3DDECLUSAGE Semantic)
	{
		int arrayLength = sizeof(VertexSemanticList) / sizeof(VertexSemanticList[0]);
		for (int i = 0; i < arrayLength; ++i)
		{
			if (VertexSemanticList[i].m_dx9Semantic == Semantic)
				return VertexSemanticList[i].m_dx10Semantic;
		}
		return 0;
	}

	void ConvertVertexDeclaration(const xr_vector<D3DVERTEXELEMENT9>& declIn,
	                              xr_vector<D3D_INPUT_ELEMENT_DESC>& declOut)
	{
		int iDeclSize = declIn.size() - 1;
		declOut.resize(iDeclSize + 1);

		for (int i = 0; i < iDeclSize; ++i)
		{
			const D3DVERTEXELEMENT9& descIn = declIn[i];
			D3D_INPUT_ELEMENT_DESC& descOut = declOut[i];

			descOut.SemanticName = ConvertSemantic((D3DDECLUSAGE)descIn.Usage);
			descOut.SemanticIndex = descIn.UsageIndex;
			descOut.Format = ConvertVertexFormat((D3DDECLTYPE)descIn.Type);
			descOut.InputSlot = descIn.Stream;
			descOut.AlignedByteOffset = descIn.Offset;
			descOut.InputSlotClass = D3D_INPUT_PER_VERTEX_DATA;
			descOut.InstanceDataStepRate = 0;
		}

		ZeroMemory(&declOut[iDeclSize], sizeof(declOut[iDeclSize]));
	}
};

#endif // USE_DX12
