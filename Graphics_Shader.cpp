//==============================================================================
// Filename: Graphics_Shader.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================

#include <fstream>
#include "Graphics.h"

#include "Graphics_Shader.h"


// *.csoファイル読み込み
bool GraphicsShader::LoadFile(const std::string fileName, std::string& buffer)
{
	std::ifstream ifs(fileName.c_str(), std::ios::binary);
	if (ifs.fail())
	{// ファイルのオープンに失敗
		return false;
	}

	ifs.seekg(0, std::ios::end);
	buffer.resize(size_t(ifs.tellg()));
	ifs.seekg(0, std::ios::beg);

	ifs.read(&buffer[0], buffer.size());
	ifs.close();

	return true;
}
