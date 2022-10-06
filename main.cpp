//==============================================================================
// Filename: main.cpp
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================


#include "Application.h"

int __stdcall WinMain(
	_In_     HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_     LPSTR lpCmdLine,
	_In_     int nCmdShow
)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	Application app(1280, 780, hInstance);
	app.Init();
	while (!app.Close())
	{
		app.Update();
		app.Draw();
	}
	app.Uninit();

	return 0;
}