#ifdef DX12
#include "../../DescriptorTablesDX12.hlsl"
#endif

cbuffer cbContents : register(b0)
{
	//Needed for pixel offset
	float2 InverseResolution;

	//The threshold of pixels that are brighter than that.
	float Threshold = 0.8f;

	//MODIFIED DURING RUNTIME, CHANGING HERE MAKES NO DIFFERENCE;
	float Radius;
	float Strength;

	//How far we stretch the pixels
	float StreakLength = 1;

#ifdef DX12
	uint g_nTexSourceIndex;
	float padding;
#else
	float2 padding;
#endif
};

#ifdef DX11

Texture2D ScreenTexture : register(t0);
SamplerState LinearSampler : register(s0);

#elif DX12

#define ScreenTexture Tex2DTable[g_nTexSourceIndex]
SamplerState LinearSampler : register(s0, space100);

#endif

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

//Just an average of 4 values.
float4 Box4(float4 p0, float4 p1, float4 p2, float4 p3)
{
	return (p0 + p1 + p2 + p3) * 0.25f;
}

//Extracts the pixels we want to blur
float4 ExtractPS(float4 pos : SV_POSITION, float2 texCoord : TEXCOORD0) : SV_Target0
{
	float4 color = ScreenTexture.Sample(LinearSampler, texCoord);

	float avg = (color.r + color.g + color.b) / 3;

	if (avg>Threshold)
	{
		return color * (avg - Threshold) / (1 - Threshold);// * (avg - Threshold);
	}

	return float4(0, 0, 0, 0);
}

//Extracts the pixels we want to blur, but considers luminance instead of average rgb
float4 ExtractLuminancePS(float4 pos : SV_POSITION, float2 texCoord : TEXCOORD0) : SV_Target0
{
	float4 color = ScreenTexture.Sample(LinearSampler, texCoord);

	float luminance = color.r * 0.21f + color.g * 0.72f + color.b * 0.07f;

	if (luminance>Threshold)
	{
		return color * (luminance - Threshold) / (1 - Threshold);// *(luminance - Threshold);
																 //return saturate((color - Threshold) / (1 - Threshold));
	}

	return float4(0, 0, 0, 0);
}

//Downsample to the next mip, blur in the process
float4 DownsamplePS(float4 pos : SV_POSITION, float2 texCoord : TEXCOORD0) : SV_Target0
{
	float2 offset = float2(StreakLength * InverseResolution.x, 1 * InverseResolution.y);

	float4 c0 = ScreenTexture.Sample(LinearSampler, texCoord + float2(-2, -2) * offset);
	float4 c1 = ScreenTexture.Sample(LinearSampler, texCoord + float2(0,-2)*offset);
	float4 c2 = ScreenTexture.Sample(LinearSampler, texCoord + float2(2, -2) * offset);
	float4 c3 = ScreenTexture.Sample(LinearSampler, texCoord + float2(-1, -1) * offset);
	float4 c4 = ScreenTexture.Sample(LinearSampler, texCoord + float2(1, -1) * offset);
	float4 c5 = ScreenTexture.Sample(LinearSampler, texCoord + float2(-2, 0) * offset);
	float4 c6 = ScreenTexture.Sample(LinearSampler, texCoord);
	float4 c7 = ScreenTexture.Sample(LinearSampler, texCoord + float2(2, 0) * offset);
	float4 c8 = ScreenTexture.Sample(LinearSampler, texCoord + float2(-1, 1) * offset);
	float4 c9 = ScreenTexture.Sample(LinearSampler, texCoord + float2(1, 1) * offset);
	float4 c10 = ScreenTexture.Sample(LinearSampler, texCoord + float2(-2, 2) * offset);
	float4 c11 = ScreenTexture.Sample(LinearSampler, texCoord + float2(0, 2) * offset);
	float4 c12 = ScreenTexture.Sample(LinearSampler, texCoord + float2(2, 2) * offset);

	return Box4(c0, c1, c5, c6) * 0.125f +
		Box4(c1, c2, c6, c7) * 0.125f +
		Box4(c5, c6, c10, c11) * 0.125f +
		Box4(c6, c7, c11, c12) * 0.125f +
		Box4(c3, c4, c8, c9) * 0.5f;
}

