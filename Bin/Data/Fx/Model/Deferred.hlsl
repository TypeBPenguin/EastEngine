#include "Common.hlsl"

#ifdef DX11

#define TexDepth(uv)	g_texDepth.Sample(SamplerPointClamp, uv)
#define TexNormal(uv)	g_texNormal.Sample(SamplerPointClamp, uv)
#define TexAlbedoSpecular(uv)	g_texAlbedoSpecular.Sample(SamplerPointClamp, uv)
#define TexDisneyBRDF(uv)	g_texDisneyBRDF.Sample(SamplerPointClamp, uv)

Texture2D<float> g_texDepth : register(t0);
Texture2D g_texNormal : register(t1);
Texture2D g_texAlbedoSpecular : register(t2);
Texture2D g_texDisneyBRDF : register(t3);

#elif DX12

#define TexDepth(uv)	Tex2DTable[g_nTexDepthIndex].Sample(SamplerPointClamp, uv)
#define TexNormal(uv)	Tex2DTable[g_nTexNormalIndex].Sample(SamplerPointClamp, uv)
#define TexAlbedoSpecular(uv)	Tex2DTable[g_nTexAlbedoSpecularIndex].Sample(SamplerPointClamp, uv)
#define TexDisneyBRDF(uv)	Tex2DTable[g_nTexDisneyBRDFIndex].Sample(SamplerPointClamp, uv)

#endif

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

float4 PS(PS_INPUT input) : SV_Target0
{
	float depth = TexDepth(input.tex).r;
	clip((1.f - 1e-5f) - depth);

	float4 normals = TexNormal(input.tex);
	float4 colors = TexAlbedoSpecular(input.tex);
	float4 disneyBRDF = TexDisneyBRDF(input.tex);

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
		albedo, emissiveColor, emissiveIntensity,
		RM.x, RM.y,
		SST.x, SST.y, SST.z,
		AST.x, AST.y, AST.z,
		CG.x, CG.y,
		input.tex);
}