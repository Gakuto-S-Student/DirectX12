//==============================================================================
// Filename: Application.cpp
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include "Graphics.h"
#include "Graphics_Cube.h"
#include "Graphics_Camera.h"

#include "Application.h"

#define APP_FOV 1.0f        /// ����p�x
#define APP_NEAR_Z 0.1f     /// ������ʒu(��O)
#define APP_FAR_Z 100.0f	/// ������ʒu(���s)


// �E�B���h�E�v���V�[�W��
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


// �R���X�g���N�^
Application::Application(const int width, const int height, HINSTANCE hInstance)
	:ApplicationWindow(width, height, L"App", L"Sample", hInstance, WndProc),
	m_ScreenW(width),
	m_ScreenH(height),
	m_cube(nullptr)
{
}

// ����������
bool Application::Init()
{
	if (!Graphics::Get()->Init(m_ScreenW, m_ScreenH, m_windowHandle))
	{
		return false;
	}

	m_cube = new GraphicsCube();
	m_cube->Init();

	return true;
}

// �I������
void Application::Uninit()
{
	m_cube->Uninit();
	delete m_cube;

	Graphics::Get()->Uninit();
}

// �X�V����
void Application::Update()
{
}

// �`�揈��
void Application::Draw()
{
	Graphics::Get()->Clear();

	GraphicsCamera::Set3D();

	m_cube->Draw();

	Graphics::Get()->Present();
}
