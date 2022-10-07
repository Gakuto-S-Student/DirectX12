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


// �C���X�^���X�̎擾
Graphics* Graphics::Get()
{
	static Graphics ret;
	return &ret;
}

// ����������
bool Graphics::Init(const int width, const int height, const HWND hWnd)
{
	if (!this->CreateDeviceAndSwapChain(width, height, hWnd))
	{// �f�o�C�X�E�X���b�v�`�F�C���̐����Ɏ��s
		return false;
	}

	if (!this->CreateRenderTargetView())
	{// �����_�[�^�[�Q�b�g�̐����Ɏ��s
		return false;
	}

	if (!this->CreateFence())
	{// �t�F���X�̐����Ɏ��s
		return false;
	}

	if (!this->CreateGraphicsPipeline())
	{// �O���t�B�b�N�X�p�C�v���C���̐����Ɏ��s
		return false;
	}

	this->SetViewport(width, height);
	this->SetScissorRect(width, height);

	return true;
}

// �I������
void Graphics::Uninit()
{
}

// ��ʃN���A
void Graphics::Clear()
{
	// ���݂̃o�b�t�@�̃C���f�b�N�X���擾
	UINT index = m_swapChain->GetCurrentBackBufferIndex();
	this->SetResourceBarrier(
		index,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT,		
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET	// �����烌���_�[�^�[�Q�b�g�Ƃ��Ďg�p
	);

	// �p�C�v���C���ݒ�
	m_commandList->SetPipelineState(m_pipelineState.Get());

	// �����_�[�^�[�Q�b�g�̎w��
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHeapHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetHeapHandle.ptr += index * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_commandList->OMSetRenderTargets(1, &renderTargetHeapHandle, false, nullptr);

	float clearColor[]{ 0.0f, 0.5f, 0.0f, 1.0f };
	m_commandList->ClearRenderTargetView(renderTargetHeapHandle, clearColor, 0, nullptr);	// ��ʃN���A�R�}���h�̔��s

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
}

// �o�b�t�@�؂�ւ� �`��R�}���h���s
void Graphics::Present()
{
	// ���݂̃o�b�t�@�̃C���f�b�N�X���擾
	UINT index = m_swapChain->GetCurrentBackBufferIndex();

	this->SetResourceBarrier(
		index,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET,		// �O�̏�Ԃ������_�[�^�[�Q�b�g
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT				
	);

	// �R�}���h���߂����
	m_commandList->Close();

	// �R�}���h���X�g�̎��s
	std::vector<ID3D12CommandList*> commandLists{ m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(commandLists.size(), &commandLists[0]);

	// �L���[�̖��ߏ������I������܂ő҂�
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

	// �R�}���h�E���߂̃��Z�b�g
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	// �t���b�v
	m_swapChain->Present(1, 0);
}

// �f�o�C�X�̃A�h���X���擾
ID3D12Device* Graphics::Device()
{
	return m_device.Get();
}

// �R�}���h���X�g�̃A�h���X���擾
ID3D12GraphicsCommandList* Graphics::Context()
{
	return m_commandList.Get();
}

// �f�o�C�X�ƃX���b�v�`�F�C���̐���
bool Graphics::CreateDeviceAndSwapChain(const int width, const int height, const HWND hWnd)
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
	{// �X���b�v�`�F�C���̐����Ɏ��s
		return false;
	}

	return true;
}