//Upsample to the former MIP, blur in the process
float4 UpsamplePS(float4 pos : SV_POSITION, float2 texCoord : TEXCOORD0) : SV_Target0
{
	float2 offset = float2(StreakLength * InverseResolution.x, 1 * InverseResolution.y) * Radius;

	float4 c0 = ScreenTexture.Sample(LinearSampler, texCoord + float2(-1, -1) * offset);
	float4 c1 = ScreenTexture.Sample(LinearSampler, texCoord + float2(0, -1) * offset);
	float4 c2 = ScreenTexture.Sample(LinearSampler, texCoord + float2(1, -1) * offset);
	float4 c3 = ScreenTexture.Sample(LinearSampler, texCoord + float2(-1, 0) * offset);
	float4 c4 = ScreenTexture.Sample(LinearSampler, texCoord);
	float4 c5 = ScreenTexture.Sample(LinearSampler, texCoord + float2(1, 0) * offset);
	float4 c6 = ScreenTexture.Sample(LinearSampler, texCoord + float2(-1,1) * offset);
	float4 c7 = ScreenTexture.Sample(LinearSampler, texCoord + float2(0, 1) * offset);
	float4 c8 = ScreenTexture.Sample(LinearSampler, texCoord + float2(1, 1) * offset);

	//Tentfilter  0.0625f    
	return 0.0625f * (c0 + 2 * c1 + c2 + 2 * c3 + 4 * c4 + 2 * c5 + c6 + 2 * c7 + c8) * Strength + float4(0, 0,0,0); //+ 0.5f * ScreenTexture.Sample(c_texture, texCoord);
}

//Upsample to the former MIP, blur in the process, change offset depending on luminance
float4 UpsampleLuminancePS(float4 pos : SV_POSITION, float2 texCoord : TEXCOORD0) : SV_Target0
{
	float4 c4 = ScreenTexture.Sample(LinearSampler, texCoord);  //middle one

	/*float luminance = c4.r * 0.21f + c4.g * 0.72f + c4.b * 0.07f;
	luminance = max(luminance, 0.4f);
	*/
	float2 offset = float2(StreakLength * InverseResolution.x, 1 * InverseResolution.y) * Radius; /// luminance;

	float4 c0 = ScreenTexture.Sample(LinearSampler, texCoord + float2(-1, -1) * offset);
	float4 c1 = ScreenTexture.Sample(LinearSampler, texCoord + float2(0, -1) * offset);
	float4 c2 = ScreenTexture.Sample(LinearSampler, texCoord + float2(1, -1) * offset);
	float4 c3 = ScreenTexture.Sample(LinearSampler, texCoord + float2(-1, 0) * offset);
	float4 c5 = ScreenTexture.Sample(LinearSampler, texCoord + float2(1, 0) * offset);
	float4 c6 = ScreenTexture.Sample(LinearSampler, texCoord + float2(-1, 1) * offset);
	float4 c7 = ScreenTexture.Sample(LinearSampler, texCoord + float2(0, 1) * offset);
	float4 c8 = ScreenTexture.Sample(LinearSampler, texCoord + float2(1, 1) * offset);

	return 0.0625f * (c0 + 2 * c1 + c2 + 2 * c3 + 4 * c4 + 2 * c5 + c6 + 2 * c7 + c8) * Strength + float4(0, 0, 0, 0); //+ 0.5f * ScreenTexture.Sample(c_texture, texCoord);
}

float4 ApplyPS(float4 pos : SV_POSITION, float2 texCoord : TEXCOORD0) : SV_Target0
{
	return ScreenTexture.Sample(LinearSampler, texCoord);
}