//==============================================================================
// Filename: Graphics.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include <algorithm>
#include "Graphics.h"
#include "Graphics_Texture.h"

#include "Graphics_Cube.h"
using namespace DirectX;

struct Vertex3D
{
	XMFLOAT3 Position;
	XMFLOAT2 TexCoord;
};


// 初期化処理
void GraphicsCube::Init()
{
	// 頂点データ
	Vertex3D vertices[]{
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}},
		{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f}},
	};

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;					// mapができる設定
	heapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN; 
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;				

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension			= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width				= sizeof(vertices);
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

	Vertex3D* vertexMap;
	m_vertexBuffer->Map(0, nullptr, (void**)&vertexMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexMap);
	m_vertexBuffer->Unmap(0, nullptr);

	m_vertexBufferView.BufferLocation	= m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes		= sizeof(vertices);
	m_vertexBufferView.StrideInBytes	= sizeof(vertices[0]);

	// テクスチャの読み込み
	GraphicsTexture::CreateTexture(m_textureBuffer.GetAddressOf(), m_textureHeap.GetAddressOf());
}

// 終了処理
void GraphicsCube::Uninit()
{
}

// 描画処理
void GraphicsCube::Draw()
{
	Graphics::Get()->Context()->SetDescriptorHeaps(1, m_textureHeap.GetAddressOf());
	Graphics::Get()->Context()->SetGraphicsRootDescriptorTable(0, m_textureHeap->GetGPUDescriptorHandleForHeapStart());

	Graphics::Get()->Context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	Graphics::Get()->Context()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	Graphics::Get()->Context()->DrawInstanced(4, 1, 0, 0);
}