// �����_�[�^�[�Q�b�g�̐���
bool Graphics::CreateRenderTargetView()
{
	HRESULT ret{};

	// �����_�[�^�[�Q�b�g�̐�������
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask		= 0;
	heapDesc.NumDescriptors = k_backBufferNum;
	heapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ret = m_device->CreateDescriptorHeap(&heapDesc, __uuidof(ID3D12DescriptorHeap), (void**)m_renderTargetViewHeap.GetAddressOf());
	if (FAILED(ret))
	{// �q�[�v�̐����Ɏ��s
		return false;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	ret = m_swapChain->GetDesc(&swapChainDesc);
	if (FAILED(ret))
	{
		return false;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();	// �q�[�v�̐擪�A�h���X�擾
	for (UINT i = 0; i < k_backBufferNum; ++i)
	{// �����_�[�^�[�Q�b�g�p�̃o�b�N�o�b�t�@���쐬
		ret = m_swapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)m_backBuffers[i].GetAddressOf());
		m_device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, handle);
		handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);	// RTV1���̃T�C�Y�����C���N�������g
	}

	return true;
}

// �t�F���X�̐���
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
	{// �t�F���X�̐����Ɏ��s
		return false;
	}

	return true;
}

// �p�C�v���C���̐���
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

	// �V�F�[�_�f�[�^�̐ݒ�
	graphicsPipeline.VS.pShaderBytecode	= vertexShader.data();
	graphicsPipeline.VS.BytecodeLength	= vertexShader.size();
	graphicsPipeline.PS.pShaderBytecode	= pixelShader.data();
	graphicsPipeline.PS.BytecodeLength	= pixelShader.size();
	
	// ���_���C�A�E�g�̐ݒ�
	graphicsPipeline.InputLayout.pInputElementDescs	= inputLayout;
	graphicsPipeline.InputLayout.NumElements		= _countof(inputLayout);

	// �u�����h�X�e�[�g�ݒ�
	graphicsPipeline.BlendState.AlphaToCoverageEnable	= false;
	graphicsPipeline.BlendState.IndependentBlendEnable	= false;

	// �����_�[�^�[�Q�b�g�ݒ�
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc{};
	renderTargetBlendDesc.BlendEnable				= false;
	renderTargetBlendDesc.RenderTargetWriteMask		= D3D12_COLOR_WRITE_ENABLE::D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.LogicOpEnable				= false;
	graphicsPipeline.BlendState.RenderTarget[0]		= renderTargetBlendDesc;

	// ���X�^���C�U�X�e�[�g�̐ݒ�
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

	// �[�x�X�e���V���X�e�[�g�̐ݒ�
	graphicsPipeline.DepthStencilState.DepthEnable		= false;
	graphicsPipeline.DepthStencilState.StencilEnable	= false;

	// �v���~�e�B�u�ݒ�
	graphicsPipeline.IBStripCutValue		= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE::D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	graphicsPipeline.PrimitiveTopologyType	= D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// �O�p�`�ō\��

	// �����_�[�^�[�Q�b�g�ݒ�
	graphicsPipeline.NumRenderTargets	= 1;
	graphicsPipeline.RTVFormats[0]		= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

	// �T���v��
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
	{// ���[�g�V�O�l�`���̐����Ɏ��s
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
	{// ���[�g�V�O�l�`���̐����Ɏ��s
		return false;
	}

	graphicsPipeline.pRootSignature = m_rootSignature.Get();
	ret = m_device->CreateGraphicsPipelineState(
		&graphicsPipeline,
		__uuidof(ID3D12PipelineState),
		(void**)m_pipelineState.GetAddressOf()
	);
	if (FAILED(ret))
	{// �p�C�v���C���X�e�[�g�̐���
		return false;
	}

	return true;
}

// ���\�[�X�o���A�̐ݒ�
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

// �r���[�|�[�g�̐ݒ�
void Graphics::SetViewport(const int width, const int height)
{
	m_viewport.Width		= FLOAT(width);
	m_viewport.Height		= FLOAT(height);
	m_viewport.MaxDepth	= D3D12_MAX_DEPTH;
}

// �V�U�[���N�g�̐ݒ�
void Graphics::SetScissorRect(const int width, const int height)
{
	m_scissorRect.left		= 0;
	m_scissorRect.top		= 0;
	m_scissorRect.right		= m_scissorRect.left + width;
	m_scissorRect.bottom	= m_scissorRect.top + height;
}
