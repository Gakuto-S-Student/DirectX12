//==============================================================================
// Filename: Application.cpp
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include "Graphics.h"
#include "Graphics_Cube.h"
#include "Graphics_Camera.h"

#include "Application.h"

#define APP_FOV 1.0f        /// 視野角度
#define APP_NEAR_Z 0.1f     /// 見える位置(手前)
#define APP_FAR_Z 100.0f	/// 見える位置(奥行)


// ウィンドウプロシージャ
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


// コンストラクタ
Application::Application(const int width, const int height, HINSTANCE hInstance)
	:ApplicationWindow(width, height, L"App", L"Sample", hInstance, WndProc),
	m_ScreenW(width),
	m_ScreenH(height),
	m_cubes()
{
}

// 初期化処理
bool Application::Init()
{
	if (!Graphics::Get()->Init(m_ScreenW, m_ScreenH, m_windowHandle))
	{
		return false;
	}

	m_cubes.resize(2);
	for (auto& cube : m_cubes)
	{
		cube = new GraphicsCube();
		cube->Init();
	}
	
	m_cubes[0]->SetPosition( 2, 0, 0);
	m_cubes[1]->SetPosition(-2, 0, 0);

	return true;
}

// 終了処理
void Application::Uninit()
{
	for (auto& cube : m_cubes)
	{
		cube->Uninit();
		delete cube;
	}

	Graphics::Get()->Uninit();
}

// 更新処理
void Application::Update()
{
}

// 描画処理
void Application::Draw()
{
	Graphics::Get()->Clear();

	GraphicsCamera::Get()->Set3D();

	for (auto& cube : m_cubes)
	{
		cube->Draw();
	}

	Graphics::Get()->Present();
}
