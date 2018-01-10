#ifndef _MODEL_
#define _MODEL_

#include "../Converter.fx"

#ifdef USE_SKINNING

Texture2D g_texVTF;

#endif

float4x4 DecodeMatrix(in float4 encodedMatrix0, in float4 encodedMatrix1, in float4 encodedMatrix2)
{
	return float4x4(float4(encodedMatrix0.xyz, 0),
		float4(encodedMatrix1.xyz, 0),
		float4(encodedMatrix2.xyz, 0),
		float4(encodedMatrix0.w, encodedMatrix1.w, encodedMatrix2.w, 1.f));
}

#ifdef USE_INSTANCING
#define MAX_INSTANCE_CONSTANTS 128

struct InstDataStatic
{
	float4 f4World1;
	float4 f4World2;
	float4 f4World3;
};

#ifdef USE_SKINNING
struct MotionData
{
	uint nVTFID;
	uint nDummy0;
	uint nDummy1;
	uint nDummy2;
};

struct InstDataSkinned
{
	InstDataStatic worldData;

	MotionData motionData;
};

cbuffer cbInstData
{
	InstDataSkinned	g_Instances[MAX_INSTANCE_CONSTANTS];
}
#else
cbuffer cbInstData
{
	InstDataStatic g_Instances[MAX_INSTANCE_CONSTANTS];
}
#endif

float4x4 ComputeWorldMatrix(in InstDataStatic worldData)
{
	return DecodeMatrix(worldData.f4World1, worldData.f4World2, worldData.f4World3);
}

#endif

#ifdef USE_SKINNING

#define VTF_WIDTH 1024

struct SkinnedInfo
{
	float4 pos;
	float3 normal;
};

float4x4 LoadBoneMatrix(in uint nVTFID, in uint bone)
{
	// 4*bone is since each bone is 4 texels to form a float4x4 
	uint baseIndex = (nVTFID + bone) * 4;

	// Now turn linear offset into 2D coords
	uint baseU = baseIndex % VTF_WIDTH;
	uint baseV = baseIndex / VTF_WIDTH;

	float4 mat1 = g_texVTF.Load(uint3(baseU, baseV, 0));
	float4 mat2 = g_texVTF.Load(uint3(baseU + 1, baseV, 0));
	float4 mat3 = g_texVTF.Load(uint3(baseU + 2, baseV, 0));

	return DecodeMatrix(mat1, mat2, mat3);
}

void ComputeSkinned(in float4 position
#ifndef USE_WRITEDEPTH
	, in float3 normal
#endif
	, in float4 blendWeight
	, in uint4 blendIndices
	, in uint nVTFID
	, inout float4 pos_out
#ifndef USE_WRITEDEPTH
	, inout float3 normal_out
#endif
)
{
	pos_out = (float4)0;

#ifndef USE_WRITEDEPTH
	normal_out = (float3)0;
#endif

	float4x4 m;

	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		m = LoadBoneMatrix(nVTFID, blendIndices[i]);
		pos_out += mul(position, m) * blendWeight[i];

#ifndef USE_WRITEDEPTH
		normal_out += mul(normal, (float3x3)m) * blendWeight[i];
#endif
	}
}

#endif

cbuffer cbMatrix
{
	float4x4 g_matProj;

#ifdef USE_CUBEMAP
	float4x4 g_matViewCM[6];
#else
	float4x4 g_matViewProj;
	float4x4 g_matView;
#endif

#ifndef USE_INSTANCING
	float4x4 g_matWorld;

#ifdef USE_SKINNING
	uint g_nVTFID;
#endif
#endif

	float4 g_f4AlbedoColor;
	float4 g_f4EmissiveColor;

	float4 g_f4DisRoughMetEmi;
	float4 g_f4SurSpecTintAniso;
	float4 g_f4SheenTintClearcoatGloss;

#ifdef USE_TESSELLATION
	float4 g_FrustumNormals[4];
	float3 g_FrustumOrigin;

	float g_fTessellationFactor;
#endif
};

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

#ifdef USE_TEX_EMISSIVECOLOR
Texture2D g_texEmissiveColor;
#endif

#ifdef USE_TEX_SURFACE
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

