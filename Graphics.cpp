//==============================================================================
// Filename: Graphics.cpp
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include <vector>
#include "Graphics_Shader.h"

#include "Graphics.h"
using namespace Microsoft::WRL;
using namespace DirectX;


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

	if (!this->CreateGraphicsPipeline())
	{// グラフィックスパイプラインの生成に失敗
		return false;
	}

	this->SetViewport(width, height);
	this->SetScissorRect(width, height);

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

	// パイプライン設定
	m_commandList->SetPipelineState(m_pipelineState.Get());

	// レンダーターゲットの指定
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHeapHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetHeapHandle.ptr += index * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_commandList->OMSetRenderTargets(1, &renderTargetHeapHandle, false, nullptr);

	float clearColor[]{ 0.0f, 0.5f, 0.0f, 1.0f };
	m_commandList->ClearRenderTargetView(renderTargetHeapHandle, clearColor, 0, nullptr);	// 画面クリアコマンドの発行

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
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
		if (fenceEvent)
		{
			m_fence->SetEventOnCompletion(m_fenceValue, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
			CloseHandle(fenceEvent);
		}
	}

	// コマンド・命令のリセット
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	// フリップ
	m_swapChain->Present(1, 0);
}

// デバイスのアドレスを取得
ID3D12Device* Graphics::Device()
{
	return m_device.Get();
}

// コマンドリストのアドレスを取得
ID3D12GraphicsCommandList* Graphics::Context()
{
	return m_commandList.Get();
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

// パイプラインの生成
bool Graphics::CreateGraphicsPipeline()
{
	std::string vertexShader, pixelShader;
	GraphicsShader::LoadFile("vertexShader.cso", vertexShader);
	GraphicsShader::LoadFile("pixelShader.cso", pixelShader);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	graphicsPipeline{};
	graphicsPipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	D3D12_INPUT_ELEMENT_DESC inputLayout[]
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	// シェーダデータの設定
	graphicsPipeline.VS.pShaderBytecode	= vertexShader.data();
	graphicsPipeline.VS.BytecodeLength	= vertexShader.size();
	graphicsPipeline.PS.pShaderBytecode	= pixelShader.data();
	graphicsPipeline.PS.BytecodeLength	= pixelShader.size();
	
	// 頂点レイアウトの設定
	graphicsPipeline.InputLayout.pInputElementDescs	= inputLayout;
	graphicsPipeline.InputLayout.NumElements		= _countof(inputLayout);

	// ブレンドステート設定
	graphicsPipeline.BlendState.AlphaToCoverageEnable	= false;
	graphicsPipeline.BlendState.IndependentBlendEnable	= false;

	// レンダーターゲット設定
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc{};
	renderTargetBlendDesc.BlendEnable				= false;
	renderTargetBlendDesc.RenderTargetWriteMask		= D3D12_COLOR_WRITE_ENABLE::D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.LogicOpEnable				= false;
	graphicsPipeline.BlendState.RenderTarget[0]		= renderTargetBlendDesc;

	// ラスタライザステートの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.MultisampleEnable		= false;
	rasterizerDesc.CullMode					= D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode					= D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
	rasterizerDesc.DepthClipEnable			= true;
	rasterizerDesc.FrontCounterClockwise	= false;
	rasterizerDesc.DepthBias				= D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp			= D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias		= D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDesc.AntialiasedLineEnable	= false;
	rasterizerDesc.ForcedSampleCount		= 0;
	rasterizerDesc.ConservativeRaster		= D3D12_CONSERVATIVE_RASTERIZATION_MODE::D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	graphicsPipeline.RasterizerState		= rasterizerDesc;

	// 深度ステンシルステートの設定
	graphicsPipeline.DepthStencilState.DepthEnable		= false;
	graphicsPipeline.DepthStencilState.StencilEnable	= false;

	// プリミティブ設定
	graphicsPipeline.IBStripCutValue		= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE::D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	graphicsPipeline.PrimitiveTopologyType	= D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// 三角形で構成

	// レンダーターゲット設定
	graphicsPipeline.NumRenderTargets	= 1;
	graphicsPipeline.RTVFormats[0]		= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

	// サンプル
	graphicsPipeline.SampleDesc.Count	= 1;
	graphicsPipeline.SampleDesc.Quality	= 0;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	HRESULT ret{};
	ComPtr<ID3DBlob> rootSignatureBlob{};
	ret = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1_0,
		rootSignatureBlob.GetAddressOf(),
		nullptr
	);
	if (FAILED(ret))
	{// ルートシグネチャの生成に失敗
		return false;
	}

	ret = m_device->CreateRootSignature(
		0,
		rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(),
		__uuidof(ID3D12RootSignature),
		(void**)m_rootSignature.GetAddressOf()
	);
	if (FAILED(ret))
	{// ルートシグネチャの生成に失敗
		return false;
	}

	graphicsPipeline.pRootSignature = m_rootSignature.Get();
	ret = m_device->CreateGraphicsPipelineState(
		&graphicsPipeline,
		__uuidof(ID3D12PipelineState),
		(void**)m_pipelineState.GetAddressOf()
	);
	if (FAILED(ret))
	{// パイプラインステートの生成
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

// ビューポートの設定
void Graphics::SetViewport(const int width, const int height)
{
	m_viewport.Width		= FLOAT(width);
	m_viewport.Height		= FLOAT(height);
	m_viewport.MaxDepth	= D3D12_MAX_DEPTH;
}

// シザーレクトの設定
void Graphics::SetScissorRect(const int width, const int height)
{
	m_scissorRect.left		= 0;
	m_scissorRect.top		= 0;
	m_scissorRect.right		= m_scissorRect.left + width;
	m_scissorRect.bottom	= m_scissorRect.top + height;
}
