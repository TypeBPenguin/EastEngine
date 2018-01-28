#ifndef _Effect_
#define _Effect_

#include "../Converter.fx"

SamplerState g_sampler;

#ifndef USE_DECAL

Texture2D g_texAlbedo;

struct VS_INPUT
{
	float4 pos			: POSITION;
	float2 uv			: TEXCOORD;
	float4 color		: COLOR;
};

struct PS_INPUT
{
	float4 pos		: SV_Position;
	float2 uv		: TEXCOORD0;
	float4 color	: TEXCOORD1;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;

	output.pos = input.pos;
	output.uv = input.uv;
	output.color = input.color;

	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return g_texAlbedo.Sample(g_sampler, input.uv) * input.color;
}

#else

SamplerState g_samplerState;

#ifdef USE_TEX_ALBEDO
Texture2D g_texAlbedo;
#endif

#ifdef USE_TEX_MASK
Texture2D g_texMask;
#endif

#ifdef USE_TEX_NORMAL
Texture2D g_texNormalMap;
#endif

#ifdef USE_TEX_DISPLACEMENT
Texture2D g_texDisplaceMap;
#endif

#ifdef USE_TEX_SPECULARCOLOR
Texture2D g_texSpecularColor;
#endif

#ifdef USE_TEX_ROUGHNESS
Texture2D g_texRoughness;
#endif

#ifdef USE_TEX_METALLIC
Texture2D g_texMetallic;
#endif

#ifdef USE_TEX_EMISSIVE
Texture2D g_texEmissive;
#endif

#ifdef USE_TEX_SUBSURFACE
Texture2D g_texSurface;
#endif

#ifdef USE_TEX_SPECULAR
Texture2D g_texSpecular;
#endif

#ifdef USE_TEX_SPECULARTINT
Texture2D g_texSpecularTint;
#endif

#ifdef USE_TEX_ANISOTROPIC
Texture2D g_texAnisotropic;
#endif

#ifdef USE_TEX_SHEEN
Texture2D g_texSheen;
#endif

#ifdef USE_TEX_SHEENTINT
Texture2D g_texSheenTint;
#endif

#ifdef USE_TEX_CLEARCOAT
Texture2D g_texClearcoat;
#endif

#ifdef USE_TEX_CLEARCOATGLOSS
Texture2D g_texClearcoatGloss;
#endif

struct VS_INPUT_DECAL
{
	float4 pos			: POSITION;

	uint InstanceID		: SV_InstanceID;
};

struct PS_INPUT_DECAL
{
	float4 pos		: SV_Position;
	float4 posWVP	: TEXCOORD0;
	float4 posWV	: TEXCOORD1;

	float3 normal : TEXCOORD2;

	uint InstanceID : TEXCOORD3;
};

cbuffer DecalInstance
{
	float4x4 g_InstancesMatWVP[32];
	float4x4 g_InstancesMatWorld[32];
	float4x4 g_InstancesMatInvWorld[32];
};

cbuffer cbCamera
{
	float3 g_f3CameraPos;
	float3 g_f3CameraTopRight;
	float4x4 g_matView;
	float4x4 g_matProj;
	float4x4 g_matInvView;
	float4x4 g_matInvProj;
};

cbuffer cbMaterial
{
	float4 g_f4AlbedoColor;
	float4 g_f4EmissiveColor;

	float4 g_f4PaddingRoughMetEmi;
	float4 g_f4SurSpecTintAniso;
	float4 g_f4SheenTintClearcoatGloss;
};

Texture2D<float> g_texDepth;

PS_INPUT_DECAL VS_DECAL(VS_INPUT_DECAL input)
{
	PS_INPUT_DECAL output;
	input.pos.w = 1.f;

	float4x4 matWVP = g_InstancesMatWVP[input.InstanceID];

	float4 posW = mul(input.pos, g_InstancesMatWorld[input.InstanceID]);

	output.pos = mul(input.pos, matWVP);
	output.posWVP = output.pos;

	output.posWV = mul(posW, g_matView);

	output.normal = mul(float3(0.f, 1.f, 0.f), (float3x3)g_InstancesMatWorld[input.InstanceID]);

	output.InstanceID = input.InstanceID;

	return output;
}

struct PS_OUTPUT
{
	float4 normals : SV_Target0;
	float4 colors : SV_Target1;
	float4 disneyBRDF : SV_Target2;
};

