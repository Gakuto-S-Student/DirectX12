//==============================================================================
// Filename: Application.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#pragma once
#include <vector>

#include "Application_Window.h"


class GraphicsCube;

class Application : public ApplicationWindow
{
public:
	//---------------------------------------------
	/// コンストラクタ 
	///
	/// \param[in] ( width )
	/// \param[in] ( height )
	/// \param[in] ( hInstance )
	/// 
	/// \return	none
	//---------------------------------------------
	Application(
		/* [in] */  const int width,
		/* [in] */  const int height,
		/* [in] */  HINSTANCE hInstance
	);

	//---------------------------------------------
	/// 初期化処理 
	///
	/// \return	true 初期化成功
	//---------------------------------------------
	bool Init();

	//---------------------------------------------
	/// 終了処理 
	///
	/// \return	none
	//---------------------------------------------
	void Uninit();

	//---------------------------------------------
	/// 更新処理
	///
	/// \return	none
	//---------------------------------------------
	void Update();

	//---------------------------------------------
	/// 描画処理 
	///
	/// \return	none
	//---------------------------------------------
	void Draw();

	//--------------------------------------------------------------------------
	int m_ScreenW;
	int m_ScreenH;
	//--------------------------------------------------------------------------

	/// <summary>
	/// int m_ScreenW;	ウィンドウ幅
	/// int m_ScreenH;	ウィンドウ高さ
	/// </summary>
	

private:
	//--------------------------------------------------------------------------
	std::vector<GraphicsCube*> m_cubes;
	//--------------------------------------------------------------------------

	/// <summary>
	/// GraphicsCube* m_cube;  キューブデータ
	/// </summary>
};

