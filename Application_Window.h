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
	/// �E�B���h�E�̏�Ԏ擾 
	///
	/// \return	�E�B���h�E������ꂽ�� true 
	//---------------------------------------------
	bool Close();
	
	//---------------------------------------------
	/// �f�X�g���N�^
	///
	/// \return	none
	//---------------------------------------------
	virtual ~ApplicationWindow();


protected:
	ApplicationWindow() = delete;
	ApplicationWindow(const ApplicationWindow&) = delete;

	//---------------------------------------------
	/// �R���X�g���N�^ 
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
	/// m_windowHandle  �E�B���h�E�̃n���h��
	/// </summary>
	

private:
	//--------------------------------------------------------------------------
	LPCWSTR		m_className;
	HINSTANCE	m_hInstance;
	//--------------------------------------------------------------------------

	/// <summary>
	/// m_className  �E�B���h�E�̊Ǘ���
	/// m_hInstance  �n���h���C���X�^���X
	/// </summary>
};