PS_OUTPUT PS_DECAL(PS_INPUT_DECAL input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;

	float2 screenPos;
	screenPos.x = input.posWVP.x / input.posWVP.w * 0.5f + 0.5f;
	screenPos.y = -input.posWVP.y / input.posWVP.w * 0.5f + 0.5f;

	float depth = g_texDepth.Sample(g_sampler, screenPos);

	float3 pos = CalcViewSpcaePosFromDepth(depth, screenPos, g_matInvProj);
	float4 f4WorldPos = float4(pos, 1.f);

	float3 f3DecalLocalPos = mul(f4WorldPos, g_InstancesMatInvWorld[input.InstanceID]).xyz;
	clip(0.5f - abs(f3DecalLocalPos));

	float2 f2DecalUV = f3DecalLocalPos.xz + 0.5f;

#ifdef USE_TEX_ALBEDO
	float4 albedo = saturate(g_f4AlbedoColor * pow(abs(g_texAlbedo.Sample(g_samplerState, f2DecalUV)), 2.2f) * 2.f);
#else
	float4 albedo = g_f4AlbedoColor;
#endif

#ifdef USE_TEX_MASK
	albedo.w = saturate(albedo.w * g_texMask.Sample(g_samplerState, f2DecalUV).x * 2.f);
#endif
	float fDist = abs(f3DecalLocalPos.y);
	albedo *= (1.f - max((fDist - 0.5f) / 0.5f, 0.f));

	clip(albedo.w - 0.1f);

	float3 normal = input.normal;
	float3 tangent = CalcTangent(input.normal);

#ifdef USE_TEX_NORMAL
	float3 binormal = CalcBinormal(normal, tangent);

	normal = g_texNormalMap.Sample(g_samplerState, f2DecalUV).xyz;
	normal = normalize(2.f * normal - 1.f);
	normal = normalize((normal.x * tangent) + (normal.y * binormal) + (normal.z * input.normal));
	normal = normalize(normal + input.normal);
	//normal.x *= -1.f;
#endif

	output.normals.xy = CompressNormal(normal);
	output.normals.zw = CompressNormal(tangent);

	float3 RM = float3(g_f4PaddingRoughMetEmi.yz, 0.f);
#ifdef USE_TEX_ROUGHNESS
	RM.x = g_texRoughness.Sample(g_samplerState, f2DecalUV).x;
#endif

#ifdef USE_TEX_METALLIC
	RM.y = g_texMetallic.Sample(g_samplerState, f2DecalUV).x;
#endif

	float emissiveIntensity = g_f4PaddingRoughMetEmi.w;
#ifdef USE_TEX_EMISSIVE
	emissiveIntensity = g_texEmissive.Sample(g_samplerState, f2DecalUV).x;
#endif

#ifdef USE_TEX_SPECULARCOLOR
	float3 specular = g_texSpecularColor.Sample(g_samplerState, f2DecalUV).xyz;
#else
	float3 specular = lerp(0.03f, albedo.xyz, RM.y);
#endif

	output.colors.x = Pack3PNForFP32(albedo);
	output.colors.y = Pack3PNForFP32(specular);
	output.colors.z = Pack3PNForFP32(g_f4EmissiveColor.xyz);
	output.colors.w = emissiveIntensity;

	float3 SST = g_f4SurSpecTintAniso.xyz;
	float3 AST = float3(g_f4SurSpecTintAniso.w, g_f4SheenTintClearcoatGloss.xy);
	float3 CG = float3(g_f4SheenTintClearcoatGloss.zw, 0.f);

#ifdef USE_TEX_SUBSURFACE
	SST.x = g_texSurface.Sample(g_samplerState, f2DecalUV).x;
#endif
#ifdef USE_TEX_SPECULAR
	SST.y = g_texSpecular.Sample(g_samplerState, f2DecalUV).x;
#endif
#ifdef USE_TEX_SPECULARTINT
	SST.z = g_texSpecularTint.Sample(g_samplerState, f2DecalUV).x;
#endif
#ifdef USE_TEX_ANISOTROPIC
	AST.x = g_texAnisotropic.Sample(g_samplerState, f2DecalUV).x;
#endif

	float4 STCG = g_f4SheenTintClearcoatGloss;
#ifdef USE_TEX_SHEEN
	AST.y = g_texSheen.Sample(g_samplerState, f2DecalUV).x;
#endif
#ifdef USE_TEX_SHEENTINT
	AST.z = g_texSheenTint.Sample(g_samplerState, f2DecalUV).x;
#endif
#ifdef USE_TEX_CLEARCOAT
	CG.x = g_texClearcoat.Sample(g_samplerState, f2DecalUV).x;
#endif
#ifdef USE_TEX_CLEARCOATGLOSS
	CG.y = g_texClearcoatGloss.Sample(g_samplerState, f2DecalUV).x;
#endif

	output.disneyBRDF.x = Pack3PNForFP32(RM);
	output.disneyBRDF.y = Pack3PNForFP32(SST);
	output.disneyBRDF.z = Pack3PNForFP32(AST);
	output.disneyBRDF.w = Pack3PNForFP32(CG);

	return output;
}

#endif

#ifndef USE_DECAL

technique11 Emitter
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

#else

technique11 Decal
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_DECAL()));
		SetPixelShader(CompileShader(ps_5_0, PS_DECAL()));
	}
}

#endif

#endif
