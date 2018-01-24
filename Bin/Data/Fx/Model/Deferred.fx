#ifndef _MODEL_DEFERRED_
#define _MODEL_DEFERRED_

#include "Common.fx"

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
};

Texture2D<float> g_texDepth;
Texture2D g_texNormal;
Texture2D g_texAlbedoSpecular;
Texture2D g_texDisneyBRDF;

cbuffer cbDeferredContents
{
	float4x4 g_matInvView;
	float4x4 g_matInvProj;
};

PS_INPUT VS(uint id : SV_VertexID)
{
	PS_INPUT output;
	output.tex = float2(((id << 1) & 2) != 0, (id & 2) != 0);
	output.pos = float4(output.tex * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);

	return output;
}

float4 PS(PS_INPUT input) : SV_Target0
{
	float depth = g_texDepth.Sample(g_samPoint, input.tex);
	clip((1.f - 1e-5f) - depth);

	float4 colors = g_texAlbedoSpecular.Sample(g_samPoint, input.tex);
	float4 normals = g_texNormal.Sample(g_samPoint, input.tex);
	float4 disneyBRDF = g_texDisneyBRDF.Sample(g_samPoint, input.tex);

	float4 posWV = 0.f;
	float3 posW = CalcWorldSpacePosFromDepth(depth, input.tex, g_matInvView, g_matInvProj, posWV);

	float3 normal = DeCompressNormal(normals.xy);
	float3 tangent = DeCompressNormal(normals.zw);
	float3 binormal = CalcBinormal(normal, tangent);

	float3 albedo = Unpack3PNFromFP32(colors.x);
	float3 specular = Unpack3PNFromFP32(colors.y);
	float3 emissiveColor = Unpack3PNFromFP32(colors.z);
	float emissiveIntensity = colors.w;

	float3 RM = Unpack3PNFromFP32(disneyBRDF.x);
	float3 SST = Unpack3PNFromFP32(disneyBRDF.y);
	float3 AST = Unpack3PNFromFP32(disneyBRDF.z);
	float3 CG = Unpack3PNFromFP32(disneyBRDF.w);

	return CalcColor(posW,
		normal, tangent, binormal,
		albedo, specular, emissiveColor, emissiveIntensity,
		RM.x, RM.y,
		SST.x, SST.y, SST.z,
		AST.x, AST.y, AST.z,
		CG.x, CG.y,
		input.tex);
}

technique11 Deferred
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

#endif