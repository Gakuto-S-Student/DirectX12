//==============================================================================
// Filename: vertexShader.hlsl
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
#include "common.hlsli"

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = g_texture.Sample(g_sampler, input.TexCoord);
	return texColor;
}