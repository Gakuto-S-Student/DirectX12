//==============================================================================
// Filename: Graphics_Camera.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#pragma once
#include <wrl/client.h>
#include <d3d12.h>

class GraphicsCamera
{
public:
	static GraphicsCamera* Get();

	bool Init();
	void Uninit();
	
	void Set3D();

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_viewBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_projectionBuffer;
};