#ifdef USE_CUBEMAP
struct GS_CUBEMAP_INPUT
{
	float4 pos		: SV_Position;
};

struct PS_CUBEMAP_INPUT
{
	float4 pos : SV_Position;     // Projection coord
	uint RTIndex : SV_RenderTargetArrayIndex;
};
#endif

struct PS_INPUT
{
	float4 pos		: SV_Position;	// 위치

#ifndef USE_WRITEDEPTH
	float2 tex		: TEXCOORD0;	// UV
	float3 normal	: NORMAL;		// 노말

	float3 tangent	: TANGENT;		// 탄젠트
	float3 binormal	: BINORMAL;
#endif
};

#ifndef USE_WRITEDEPTH
struct PS_OUTPUT
{
	float4 normals : SV_Target0;
	float4 colors : SV_Target1;
	float4 disneyBRDF : SV_Target2;
};
#endif

#ifdef USE_TESSELLATION

struct HS_INPUT
{
	float4 pos		: POSITION;	// 위치

#ifndef USE_WRITEDEPTH
	float2 tex		: TEXCOORD0;	// UV
	float3 normal	: NORMAL;		// 노말
#endif

#ifdef USE_INSTANCING
	uint instanceID : TEXCOORD1;
#endif
};

HS_INPUT VS(in float4 inPos : POSITION
	, in float2 inTex : TEXCOORD
	, in float3 inNormal : NORMAL

#ifdef USE_SKINNING
	, in float4 inBlendWeight : BLENDWEIGHT
	, in uint4 inBlendIndices : BLENDINDICES
#endif

#ifdef USE_INSTANCING
	, in uint InstanceID : SV_InstanceID
#endif
)
{
	HS_INPUT output = (HS_INPUT)0;
	inPos.w = 1.f;

#ifdef USE_INSTANCING
	output.instanceID = InstanceID;
#endif

#ifdef USE_WRITEDEPTH
	#ifdef USE_INSTANCING
		#ifdef USE_SKINNING
			inBlendWeight.w = 1.f - (inBlendWeight.x + inBlendWeight.y + inBlendWeight.z);
			ComputeSkinned(inPos, inBlendWeight, inBlendIndices, g_Instances[InstanceID].motionData.nVTFID, output.pos);
		#else
			output.pos = inPos;
		#endif
	#else
		#ifdef USE_SKINNING
			inBlendWeight.w = 1.f - (inBlendWeight.x + inBlendWeight.y + inBlendWeight.z);
			ComputeSkinned(inPos, inBlendWeight, inBlendIndices, g_nVTFID, output.pos);
		#else
			output.pos = inPos;
		#endif
	#endif
#else
	#ifdef USE_INSTANCING
		#ifdef USE_SKINNING
			inBlendWeight.w = 1.f - (inBlendWeight.x + inBlendWeight.y + inBlendWeight.z);
			ComputeSkinned(inPos, inNormal, inBlendWeight, inBlendIndices, g_Instances[InstanceID].motionData.nVTFID, output.pos, output.normal);
		#else
			output.pos = inPos;
			output.normal = inNormal;
		#endif
	#else
		#ifdef USE_SKINNING
			inBlendWeight.w = 1.f - (inBlendWeight.x + inBlendWeight.y + inBlendWeight.z);
			ComputeSkinned(inPos, inNormal, inBlendWeight, inBlendIndices, g_nVTFID, output.pos, output.normal);
		#else
			output.pos = inPos;
			output.normal = inNormal;
		#endif
	#endif
#endif

#ifndef USE_WRITEDEPTH
	// UV
	output.tex = inTex;
#endif

	return output;
}

struct HS_ConstantOutput
{
	float Edges[3]  : SV_TessFactor;
	float Inside : SV_InsideTessFactor;

	float3 f3B210   : POSITION3;
	float3 f3B120   : POSITION4;
	float3 f3B021   : POSITION5;
	float3 f3B012   : POSITION6;
	float3 f3B102   : POSITION7;
	float3 f3B201   : POSITION8;
	float3 f3B111   : CENTER;
};

