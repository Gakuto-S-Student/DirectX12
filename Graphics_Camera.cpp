//==============================================================================
// Filename: Graphics_Camera.cpp
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include "Graphics.h"

#include "Graphics_Camera.h"
using namespace DirectX;

// 3DƒJƒƒ‰ƒ‚[ƒh‚É‚·‚é
void GraphicsCamera::Set3D()
{
	XMFLOAT3 eye(0, 5, -5);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);

	XMMATRIX view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	XMMATRIX projection = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,
		1280 / 780,
		0.1f,
		100.0f
	);

	Graphics::Get()->SetViewMatrix(view);
	Graphics::Get()->SetProjectionMatrix(projection);
}
