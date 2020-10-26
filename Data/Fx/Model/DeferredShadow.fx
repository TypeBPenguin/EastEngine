#ifndef _MODEL_DEFERRED_
#define _MODEL_DEFERRED_

#include "../Converter.fx"
#include "Shadow.hlsl"

Texture2D<float> g_texDepth;
SamplerState g_samPoint : register(s0);

cbuffer cbContents
{
	float3 g_f3CameraPos;
	float padding;

	float4x4 g_matInvView;
	float4x4 g_matInvProj;
};

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
};

PS_INPUT VS(uint id : SV_VertexID)
{
	PS_INPUT output;
	output.tex = float2(((id << 1) & 2) != 0, (id & 2) != 0);
	output.pos = float4(output.tex * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);

	return output;
}

float2 PS_CascadedShadow(PS_INPUT input) : SV_Target0
{
	float depth = g_texDepth.Sample(g_samPoint, input.tex);

	float4 posWV = 0.f;
	float3 posW = CalcWorldSpacePosFromDepth(depth, input.tex, g_matInvView, g_matInvProj, posWV);

	return float2(CalculateCascadedShadow(posW, posWV.xyz), 0.01f);
}

float2 PS_ShadowMap(PS_INPUT input) : SV_Target0
{
	float depth = g_texDepth.Sample(g_samPoint, input.tex);

	float3 posW = CalcWorldSpacePosFromDepth(depth, input.tex, g_matInvView, g_matInvProj);

	return CalculateShadowMap(posW);
}

float2 PS_ShadowCubeMap(PS_INPUT input) : SV_Target0
{
	float depth = g_texDepth.Sample(g_samPoint, input.tex);

	float3 posW = CalcWorldSpacePosFromDepth(depth, input.tex, g_matInvView, g_matInvProj);

	return CalculateShadowCubeMap(posW);
}

technique11 Deferred_CascadedShadow
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS_CascadedShadow()));
	}
}

technique11 Deferred_ShadowMap
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS_ShadowMap()));
	}
}

technique11 Deferred_ShadowCubeMap
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS_ShadowCubeMap()));
	}
}

#endif
