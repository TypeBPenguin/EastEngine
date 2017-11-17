#ifndef _SKY_
#define _SKY_

cbuffer cbMatrix
{
	float4x4 g_matWVP;
};

cbuffer cbColor
{
	float4 g_colorApex;
	float4 g_colorCenter;

	float g_fBlend;
};

Texture2D g_texEffect;
Texture2D g_texCloud;
Texture2D g_texCloudBlend;

SamplerState g_samLinearWrap;

struct VS_INPUT
{
	float4 pos : POSITION;
	float2 tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
	float4 posWVP : TEXCOORD1;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	input.pos.w = 1.f;

	output.pos = mul(input.pos, g_matWVP);

	output.tex = input.tex;

	output.posWVP = input.pos;

	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	float fHeight = max(input.posWVP.y, 0.f) * 0.0025f;

	return lerp(g_colorCenter, g_colorApex, fHeight);
}

float4 PS_Effect(PS_INPUT input) : SV_Target
{
	return g_texEffect.Sample(g_samLinearWrap, input.tex);
}

float4 PS_Cloud(PS_INPUT input) : SV_Target
{
	float4 cloud = g_texCloud.Sample(g_samLinearWrap, input.tex);

	if (g_fBlend == 0.f)
		return cloud;

	float4 cloudBlend = g_texCloudBlend.Sample(g_samLinearWrap, input.tex);

	return lerp(cloud, cloudBlend, g_fBlend);
}

technique11 Sky
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 SkyEffect
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS_Effect()));
	}
}

technique11 SkyCloud
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS_Cloud()));
	}
}

#endif
