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

cbuffer g_worldBuffer : register(b0)
{
    matrix world;
};
cbuffer g_viewBuffer : register(b1)
{
    matrix view;
};
cbuffer g_projectionBuffer : register(b2)
{
    matrix projection;
};


struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

Texture2D<float4>   g_texture : register(t0);
SamplerState        g_sampler : register(s0);