HS_ConstantOutput HS_Constant(InputPatch<HS_INPUT, 3> inputPatch)
{
	HS_ConstantOutput output = (HS_ConstantOutput)0;

	// tessellation factors are proportional to model space edge length
	for (uint ie = 0; ie < 3; ++ie)
	{
		float3 edge = inputPatch[(ie + 1) % 3].pos - inputPatch[ie].pos;
		float3 vec = (inputPatch[(ie + 1) % 3].pos + inputPatch[ie].pos) * 0.5f - g_FrustumOrigin;
		float len = sqrt(dot(edge, edge) / dot(vec, vec));
		output.Edges[(ie + 1) % 3] = max(1.f, g_fTessellationFactor * len);

		//float len = dot(edge, edge) / dot(vec, vec);
		//output.Edges[(ie + 1) % 3] = max(1.f, g_fTessellationFactor * len);
	}

	//// culling
	//int culled[4];
	//for (int ip = 0; ip < 4; ++ip)
	//{
	//	culled[ip] = 1;
	//	culled[ip] &= dot(inputPatch[0].pos - g_FrustumOrigin, g_FrustumNormals[ip].xyz) > 0;
	//	culled[ip] &= dot(inputPatch[1].pos - g_FrustumOrigin, g_FrustumNormals[ip].xyz) > 0;
	//	culled[ip] &= dot(inputPatch[2].pos - g_FrustumOrigin, g_FrustumNormals[ip].xyz) > 0;
	//}
	//
	//if (culled[0] || culled[1] || culled[2] || culled[3])
	//{
	//	output.Edges[0] = 0;
	//}

	// compute the cubic geometry control points
	// edge control points
	output.f3B210 = ((2.f * inputPatch[0].pos) + inputPatch[1].pos - (dot((inputPatch[1].pos - inputPatch[0].pos), inputPatch[0].normal) * inputPatch[0].normal)) / 3.f;
	output.f3B120 = ((2.f * inputPatch[1].pos) + inputPatch[0].pos - (dot((inputPatch[0].pos - inputPatch[1].pos), inputPatch[1].normal) * inputPatch[1].normal)) / 3.f;
	output.f3B021 = ((2.f * inputPatch[1].pos) + inputPatch[2].pos - (dot((inputPatch[2].pos - inputPatch[1].pos), inputPatch[1].normal) * inputPatch[1].normal)) / 3.f;
	output.f3B012 = ((2.f * inputPatch[2].pos) + inputPatch[1].pos - (dot((inputPatch[1].pos - inputPatch[2].pos), inputPatch[2].normal) * inputPatch[2].normal)) / 3.f;
	output.f3B102 = ((2.f * inputPatch[2].pos) + inputPatch[0].pos - (dot((inputPatch[0].pos - inputPatch[2].pos), inputPatch[2].normal) * inputPatch[2].normal)) / 3.f;
	output.f3B201 = ((2.f * inputPatch[0].pos) + inputPatch[2].pos - (dot((inputPatch[2].pos - inputPatch[0].pos), inputPatch[0].normal) * inputPatch[0].normal)) / 3.f;
	// center control point
	float3 f3E = (output.f3B210 + output.f3B120 + output.f3B021 + output.f3B012 + output.f3B102 + output.f3B201) / 6.0f;
	float3 f3V = (inputPatch[0].pos + inputPatch[1].pos + inputPatch[2].pos) / 3.0f;
	output.f3B111 = f3E + ((f3E - f3V) / 2.0f);

	output.Inside = (output.Edges[0] + output.Edges[1] + output.Edges[2]) / 3;

	//float2 t01 = inputPatch[1].tex - inputPatch[0].tex;
	//float2 t02 = inputPatch[2].tex - inputPatch[0].tex;
	//output.sign = t01.x * t02.y - t01.y * t02.x > 0.0f ? 1 : -1;

	return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HS_Constant")]
[maxtessfactor(64.0)]
HS_INPUT HS(InputPatch<HS_INPUT, 3> inputPatch, uint i : SV_OutputControlPointID)
{
	return inputPatch[i];
}

[domain("tri")]
PS_INPUT DS(HS_ConstantOutput input,
	float3 barycentricCoords : SV_DomainLocation,
	OutputPatch<HS_INPUT, 3> inputPatch)
{
	PS_INPUT output = (PS_INPUT)0;

	float3 coordinates = barycentricCoords;

	// The barycentric coordinates
	float fU = barycentricCoords.x;
	float fV = barycentricCoords.y;
	float fW = barycentricCoords.z;

	// Precompute squares and squares * 3 
	float fUU = fU * fU;
	float fVV = fV * fV;
	float fWW = fW * fW;
	float fUU3 = fUU * 3.f;
	float fVV3 = fVV * 3.f;
	float fWW3 = fWW * 3.f;

	// Compute position from cubic control points and barycentric coords
	float3 position = inputPatch[0].pos * fWW * fW + inputPatch[1].pos * fUU * fU + inputPatch[2].pos * fVV * fV +
		input.f3B210 * fWW3 * fU + input.f3B120 * fW * fUU3 + input.f3B201 * fWW3 * fV + input.f3B021 * fUU3 * fV +
		input.f3B102 * fW * fVV3 + input.f3B012 * fU * fVV3 + input.f3B111 * 6.0f * fW * fU * fV;

	float2 texCoord = inputPatch[0].tex * coordinates.z + inputPatch[1].tex * coordinates.x + inputPatch[2].tex * coordinates.y;

	// Compute normal from quadratic control points and barycentric coords
	float3 normal = inputPatch[0].normal * coordinates.z + inputPatch[1].normal * coordinates.x + inputPatch[2].normal * coordinates.y;
	normal = normalize(normal);

	#ifdef USE_TEX_DISPLACEMENT
		float offset = g_texDisplaceMap.SampleLevel(g_samplerState, texCoord, 0).x;
		position += normal * offset;
	#endif

	#ifdef USE_INSTANCING
		float4x4 matWorld = ComputeWorldMatrix(g_Instances[inputPatch[0].instanceID]);
	#else
		float4x4 matWorld = g_matWorld;
	#endif

	float4 pos = mul(float4(position, 1.f), matWorld);
	output.pos = mul(pos, g_matViewProj);
	output.tex = texCoord;

	output.normal = normalize(mul(normal, (float3x3)matWorld));
	output.tangent = CalcTangent(output.normal);
	output.binormal = CalcBinormal(output.normal, output.tangent);

	return output;
}

#else

#ifndef USE_CUBEMAP
PS_INPUT VS(
#else
GS_CUBEMAP_INPUT VS(
#endif
	in float4 inPos : POSITION
	, in float2 inTex : TEXCOORD
	, in float3 inNormal : NORMAL
#ifdef USE_SKINNING
	, in float4 inBlendWeight : BLENDWEIGHT
	, in uint4 inBlendIndices : BLENDINDICES
#endif

#ifdef USE_INSTANCING
	, in uint InstanceID : SV_InstanceID
#endif
)
{
#ifndef USE_CUBEMAP
	PS_INPUT output = (PS_INPUT)0;
#else
	GS_CUBEMAP_INPUT output = (GS_CUBEMAP_INPUT)0;
#endif
	inPos.w = 1.f;

#ifdef USE_WRITEDEPTH
	#ifdef USE_INSTANCING
		#ifdef USE_SKINNING
			float4x4 matWorld = ComputeWorldMatrix(g_Instances[InstanceID].worldData);

			inBlendWeight.w = 1.f - (inBlendWeight.x + inBlendWeight.y + inBlendWeight.z);
			ComputeSkinned(inPos, inBlendWeight, inBlendIndices, g_Instances[InstanceID].motionData.nVTFID, output.pos);

			output.pos = mul(output.pos, matWorld);
		#else
			float4x4 matWorld = ComputeWorldMatrix(g_Instances[InstanceID]);

			output.pos = mul(inPos, matWorld);
		#endif
	#else
		#ifdef USE_SKINNING
			float4x4 matWorld = g_matWorld;

			inBlendWeight.w = 1.f - (inBlendWeight.x + inBlendWeight.y + inBlendWeight.z);
			ComputeSkinned(inPos, inBlendWeight, inBlendIndices, g_nVTFID, output.pos);

			output.pos = mul(output.pos, matWorld);
		#else
			output.pos = mul(inPos, g_matWorld);
		#endif
	#endif
#else
	#ifdef USE_INSTANCING
		#ifdef USE_SKINNING
			float4x4 matWorld = ComputeWorldMatrix(g_Instances[InstanceID].worldData);

			inBlendWeight.w = 1.f - (inBlendWeight.x + inBlendWeight.y + inBlendWeight.z);
			ComputeSkinned(inPos, inNormal, inBlendWeight, inBlendIndices, g_Instances[InstanceID].motionData.nVTFID, output.pos, output.normal);

			output.pos = mul(output.pos, matWorld);
			output.normal = normalize(mul(output.normal, (float3x3)matWorld));
		#else
			float4x4 matWorld = ComputeWorldMatrix(g_Instances[InstanceID]);

			output.pos = mul(inPos, matWorld);
			output.normal = normalize(mul(inNormal, (float3x3)matWorld));
		#endif
	#else
		#ifdef USE_SKINNING
			float4x4 matWorld = g_matWorld;

			inBlendWeight.w = 1.f - (inBlendWeight.x + inBlendWeight.y + inBlendWeight.z);
			ComputeSkinned(inPos, inNormal, inBlendWeight, inBlendIndices, g_nVTFID, output.pos, output.normal);

			output.pos = mul(output.pos, matWorld);
			output.normal = normalize(mul(output.normal, (float3x3)matWorld));
		#else
			float4x4 matWorld = g_matWorld;
			output.pos = mul(inPos, matWorld);
			output.normal = normalize(mul(inNormal, (float3x3)matWorld));
		#endif
	#endif
#endif

	// 정점 위치
#ifndef USE_CUBEMAP
	output.pos = mul(output.pos, g_matViewProj);
#endif

#ifndef USE_WRITEDEPTH
	// UV
	output.tex = inTex;

	output.tangent = CalcTangent(output.normal);
	output.binormal = CalcBinormal(output.normal, output.tangent);

#endif

	return output;
}

#endif

#ifndef USE_WRITEDEPTH
PS_OUTPUT D_PS(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;

#ifdef USE_TEX_ALBEDO
	float4 albedo = saturate(g_f4AlbedoColor * pow(abs(g_texAlbedo.Sample(g_samplerState, input.tex)), 2.2f) * 2.f);
#else
	float4 albedo = g_f4AlbedoColor;
#endif

#ifdef USE_TEX_MASK
	albedo.w = saturate(albedo.w * g_texMask.Sample(g_samplerState, input.tex).x * 2.f);
	clip(albedo.w - 0.1f);
#elif USE_MASK_IN_ALBEDO
	clip(albedo.w - 0.1f);
#endif

#ifdef USE_TEX_NORMAL
	float3 normal = g_texNormalMap.Sample(g_samplerState, input.tex).xyz;
	normal = normalize(2.f * normal - 1.f);
	normal = normalize((normal.x * input.tangent) + (normal.y * input.binormal) + (input.normal));
#else
	float3 normal = input.normal;
#endif

	output.normals.xy = CompressNormal(normal);
	output.normals.zw = CompressNormal(input.tangent);

	float3 RM = float3(g_f4DisRoughMetEmi.yz, 0.f);
#ifdef USE_TEX_ROUGHNESS
	RM.x = g_texRoughness.Sample(g_samplerState, input.tex).x;
#endif

#ifdef USE_TEX_METALLIC
	RM.y = g_texMetallic.Sample(g_samplerState, input.tex).x;
#endif

	float emissiveIntensity = g_f4DisRoughMetEmi.w;
#ifdef USE_TEX_EMISSIVE
	emissiveIntensity = g_texEmissive.Sample(g_samplerState, input.tex).x;
#endif

#ifdef USE_TEX_SPECULARCOLOR
	float3 specular = g_texSpecularColor.Sample(g_samplerState, input.tex).xyz;
#else
	float3 specular = lerp(0.03f, albedo.xyz, RM.y);
#endif

#ifdef USE_TEX_EMISSIVECOLOR
	float3 emissiveColor = g_texEmissiveColor.Sample(g_samplerState, input.tex).xyz;
#else
	float3 emissiveColor = g_f4EmissiveColor.xyz;
#endif

	output.colors.x = Pack3PNForFP32(saturate(albedo));
	output.colors.y = Pack3PNForFP32(specular);
	output.colors.z = Pack3PNForFP32(emissiveColor);
	output.colors.w = emissiveIntensity;

	float3 SST = g_f4SurSpecTintAniso.xyz;
	float3 AST = float3(g_f4SurSpecTintAniso.w, g_f4SheenTintClearcoatGloss.xy);
	float3 CG = float3(g_f4SheenTintClearcoatGloss.zw, 0.f);

#ifdef USE_TEX_SURFACE
	SST.x = g_texSurface.Sample(g_samplerState, input.tex).x;
#endif
#ifdef USE_TEX_SPECULAR
	SST.y = g_texSpecular.Sample(g_samplerState, input.tex).x;
#endif
#ifdef USE_TEX_SPECULARTINT
	SST.z = g_texSpecularTint.Sample(g_samplerState, input.tex).x;
#endif
#ifdef USE_TEX_ANISOTROPIC
	AST.x = g_texAnisotropic.Sample(g_samplerState, input.tex).x;
#endif

	float4 STCG = g_f4SheenTintClearcoatGloss;
#ifdef USE_TEX_SHEEN
	AST.y = g_texSheen.Sample(g_samplerState, input.tex).x;
#endif
#ifdef USE_TEX_SHEENTINT
	AST.z = g_texSheenTint.Sample(g_samplerState, input.tex).x;
#endif
#ifdef USE_TEX_CLEARCOAT
	CG.x = g_texClearcoat.Sample(g_samplerState, input.tex).x;
#endif
#ifdef USE_TEX_CLEARCOATGLOSS
	CG.y = g_texClearcoatGloss.Sample(g_samplerState, input.tex).x;
#endif

	output.disneyBRDF.x = Pack3PNForFP32(RM);
	output.disneyBRDF.y = Pack3PNForFP32(SST);
	output.disneyBRDF.z = Pack3PNForFP32(AST);
	output.disneyBRDF.w = Pack3PNForFP32(CG);

	return output;
}
#else
	#ifdef USE_CUBEMAP
	[maxvertexcount(18)]
	void GS_CubeMap(triangle GS_CUBEMAP_INPUT input[3], inout TriangleStream<PS_CUBEMAP_INPUT> CubeMapStream)
	{
		for (int f = 0; f < 6; ++f)
		{
			// Compute screen coordinates
			PS_CUBEMAP_INPUT output;
			output.RTIndex = f;
			for (int v = 0; v < 3; v++)
			{
				output.pos = mul(input[v].pos, g_matViewCM[f]);
				output.pos = mul(output.pos, g_matProj);
				CubeMapStream.Append(output);
			}
			CubeMapStream.RestartStrip();
		}
	}
	#endif
#endif

#ifdef USE_TESSELLATION
	#ifdef USE_SKINNING
	technique11 ModelSkinned_Tessellation
	#else
	technique11 ModelStatic_Tessellation
	#endif
	{
		pass Pass_0
		{
			SetVertexShader(CompileShader(vs_5_0, VS()));

	#ifdef USE_WRITEDEPTH
			SetHullShader(NULL);
			SetDomainShader(NULL);
			SetGeometryShader(NULL);
			SetPixelShader(NULL);
	#else
			SetHullShader(CompileShader(hs_5_0, HS()));
			SetDomainShader(CompileShader(ds_5_0, DS()));
			SetGeometryShader(NULL);
			SetPixelShader(CompileShader(ps_5_0, D_PS()));
	#endif
		}
	}
#else
	#ifdef USE_SKINNING
	technique11 ModelSkinned
	#else
	technique11 ModelStatic
	#endif
	{
		pass Pass_0
		{
			SetVertexShader(CompileShader(vs_5_0, VS()));
			SetHullShader(NULL);
			SetDomainShader(NULL);

	#ifdef USE_CUBEMAP
			SetGeometryShader(CompileShader(gs_5_0, GS_CubeMap()));
	#else
			SetGeometryShader(NULL);
	#endif

	#ifdef USE_WRITEDEPTH
			SetPixelShader(NULL);
	#else
			SetPixelShader(CompileShader(ps_5_0, D_PS()));
	#endif
		}
	}
#endif

#endif