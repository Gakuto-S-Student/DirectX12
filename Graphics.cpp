//==============================================================================
// Filename: Graphics.cpp
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include <vector>

#include "Graphics.h"
using namespace Microsoft::WRL;


// インスタンスの取得
Graphics* Graphics::Get()
{
	static Graphics ret;
	return &ret;
}

// 初期化処理
bool Graphics::Init(const int width, const int height, const HWND hWnd)
{
	if (!this->CreateDeviceAndSwapChain(width, height, hWnd))
	{// デバイス・スワップチェインの生成に失敗
		return false;
	}

	if (!this->CreateRenderTargetView())
	{// レンダーターゲットの生成に失敗
		return false;
	}

	if (!this->CreateFence())
	{// フェンスの生成に失敗
		return false;
	}


	return true;
}

// 終了処理
void Graphics::Uninit()
{
}

// 画面クリア
void Graphics::Clear()
{
	// 現在のバッファのインデックスを取得
	UINT index = m_swapChain->GetCurrentBackBufferIndex();
	this->SetResourceBarrier(
		index,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT,		
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET	// 今からレンダーターゲットとして使用
	);

	// レンダーターゲットの指定
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHeapHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetHeapHandle.ptr += index * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_commandList->OMSetRenderTargets(1, &renderTargetHeapHandle, false, nullptr);

	float clearColor[]{ 0.0f, 0.5f, 0.0f, 1.0f };
	m_commandList->ClearRenderTargetView(renderTargetHeapHandle, clearColor, 0, nullptr);	// 画面クリアコマンドの発行
}

// バッファ切り替え 描画コマンド実行
void Graphics::Present()
{
	// 現在のバッファのインデックスを取得
	UINT index = m_swapChain->GetCurrentBackBufferIndex();

	this->SetResourceBarrier(
		index,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET,		// 前の状態がレンダーターゲット
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT				
	);

	// コマンド命令を閉じる
	m_commandList->Close();

	// コマンドリストの実行
	std::vector<ID3D12CommandList*> commandLists{ m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(commandLists.size(), &commandLists[0]);

	// キューの命令処理が終了するまで待つ
	m_commandQueue->Signal(m_fence.Get(), ++m_fenceValue);

	if (m_fence->GetCompletedValue() != m_fenceValue)
	{
		HANDLE fenceEvent = CreateEvent(nullptr, false, false, nullptr);
		m_fence->SetEventOnCompletion(m_fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
	}

	// コマンド・命令のリセット
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	// フリップ
	m_swapChain->Present(1, 0);
}

// デバイスとスワップチェインの生成
bool Graphics::CreateDeviceAndSwapChain(const int width, const int height, const HWND hWnd)
{
	HRESULT ret{};

	// デバイスの生成
	ret = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0,
		__uuidof(ID3D12Device),
		(void**)m_device.GetAddressOf()
	);
	if (FAILED(ret))
	{// デバイスの生成処理に失敗
		return false;
	}

	// コマンドアロケータの生成
	ret = m_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator),
		(void**)m_commandAllocator.GetAddressOf()
	);
	if (FAILED(ret))
	{// コマンドアロケータの生成処理に失敗
		return false;
	}

	// コマンドリストの生成
	ret = m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		nullptr,
		__uuidof(ID3D12GraphicsCommandList),
		(void**)m_commandList.GetAddressOf()
	);
	if (FAILED(ret))
	{// コマンドリストの生成処理に失敗
		return false;
	}

	// コマンドキューの生成
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Flags		= D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask	= 0;
	queueDesc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Type		= D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
	ret = m_device->CreateCommandQueue(
		&queueDesc,
		__uuidof(ID3D12CommandQueue),
		(void**)m_commandQueue.GetAddressOf()
	);
	if (FAILED(ret))
	{
		return false;
	}

	// ファクトリーの生成
	ComPtr<IDXGIFactory6> factory{};
	ret = CreateDXGIFactory2(
		0,
		__uuidof(IDXGIFactory6),
		(void**)factory.GetAddressOf()
	);
	if (FAILED(ret) || !factory)
	{// ファクトリーの生成に失敗
		return false;
	}


	// スワップチェインの生成
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width					= width;
	swapChainDesc.Height				= height;
	swapChainDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.Stereo				= false;
	swapChainDesc.SampleDesc.Count		= 1;
	swapChainDesc.SampleDesc.Quality	= 0;
	swapChainDesc.BufferUsage			= DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount			= k_backBufferNum;
	swapChainDesc.Scaling				= DXGI_SCALING::DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode				= DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags					= DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	ret = factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)m_swapChain.GetAddressOf()
	);
	if (FAILED(ret) || !m_swapChain)
	{// スワップチェインの生成に失敗
		return false;
	}

	return true;
}

// レンダーターゲットの生成
bool Graphics::CreateRenderTargetView()
{
	HRESULT ret{};

	// レンダーターゲットの生成処理
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask		= 0;
	heapDesc.NumDescriptors = k_backBufferNum;
	heapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ret = m_device->CreateDescriptorHeap(&heapDesc, __uuidof(ID3D12DescriptorHeap), (void**)m_renderTargetViewHeap.GetAddressOf());
	if (FAILED(ret))
	{// ヒープの生成に失敗
		return false;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	ret = m_swapChain->GetDesc(&swapChainDesc);
	if (FAILED(ret))
	{
		return false;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();	// ヒープの先頭アドレス取得
	for (UINT i = 0; i < k_backBufferNum; ++i)
	{// レンダーターゲット用のバックバッファを作成
		ret = m_swapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)m_backBuffers[i].GetAddressOf());
		m_device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, handle);
		handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);	// RTV1つ分のサイズ分をインクリメント
	}

	return true;
}

// フェンスの生成
bool Graphics::CreateFence()
{
	HRESULT ret{};
	ret = m_device->CreateFence(
		m_fenceValue,
		D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE,
		__uuidof(ID3D12Fence),
		(void**)m_fence.GetAddressOf()
	);
	if (FAILED(ret))
	{// フェンスの生成に失敗
		return false;
	}

	return true;
}

// リソースバリアの設定
void Graphics::SetResourceBarrier(const UINT index, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER barrierDesc{};
	barrierDesc.Type					= D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Flags					= D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.Transition.pResource	= m_backBuffers[index].Get();
	barrierDesc.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore	= before;
	barrierDesc.Transition.StateAfter	= after;
	m_commandList->ResourceBarrier(1, &barrierDesc);
}
