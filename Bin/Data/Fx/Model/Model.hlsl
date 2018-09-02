#ifdef DX12
#include "../DescriptorTablesDX12.hlsl"
#endif

#ifdef USE_ALPHABLENDING

#include "Common.hlsl"

#else

#include "../Converter.hlsl"

#endif

float4x4 DecodeMatrix(in float4 encodedMatrix0, in float4 encodedMatrix1, in float4 encodedMatrix2)
{
	return float4x4(float4(encodedMatrix0.xyz, 0),
		float4(encodedMatrix1.xyz, 0),
		float4(encodedMatrix2.xyz, 0),
		float4(encodedMatrix0.w, encodedMatrix1.w, encodedMatrix2.w, 1.f));
}

#ifdef USE_INSTANCING

#define eMaxInstancingCount 64

struct TransformData
{
	float4 matrix1;
	float4 matrix2;
	float4 matrix3;
};

struct MotionData
{
	uint nVTFID;
	uint nDummy0;
	uint nDummy1;
	uint nDummy2;
};

struct SkinningInstancingData
{
	TransformData transformData;
	MotionData motionData;
};

struct SkinnedInfo
{
	float4 pos;
	float3 normal;
};

float4x4 ComputeWorldMatrix(in TransformData transformData)
{
	return DecodeMatrix(transformData.matrix1, transformData.matrix2, transformData.matrix3);
}

#ifdef USE_SKINNING
cbuffer cbSkinningInstancingData : register(b0)
{
	SkinningInstancingData g_Instances[eMaxInstancingCount];
};
#else
cbuffer cbStaticInstancingData : register(b1)
{
	float4x4 g_Instances[eMaxInstancingCount];
};
#endif	// USE_SKINNING

#endif	// USE_INSTANCING

cbuffer cbObjectData : register(b2)
{
	float4x4 g_matWorld;

	float4 g_f4AlbedoColor;
	float4 g_f4EmissiveColor;

	float4 g_f4PaddingRoughMetEmi;
	float4 g_f4SurSpecTintAniso;
	float4 g_f4SheenTintClearcoatGloss;

	float g_fStippleTransparencyFactor = 0.f;
	uint g_nVTFID;

	float2 g_f2Padding = 0.f;
}

cbuffer cbVSConstants : register(b3)
{
	float4x4 g_matViewProj;

#ifdef DX12
	uint g_nTexVTFIndex;
	float3 VSConstants_padding;
#endif

	//#ifdef USE_TESSELLATION
	//	float4 g_FrustumNormals[4];
	//	float3 g_FrustumOrigin;
	//
	//	float g_fTessellationFactor;
	//#endif
};

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

#ifdef USE_SKINNING
#define VTF_WIDTH 1024

#ifdef DX11

Texture2D g_texVTF : register(t15);
#define TexVTF(u, v)	g_texVTF.Load(uint3(u, v, 0));

#elif DX12

#define TexVTF(u, v)	Tex2DTable[g_nTexVTFIndex].Load(uint3(u, v, 0));

#endif

float4x4 LoadBoneMatrix(in uint nVTFID, in uint bone)
{
	// 4*bone is since each bone is 4 texels to form a float4x4 
	uint baseIndex = (nVTFID + bone) * 4;

	// Now turn linear offset into 2D coords
	uint baseU = baseIndex % VTF_WIDTH;
	uint baseV = baseIndex / VTF_WIDTH;

	float4 mat1 = TexVTF(baseU, baseV);
	float4 mat2 = TexVTF(baseU + 1, baseV);
	float4 mat3 = TexVTF(baseU + 2, baseV);

	return DecodeMatrix(mat1, mat2, mat3);
}

void ComputeSkinning(in float4 position
	, in float3 normal
	, in float4 blendWeight
	, in uint4 blendIndices
	, in uint nVTFID
	, inout float4 pos_out
	, inout float3 normal_out
)
{
	pos_out = (float4)0;
	normal_out = (float3)0;

	float4x4 m;

	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		m = LoadBoneMatrix(nVTFID, blendIndices[i]);
		pos_out += mul(position, m) * blendWeight[i];
		normal_out += mul(normal, (float3x3)m) * blendWeight[i];
	}
}
#endif // USE_SKINNING

