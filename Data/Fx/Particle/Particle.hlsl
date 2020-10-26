#ifdef DX12
#include "../DescriptorTablesDX12.hlsl"
#endif

#include "../Converter.hlsl"

#ifndef USE_DECAL

#ifdef DX11

Texture2D g_texColor : register(t0);
SamplerState g_sampler : register(s0);

#define TexColor(uv)	g_texColor.Sample(g_sampler, uv)

#elif DX12

#define TexColor(uv)	Tex2DTable[g_nTexColorIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)

cbuffer cbSRVIndicesConstants : register(b0)
{
	uint g_nTexColorIndex;
	uint g_nSamplerStateIndex;
};

#endif

struct VS_INPUT
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD;
	float4 color : COLOR;
};

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORD0;
	float4 color : TEXCOORD1;
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
	return TexColor(input.uv) * input.color;
}

#else

#ifdef DX11

#define TexAlbedo(uv)			g_texAlbedo.Sample(g_samplerState, uv)
#define TexMask(uv)				g_texMask.Sample(g_samplerState, uv)
#define TexNormal(uv)			g_texNormalMap.Sample(g_samplerState, uv)
#define TexRoughness(uv)		g_texRoughness.Sample(g_samplerState, uv)
#define TexMetallic(uv)			g_texMetallic.Sample(g_samplerState, uv)
#define TexEmissive(uv)			g_texEmissive.Sample(g_samplerState, uv)
#define TexEmissiveColor(uv)	g_texEmissiveColor.Sample(g_samplerState, uv)
#define TexSubsurface(uv)		g_texSurface.Sample(g_samplerState, uv)
#define TexSpecular(uv)			g_texSpecular.Sample(g_samplerState, uv)
#define TexSpecularTint(uv)		g_texSpecularTint.Sample(g_samplerState, uv)
#define TexAnisotropic(uv)		g_texAnisotropic.Sample(g_samplerState, uv)
#define TexSheen(uv)			g_texSheen.Sample(g_samplerState, uv)
#define TexSheenTint(uv)		g_texSheenTint.Sample(g_samplerState, uv)
#define TexClearcoat(uv)		g_texClearcoat.Sample(g_samplerState, uv)
#define TexClearcoatGloss(uv)	g_texClearcoatGloss.Sample(g_samplerState, uv)

SamplerState g_samplerState : register(s0);

#ifdef USE_TEX_ALBEDO
Texture2D g_texAlbedo : register(t0);
#endif	// USE_TEX_ALBEDO

#ifdef USE_TEX_MASK
Texture2D g_texMask : register(t1);
#endif	// USE_TEX_MASK

#ifdef USE_TEX_NORMAL
Texture2D g_texNormalMap : register(t2);
#endif	// USE_TEX_NORMAL

#ifdef USE_TEX_ROUGHNESS
Texture2D g_texRoughness : register(t3);
#endif	// USE_TEX_ROUGHNESS

#ifdef USE_TEX_METALLIC
Texture2D g_texMetallic : register(t4);
#endif	// USE_TEX_METALLIC

#ifdef USE_TEX_EMISSIVE
Texture2D g_texEmissive : register(t5);
#endif	// USE_TEX_EMISSIVE

#ifdef USE_TEX_EMISSIVECOLOR
Texture2D g_texEmissiveColor : register(t6);
#endif	// USE_TEX_EMISSIVECOLOR

#ifdef USE_TEX_SUBSURFACE
Texture2D g_texSurface : register(t7);
#endif	// USE_TEX_SUBSURFACE

#ifdef USE_TEX_SPECULAR
Texture2D g_texSpecular : register(t8);
#endif	// USE_TEX_SPECULAR

#ifdef USE_TEX_SPECULARTINT
Texture2D g_texSpecularTint : register(t9);
#endif	// USE_TEX_SPECULARTINT

#ifdef USE_TEX_ANISOTROPIC
Texture2D g_texAnisotropic : register(t10);
#endif	// USE_TEX_ANISOTROPIC

#ifdef USE_TEX_SHEEN
Texture2D g_texSheen : register(t11);
#endif	// USE_TEX_SHEEN

