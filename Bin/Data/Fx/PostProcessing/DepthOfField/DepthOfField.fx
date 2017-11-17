#include "../../Converter.fx"

static const int NUM_DOF_TAPS = 12;
static const float MAX_COC = 10.0f;

cbuffer cbContents
{
	float g_fFocalDistance;
	float g_fFocalWidth;

	float2 g_f2FilterTaps[NUM_DOF_TAPS];

	float4x4 g_matInvProj;
};

Texture2D g_texColor;
sampler g_samplerPoint;
sampler g_samplerLinear;

Texture2D<float> g_texDepth;

float GetBlurFactor(in float fDepthVS)
{
	return smoothstep(0.f, g_fFocalWidth, abs(g_fFocalDistance - fDepthVS));
}

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

float4 DofDiscPS(PS_INPUT input) : SV_Target0
{
	// Start with center sample color
	float4 f4ColorSum = g_texColor.Sample(g_samplerPoint, input.tex);
	float fTotalContribution = 1.f;

	// Depth and blurriness values for center sample
	float fDepth = g_texDepth.Sample(g_samplerPoint, input.tex).x;

	float fCenterDepth = CalcViewSpcaePosFromDepth(fDepth, input.tex, g_matInvProj).z;

	float fCenterBlur = GetBlurFactor(fCenterDepth);

	if (fCenterBlur > 0.f)
	{
		// Compute CoC size based on blurriness
		float fSizeCoC = fCenterBlur * MAX_COC;

		// Run through all filter taps
		[unroll]
		for (int i = 0; i < NUM_DOF_TAPS; ++i)
		{
			// Compute sample coordinates
			float2 f2TapCoord = input.tex + g_f2FilterTaps[i] * fSizeCoC;

			// Fetch filter tap sample
			float4 f4TapColor = g_texColor.Sample(g_samplerLinear, f2TapCoord);
			float fTapDepth = g_texDepth.Sample(g_samplerPoint, f2TapCoord).x;
			float fTapBlur = GetBlurFactor(fTapDepth);

			// Compute tap contribution based on depth and blurriness
			float fTapContribution = (fTapDepth > fCenterDepth) ? 1.0f : fTapBlur;

			// Accumulate color and sample contribution
			f4ColorSum += f4TapColor * fTapContribution;
			fTotalContribution += fTapContribution;
		}
	}

	// Normalize color sum
	return f4ColorSum / fTotalContribution;
}

technique11 DOFDiscBlur
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, DofDiscPS()));
	}
}