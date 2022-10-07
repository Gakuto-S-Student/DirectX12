//==============================================================================
// Filename: Graphics.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class Graphics
{
public:
	//---------------------------------------------
	/// �C���X�^���X�̎擾 
	/// 
	/// \return	Graphics class pointer
	//---------------------------------------------
	static Graphics* Get();

	//---------------------------------------------
	/// DirectX12�̏��������� 
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
	/// DirectX12�̏I������ 
	/// 
	/// \return	none
	//---------------------------------------------
	void Uninit();

	//---------------------------------------------
	/// ��ʃN���A���� 
	/// 
	/// \return	none
	//---------------------------------------------
	void Clear();

	//---------------------------------------------
	/// �o�b�t�@�؂�ւ� �R�}���h���s ���� 
	/// 
	/// \return	none
	//---------------------------------------------
	void Present();

private:
	//---------------------------------------------
	/// �f�o�C�X�ƃX���b�v�`�F�C���̐��� 
	/// 
	/// \param[in] ( width )
	/// \param[in] ( height )
	/// \param[in] ( hWnd )
	/// 
	/// \return	true ����
	//---------------------------------------------
	bool CreateDeviceAndSwapChain(
		/* [in] */  const int width,
		/* [in] */  const int height,
		/* [in] */  const HWND hWnd
		);

	//---------------------------------------------
	/// �����_�[�^�[�Q�b�g�̐���
	/// 
	/// \return	none
	//---------------------------------------------
	bool CreateRenderTargetView();

	//---------------------------------------------
	/// �t�F���X�̐���
	/// 
	/// \return	none
	//---------------------------------------------
	bool CreateFence();

	//---------------------------------------------
	/// ���\�[�X�o���A�̐ݒ�
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

	//--------------------------------------------------------------------------
	static const UINT									k_backBufferNum = 2;
	Microsoft::WRL::ComPtr<ID3D12Device>				m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>			m_commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4>				m_swapChain;
	//--------------------------------------------------------------------------

	/// <summary>
	/// ID3D12Device				m_device;			DirectX12 �f�o�C�X
	/// ID3D12CommandAllocator		m_commandAllocator;	DirectX12 �R�}���h�A���P�[�^
	/// ID3D12GraphicsCommandList	m_commandList;		DirectX12 �R�}���h���X�g
	/// ID3D12CommandQueue			m_commandQueue;		DirectX12 �R�}���h�L���[
	/// </summary>


	//--------------------------------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_renderTargetViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>				m_backBuffers[k_backBufferNum];
	//--------------------------------------------------------------------------

	/// <summary>
	/// ID3D12DescriptorHeap		m_renderTargetViewHeap;			DirectX12 �����_�[�^�[�Q�b�g�p�̃q�[�v�̈�
	/// ID3D12Resource				m_backBuffers[k_backBufferNum]; DirectX12 �����_�[�^�[�Q�b�g�p�o�b�N�o�b�t�@(�_�u���o�b�t�@)
	/// </summary>
	
	//--------------------------------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D12Fence>		m_fence;
	UINT									m_fenceValue;
	//--------------------------------------------------------------------------

	/// <summary>
	/// ID3D12Fence					m_fence;		DirectX12 �t�F���X
	/// UINT						m_fenceValue;	DirectX12 �t�F���X�o�����[			
	/// </summary>
};

