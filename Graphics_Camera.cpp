//==============================================================================
// Filename: Graphics_Camera.cpp
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include "Graphics.h"

#include "Graphics_Camera.h"
using namespace DirectX;

// インスタンスの取得処理
GraphicsCamera* GraphicsCamera::Get()
{
	static GraphicsCamera ret;
	return &ret;
}

// 初期化処理
bool GraphicsCamera::Init()
{
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;					// mapができる設定
	heapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN; 
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;				

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension			= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = (sizeof(XMMATRIX) + 0xff) & ~0xff;
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
		(void**)m_viewBuffer.GetAddressOf()
	);
	if (FAILED(ret))
	{// 頂点バッファの生成に失敗
		return false;
	}

	ret = Graphics::Get()->Device()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)m_projectionBuffer.GetAddressOf()
	);
	if (FAILED(ret))
	{// 頂点バッファの生成に失敗
		return false;
	}

	return true;
}

// 終了処理
void GraphicsCamera::Uninit()
{
}

// 3Dカメラモードにする
void GraphicsCamera::Set3D()
{
	XMFLOAT3 eye(0, 5, -5);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);

	XMMATRIX view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	XMMATRIX projection = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,
		1280 / 780,
		0.1f,
		100.0f
	);

	view = XMMatrixTranspose(view);
	projection = XMMatrixTranspose(projection);

	// カメラの行列を設定
	XMMATRIX* viewBuffer;
	m_viewBuffer->Map(0, nullptr, (void**)&viewBuffer);
	*viewBuffer = view;
	m_viewBuffer->Unmap(0, nullptr);

	// 射影行列を設定
	XMMATRIX* projectionBuffer;
	m_projectionBuffer->Map(0, nullptr, (void**)&projectionBuffer);
	*projectionBuffer = projection;
	m_projectionBuffer->Unmap(0, nullptr);

	Graphics::Get()->SetViewMatrix(m_viewBuffer.Get());
	Graphics::Get()->SetProjectionMatrix(m_projectionBuffer.Get());
}