struct PS_INPUT
{
	float4 pos		: SV_Position;	// 위치
	float2 tex		: TEXCOORD0;	// UV
	float3 normal	: NORMAL;		// 노말

	float3 tangent	: TANGENT;		// 탄젠트
	float3 binormal	: BINORMAL;
	
#ifdef USE_ALPHABLENDING
	float4 posW		: TEXCOORD1;
#endif
};

#ifdef USE_ALPHABLENDING

struct PS_OUTPUT
{
	float4 color : SV_Target0;
};

#else

struct PS_OUTPUT
{
	float4 normals : SV_Target0;
	float4 colors : SV_Target1;
	float4 disneyBRDF : SV_Target2;
};

#endif

PS_INPUT VS(in float4 inPos : POSITION
	, in float2 inTex : TEXCOORD
	, in float3 inNormal : NORMAL
#ifdef USE_SKINNING
	, in float4 inBlendWeight : BLENDWEIGHT
	, in uint4 inBlendIndices : BLENDINDICES
#endif	// USE_SKINNING

#ifdef USE_INSTANCING
	, in uint InstanceID : SV_InstanceID
#endif	// USE_INSTANCING
)
{
	PS_INPUT output = (PS_INPUT)0;
	inPos.w = 1.f;

#ifdef USE_SKINNING
	#ifdef USE_INSTANCING

	float4x4 matWorld = ComputeWorldMatrix(g_Instances[InstanceID].transformData);
	uint nVTFID = g_Instances[InstanceID].motionData.nVTFID;

	#else	// USE_INSTANCING

	float4x4 matWorld = g_matWorld;
	uint nVTFID = g_nVTFID;

	#endif	// USE_INSTANCING

	inBlendWeight.w = 1.f - (inBlendWeight.x + inBlendWeight.y + inBlendWeight.z);
	ComputeSkinning(inPos, inNormal, inBlendWeight, inBlendIndices, nVTFID, output.pos, output.normal);
#else	// USE_SKINNING
	#ifdef USE_INSTANCING

	float4x4 matWorld = g_Instances[InstanceID];

	#else	// USE_INSTANCING

	float4x4 matWorld = g_matWorld;

	#endif	// USE_INSTANCING

	output.pos = inPos;
	output.normal = inNormal;
#endif	// USE_SKINNING

	output.pos = mul(output.pos, matWorld);

#ifdef USE_ALPHABLENDING
	output.posW = output.pos;
#endif
	output.pos = mul(output.pos, g_matViewProj);

	output.normal = normalize(mul(output.normal, (float3x3)matWorld));

	// UV
	output.tex = inTex;

	output.tangent = CalcTangent(output.normal);
	output.binormal = CalcBinormal(output.normal, output.tangent);

	return output;
}

PS_OUTPUT PS(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;

	if (g_fStippleTransparencyFactor > 0.f)
	{
		float2 screenPos = 0.f;
		screenPos = (floor(input.pos * g_fStippleTransparencyFactor.xxxx) * 0.5f).xy;

		float checker = -frac(screenPos.r + screenPos.g);
		clip(checker);
	}

	float alpha = 1.f;

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

#ifdef USE_ALPHABLENDING
	// 검증 필요
	float3 dir = normalize(input.posW.xyz - g_f3CameraPos);
	float fdot = dot(normal, dir);
	if (fdot > 0.f)
	{
		normal = -normal;
	}
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

#ifdef USE_ALPHABLENDING
	output.color = CalcColor(input.posW.xyz,
		normal, input.tangent, input.binormal,
		saturate(albedo.xyz), emissiveColor, emissiveIntensity,
		RM.x, RM.y,
		SST.x, SST.y, SST.z,
		AST.x, AST.y, AST.z,
		CG.x, CG.y,
		float2(0.f, 0.f));

	output.color.w = alpha;
#else
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
#endif

	return output;
}