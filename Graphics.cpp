//==============================================================================
// Filename: Graphics.cpp
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include "Graphics.h"
using namespace Microsoft::WRL;


// �C���X�^���X�̎擾
Graphics* Graphics::Get()
{
	static Graphics ret;
	return &ret;
}

// ����������
bool Graphics::Init(const int width, const int height, const HWND hWnd)
{
	HRESULT ret{};

	// �f�o�C�X�̐���
	ret = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0,
		__uuidof(ID3D12Device),
		(void**)m_device.GetAddressOf()
	);
	if (FAILED(ret))
	{// �f�o�C�X�̐��������Ɏ��s
		return false;
	}

	// �R�}���h�A���P�[�^�̐���
	ret = m_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator),
		(void**)m_commandAllocator.GetAddressOf()
	);
	if (FAILED(ret))
	{// �R�}���h�A���P�[�^�̐��������Ɏ��s
		return false;
	}

	// �R�}���h���X�g�̐���
	ret = m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		nullptr,
		__uuidof(ID3D12GraphicsCommandList),
		(void**)m_commandList.GetAddressOf()
	);
	if (FAILED(ret))
	{// �R�}���h���X�g�̐��������Ɏ��s
		return false;
	}

	// �R�}���h�L���[�̐���
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

	// �t�@�N�g���[�̐���
	ComPtr<IDXGIFactory6> factory{};
	ret = CreateDXGIFactory2(
		0,
		__uuidof(IDXGIFactory6),
		(void**)factory.GetAddressOf()
	);
	if (FAILED(ret) || !factory)
	{// �t�@�N�g���[�̐����Ɏ��s
		return false;
	}


	// �X���b�v�`�F�C���̐���
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width					= width;
	swapChainDesc.Height				= height;
	swapChainDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.Stereo				= false;
	swapChainDesc.SampleDesc.Count		= 1;
	swapChainDesc.SampleDesc.Quality	= 0;
	swapChainDesc.BufferUsage			= DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount			= k_BackBufferNum;
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
	{// �X���b�v�`�F�C���̐����Ɏ��s
		return false;
	}
	


	return true;
}

// �I������
void Graphics::Uninit()
{
}

// ��ʃN���A
void Graphics::Clear()
{
	
}

// �o�b�t�@�؂�ւ� �`��R�}���h���s
void Graphics::Present()
{
	
}
