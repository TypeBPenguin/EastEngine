Texture2D tex1 : register(t0);
SamplerState sam1 : register(s0);

float4 main(in float4 inPos : SV_POSITION, in float2 inTexCoord : TEXCOORD) : SV_TARGET
{
	return tex1.Sample(sam1, inTexCoord);
}