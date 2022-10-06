//==============================================================================
// Filename: Application_Window.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#pragma once

#include <Windows.h>

class ApplicationWindow
{
public:
	//---------------------------------------------
	/// ウィンドウの状態取得 
	///
	/// \return	ウィンドウが閉じられたら true 
	//---------------------------------------------
	bool Close();
	
	//---------------------------------------------
	/// デストラクタ
	///
	/// \return	none
	//---------------------------------------------
	virtual ~ApplicationWindow();


protected:
	ApplicationWindow() = delete;
	ApplicationWindow(const ApplicationWindow&) = delete;

	//---------------------------------------------
	/// コンストラクタ 
	///
	/// \param[in] ( width )
	/// \param[in] ( height )
	/// \param[in] ( caption )
	/// \param[in] ( className )
	/// \param[in] ( hInstance )
	/// \param[in] ( wndProc )  WNDPROC
	/// 
	/// \return	none
	//---------------------------------------------
	ApplicationWindow(
		/* [in] */  int width,
		/* [in] */  int height,
		/* [in] */  LPCWSTR caption,
		/* [in] */  LPCWSTR className,
		/* [in] */  HINSTANCE hInstance,
		/* [in] */  WNDPROC wndProc
	);

	//--------------------------------------------------------------------------
	HWND m_windowHandle;
	//--------------------------------------------------------------------------

	/// <summary>
	/// m_windowHandle  ウィンドウのハンドル
	/// </summary>
	

private:
	//--------------------------------------------------------------------------
	LPCWSTR		m_className;
	HINSTANCE	m_hInstance;
	//--------------------------------------------------------------------------

	/// <summary>
	/// m_className  ウィンドウの管理名
	/// m_hInstance  ハンドルインスタンス
	/// </summary>
};

