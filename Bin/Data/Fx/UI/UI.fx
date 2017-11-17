#ifndef _UI_
#define _UI_

struct InstUIData
{
	/*
	x : Left
	y : Right
	z : Top
	w : Bottom
	*/
	float4 rect;
	float4 uv;
	float4 color;
	float depth;
};

cbuffer cbInstData
{
	InstUIData g_instUIData[128];
}

Texture2D g_texDiffuse;
SamplerState g_samLinearWrap;;

struct HS_INPUT
{
	float4 pos		: POSITION;
	float2 uv		: TEXCOORD0;
	float4 color	: TEXCOORD1;
};

struct DS_INPUT
{
	float4 pos		: POSITION;
	float2 uv		: TEXCOORD0;
	float4 color	: TEXCOORD1;
};

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 uv		: TEXCOORD0;
	float4 color	: TEXCOORD1;
};

struct ConstantOutputType
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

ConstantOutputType ConstantsHs(InputPatch<HS_INPUT, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
	ConstantOutputType output;

	output.edges[0] = 2.f;
	output.edges[1] = 2.f;
	output.edges[2] = 2.f;
	output.inside = 2.f;

	return output;
}

HS_INPUT VS(uint vertexID : SV_VertexID, uint InstanceID : SV_InstanceID)
{
	HS_INPUT output;
	InstUIData ui = g_instUIData[InstanceID];

	if (vertexID == 0)
	{
		output.pos = float4(ui.rect.x, ui.rect.z, ui.depth, 1.f);
		output.uv = float2(ui.uv.x, ui.uv.z);
	}
	else if (vertexID == 1)
	{
		output.pos = float4(ui.rect.y, ui.rect.z, ui.depth, 1.f);
		output.uv = float2(ui.uv.y, ui.uv.z);
	}
	else if (vertexID == 2)
	{
		output.pos = float4(ui.rect.x, ui.rect.w, ui.depth, 1.f);
		output.uv = float2(ui.uv.x, ui.uv.w);
	}
	else
	{
		output.pos = float4(ui.rect.y, ui.rect.w, ui.depth, 1.f);
		output.uv = float2(ui.uv.y, ui.uv.w);
	}
	output.color = ui.color;

	return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantsHs")]
DS_INPUT HS(InputPatch<HS_INPUT, 3> patch, uint pointId : SV_outputControlPointID)
{
	DS_INPUT output;

	output.pos = patch[pointId].pos;
	output.uv = patch[pointId].uv;
	output.color = patch[pointId].color;

	return output;
}

[domain("tri")]
PS_INPUT DS(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<DS_INPUT, 3> patch)
{
	DS_INPUT output;

	output.pos = uvwCoord.x * patch[0].pos + uvwCoord.y * patch[1].pos + uvwCoord.z * patch[2].pos;
	output.uv = uvwCoord.x * patch[0].uv + uvwCoord.y * patch[1].uv + uvwCoord.z * patch[2].uv;
	output.color = uvwCoord.x * patch[0].color + uvwCoord.y * patch[1].color + uvwCoord.z * patch[2].color;

	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return g_texDiffuse.Sample(g_samLinearWrap, input.uv) * input.color;
}

technique11 UI
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

#endif
