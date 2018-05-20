#version 450
#extension GL_ARB_separate_shader_objects : enable

//#define SampleLevelZero(tex, coord) textureLod(tex, coord, 0.0)
//#define SampleLevelZeroPoint(tex, coord) textureLod(tex, coord, 0.0)
//#define Sample(tex, coord) texture(tex, coord)
//#define SamplePoint(tex, coord) texture(tex, coord)
//#define SampleLevelZeroOffset(tex, coord, offset) textureLodOffset(tex, coord, 0.0, offset)
//#define SampleOffset(tex, coord, offset) texture(tex, coord, offset)
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

#include "../Converter.hlsl"

#define TexAlbedo(uv)			texture(g_texAlbedo, uv)
#define TexMask(uv)				texture(g_texMask, uv)
#define TexNormal(uv)			texture(g_texNormalMap, uv)
#define TexRoughness(uv)		texture(g_texRoughness, uv)
#define TexMetallic(uv)			texture(g_texMetallic, uv)
#define TexEmissive(uv)			texture(g_texEmissive, uv)
#define TexEmissiveColor(uv)	texture(g_texEmissiveColor, uv)
#define TexSubsurface(uv)		texture(g_texSurface, uv)
#define TexSpecular(uv)			texture(g_texSpecular, uv)
#define TexSpecularTint(uv)		texture(g_texSpecularTint, uv)
#define TexAnisotropic(uv)		texture(g_texAnisotropic, uv)
#define TexSheen(uv)			texture(g_texSheen, uv)
#define TexSheenTint(uv)		texture(g_texSheenTint, uv)
#define TexClearcoat(uv)		texture(g_texClearcoat, uv)
#define TexClearcoatGloss(uv)	texture(g_texClearcoatGloss, uv)

layout(set = 2, binding = 0) uniform ubMaterialData
{
	float4 f4AlbedoColor;
	float4 f4EmissiveColor;

	float4 f4PaddingRoughMetEmi;
	float4 f4SurSpecTintAniso;
	float4 f4SheenTintClearcoatGloss;

	float fStippleTransparencyFactor;
	float3 f3Padding;
} MaterialData;

#ifdef USE_TEX_ALBEDO
layout(set = 3, binding = 0) uniform sampler2D g_texAlbedo;
#endif	// USE_TEX_ALBEDO

#ifdef USE_TEX_MASK
layout(set = 3, binding = 1) uniform sampler2D g_texMask;
#endif	// USE_TEX_MASK

#ifdef USE_TEX_NORMAL
layout(set = 3, binding = 2) uniform sampler2D g_texNormalMap;
#endif	// USE_TEX_NORMAL

#ifdef USE_TEX_ROUGHNESS
layout(set = 3, binding = 3) uniform sampler2D g_texRoughness;
#endif	// USE_TEX_ROUGHNESS

#ifdef USE_TEX_METALLIC
layout(set = 3, binding = 4) uniform sampler2D g_texMetallic;
#endif	// USE_TEX_METALLIC

#ifdef USE_TEX_EMISSIVE
layout(set = 3, binding = 5) uniform sampler2D g_texEmissive;
#endif	// USE_TEX_EMISSIVE

#ifdef USE_TEX_EMISSIVECOLOR
layout(set = 3, binding = 6) uniform sampler2D g_texEmissiveColor;
#endif	// USE_TEX_EMISSIVECOLOR

#ifdef USE_TEX_SUBSURFACE
layout(set = 3, binding = 7) uniform sampler2D g_texSurface;
#endif	// USE_TEX_SUBSURFACE

#ifdef USE_TEX_SPECULAR
layout(set = 3, binding = 8) uniform sampler2D g_texSpecular;
#endif	// USE_TEX_SPECULAR

#ifdef USE_TEX_SPECULARTINT
layout(set = 3, binding = 9) uniform sampler2D g_texSpecularTint;
#endif	// USE_TEX_SPECULARTINT

#ifdef USE_TEX_ANISOTROPIC
layout(set = 3, binding = 10) uniform sampler2D g_texAnisotropic;
#endif	// USE_TEX_ANISOTROPIC

#ifdef USE_TEX_SHEEN
layout(set = 3, binding = 11) uniform sampler2D g_texSheen;
#endif	// USE_TEX_SHEEN

#ifdef USE_TEX_SHEENTINT
layout(set = 3, binding = 12) uniform sampler2D g_texSheenTint;
#endif	// USE_TEX_SHEENTINT

#ifdef USE_TEX_CLEARCOAT
layout(set = 3, binding = 13) uniform sampler2D g_texClearcoat;
#endif	// USE_TEX_CLEARCOAT

#ifdef USE_TEX_CLEARCOATGLOSS
layout(set = 3, binding = 14) uniform sampler2D g_texClearcoatGloss;
#endif	// USE_TEX_CLEARCOATGLOSS

layout(location = 0) in float4 inPos;
layout(location = 1) in float2 inUV;
layout(location = 2) in float3 inNormal;
layout(location = 3) in float3 inTangent;
layout(location = 4) in float3 inBinormal;

