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

// マクロ定義
#define WORLD_MATRIX_INDEX		0	// ワールド行列設定用インデックス
#define VIEW_MATRIX_INDEX		1	// カメラ行列設定用インデックス
#define PROJECTION_MATRIX_INDEX 2	// 射影行列設定用インデックス

// 定数バッファの構造体
struct ConstantBuffer
{
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
};


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

	if (!this->CreateDepthBuffer(width, height))
	{// 深度バッファの生成に失敗
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

	if (!this->CreateConstantBuffers())
	{// 定数バッファの生成に失敗
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

	D3D12_CPU_DESCRIPTOR_HANDLE depthBufferHeapHandle = m_depthBufferHeap->GetCPUDescriptorHandleForHeapStart();
	m_commandList->OMSetRenderTargets(1, &renderTargetHeapHandle, false, &depthBufferHeapHandle);

	float clearColor[]{ 0.0f, 0.5f, 0.0f, 1.0f };
	m_commandList->ClearRenderTargetView(renderTargetHeapHandle, clearColor, 0, nullptr);	// 画面クリアコマンドの発行
	m_commandList->ClearDepthStencilView(													// 深度バッファのクリア
		m_depthBufferHeap->GetCPUDescriptorHandleForHeapStart(),
		D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH,
		D3D12_MAX_DEPTH,
		0,
		0,
		nullptr
	);

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

// 定数バッファの値更新(World)
void Graphics::SetWorldMatrix(const DirectX::XMMATRIX world)
{
	this->UpdateConstantBufferResouce(WORLD_MATRIX_INDEX, world);
}

// 定数バッファの値更新(View)
void Graphics::SetViewMatrix(const DirectX::XMMATRIX view)
{
	this->UpdateConstantBufferResouce(VIEW_MATRIX_INDEX, view);
}

// 定数バッファの値更新(Projection)
void Graphics::SetProjectionMatrix(const DirectX::XMMATRIX proj)
{
	this->UpdateConstantBufferResouce(PROJECTION_MATRIX_INDEX, proj);
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

// 深度バッファの生成
bool Graphics::CreateDepthBuffer(const int width, const int height)
{
	HRESULT ret{};
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension			= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width				= width;
	resourceDesc.Height				= height;
	resourceDesc.DepthOrArraySize	= 1;
	resourceDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	resourceDesc.SampleDesc.Count	= 1;
	resourceDesc.Flags				= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	resourceDesc.MipLevels			= 1;
	resourceDesc.Layout				= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Alignment			= 0;

	// ヒーププロパティ
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;

	// クリアバリュー
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.DepthStencil.Depth	= D3D12_MAX_DEPTH;
	clearValue.Format				= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;

	ret = m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		__uuidof(ID3D12Resource),
		(void**)m_depthBuffer.GetAddressOf()
	);
	if (FAILED(ret))
	{// 深度バッファの生成に失敗
		return false;
	}

	// ヒープの生成処理
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descriptorHeapDesc.NumDescriptors = 1;
	ret = m_device->CreateDescriptorHeap(
		&descriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap),
		(void**)m_depthBufferHeap.GetAddressOf()
	);
	if (FAILED(ret))
	{// ヒープの生成に失敗
		return false;
	}

	// 深度ビューの生成
	D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc{};
	viewDesc.Format			= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	viewDesc.ViewDimension	= D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
	viewDesc.Flags			= D3D12_DSV_FLAGS::D3D12_DSV_FLAG_NONE;
	m_device->CreateDepthStencilView(m_depthBuffer.Get(), &viewDesc, m_depthBufferHeap->GetCPUDescriptorHandleForHeapStart());

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
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
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

	// 深度ステンシルステートの設定
	graphicsPipeline.DepthStencilState.DepthEnable		= true;
	graphicsPipeline.DepthStencilState.StencilEnable	= false;
	graphicsPipeline.DSVFormat							= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	graphicsPipeline.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
	graphicsPipeline.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL;

	// ラスタライザステートの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.MultisampleEnable		= false;
	rasterizerDesc.CullMode					= D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
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

	D3D12_DESCRIPTOR_RANGE descriptorRange[2]{};
	// テクスチャのレンジ設定
	descriptorRange[0].NumDescriptors						= 1;	// テクスチャ一つ
	descriptorRange[0].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].BaseShaderRegister					= 0;	// 0番スロット
	descriptorRange[0].OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// 定数レンジ設定
	descriptorRange[1].NumDescriptors						= 1;	// 定数バッファ一つ
	descriptorRange[1].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descriptorRange[1].BaseShaderRegister					= 0;	// 0番スロット
	descriptorRange[1].OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameter[2]{};
	rootParameter[0].ParameterType							= D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[0].DescriptorTable.pDescriptorRanges		= &descriptorRange[0];
	rootParameter[0].DescriptorTable.NumDescriptorRanges	= 1;
	rootParameter[0].ShaderVisibility						= D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

	rootParameter[1].ParameterType							= D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[1].DescriptorTable.pDescriptorRanges		= &descriptorRange[1];
	rootParameter[1].DescriptorTable.NumDescriptorRanges	= 1;
	rootParameter[1].ShaderVisibility						= D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;

	rootSignatureDesc.pParameters	= rootParameter;
	rootSignatureDesc.NumParameters = 2;

	// サンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU			= D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV			= D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW			= D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor			= D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter				= D3D12_FILTER::D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.MaxLOD				= D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD				= 0.0f;
	samplerDesc.ComparisonFunc		= D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility	= D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers	= &samplerDesc;


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

// 定数バッファの生成処理
bool Graphics::CreateConstantBuffers()
{
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;					// mapができる設定
	heapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN; 
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;				

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension			= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width				= (sizeof(ConstantBuffer) + 0xff) & ~0xff;
	resourceDesc.Height				= 1;
	resourceDesc.DepthOrArraySize	= 1;
	resourceDesc.MipLevels			= 1;
	resourceDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count	= 1;
	resourceDesc.Flags				= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	resourceDesc.Layout				= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 定数バッファ
	HRESULT ret{};
	ret = m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)m_worldViewProjectionBuffer.GetAddressOf()
	);
	if (FAILED(ret))
	{// 定数バッファの生成に失敗
		return false;
	}

	// ヒープを作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask		= 0;
	heapDesc.NumDescriptors = 1;
	ret = m_device->CreateDescriptorHeap(
		&heapDesc,
		__uuidof(ID3D12DescriptorHeap),
		(void**)m_worldViewProjectionBufferHeap.GetAddressOf()
	);
	if (FAILED(ret))
	{// ヒープの生成に失敗
		return false;
	}

	// 定数バッファのビュー作成
	D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc{};
	viewDesc.BufferLocation = m_worldViewProjectionBuffer->GetGPUVirtualAddress();
	viewDesc.SizeInBytes	= UINT(m_worldViewProjectionBuffer->GetDesc().Width);
	m_device->CreateConstantBufferView(&viewDesc, m_worldViewProjectionBufferHeap->GetCPUDescriptorHandleForHeapStart());

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

// 定数バッファのリソース更新
bool Graphics::UpdateConstantBufferResouce(const int index, const XMMATRIX mat)
{
	HRESULT ret{};

	ConstantBuffer* mapMatrix;
	ret = m_worldViewProjectionBuffer->Map(0, nullptr, (void**)&mapMatrix);
	if (FAILED(ret))
	{
		return false;
	}

	// 
	XMMATRIX transposeMat = XMMatrixTranspose(mat);

	// データ更新
	switch (index)
	{
	case WORLD_MATRIX_INDEX		: mapMatrix->worldMatrix		= transposeMat;	break;
	case VIEW_MATRIX_INDEX		: mapMatrix->viewMatrix			= transposeMat;	break;
	case PROJECTION_MATRIX_INDEX: mapMatrix->projectionMatrix	= transposeMat;	break;
	}

	m_worldViewProjectionBuffer->Unmap(0, nullptr);

	m_commandList->SetDescriptorHeaps(1, m_worldViewProjectionBufferHeap.GetAddressOf());
	m_commandList->SetGraphicsRootDescriptorTable(1, m_worldViewProjectionBufferHeap->GetGPUDescriptorHandleForHeapStart());
	return true;
}
