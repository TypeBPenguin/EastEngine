#ifdef DX12
#include "../../DescriptorTablesDX12.hlsl"
#endif

cbuffer cbContents : register(b0)
{
	float2 g_f2SourceDimensions;

#ifdef DX12
	uint g_nTexColorIndex;
	float padding;
#else
	float2 padding;
#endif
};

#ifdef DX11

Texture2D g_texColor : register(t0);
SamplerState g_samplerPoint : register(s0);
SamplerState g_samplerLinear : register(s1);

#elif DX12

#define g_texColor Tex2DTable[g_nTexColorIndex]
SamplerState g_samplerPoint : register(s0, space100);
SamplerState g_samplerLinear : register(s1, space100);

#endif

#ifndef IS_DECODE_LUMINANCE
#define IS_DECODE_LUMINANCE 0
#endif

static const float g_fOffsets[4] = { -1.5f, -0.5f, 0.5f, 1.5f };

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
float4 DownscalePS(PS_INPUT input) : SV_Target0
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

#if IS_DECODE_LUMINANCE == 1
	f4Color = float4(exp(f4Color.r), 1.f, 1.f, 1.f);
#endif

	return f4Color;
}

// Downscales to 1/16 size, using 16 samples
float4 DownscaleLuminancePS(PS_INPUT input) : SV_Target0
{
	float4 f4Color = DownscalePS(input);

	f4Color = float4(exp(f4Color.r), 1.f, 1.f, 1.f);

	return f4Color;
}

// Upscales or downscales using hardware bilinear filtering
float4 HWScalePS(PS_INPUT input) : SV_Target0
{
	return g_texColor.Sample(g_samplerLinear, input.tex);
}