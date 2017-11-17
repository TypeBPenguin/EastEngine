cbuffer cbContents
{
	float2 g_f2SourceDimensions;

	static const float g_fOffsets[4] = { -1.5f, -0.5f, 0.5f, 1.5f };
};

Texture2D g_texColor;
sampler g_samplerPoint;
sampler g_samplerLinear;

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
};

PS_INPUT VS(uint id : SV_VertexID)
{
	PS_INPUT output;
	output.tex = float2(((id << 1) & 2) != 0, (id & 2) != 0);
	output.pos = float4(output.tex * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);

	return output;
}

// Downscales to 1/16 size, using 16 samples
float4 DownscalePS(PS_INPUT input, uniform bool isDecodeLuminance) : SV_Target0
{
	float4 f4Color = 0.f;
	for (int x = 0; x < 4; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			float2 f2Offset = float2(g_fOffsets[x], g_fOffsets[y]) / g_f2SourceDimensions;
			float4 f4Sample = g_texColor.Sample(g_samplerPoint, input.tex + f2Offset);
			f4Color += f4Sample;
		}
	}

	f4Color /= 16.f;

	if (isDecodeLuminance == true)
	{
		f4Color = float4(exp(f4Color.r), 1.f, 1.f, 1.f);
	}

	return f4Color;
}

// Upscales or downscales using hardware bilinear filtering
float4 HWScalePS(PS_INPUT input) : SV_Target0
{
	return g_texColor.Sample(g_samplerLinear, input.tex);
}

technique11 Downscale4
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, DownscalePS(false)));
	}
}

technique11 Downscale4Luminance
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, DownscalePS(true)));
	}
}

technique11 DownscaleHW
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, HWScalePS()));
	}
}