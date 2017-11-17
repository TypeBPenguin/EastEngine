#ifndef _VERTEX_VERTEX_
#define _VERTEX_VERTEX_

cbuffer cbMatrix
{
	float4x4 g_matWVP;
};

cbuffer cbColor
{
	float4 g_Color;
};

struct VS_INPUT
{
	float4 pos : POSITION;
};

struct PS_INPUT
{
	float4 pos : SV_Position;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	input.pos.w = 1.f;

	output.pos = mul(input.pos, g_matWVP);

	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return g_Color;
}

struct VS_INPUT_COLOR
{
	float4 pos : POSITION;
	float4 color : COLOR;
};

struct PS_INPUT_COLOR
{
	float4 pos : SV_Position;
	float4 color : TEXCOORD0;
};

PS_INPUT_COLOR VS_COLOR(VS_INPUT_COLOR input)
{
	PS_INPUT_COLOR output;
	input.pos.w = 1.f;

	output.pos = mul(input.pos, g_matWVP);
	output.color = input.color;

	return output;
}

float4 PS_COLOR(PS_INPUT_COLOR input) : SV_Target
{
	return input.color;
}

technique11 Vertex
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_COLOR()));
		SetPixelShader(CompileShader(ps_5_0, PS_COLOR()));
	}
}

technique11 Color
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

#endif