#ifdef USE_TEX_SHEENTINT
Texture2D g_texSheenTint : register(t12);
#endif	// USE_TEX_SHEENTINT

#ifdef USE_TEX_CLEARCOAT
Texture2D g_texClearcoat : register(t13);
#endif	// USE_TEX_CLEARCOAT

#ifdef USE_TEX_CLEARCOATGLOSS
Texture2D g_texClearcoatGloss : register(t14);
#endif	// USE_TEX_CLEARCOATGLOSS

#elif DX12

#define TexAlbedo(uv)			Tex2DTable[g_nTexAlbedoIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexMask(uv)				Tex2DTable[g_nTexMaskIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexNormal(uv)			Tex2DTable[g_nTexNormalMapIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexRoughness(uv)		Tex2DTable[g_nTexRoughnessIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexMetallic(uv)			Tex2DTable[g_nTexMetallicIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexEmissive(uv)			Tex2DTable[g_nTexEmissiveIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexEmissiveColor(uv)	Tex2DTable[g_nTexEmissiveColorIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexSubsurface(uv)		Tex2DTable[g_nTexSubsurfaceIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexSpecular(uv)			Tex2DTable[g_nTexSpecularIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexSpecularTint(uv)		Tex2DTable[g_nTexSpecularTintIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexAnisotropic(uv)		Tex2DTable[g_nTexAnisotropicIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexSheen(uv)			Tex2DTable[g_nTexSheenIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexSheenTint(uv)		Tex2DTable[g_nTexSheenTintIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexClearcoat(uv)		Tex2DTable[g_nTexClearcoatIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)
#define TexClearcoatGloss(uv)	Tex2DTable[g_nTexClearcoatGlossIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)

cbuffer cbSRVIndicesConstants : register(b4)
{
	uint g_nTexAlbedoIndex;
	uint g_nTexMaskIndex;
	uint g_nTexNormalMapIndex;
	uint g_nTexRoughnessIndex;
	uint g_nTexMetallicIndex;
	uint g_nTexEmissiveIndex;
	uint g_nTexEmissiveColorIndex;
	uint g_nTexSubsurfaceIndex;
	uint g_nTexSpecularIndex;
	uint g_nTexSpecularTintIndex;
	uint g_nTexAnisotropicIndex;
	uint g_nTexSheenIndex;
	uint g_nTexSheenTintIndex;
	uint g_nTexClearcoatIndex;
	uint g_nTexClearcoatGlossIndex;

	uint g_nSamplerStateIndex;
};

#endif

struct VS_INPUT_DECAL
{
	float4 pos : POSITION;
	uint InstanceID : SV_InstanceID;
};

struct PS_INPUT_DECAL
{
	float4 pos : SV_Position;
	float4 posWVP : TEXCOORD0;
	float4 posWV : TEXCOORD1;

	float3 normal : TEXCOORD2;

	uint InstanceID : TEXCOORD3;
};

cbuffer cbCamera : register(b0)
{
	float3 g_f3CameraPos;
	float3 g_f3CameraTopRight;
	float4x4 g_matView;
	float4x4 g_matProj;
	float4x4 g_matInvView;
	float4x4 g_matInvProj;
};

cbuffer cbMaterial : register(b1)
{
	float4 g_f4AlbedoColor;
	float4 g_f4EmissiveColor;

	float4 g_f4PaddingRoughMetEmi;
	float4 g_f4SurSpecTintAniso;
	float4 g_f4SheenTintClearcoatGloss;
};

cbuffer DecalInstance : register(b2)
{
	float4x4 g_InstancesMatWVP[32];
	float4x4 g_InstancesMatWorld[32];
	float4x4 g_InstancesMatInvWorld[32];
};

#ifdef DX11

Texture2D<float> g_texDepth : register(t15);
SamplerState g_sampler : register(s0);

#define TexDepth(uv)	g_texDepth.Sample(g_sampler, uv)

#elif DX12

#define TexColor(uv)	Tex2DTable[g_nTexDepthIndex].Sample(SamplerTable[g_nSamplerStateIndex], uv)

