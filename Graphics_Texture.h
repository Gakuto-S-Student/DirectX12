//==============================================================================
// Filename: Graphics_Texture.h
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================

#pragma once
class GraphicsTexture
{
public:
	static void CreateTexture(ID3D12Resource** textureBuffer, ID3D12DescriptorHeap** descriptorHeap);
};

