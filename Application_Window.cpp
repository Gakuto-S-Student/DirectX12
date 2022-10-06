//==============================================================================
// Filename: Application_Window.cpp
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include "Application_Window.h"

// �R���X�g���N�^
ApplicationWindow::ApplicationWindow(int width, int height, LPCWSTR caption, LPCWSTR className, HINSTANCE hInstance, WNDPROC wndProc)
    :m_className(className),
    m_windowHandle(nullptr),
    m_hInstance(hInstance)
{
    WNDCLASSEX wcex{};
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_CLASSDC;
    wcex.lpfnWndProc    = wndProc;
    wcex.hInstance      = hInstance;
    wcex.lpszClassName  = className;
    RegisterClassEx(&wcex);

    m_windowHandle = CreateWindow(
        m_className,
        caption,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width, height,
        nullptr,
        nullptr,
        m_hInstance,
        nullptr
    );

    // �E�B���h�E�̕\���J�n
    ShowWindow(m_windowHandle, true);
    UpdateWindow(m_windowHandle);
}

// �f�X�g���N�^
ApplicationWindow::~ApplicationWindow()
{
    UnregisterClass(m_className, m_hInstance);
}

// �E�B���h�E���
bool ApplicationWindow::Close()
{
    MSG msg;
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return true;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return false;
}
