#version 450
#extension GL_ARB_separate_shader_objects : enable

#define lerp(a, b, t) mix(a, b, t)
#define saturate(a) clamp(a, 0.0, 1.0)
#define mad(a, b, c) (a * b + c)
#define mul(v, m) (m * v)
#define frac(v) fract(v)
#define clip(v) if (v < 0.f) discard
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define float4x4 mat4x4

#include "Common.frag"

#define TexDepth(uv)			texture(g_texDepth, uv)
#define TexNormal(uv)			texture(g_texNormal, uv)
#define TexAlbedoSpecular(uv)	texture(g_texAlbedoSpecular, uv)
#define TexDisneyBRDF(uv)		texture(g_texDisneyBRDF, uv)

layout(set = 0, binding = 0) uniform sampler2D g_texDepth;
layout(set = 0, binding = 1) uniform sampler2D g_texNormal;
layout(set = 0, binding = 2) uniform sampler2D g_texAlbedoSpecular;
layout(set = 0, binding = 3) uniform sampler2D g_texDisneyBRDF;

layout(location = 0) in float2 inUV;

layout(location = 0) out float4 outColor;

void main()
{
	float depth = TexDepth(inUV).r;
	clip((1.f - 1e-5f) - depth);

	float4 normals = TexNormal(inUV);
	float4 colors = TexAlbedoSpecular(inUV);
	float4 disneyBRDF = TexDisneyBRDF(inUV);

	float4 posWV = float4(0.f);
	float3 posW = CalcWorldSpacePosFromDepth(depth, inUV, CommonContents.matInvView, CommonContents.matInvProj, posWV);

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

	outColor = CalcColor(posW,
		normal, tangent, binormal,
		albedo, emissiveColor, emissiveIntensity,
		RM.x, RM.y,
		SST.x, SST.y, SST.z,
		AST.x, AST.y, AST.z,
		CG.x, CG.y,
		inUV);
}