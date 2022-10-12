//==============================================================================
// Filename: Graphics.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include <algorithm>
#include "Graphics.h"

#include "Graphics_Cube.h"
using namespace DirectX;

// 頂点の構造体
struct Vertex3D
{
	XMFLOAT3 pos;   // 頂点位置
	XMFLOAT2 uv;    // uv座標
};


// モデルデータ
const Vertex3D g_CubeMeta[]
{
	// 頂点座標              // uv座標
	{{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}},
	{{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f}},
	{{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f}},

	{{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},
	{{ 1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
	{{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},

	{{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},
	{{ 1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},
	{{ 1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},

	{{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},
	{{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},
	{{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f}},

	{{-1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}},
	{{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
	{{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f}},

	{{ 1.0f,  1.0f,  1.0f}, {0.0f, 1.0f}},
	{{-1.0f, -1.0f,  1.0f}, {1.0f, 0.0f}},
	{{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f}},

	{{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}},
	{{ 1.0f,  1.0f, -1.0f}, {1.0f, 1.0f}},
	{{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f}},

	{{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},
	{{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},
	{{ 1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},

	{{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},
	{{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f}},
	{{ 1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},

	{{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},
	{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
	{{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},

	{{-1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}},
	{{-1.0f,  1.0f, -1.0f}, {1.0f, 1.0f}},
	{{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},

	{{ 1.0f,  1.0f,  1.0f}, {0.0f, 1.0f}},
	{{-1.0f,  1.0f,  1.0f}, {1.0f, 1.0f}},
	{{-1.0f, -1.0f,  1.0f}, {1.0f, 0.0f}},
};

// 初期化処理
void GraphicsCube::Init()
{
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;					// mapができる設定
	heapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN; 
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;				

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension			= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width				= sizeof(Vertex3D) * ARRAYSIZE(g_CubeMeta);
	resourceDesc.Height				= 1;
	resourceDesc.DepthOrArraySize	= 1;
	resourceDesc.MipLevels			= 1;
	resourceDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count	= 1;
	resourceDesc.Flags				= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	resourceDesc.Layout				= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT ret{};
	ret = Graphics::Get()->Device()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)m_vertexBuffer.GetAddressOf()
	);
	if (FAILED(ret))
	{// 頂点バッファの生成に失敗
		return;
	}

	resourceDesc.Width = (sizeof(XMMATRIX) + 0xff) & ~0xff;
	ret = Graphics::Get()->Device()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)m_worldBuffer.GetAddressOf()
	);
	if (FAILED(ret))
	{// 定数バッファの生成に失敗
		return;
	}

	Vertex3D* vertexMap;
	m_vertexBuffer->Map(0, nullptr, (void**)&vertexMap);
	std::copy(std::begin(g_CubeMeta), std::end(g_CubeMeta), vertexMap);
	m_vertexBuffer->Unmap(0, nullptr);

	angle = rand();
}

// 終了処理
void GraphicsCube::Uninit()
{
}

// 描画処理
void GraphicsCube::Draw()
{
	D3D12_VERTEX_BUFFER_VIEW bufferView{};
	bufferView.BufferLocation	= m_vertexBuffer->GetGPUVirtualAddress();
	bufferView.SizeInBytes		= sizeof(Vertex3D) * ARRAYSIZE(g_CubeMeta);
	bufferView.StrideInBytes	= sizeof(Vertex3D);
	
	XMMATRIX rot = XMMatrixRotationY(angle += 0.01f);
	rot = XMMatrixTranspose(rot);

	XMMATRIX* buffer;
	m_worldBuffer->Map(0, nullptr, (void**)&buffer);
	*buffer = rot;
	m_worldBuffer->Unmap(0, nullptr);

	// model
	Graphics::Get()->Context()->SetGraphicsRootConstantBufferView(0, m_worldBuffer->GetGPUVirtualAddress());

	Graphics::Get()->Context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Graphics::Get()->Context()->IASetVertexBuffers(0, 1, &bufferView);
	Graphics::Get()->Context()->DrawInstanced(ARRAYSIZE(g_CubeMeta), 1, 0, 0);
}
