#ifdef DX12
#include "DescriptorTablesDX12.hlsl"
#endif

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
};

#ifdef DX11

SamplerState g_sampler : register(s0);
Texture2D g_texture : register(t0);

#else

#define g_texture Tex2DTable[g_texIndex]

cbuffer cbMotionBlur : register(b0)
{
	uint g_texIndex;
	float3 padding;
}
SamplerState g_sampler : register(s0, space100);

#endif

PS_INPUT VS(uint id : SV_VertexID)
{
	PS_INPUT output;
	output.tex = float2(((id << 1) & 2) != 0, (id & 2) != 0);
	output.pos = float4(output.tex * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);
	return output;
}

float4 PS_RGBA(PS_INPUT input) : SV_Target
{
	return g_texture.Sample(g_sampler, input.tex);
}

float4 PS_RGB(PS_INPUT input) : SV_Target
{
	return float4(g_texture.Sample(g_sampler, input.tex).xyz, 1.f);
}