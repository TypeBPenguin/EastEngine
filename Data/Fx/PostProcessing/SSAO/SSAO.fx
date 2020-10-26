#ifndef _SSAO_
#define _SSAO_

#include "../../Converter.fx"

struct Constants
{
	float4 f4OffsetVectors[14];

	// Coordinates given in view space.
	float fOcclusionRadius;
	float fOcclusionFadeStart;
	float fOcclusionFadeEnd;
	float fSurfaceEpsilon;
};

cbuffer cbConstants
{
	Constants g_constants;
};

cbuffer cbPerObject
{
	float4x4 g_matViewToTexSpace; // Proj * texture
	float4x4 g_matView; // Proj * texture
	float4	g_f4FrustumCorners[4];
};

Texture2D g_texNormal;
Texture2D g_texDepth;
Texture2D g_texRandomVector;

SamplerState g_samLinearPointBorder;
SamplerState g_samLinearPointWrap;

struct PS_INPUT
{
	float4 pos		: SV_POSITION;
	float2 tex			: TEXCOORD0;
	float3 toFarPlane	: TEXCOORD1;
};

PS_INPUT VS(uint id : SV_VertexID)
{
	PS_INPUT output;
	output.tex = float2((id << 1) & 2, id & 2);
	output.pos = float4(output.tex * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);
	output.toFarPlane = g_f4FrustumCorners[id].xyz;

	return output;
}

float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;
	if (distZ > g_constants.fSurfaceEpsilon)
	{
		float fadeLength = g_constants.fOcclusionFadeEnd - g_constants.fOcclusionFadeStart;

		occlusion = saturate((g_constants.fOcclusionFadeEnd - distZ) / fadeLength);
	}

	return occlusion;
}

float4 PS(PS_INPUT input, uniform int gSampleCount) : SV_Target
{
	float3 normal = DeCompressNormal(g_texNormal.Sample(g_samLinearPointBorder, input.tex).xy);
	normal = mul(normal, (float3x3)g_matView);
	//normal = normalize(normal);

	//float pz = g_texDepth.Sample(g_samLinearPointBorder, input.tex).w * 10000.f;
	
	//float3 p = (pz / input.toFarPlane.z) * input.toFarPlane;
	float3 p = g_texDepth.Sample(g_samLinearPointBorder, input.tex).xyz;
	p = mul(float4(p, 1.f), g_matView).xyz;
		
	float3 randVec = 2.0f * g_texRandomVector.Sample(g_samLinearPointWrap, 4.0f * input.tex).rgb - 1.0f;
	randVec = normalize(randVec);

	float occlusionSum = 0.0f;

	[unroll]
	for (int i = 0; i < gSampleCount; ++i)
	{
		float3 offset = reflect(g_constants.f4OffsetVectors[i].xyz, randVec);

		float flip = sign(dot(offset, normal));

		float3 q = p + g_constants.fOcclusionRadius * offset;
		 
		float4 projQ = mul(float4(q, 1.0f), g_matViewToTexSpace);
		projQ /= projQ.w;

		//float rz = g_texDepth.Sample(g_samLinearPointBorder, projQ.xy).w * 10000.f;
		//
		//float3 r = (rz / q.z) * q;

		float3 r = g_texDepth.Sample(g_samLinearPointBorder, projQ.xy).xyz;
		r = mul(float4(r, 1.f), g_matView).xyz;
			
		float distZ = p.z - r.z;
		float dp = max(dot(normal, normalize(r - p)), 0.0f);
		float occlusion = dp * OcclusionFunction(distZ);

		occlusionSum += occlusion;
	}

	occlusionSum /= gSampleCount;

	float access = 1.0f - occlusionSum;

	return saturate(pow(access, 4.0f));
}

///////////////
// Ssao Blur
cbuffer cbPerObjectBlur
{
	float g_fTexelSize;
	int g_fBlurRadius = 5;

	static const float g_fWeights[11] =
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
	};
};

Texture2D g_texSSAO;
SamplerState g_samLinearPointClamp;

struct PS_INPUT_BLUR
{
	float4 pos  : SV_POSITION;
	float2 tex   : TEXCOORD0;
};

PS_INPUT_BLUR VS_BLUR(uint id : SV_VertexID)
{
	PS_INPUT_BLUR output;
	output.tex = float2((id << 1) & 2, id & 2);
	output.pos = float4(output.tex * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);

	return output;
}

float4 PS_BLUR(PS_INPUT_BLUR input, uniform bool isHorizontalBlur) : SV_Target
{
	float2 texOffset;
	if (isHorizontalBlur)
	{
		texOffset = float2(g_fTexelSize, 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, g_fTexelSize);
	}

	// The center value always contributes to the sum.
	float4 color = g_fWeights[5] * g_texSSAO.SampleLevel(g_samLinearPointClamp, input.tex, 0.0);
	float totalWeight = g_fWeights[5];

	float3 centerNormal = DeCompressNormal(g_texNormal.SampleLevel(g_samLinearPointClamp, input.tex, 0.0f).xy);
	float centerDepth = g_texDepth.SampleLevel(g_samLinearPointClamp, input.tex, 0.0f).w * 10000.f;

	for (float i = -g_fBlurRadius; i <= g_fBlurRadius; ++i)
	{
		// We already added in the center weight.
		if (i == 0)
			continue;

		float2 tex = input.tex + i * texOffset;

		float3 neighborNormal = DeCompressNormal(g_texNormal.SampleLevel(g_samLinearPointClamp, tex, 0.0f).xy);
		float neighborDepth = g_texDepth.SampleLevel(g_samLinearPointClamp, tex, 0.0f).w * 10000.f;

		//
		// If the center value and neighbor values differ too much (either in 
		// normal or depth), then we assume we are sampling across a discontinuity.
		// We discard such samples from the blur.
		//

		if (dot(neighborNormal, centerNormal) >= 0.8f &&
			abs(neighborDepth - centerDepth) <= 0.2f)
		{
			float weight = g_fWeights[i + g_fBlurRadius];

			// Add neighbor pixel to blur.
			color += weight * g_texSSAO.SampleLevel(g_samLinearPointClamp, tex, 0.0);

			totalWeight += weight;
		}
	}

	// Compensate for discarded samples by making total weights sum to 1.
	return color / totalWeight;
}

Texture2D g_texScreen;

struct PS_INPUT_APPLY
{
	float4 pos  : SV_POSITION;
	float2 tex   : TEXCOORD0;
};

PS_INPUT_APPLY VS_APPLY(uint id : SV_VertexID)
{
	PS_INPUT_BLUR output;
	output.tex = float2((id << 1) & 2, id & 2);
	output.pos = float4(output.tex * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);

	return output;
}

float4 PS_APPLY(PS_INPUT_APPLY input) : SV_Target
{
	float4 screenColor = g_texScreen.Sample(g_samLinearPointClamp, input.tex);
	float ssao = g_texSSAO.Sample(g_samLinearPointClamp, input.tex).r;

	return screenColor * ssao;
}

technique11 Ssao
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS(14)));
	}
}

technique11 SsaoBlur_Horz
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_BLUR()));
		SetPixelShader(CompileShader(ps_5_0, PS_BLUR(true)));
	}
}

technique11 SsaoBlur_Vert
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_BLUR()));
		SetPixelShader(CompileShader(ps_5_0, PS_BLUR(false)));
	}
}

technique11 SsaoApply
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_APPLY()));
		SetPixelShader(CompileShader(ps_5_0, PS_APPLY()));
	}
}

#endif