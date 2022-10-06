//==============================================================================
// Filename: Application.cpp
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include "Graphics.h"

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
	m_cube(nullptr)
{
}

// 初期化処理
bool Application::Init()
{
	if (!Graphics::Get()->Init(m_ScreenW, m_ScreenH, m_windowHandle))
	{
		return false;
	}

	return true;
}

// 終了処理
void Application::Uninit()
{
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


	Graphics::Get()->Present();
}
