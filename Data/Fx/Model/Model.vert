#version 450
#extension GL_ARB_separate_shader_objects : enable

#define lerp(a, b, t) mix(a, b, t)
#define saturate(a) clamp(a, 0.0, 1.0)
#define mad(a, b, c) (a * b + c)
#define mul(v, m) (m * v)
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define float4x4 mat4x4
#define float3x3 mat3x3

#include "../Converter.hlsl"

float4x4 DecodeMatrix(in float4 encodedMatrix0, in float4 encodedMatrix1, in float4 encodedMatrix2)
{
	return float4x4(float4(encodedMatrix0.xyz, 0),
		float4(encodedMatrix1.xyz, 0),
		float4(encodedMatrix2.xyz, 0),
		float4(encodedMatrix0.w, encodedMatrix1.w, encodedMatrix2.w, 1.f));
}

#ifdef USE_INSTANCING
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
layout(set = 0, binding = 0) uniform ubSkinningInstancingData
{
	SkinningInstancingData data[MAX_INSTANCE_CONSTANTS];
} InstancingData;
#else
layout(set = 0, binding = 0) uniform ubStaticInstancingData
{
	float4x4 data[MAX_INSTANCE_CONSTANTS];
} InstancingData;
#endif	// USE_SKINNING

#else

layout(set = 0, binding = 0) uniform ubObjectData
{
	float4x4 matWorld;

	uint nVTFID;
	float3 f3Padding;
} ObjectData;

#endif	// USE_INSTANCING

layout(set = 1, binding = 0) uniform ubVSConstants
{
	float4x4 matViewProj;

	//#ifdef USE_TESSELLATION
	//	float4 g_FrustumNormals[4];
	//	float3 g_FrustumOrigin;
	//
	//	float g_fTessellationFactor;
	//#endif
} VSConstants;

#ifdef USE_SKINNING
#define VTF_WIDTH 1024
layout(set = 0, binding = 1) uniform sampler2D g_texVTF;

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

layout(location = 0) in float3 inPos;
layout(location = 1) in float2 inTex;
layout(location = 2) in float3 inNormal;

#ifdef USE_SKINNING
layout(location = 3) in float4 inBlendWeight;
layout(location = 4) in uint4 inBlendIndices;
#endif	// USE_SKINNING

//#ifdef USE_INSTANCING
//	, in uint InstanceID : SV_InstanceID
//#endif	// USE_INSTANCING

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) out float4 outPos;
layout(location = 1) out float2 outUV;
layout(location = 2) out float3 outNormal;
layout(location = 3) out float3 outTangent;
layout(location = 4) out float3 outBinormal;

void main()
{
	outPos.w = 1.f;

#ifdef USE_SKINNING
	#ifdef USE_INSTANCING

	float4x4 matWorld = ComputeWorldMatrix(InstancingData.data[InstanceID].transformData);
	uint nVTFID = InstancingData.data[InstanceID].motionData.nVTFID;

	#else	// USE_INSTANCING

	float4x4 matWorld = ObjectData.matWorld;
	uint nVTFID = ObjectData.nVTFID;

	#endif	// USE_INSTANCING

	inBlendWeight.w = 1.f - (inBlendWeight.x + inBlendWeight.y + inBlendWeight.z);
	ComputeSkinning(float4(inPos, 1.f), inNormal, inBlendWeight, inBlendIndices, nVTFID, outPos, outNormal);
#else	// USE_SKINNING
	#ifdef USE_INSTANCING

	float4x4 matWorld = InstancingData.data[InstanceID];

	#else	// USE_INSTANCING

	float4x4 matWorld = ObjectData.matWorld;

	#endif	// USE_INSTANCING

	outPos.xyz = inPos;
	outNormal = inNormal;
#endif	// USE_SKINNING

	outPos = mul(outPos, matWorld);
	outPos = mul(outPos, VSConstants.matViewProj);

	outNormal = normalize(mul(outNormal, float3x3(matWorld)));

	// UV
	outUV = inTex;

	outTangent = CalcTangent(outNormal);
	outBinormal = CalcBinormal(outNormal, outTangent);

	gl_Position = outPos;
}