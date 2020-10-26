#ifdef DX12
#include "../../DescriptorTablesDX12.hlsl"
#endif

#include "../../Converter.hlsl"

cbuffer cbMotionBlur : register(b0)
{
	float g_blurAmount;
	float3 padding;

	float2 g_vSourceDimensions;
	float2 g_vDestinationDimensions;

#ifdef DX12
	uint g_texColorIndex;
	uint g_texDepthIndex;

	uint g_texVelocityIndex;
	uint g_texPrevVelocityIndex;
#endif
};

cbuffer cbDepthMotionBlur : register(b1)
{
	float4x4 g_matInvView;
	float4x4 g_matInvProj;
	float4x4 g_matLastViewProj;
};

#ifdef DX11

Texture2D g_texColor : register(t0);
Texture2D g_texDepth : register(t1);

Texture2D<float2> g_texVelocity : register(t1);
Texture2D<float2> g_texPrevVelocity : register(t2);

SamplerState g_samplerPoint : register(s0);

#elif DX12

#define g_texColor Tex2DTable[g_texColorIndex]
#define g_texDepth Tex2DTable[g_texDepthIndex]
#define g_texVelocity Tex2DTable[g_texVelocityIndex]
#define g_texPrevVelocity Tex2DTable[g_texPrevVelocityIndex]
SamplerState g_samplerPoint : register(s0, space100);

#endif

float4 MotionBlur(float2 vTexCoord, float2 vPixelVelocity, int iNumSamples)
{
	// Clamp to a max velocity.  The max we can go without artifacts os
	// is 1.4f * iNumSamples...but we can fudge things a little.
	float2 maxVelocity = (2.0f * iNumSamples) / g_vSourceDimensions;
	vPixelVelocity = clamp(vPixelVelocity * g_blurAmount, -maxVelocity, maxVelocity);

	float2 vFinalSamplePos = vTexCoord + vPixelVelocity;

	// For each sample, sum up each sample's color in "vSum" and then divide
	// to average the color after all the samples are added.
	float4 vSum = 0;
	for (int i = 0; i < iNumSamples; i++)
	{
		// Sample texture in a new spot based on vPixelVelocity vector 
		// and average it with the other samples    
		float2 vSampleCoord = vTexCoord + (vPixelVelocity * (i / (float)iNumSamples));

		// Lookup the color at this new spot
		float4 vSample = g_texColor.Sample(g_samplerPoint, vSampleCoord);

		// Add it with the other samples
		vSum += vSample;
	}

	// Return the average color of all the samples
	return vSum / (float)iNumSamples;
}

float4 DepthMotionBlur(in float2 inTex, uniform int iNumSamples)
{
	// Reconstruct view-space position from the depth buffer
	float fPixelDepthVS = g_texDepth.Sample(g_samplerPoint, inTex).x;

	float4 vPixelPositionWS = float4(CalcWorldSpacePosFromDepth(fPixelDepthVS, inTex, g_matInvView, g_matInvProj), 1.f);
	float4 vLastPixelPosCS = mul(vPixelPositionWS, g_matLastViewProj);

	// Find the corresponding texture coordinate
	float2 vLastTexCoord = vLastPixelPosCS.xy / vLastPixelPosCS.w;
	vLastTexCoord = (vLastTexCoord / 2.0f) + 0.5f;
	vLastTexCoord.y = 1.0f - vLastTexCoord.y;
	vLastTexCoord += 0.5f / g_vDestinationDimensions;

	float2 vPixelVelocity = inTex - vLastTexCoord;

	return MotionBlur(inTex, vPixelVelocity, iNumSamples);
}

float4 VelocityMotionBlur(in float2 inTex, uniform int iNumSamples)
{
	// Sample velocity from our velocity buffer
	float2 vCurFramePixelVelocity = g_texVelocity.Sample(g_samplerPoint, inTex).xy;

	return MotionBlur(inTex, vCurFramePixelVelocity, iNumSamples);
}

float4 DualVelocityMotionBlur(in float2 inTex, uniform int iNumSamples)
{
	// Sample velocity from our velocity buffers
	float2 vCurFramePixelVelocity = g_texVelocity.Sample(g_samplerPoint, inTex).xy;
	float2 vLastFramePixelVelocity = g_texPrevVelocity.Sample(g_samplerPoint, inTex).xy;

	// We'll compare the magnitude of the velocity from the current frame and from
	// the previous frame, and then use whichever is larger
	float2 vPixelVelocity = 0;
	float fCurVelocitySqMag = vCurFramePixelVelocity.x * vCurFramePixelVelocity.x +
						   vCurFramePixelVelocity.y * vCurFramePixelVelocity.y;
	float fLastVelocitySqMag = vLastFramePixelVelocity.x * vLastFramePixelVelocity.x +
							  vLastFramePixelVelocity.y * vLastFramePixelVelocity.y;

	if (fLastVelocitySqMag > fCurVelocitySqMag)
		vPixelVelocity = vLastFramePixelVelocity;
	else
		vPixelVelocity = vCurFramePixelVelocity;

	return MotionBlur(inTex, vPixelVelocity, iNumSamples);
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

float4 DepthBuffer4SamplesPS(PS_INPUT input) : SV_Target0
{
	return DepthMotionBlur(input.tex, 4);
}

float4 DepthBuffer8SamplesPS(PS_INPUT input) : SV_Target0
{
	return DepthMotionBlur(input.tex, 8);
}

float4 DepthBuffer12SamplesPS(PS_INPUT input) : SV_Target0
{
	return DepthMotionBlur(input.tex, 12);
}

float4 VelocityBuffer4SamplesPS(PS_INPUT input) : SV_Target0
{
	return VelocityMotionBlur(input.tex, 4);
}

float4 VelocityBuffer8SamplesPS(PS_INPUT input) : SV_Target0
{
	return VelocityMotionBlur(input.tex, 8);
}

float4 VelocityBuffer12SamplesPS(PS_INPUT input) : SV_Target0
{
	return VelocityMotionBlur(input.tex, 12);
}

float4 DualVelocityBuffer4SamplesPS(PS_INPUT input) : SV_Target0
{
	return DualVelocityMotionBlur(input.tex, 4);
}

float4 DualVelocityBuffer8SamplesPS(PS_INPUT input) : SV_Target0
{
	return DualVelocityMotionBlur(input.tex, 8);
}

float4 DualVelocityBuffer12SamplesPS(PS_INPUT input) : SV_Target0
{
	return DualVelocityMotionBlur(input.tex, 12);
}