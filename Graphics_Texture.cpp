//==============================================================================
// Filename: Graphics_Texture.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include "Graphics.h"

#include "Graphics_Texture.h"

// �e�N�X�`���̐���
void GraphicsTexture::CreateTexture(ID3D12Resource** textureBuffer, ID3D12DescriptorHeap** descriptorHeap)
{
	const int width = 256;
	const int height = 256;

	// �e�N�X�`���̐�������
	struct RGBA
	{
		unsigned char R, G, B, A;
	}rgba[width * height];

	const RGBA white{ 0xff, 0xff, 0xff, 0xff };
	const RGBA black{ 0x00, 0x00, 0x00, 0xff };

	// �`�F�b�J�[�͗l�̐���
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (x < width / 2 && y < height / 2)
			{
				rgba[y * width + x] = white;
			}
			else if (x > width / 2 && y > height / 2)
			{
				rgba[y * width + x] = white;
			}
			else
			{
				rgba[y * width + x] = black;
			}
		}
	}

	// WriteToSubResource�œ]�����邽�߂̐ݒ�
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_CUSTOM;
	heapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_L0;	//CPU���璼�ڍs��

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	resourceDesc.Width				= width;
	resourceDesc.Height				= height;
	resourceDesc.DepthOrArraySize	= 1;
	resourceDesc.SampleDesc.Count	= 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels			= 1;
	resourceDesc.Dimension			= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Layout				= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags				= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	// �e�N�X�`�����\�[�X�̐ݒ�
	HRESULT ret{};
	ID3D12Resource* texBuf;
	ret = Graphics::Get()->Device()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&texBuf
	);

	// �e�N�X�`����]������
	ret = texBuf->WriteToSubresource(
		0,
		nullptr,
		rgba,
		sizeof(RGBA) * width,
		sizeof(RGBA) * width * height
	);

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask			= 0;
	descriptorHeapDesc.NumDescriptors	= 1;
	descriptorHeapDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	ID3D12DescriptorHeap* descHeap;
	ret = Graphics::Get()->Device()->CreateDescriptorHeap(
		&descriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap),
		(void**)&descHeap
	);

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceView{};
	shaderResourceView.Format					= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	shaderResourceView.Shader4ComponentMapping	= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceView.ViewDimension			= D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceView.Texture2D.MipLevels		= 1;

	Graphics::Get()->Device()->CreateShaderResourceView(
		texBuf,
		&shaderResourceView,
		descHeap->GetCPUDescriptorHandleForHeapStart()
	);

	// �A�h���X���R�s�[
	*textureBuffer	= texBuf;
	*descriptorHeap	= descHeap;
}
