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
	/// 初期化処理 
	/// 
	/// \return	none
	//---------------------------------------------
	void Init();

	//---------------------------------------------
	/// 終了処理 
	/// 
	/// \return	none
	//---------------------------------------------
	void Uninit();

	//---------------------------------------------
	/// 描画処理 
	/// 
	/// \return	none
	//---------------------------------------------
	void Draw();


private:
	//--------------------------------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW						m_vertexBufferView;
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_textureBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_textureHeap;
	//--------------------------------------------------------------------------

	/// <summary>
	/// ID3D12Resource		m_vertexBuffer;
	/// </summary>
};

