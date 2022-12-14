//==============================================================================
// Filename: Graphics.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class Graphics
{
public:
	//---------------------------------------------
	/// インスタンスの取得 
	/// 
	/// \return	Graphics class pointer
	//---------------------------------------------
	static Graphics* Get();

	//---------------------------------------------
	/// DirectX12の初期化処理 
	/// 
	/// \param[in] ( width )
	/// \param[in] ( height )
	/// \param[in] ( hWnd )
	/// 
	/// \return	none
	//---------------------------------------------
	bool Init(
		/* [in] */  const int width,
		/* [in] */  const int height,
		/* [in] */  const HWND hWnd
	);
	
	//---------------------------------------------
	/// DirectX12の終了処理 
	/// 
	/// \return	none
	//---------------------------------------------
	void Uninit();

	//---------------------------------------------
	/// 画面クリア処理 
	/// 
	/// \return	none
	//---------------------------------------------
	void Clear();

	//---------------------------------------------
	/// バッファ切り替え コマンド実行 処理 
	/// 
	/// \return	none
	//---------------------------------------------
	void Present();

	//---------------------------------------------
	/// デバイスポインタの取得 
	/// 
	/// \return	ID3D12Device pointer 成功
	//---------------------------------------------
	ID3D12Device* Device();

	//---------------------------------------------
	/// コマンドリストポインタの取得 
	/// 
	/// \return	ID3D12GraphicsCommandList pointer 成功
	//---------------------------------------------
	ID3D12GraphicsCommandList* Context();

	//---------------------------------------------
	/// world行列の設定 
	/// 
	/// \param[in] ( world )
	/// 
	/// \return	none
	//---------------------------------------------
	void SetWorldMatrix(ID3D12Resource* world);

	//---------------------------------------------
	/// view行列の設定 
	/// 
	/// \param[in] ( view )
	/// 
	/// \return	none
	//---------------------------------------------
	void SetViewMatrix(ID3D12Resource* view);

	//---------------------------------------------
	/// projection行列の設定 
	///
	/// \param[in] ( projection ) 
	///  
	/// \return	none
	//---------------------------------------------
	void SetProjectionMatrix(ID3D12Resource* projection);

	//---------------------------------------------
	/// projection行列の設定 
	/// 
	/// \param[in] ( textureHeap )
	/// 
	/// \return	none
	//---------------------------------------------
	void SetTexture(ID3D12DescriptorHeap* textureHeap);

private:
	//---------------------------------------------
	/// デバイスとスワップチェインの生成 
	/// 
	/// \param[in] ( width )
	/// \param[in] ( height )
	/// \param[in] ( hWnd )
	/// 
	/// \return	true 成功
	//---------------------------------------------
	bool CreateDeviceAndSwapChain(
		/* [in] */  const int width,
		/* [in] */  const int height,
		/* [in] */  const HWND hWnd
		);

	//---------------------------------------------
	/// レンダーターゲットの生成
	/// 
	/// \return	true 成功
	//---------------------------------------------
	bool CreateRenderTargetView();

	//---------------------------------------------
	/// 深度バッファの生成
	///
	/// \param[in] ( width )
	/// \param[in] ( height )
	///  
	/// \return	true 成功
	//---------------------------------------------
	bool CreateDepthBuffer(
		/* [in] */  const int width,
		/* [in] */  const int height
	);

	//---------------------------------------------
	/// フェンスの生成
	/// 
	/// \return	true 成功
	//---------------------------------------------
	bool CreateFence();

	//---------------------------------------------
	/// パイプラインの生成
	/// 
	/// \return	true 成功
	//---------------------------------------------
	bool CreateGraphicsPipeline();

	//---------------------------------------------
	/// リソースバリアの設定
	/// 
	/// \param[in] ( index )
	/// \param[in] ( before )
	/// \param[in] ( after )
	/// 
	/// \return	none
	//---------------------------------------------
	void SetResourceBarrier(
		/* [in] */  const UINT index,
		/* [in] */  D3D12_RESOURCE_STATES before,
		/* [in] */  D3D12_RESOURCE_STATES after
	);

	//---------------------------------------------
	/// ビューポートの設定
	/// 
	/// \param[in] ( width )
	/// \param[in] ( height )
	/// 
	/// \return	none
	//---------------------------------------------
	void SetViewport(
		/* [in] */  const int width,
		/* [in] */  const int height
	);

	//---------------------------------------------
	/// シザーレクトの設定
	/// 
	/// \param[in] ( width )
	/// \param[in] ( height )
	/// 
	/// \return	none
	//---------------------------------------------
	void SetScissorRect(
		/* [in] */  const int width,
		/* [in] */  const int height
	);

	//--------------------------------------------------------------------------
	static const UINT									k_backBufferNum = 2;
	Microsoft::WRL::ComPtr<ID3D12Device>				m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>			m_commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4>				m_swapChain;
	//--------------------------------------------------------------------------

	/// <summary>
	/// ID3D12Device				m_device;			DirectX12 デバイス
	/// ID3D12CommandAllocator		m_commandAllocator;	DirectX12 コマンドアロケータ
	/// ID3D12GraphicsCommandList	m_commandList;		DirectX12 コマンドリスト
	/// ID3D12CommandQueue			m_commandQueue;		DirectX12 コマンドキュー
	/// </summary>


	//--------------------------------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_renderTargetViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>				m_backBuffers[k_backBufferNum];
	//--------------------------------------------------------------------------

	/// <summary>
	/// ID3D12DescriptorHeap		m_renderTargetViewHeap;			DirectX12 レンダーターゲット用のヒープ領域
	/// ID3D12Resource				m_backBuffers[k_backBufferNum]; DirectX12 レンダーターゲット用バックバッファ(ダブルバッファ)
	/// </summary>
	
	//--------------------------------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_depthBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_depthBufferHeap;
	//--------------------------------------------------------------------------

	/// <summary>
	/// ID3D12Resource			m_depthBuffer;		DirectX12 深度バッファ
	/// ID3D12DescriptorHeap	m_depthBufferHeap;	DirectX12 深度バッファ用ヒープ
	/// </summary>

	//--------------------------------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D12Fence>		m_fence;
	UINT									m_fenceValue = 0;
	//--------------------------------------------------------------------------

	/// <summary>
	/// ID3D12Fence					m_fence;		DirectX12 フェンス
	/// UINT						m_fenceValue;	DirectX12 フェンスバリュー			
	/// </summary>

	//--------------------------------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D12RootSignature>	m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>	m_pipelineState;
	//--------------------------------------------------------------------------

	/// <summary>
	///ID3D12RootSignature	m_rootSignature;	// ルートシグネチャ
	///ID3D12PipelineState	m_pipelineState;	// パイプラインステート
	/// </summary>
	
	//--------------------------------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_worldViewProjectionBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_worldViewProjectionBufferHeap;
	//--------------------------------------------------------------------------

	/// <summary>
	///ID3D12Resource		m_worldViewProjectionBuffer;		// コンスタントバッファ
	///ID3D12DescriptorHeap	m_worldViewProjectionBufferHeap;	// コンスタントバッファヒープ
	/// </summary>
	
	//--------------------------------------------------------------------------
	D3D12_VIEWPORT	m_viewport{};
	D3D12_RECT		m_scissorRect{};
	//--------------------------------------------------------------------------

	/// <summary>
	/// D3D12_VIEWPORT	m_viewport;
	/// D3D12_RECT		m_scissorRect;
	/// </summary>
};

