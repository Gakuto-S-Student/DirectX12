#include "common.hlsli"

PS_INPUT main( VS_INPUT input )
{
    PS_INPUT output;
    
    matrix wvp = mul(model, view);
    wvp = mul(wvp, projection);
    
    output.Position = mul(input.Position, wvp);
    output.TexCoord = input.TexCoord;
    
    return output;
}