cbuffer cbSRVIndicesConstants : register(b0)
{
	uint g_nTexDepthIndex;
	uint g_nSamplerStateIndex;
};

#endif

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

	float depth = TexColor(screenPos);

	float3 pos = CalcViewSpcaePosFromDepth(depth, screenPos, g_matInvProj);
	float4 f4WorldPos = float4(pos, 1.f);

	float3 f3DecalLocalPos = mul(f4WorldPos, g_InstancesMatInvWorld[input.InstanceID]).xyz;
	clip(0.5f - abs(f3DecalLocalPos));

	float2 f2DecalUV = f3DecalLocalPos.xz + 0.5f;

#ifdef USE_TEX_ALBEDO
	float4 texAlbedo = TexAlbedo(input.tex);
	float4 albedo = saturate(g_f4AlbedoColor * pow(abs(texAlbedo), 2.2f));
	alpha = texAlbedo.a;
#else
	float4 albedo = g_f4AlbedoColor;
	alpha = albedo.a;
#endif

#ifdef USE_TEX_MASK
	albedo.w = saturate(albedo.w * TexMask(input.tex).x * 2.f);
	clip(albedo.w - 0.1f);
#elif USE_ALBEDO_ALPHA_IS_MASK_MAP
	clip(albedo.w - 0.1f);
#endif

#ifdef USE_TEX_NORMAL
	float3 normal = TexNormal(input.tex).xyz;
	normal = normalize(2.f * normal - 1.f);
	normal = normalize((normal.x * input.tangent) + (normal.y * input.binormal) + (input.normal));
#else
	float3 normal = input.normal;
#endif
	
	float3 RM = float3(g_f4PaddingRoughMetEmi.yz, 0.f);
#ifdef USE_TEX_ROUGHNESS
	RM.x = TexRoughness(input.tex).x;
#endif

#ifdef USE_TEX_METALLIC
	RM.y = TexMetallic(input.tex).x;
#endif

	float emissiveIntensity = g_f4PaddingRoughMetEmi.w;
#ifdef USE_TEX_EMISSIVE
	emissiveIntensity = TexEmissive(input.tex).x;
#endif

#ifdef USE_TEX_EMISSIVECOLOR
	float3 emissiveColor = TexEmissiveColor(input.tex).xyz;
#else
	float3 emissiveColor = g_f4EmissiveColor.xyz;
#endif

	float3 SST = g_f4SurSpecTintAniso.xyz;
	float3 AST = float3(g_f4SurSpecTintAniso.w, g_f4SheenTintClearcoatGloss.xy);
	float3 CG = float3(g_f4SheenTintClearcoatGloss.zw, 0.f);

#ifdef USE_TEX_SUBSURFACE
	SST.x = TexSurface(input.tex).x;
#endif
#ifdef USE_TEX_SPECULAR
	SST.y = TexSpecular(input.tex).x;
#endif
#ifdef USE_TEX_SPECULARTINT
	SST.z = TexSpecularTint(input.tex).x;
#endif
#ifdef USE_TEX_ANISOTROPIC
	AST.x = TexAnisotropic(input.tex).x;
#endif

	float4 STCG = g_f4SheenTintClearcoatGloss;
#ifdef USE_TEX_SHEEN
	AST.y = TexSheen(input.tex).x;
#endif
#ifdef USE_TEX_SHEENTINT
	AST.z = TexSheenTint(input.tex).x;
#endif
#ifdef USE_TEX_CLEARCOAT
	CG.x = TexClearcoat(input.tex).x;
#endif
#ifdef USE_TEX_CLEARCOATGLOSS
	CG.y = TexClearcoatGloss(input.tex).x;
#endif

	output.normals.xy = CompressNormal(normal);
	output.normals.zw = CompressNormal(input.tangent);

	output.colors.x = Pack3PNForFP32(saturate(albedo.xyz));
	output.colors.y = 0.f;	// padding
	output.colors.z = Pack3PNForFP32(emissiveColor);
	output.colors.w = emissiveIntensity;

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
