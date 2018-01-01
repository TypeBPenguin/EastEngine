#ifndef _VERTEX_VERTEX_
#define _VERTEX_VERTEX_

#define MAX_INSTANCE_CONSTANTS 128

struct InstDataStatic
{
	float4 f4World1;
	float4 f4World2;
	float4 f4World3;
};

struct InstDataVertex
{
	InstDataStatic worldData;

	float4 colorData;
};

cbuffer cbInstData
{
	InstDataVertex g_Instances[MAX_INSTANCE_CONSTANTS];
}

float4x4 DecodeMatrix(in float4 encodedMatrix0, in float4 encodedMatrix1, in float4 encodedMatrix2)
{
	return float4x4(float4(encodedMatrix0.xyz, 0),
		float4(encodedMatrix1.xyz, 0),
		float4(encodedMatrix2.xyz, 0),
		float4(encodedMatrix0.w, encodedMatrix1.w, encodedMatrix2.w, 1.f));
}

float4x4 ComputeWorldMatrix(in InstDataStatic worldData)
{
	return DecodeMatrix(worldData.f4World1, worldData.f4World2, worldData.f4World3);
}

cbuffer cbMatrix
{
	float4x4 g_matWVP;
	float4x4 g_matViewProjection;
};

cbuffer cbColor
{
	float4 g_color;
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
	float4x4 matWorld = ComputeWorldMatrix(g_Instances[InstanceID].worldData);

	PS_INPUT output;
	inPos.w = 1.f;

	output.pos = mul(inPos, matWorld);
	output.pos = mul(output.pos, g_matViewProjection);
	output.color = g_Instances[InstanceID].colorData;

	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return input.color;
}

struct PS_INPUT_COLOR
{
	float4 pos : SV_Position;
	float4 color : TEXCOORD0;
};

PS_INPUT_COLOR VS_COLOR(in float4 inPos : POSITION, in float4 inColor : COLOR)
{
	PS_INPUT_COLOR output;
	inPos.w = 1.f;

	output.pos = mul(inPos, g_matWVP);
	output.color = inColor;

	return output;
}

float4 PS_COLOR(PS_INPUT_COLOR input) : SV_Target
{
	return input.color;
}

technique11 Color
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_COLOR()));
		SetPixelShader(CompileShader(ps_5_0, PS_COLOR()));
	}
}

technique11 Vertex
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 VertexInstancing
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Instancing()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

#endif