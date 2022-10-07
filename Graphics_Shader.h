//==============================================================================
// Filename: Graphics_Shader.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#pragma once
#include <string>

class GraphicsShader
{
public:
	//---------------------------------------------
	/// Load shader file
	///
	/// \param[in] ( fileName )
	/// \param[out] ( buffer )
	/// 
	/// \return true on success
	//---------------------------------------------
	static bool LoadFile(
		/* [in] */  const std::string fileName,
		/* [out] */ std::string& buffer
	);
};