layout(location = 0) out float4 outNormals;
layout(location = 1) out float4 outColors;
layout(location = 2) out float4 outDisneyBRDF;

void main()
{
	if (MaterialData.fStippleTransparencyFactor > 0.f)
	{
		float2 screenPos = float2(0.f);
		screenPos = (floor(inPos * MaterialData.fStippleTransparencyFactor.xxxx) * 0.5f).xy;

		float checker = -frac(screenPos.r + screenPos.g);
		clip(checker);
	}

	float alpha = 1.f;

#ifdef USE_TEX_ALBEDO
	float4 texAlbedo = TexAlbedo(inUV);
	float4 albedo = saturate(MaterialData.f4AlbedoColor * pow(abs(texAlbedo), 2.2f));
	alpha = texAlbedo.a;
#else
	float4 albedo = MaterialData.f4AlbedoColor;
	alpha = albedo.a;
#endif

#ifdef USE_TEX_MASK
	albedo.w = saturate(albedo.w * TexMask(inUV).x * 2.f);
	clip(albedo.w - 0.1f);
#elif USE_ALBEDO_ALPHA_IS_MASK_MAP
	clip(albedo.w - 0.1f);
#endif

#ifdef USE_TEX_NORMAL
	float3 normal = TexNormal(inUV).xyz;
	normal = normalize(2.f * normal - 1.f);
	normal = normalize((normal.x * inTangent) + (normal.y * inBinormal) + (inNormal));
#else
	float3 normal = inNormal;
#endif

	//#ifdef USE_ALPHABLENDING
	//	// 검증 필요
	//	float3 dir = normalize(inPosW - g_f3CameraPos);
	//	float fdot = dot(normal, dir);
	//	if (fdot > 0.f)
	//	{
	//		normal = -normal;
	//	}
	//#endif

	float3 RM = float3(MaterialData.f4PaddingRoughMetEmi.yz, 0.f);
#ifdef USE_TEX_ROUGHNESS
	RM.x = TexRoughness(inUV).x;
#endif

#ifdef USE_TEX_METALLIC
	RM.y = TexMetallic(inUV).x;
#endif

	float emissiveIntensity = MaterialData.f4PaddingRoughMetEmi.w;
#ifdef USE_TEX_EMISSIVE
	emissiveIntensity = TexEmissive(inUV).x;
#endif

#ifdef USE_TEX_EMISSIVECOLOR
	float3 emissiveColor = TexEmissiveColor(inUV).xyz;
#else
	float3 emissiveColor = MaterialData.f4EmissiveColor.xyz;
#endif

	float3 SST = MaterialData.f4SurSpecTintAniso.xyz;
	float3 AST = float3(MaterialData.f4SurSpecTintAniso.w, MaterialData.f4SheenTintClearcoatGloss.xy);
	float3 CG = float3(MaterialData.f4SheenTintClearcoatGloss.zw, 0.f);

#ifdef USE_TEX_SUBSURFACE
	SST.x = TexSurface(inUV).x;
#endif
#ifdef USE_TEX_SPECULAR
	SST.y = TexSpecular(inUV).x;
#endif
#ifdef USE_TEX_SPECULARTINT
	SST.z = TexSpecularTint(inUV).x;
#endif
#ifdef USE_TEX_ANISOTROPIC
	AST.x = TexAnisotropic(inUV).x;
#endif

	float4 STCG = MaterialData.f4SheenTintClearcoatGloss;
#ifdef USE_TEX_SHEEN
	AST.y = TexSheen(inUV).x;
#endif
#ifdef USE_TEX_SHEENTINT
	AST.z = TexSheenTint(inUV).x;
#endif
#ifdef USE_TEX_CLEARCOAT
	CG.x = TexClearcoat(inUV).x;
#endif
#ifdef USE_TEX_CLEARCOATGLOSS
	CG.y = TexClearcoatGloss(inUV).x;
#endif

#ifdef USE_ALPHABLENDING
	output = CalcColor(inPosW,
		normal, inTangent, inBinormal,
		saturate(albedo), emissiveColor, emissiveIntensity,
		RM.x, RM.y,
		SST.x, SST.y, SST.z,
		AST.x, AST.y, AST.z,
		CG.x, CG.y,
		float2(0.f, 0.f));

	output.w = alpha;
#else
	outNormals.xy = CompressNormal(normal);
	outNormals.zw = CompressNormal(inTangent);

	outColors.x = Pack3PNForFP32(saturate(albedo.xyz));
	outColors.y = 0.f;	// padding
	outColors.z = Pack3PNForFP32(emissiveColor);
	outColors.w = emissiveIntensity;

	outDisneyBRDF.x = Pack3PNForFP32(RM);
	outDisneyBRDF.y = Pack3PNForFP32(SST);
	outDisneyBRDF.z = Pack3PNForFP32(AST);
	outDisneyBRDF.w = Pack3PNForFP32(CG);
#endif
}