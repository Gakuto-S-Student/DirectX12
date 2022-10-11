//==============================================================================
// Filename: commpn.hlsli
// Description:
// Copyright (C) 2022 Silicon Studio Co., Ltd. All rights reserved.
//==============================================================================
struct VS_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer g_worldViewProjection : register(b0)
{
    matrix wvp;
};


struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

Texture2D<float4>   g_texture : register(t0);
SamplerState        g_sampler : register(s0);