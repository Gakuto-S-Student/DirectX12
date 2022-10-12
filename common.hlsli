struct VS_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer g_Model : register(b0)
{
    matrix model;
}
cbuffer g_View : register(b1)
{
    matrix view;
}
cbuffer g_Projection : register(b2)
{
    matrix projection;
}

struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};