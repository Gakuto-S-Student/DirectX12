#include "common.hlsli"

float4 main(PS_INPUT input) : SV_TARGET
{
	return float4(input.TexCoord, 1.0f, 1.0f);
}