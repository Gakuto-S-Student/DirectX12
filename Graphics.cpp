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

// �}�N����`
#define WORLD_MATRIX_INDEX		0	// ���[���h�s��ݒ�p�C���f�b�N�X
#define VIEW_MATRIX_INDEX		1	// �J�����s��ݒ�p�C���f�b�N�X
#define PROJECTION_MATRIX_INDEX 2	// �ˉe�s��ݒ�p�C���f�b�N�X

// �萔�o�b�t�@�̍\����
struct ConstantBuffer
{
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
};


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

	if (!this->CreateDepthBuffer(width, height))
	{// �[�x�o�b�t�@�̐����Ɏ��s
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

	if (!this->CreateConstantBuffers())
	{// �萔�o�b�t�@�̐����Ɏ��s
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

	D3D12_CPU_DESCRIPTOR_HANDLE depthBufferHeapHandle = m_depthBufferHeap->GetCPUDescriptorHandleForHeapStart();
	m_commandList->OMSetRenderTargets(1, &renderTargetHeapHandle, false, &depthBufferHeapHandle);

	float clearColor[]{ 0.0f, 0.5f, 0.0f, 1.0f };
	m_commandList->ClearRenderTargetView(renderTargetHeapHandle, clearColor, 0, nullptr);	// ��ʃN���A�R�}���h�̔��s
	m_commandList->ClearDepthStencilView(													// �[�x�o�b�t�@�̃N���A
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

// �萔�o�b�t�@�̒l�X�V(World)
void Graphics::SetWorldMatrix(const DirectX::XMMATRIX world)
{
	this->UpdateConstantBufferResouce(WORLD_MATRIX_INDEX, world);
}

// �萔�o�b�t�@�̒l�X�V(View)
void Graphics::SetViewMatrix(const DirectX::XMMATRIX view)
{
	this->UpdateConstantBufferResouce(VIEW_MATRIX_INDEX, view);
}

// �萔�o�b�t�@�̒l�X�V(Projection)
void Graphics::SetProjectionMatrix(const DirectX::XMMATRIX proj)
{
	this->UpdateConstantBufferResouce(PROJECTION_MATRIX_INDEX, proj);
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

// �[�x�o�b�t�@�̐���
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

	// �q�[�v�v���p�e�B
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;

	// �N���A�o�����[
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
	{// �[�x�o�b�t�@�̐����Ɏ��s
		return false;
	}

	// �q�[�v�̐�������
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descriptorHeapDesc.NumDescriptors = 1;
	ret = m_device->CreateDescriptorHeap(
		&descriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap),
		(void**)m_depthBufferHeap.GetAddressOf()
	);
	if (FAILED(ret))
	{// �q�[�v�̐����Ɏ��s
		return false;
	}

	// �[�x�r���[�̐���
	D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc{};
	viewDesc.Format			= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	viewDesc.ViewDimension	= D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
	viewDesc.Flags			= D3D12_DSV_FLAGS::D3D12_DSV_FLAG_NONE;
	m_device->CreateDepthStencilView(m_depthBuffer.Get(), &viewDesc, m_depthBufferHeap->GetCPUDescriptorHandleForHeapStart());

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
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
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

	// �[�x�X�e���V���X�e�[�g�̐ݒ�
	graphicsPipeline.DepthStencilState.DepthEnable		= true;
	graphicsPipeline.DepthStencilState.StencilEnable	= false;
	graphicsPipeline.DSVFormat							= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	graphicsPipeline.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
	graphicsPipeline.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL;

	// ���X�^���C�U�X�e�[�g�̐ݒ�
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

	D3D12_DESCRIPTOR_RANGE descriptorRange[2]{};
	// �e�N�X�`���̃����W�ݒ�
	descriptorRange[0].NumDescriptors						= 1;	// �e�N�X�`�����
	descriptorRange[0].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].BaseShaderRegister					= 0;	// 0�ԃX���b�g
	descriptorRange[0].OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// �萔�����W�ݒ�
	descriptorRange[1].NumDescriptors						= 1;	// �萔�o�b�t�@���
	descriptorRange[1].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descriptorRange[1].BaseShaderRegister					= 0;	// 0�ԃX���b�g
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

	// �T���v���[�̐ݒ�
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

// �萔�o�b�t�@�̐�������
bool Graphics::CreateConstantBuffers()
{
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;					// map���ł���ݒ�
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

	// �萔�o�b�t�@
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
	{// �萔�o�b�t�@�̐����Ɏ��s
		return false;
	}

	// �q�[�v���쐬
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
	{// �q�[�v�̐����Ɏ��s
		return false;
	}

	// �萔�o�b�t�@�̃r���[�쐬
	D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc{};
	viewDesc.BufferLocation = m_worldViewProjectionBuffer->GetGPUVirtualAddress();
	viewDesc.SizeInBytes	= UINT(m_worldViewProjectionBuffer->GetDesc().Width);
	m_device->CreateConstantBufferView(&viewDesc, m_worldViewProjectionBufferHeap->GetCPUDescriptorHandleForHeapStart());

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

// �萔�o�b�t�@�̃��\�[�X�X�V
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

	// �f�[�^�X�V
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
