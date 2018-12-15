#ifdef DX12
#include "../DescriptorTablesDX12.hlsl"
#endif

#include "../Converter.hlsl"

cbuffer cbSingle : register(b0)
{
	float4x4 g_matWVP;
	float4 g_color;
};

#define eMaxInstancingCount 64

struct TransformData
{
	float4 matrix1;
	float4 matrix2;
	float4 matrix3;
	float4 color;
};

float4x4 ComputeWorldMatrix(in TransformData transformData)
{
	return DecodeMatrix(transformData.matrix1, transformData.matrix2, transformData.matrix3);
}

cbuffer cbInstance : register(b1)
{
	float4x4 g_matViewProjection;
	TransformData g_Instances[eMaxInstancingCount];
};

struct PS_INPUT
{
	float4 pos : SV_Position;
	float4 color : TEXCOORD0;
};

PS_INPUT VS(in float4 inPos : POSITION)
{
	PS_INPUT output;
	inPos.w = 1.f;

	output.pos = mul(inPos, g_matWVP);
	output.color = g_color;

	return output;
}

PS_INPUT VS_Instancing(in float4 inPos : POSITION, in uint InstanceID : SV_InstanceID)
{
	float4x4 matWorld = ComputeWorldMatrix(g_Instances[InstanceID]);

	PS_INPUT output;
	inPos.w = 1.f;

	output.pos = mul(inPos, matWorld);
	output.pos = mul(output.pos, g_matViewProjection);
	output.color = g_Instances[InstanceID].color;

	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return input.color;
}