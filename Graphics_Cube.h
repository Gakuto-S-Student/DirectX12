//==============================================================================
// Filename: Graphics.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#pragma once

class GraphicsCube
{
public:
	//---------------------------------------------
	/// èâä˙âªèàóù 
	/// 
	/// \return	none
	//---------------------------------------------
	void Init();

	//---------------------------------------------
	/// èIóπèàóù 
	/// 
	/// \return	none
	//---------------------------------------------
	void Uninit();

	//---------------------------------------------
	/// ï`âÊèàóù 
	/// 
	/// \return	none
	//---------------------------------------------
	void Draw();

	void SetPosition(float x, float y, float z)
	{
		m_translate.x = x;
		m_translate.y = y;
		m_translate.z = z;
	}

private:
	//--------------------------------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_textureBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_worldMatrixBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_textureHeap;
	//--------------------------------------------------------------------------

	/// <summary>
	/// ID3D12Resource		m_vertexBuffer;
	/// </summary>

	//--------------------------------------------------------------------------
	DirectX::XMFLOAT3 m_translate;
	DirectX::XMFLOAT3 m_rotate;
	DirectX::XMFLOAT3 m_scale;
	//--------------------------------------------------------------------------

	/// <summary>
	/// ID3D12Resource		m_vertexBuffer;
	/// </summary>
};

