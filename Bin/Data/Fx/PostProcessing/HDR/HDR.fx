#include "../../Converter.fx"

static const float3 LUM_CONVERT = float3(0.299f, 0.587f, 0.114f);
static const int MAX_SAMPLES = 16; // Maximum texture grabs
const static float4 COLOR_PURPLE = float4(0.7f, 0.2f, 0.9f, 1.f);
const static float4 COLOR_ORANGE = float4(0.7f, 0.4f, 0.2f, 1.f);
const static float LENSFLARE_THRESHOLD = 0.1f;

cbuffer cbContents
{
	float g_fMiddleGrey = 0.6f;
	float g_fMaxLuminance = 16.f;

	float g_fElapsedTime;
	float g_fBloomMultiplier;

	float g_fThreshold = 0.7f;

	float2 g_f2SourceDimensions;
};

Texture2D g_texColor;
Texture2D g_texBloom;
Texture2D<float> g_texLuminanceCur;
Texture2D<float> g_texLuminanceLast;
Texture2D g_texLensFlare1;
Texture2D g_texLensFlare2;
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

float LuminancePS(PS_INPUT input) : SV_Target0
{
	float3 f3Color = g_texColor.Sample(g_samplerLinear, input.tex).rgb;

	// calculate the luminance using a weighted average
	float fLuminance = dot(f3Color, LUM_CONVERT);

	float fLogLuminace = log(1e-5 + fLuminance);

	// Output the luminance to the render target
	return fLogLuminace;
}

float CalcAdaptedLumPS(PS_INPUT input) : SV_Target0
{
	float fLastLum = g_texLuminanceLast.Sample(g_samplerPoint, float2(0.5f, 0.5f));
	float fCurrentLum = g_texLuminanceCur.Sample(g_samplerPoint, float2(0.5f, 0.5f));

	// Adapt the luminance using Pattanaik's technique11
	const float fTau = 0.5f;
	float fAdaptedLum = fLastLum + (fCurrentLum - fLastLum) * (1 - exp(-g_fElapsedTime * fTau));

	return fAdaptedLum;
}

float3 ApplyToneMap(float3 f3CColor)
{
	// Get the calculated average luminance
	float fLumAvg = g_texLuminanceCur.Sample(g_samplerPoint, float2(0.5f, 0.5f));

	// Calculate the luminance of the current pixel
	float fLumPixel = dot(f3CColor, LUM_CONVERT);

	// Apply the modified operator (Eq. 4)
	float fLumScaled = (fLumPixel * g_fMiddleGrey) / fLumAvg;
	float fLumCompressed = (fLumScaled * (1 + (fLumScaled / (g_fMaxLuminance * g_fMaxLuminance)))) / (1 + fLumScaled);

	return fLumCompressed * f3CColor;
}

float4 ToneMapPS(PS_INPUT input) : SV_Target0
{
	// Sample the original HDR image
	float3 f3HDRColor = g_texColor.Sample(g_samplerPoint, input.tex).rgb;

	// Do the tone-mapping
	float3 f3ToneMapped = ApplyToneMap(f3HDRColor);

	// Add in the bloom component
	float3 f3Color = f3ToneMapped + g_texBloom.Sample(g_samplerLinear, input.tex).rgb * g_fBloomMultiplier;

	return float4(f3Color, 1.f);
}

float4 ThresholdPS(PS_INPUT input) : SV_Target0
{
	float4 f4Sample = g_texColor.Sample(g_samplerPoint, input.tex);
	f4Sample = float4(ApplyToneMap(f4Sample.rgb), 1.f);

	f4Sample -= g_fThreshold;
	f4Sample = max(f4Sample, 0.f);

	return f4Sample;
}

float4 LensFlarePS(PS_INPUT input,
	uniform int NumSamples,
	uniform float4 f4Tint,
	uniform float fTexScale,
	uniform float fBlurScale) : SV_Target0
{
	// The flare should appear on the opposite side of the screen as the
	// source of the light, so first we mirror the texture coordinate.
	// Then we normalize so we can apply a scaling factor.
	float2 f2MirrorCoord = float2(1.f, 1.f) - input.tex;
	float2 f2NormalizedCoord = f2MirrorCoord * 2.f - 1.f;
	f2NormalizedCoord *= fTexScale;

	// We'll blur towards the center of screen, and also away from it.

	float2 f2TowardCenter = normalize(-f2NormalizedCoord);
	float2 fBlurDist = fBlurScale * NumSamples;
	float2 f2StartPoint = f2NormalizedCoord + ((f2TowardCenter / g_f2SourceDimensions) * fBlurDist);
	float2 f2Step = -(f2TowardCenter / g_f2SourceDimensions) * 2 * fBlurDist;

	// For the first half of the samples we want an orange tint, and for 
	// the second half we want a purple tint. 
	float4 f4Sum = 0.f;
	float2 f2SamplePos = f2StartPoint;
	for (int i = 0; i < NumSamples; i++)
	{
		float2 f2SampleTexCoord = f2SamplePos * 0.5f + 0.5f;

		// Don't add in samples past texture border
		if (f2SampleTexCoord.x >= 0 && f2SampleTexCoord.x <= 1.f && f2SampleTexCoord.y >= 0 && f2SampleTexCoord.y <= 1.f)
		{
			float4 f4Sample = g_texColor.Sample(g_samplerPoint, f2SampleTexCoord);
			f4Sum += max(0, f4Sample - LENSFLARE_THRESHOLD) * f4Tint;
		}

		f2SamplePos += f2Step;
	}

	return f4Sum / NumSamples;
}

float4 CombinePS(PS_INPUT input) : SV_Target0
{
	float4 f4Color = g_texColor.Sample(g_samplerPoint, input.tex);
	f4Color += g_texLensFlare1.Sample(g_samplerPoint, input.tex);
	f4Color += g_texLensFlare2.Sample(g_samplerPoint, input.tex);
	return f4Color;
}

technique11 Luminance
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, LuminancePS()));
	}
}

technique11 CalcAdaptedLuminance
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, CalcAdaptedLumPS()));
	}
}

technique11 ToneMap
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, ToneMapPS()));
	}
}

technique11 Threshold
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, ThresholdPS()));
	}
}

technique11 LensFlareFirstPass
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, LensFlarePS(12, COLOR_ORANGE, 2.00f, 0.15f)));
	}
}

technique11 LensFlareSecondPass
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, LensFlarePS(12, COLOR_PURPLE, 0.5f, 0.1f)));
	}
}

technique11 Combine
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, CombinePS()));
	}
}