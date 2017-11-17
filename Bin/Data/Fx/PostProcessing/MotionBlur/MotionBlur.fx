#ifndef _MOTION_BLUR_
#define _MOTION_BLUR_

//-----------------------------------------------------------------------------
// File: PixelMotionBlur.fx
//
// Desc: Effect file for image based motion blur. The HLSL shaders are used to
//       calculate the velocity of each pixel based on the last frame's matrix 
//       transforms.  This per-pixel velocity is then used in a blur filter to 
//       create the motion blur effect.
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "../../Converter.fx"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

cbuffer cbMotionblur
{
	float g_fPixelBlurConst = 1.f;
};

Texture2D g_texRenderTarget;
Texture2D g_texNormalVelocity;
Texture2D g_texPrevVelocity;

SamplerState g_samplerPointClamp;

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
};

PS_INPUT VS(uint id : SV_VertexID)
{
	PS_INPUT output;
	output.tex = float2((id << 1) & 2, id & 2);
	output.pos = float4(output.tex * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);

	return output;
}

//-----------------------------------------------------------------------------
// Name: PS 
// Type: Pixel shader                                      
// Desc: Uses the pixel's velocity to sum up and average pixel in that direction
//       to create a blur effect based on the velocity in a fullscreen
//       post process pass.
//-----------------------------------------------------------------------------

struct PS_OUTPUT
{
	float4 color : SV_Target0;
	float velocity : SV_Target1;
};

PS_OUTPUT PS(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;

    // Get this pixel's current velocity and this pixel's last frame velocity
    // The velocity is stored in .r & .g channels
	float2 curFrameVelocity = FloatToHalf2(g_texNormalVelocity.Sample(g_samplerPointClamp, input.tex).z);
	float2 prevFrameVelocity = FloatToHalf2(g_texPrevVelocity.Sample(g_samplerPointClamp, input.tex).x);

    // If this pixel's current velocity is zero, then use its last frame velocity
    // otherwise use its current velocity.  We don't want to add them because then 
    // you would get double the current velocity in the center.  
    // If you just use the current velocity, then it won't blur where the object 
    // was last frame because the current velocity at that point would be 0.  Instead 
    // you could do a filter to find if any neighbors are non-zero, but that requires a lot 
    // of texture lookups which are limited and also may not work if the object moved too 
    // far, but could be done multi-pass.
    float curVelocitySqMag = curFrameVelocity.x * curFrameVelocity.x + curFrameVelocity.y * curFrameVelocity.y;
    float lastVelocitySqMag = prevFrameVelocity.x * prevFrameVelocity.x + prevFrameVelocity.y * prevFrameVelocity.y;
	
	float2 pixelVelocity;
    if( lastVelocitySqMag > curVelocitySqMag )
    {
        pixelVelocity.x = prevFrameVelocity.x * g_fPixelBlurConst;
        pixelVelocity.y = -prevFrameVelocity.y * g_fPixelBlurConst;
    }
    else
    {
        pixelVelocity.x = curFrameVelocity.x * g_fPixelBlurConst;
        pixelVelocity.y = -curFrameVelocity.y * g_fPixelBlurConst;
    }

    // For each sample, sum up each sample's color in "Blurred" and then divide
    // to average the color after all the samples are added.
    float3 Blurred = 0.f;

	const int nSamples = 8;
	[unroll]
	for (int i = 0; i < nSamples; ++i)
	{
		// Sample texture in a new spot based on pixelVelocity vector 
		// and average it with the other samples        
		float2 lookup = pixelVelocity * i / nSamples + input.tex;

		// Lookup the color at this new spot
		float4 Current = g_texRenderTarget.Sample(g_samplerPointClamp, lookup);

		// Add it with the other samples
		Blurred += Current.rgb;
	}

    // Return the average color of all the samples
	output.color = float4(Blurred / nSamples, 1.0f);
	output.velocity = Half2ToFloat(curFrameVelocity.x, curFrameVelocity.y);

	return output;
}

//-----------------------------------------------------------------------------
// Name: MotionBlur
// Type: Technique                                     
// Desc: Renders a full screen quad and uses velocity information stored in 
//       the textures to blur image.
//-----------------------------------------------------------------------------
technique11 MotionBlur
{
    pass Pass_0
    {
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}

#endif