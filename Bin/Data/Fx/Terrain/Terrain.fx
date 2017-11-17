#ifndef _TERRAIN_
#define _TERRAIN_

#include "../Converter.fx"

Texture2D g_HeightfieldTexture;
Texture2D g_LayerdefTexture;
Texture2D g_RockBumpTexture;
Texture2D g_RockMicroBumpTexture;
Texture2D g_RockDiffuseTexture;
Texture2D g_SandBumpTexture;
Texture2D g_SandMicroBumpTexture;
Texture2D g_SandDiffuseTexture;
Texture2D g_GrassDiffuseTexture;
Texture2D g_SlopeDiffuseTexture;

Texture2D g_DepthTexture;

SamplerState SamplerLinearWrap
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState SamplerAnisotropicWrap
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
	MaxAnisotropy = 16;
};

SamplerComparisonState SamplerDepthAnisotropic
{
	Filter = COMPARISON_ANISOTROPIC;
	AddressU = Border;
	AddressV = Border;
	ComparisonFunc = LESS;
	BorderColor = float4(1, 1, 1, 1);
	MaxAnisotropy = 16;
};

RasterizerState CullBackMS
{
	CullMode = Back;
	FrontCounterClockwise = TRUE;
	MultisampleEnable = TRUE;
};

RasterizerState WireframeMS
{
	CullMode = NONE;
	FillMode = WIREFRAME;
	MultisampleEnable = TRUE;
};

DepthStencilState DepthNormal
{
	DepthFunc = LESS_EQUAL;
};

BlendState NoBlending
{
	BlendEnable[0] = FALSE;
};

cbuffer cbContents
{
	float g_RenderCaustics;
	float g_UseDynamicLOD;
	float g_FrustumCullInHS;
	float g_DynamicTessFactor;
	float g_StaticTessFactor;
	float g_TerrainBeingRendered;
	float g_HalfSpaceCullSign;
	float g_HalfSpaceCullPosition;
	float g_SkipCausticsCalculation;

	float4x4 g_ModelViewProjectionMatrix;

	float4x4 g_LightModelViewProjectionMatrix;
	float4x4 g_matView;
	float3 g_CameraPosition;
	float3 g_CameraDirection;

	float3 g_LightPosition;

	float g_HeightFieldSize = 512;
};

static const float3 AtmosphereBrightColor = { 1.0,1.1,1.4 };
static const float3 AtmosphereDarkColor = { 0.6,0.6,0.7 };
static const float FogDensity = 1.0f / 700.0f;
static const float SandBumpHeightScale = 0.5;
static const float2 DiffuseTexcoordScale = { 130.0,130.0 };
static const float2 RockBumpTexcoordScale = { 10.0,10.0 };
static const float RockBumpHeightScale = 3.0;
static const float2 SandBumpTexcoordScale = { 3.5,3.5 };

struct DUMMY
{
	float Dummmy : DUMMY;
};

struct HSIn_Heightfield
{
	float2 origin   : ORIGIN;
	float2 size     : SIZE;
};

struct PatchData
{
	float Edges[4]  : SV_TessFactor;
	float Inside[2]	: SV_InsideTessFactor;

	float2 origin   : ORIGIN;
	float2 size     : SIZE;
};

struct PSIn_Diffuse
{
	float4 position     : SV_Position;
	centroid float2 texcoord     : TEXCOORD0;
	centroid float3 normal       : NORMAL;
	centroid float3 positionWS   : TEXCOORD1;
	centroid float3 positionVS   : TEXCOORD2;
	centroid float4 layerdef		: TEXCOORD3;
	centroid float4 depthmap_scaler: TEXCOORD4;
};

// calculating tessellation factor. It is either constant or hyperbolic depending on g_UseDynamicLOD switch
float CalculateTessellationFactor(float distance)
{
	return lerp(g_StaticTessFactor, g_DynamicTessFactor*(1 / (0.015*distance)), g_UseDynamicLOD);
}

// to avoid vertex swimming while tessellation varies, one can use mipmapping for displacement maps
// it's not always the best choice, but it effificiently suppresses high frequencies at zero cost
float CalculateMIPLevelForDisplacementTextures(float distance)
{
	return log2(128 / CalculateTessellationFactor(distance));
}

// primitive simulation of non-uniform atmospheric fog
float3 CalculateFogColor(float3 pixel_to_light_vector, float3 pixel_to_eye_vector)
{
	return lerp(AtmosphereDarkColor, AtmosphereBrightColor, 0.5*dot(pixel_to_light_vector, -pixel_to_eye_vector) + 0.5);
}

// calculating water refraction caustics intensity
//float CalculateWaterCausticIntensity(float3 worldpos)
//{
//	float distance_to_camera = length(g_CameraPosition - worldpos);
//
//	float2 refraction_disturbance;
//	float3 n;
//	float m = 0.2;
//	float cc = 0;
//	float k = 0.15;
//	float water_depth = 0.5 - worldpos.y;
//
//	float3 pixel_to_light_vector = normalize(g_LightPosition - worldpos);
//
//	worldpos.xz -= worldpos.y*pixel_to_light_vector.xz;
//	float3 pixel_to_water_surface_vector = pixel_to_light_vector*water_depth;
//	float3 refracted_pixel_to_light_vector;
//
//	// tracing approximately refracted rays back to light
//	for (float i = -3; i <= 3; i += 1)
//	{
//		for (float j = -3; j <= 3; j += 1)
//		{
//			n = 2.0f*g_WaterNormalMapTexture.SampleLevel(SamplerLinearWrap, (worldpos.xz - g_CameraPosition.xz - float2(200.0, 200.0) + float2(i*k, j*k)*m*water_depth) / 400.0, 0).rgb - float3(1.0f, 1.0f, 1.0f);
//			refracted_pixel_to_light_vector = m*(pixel_to_water_surface_vector + float3(i*k, 0, j*k)) - 0.5*float3(n.x, 0, n.z);
//			cc += 0.05*max(0, pow(max(0, dot(normalize(refracted_pixel_to_light_vector), normalize(pixel_to_light_vector))), 500.0f));
//		}
//	}
//	return cc;
//}

HSIn_Heightfield PassThroughVS(float4 PatchParams : POSITION)
{
	HSIn_Heightfield output;
	output.origin = PatchParams.xy;
	output.size = PatchParams.zw;
	return output;
}

PatchData PatchConstantHS(InputPatch<HSIn_Heightfield, 1> inputPatch)
{
	PatchData output;

	float distance_to_camera;
	float tesselation_factor;
	float inside_tessellation_factor = 0;
	float in_frustum = 0;

	output.origin = inputPatch[0].origin;
	output.size = inputPatch[0].size;

	float2 texcoord0to1 = (inputPatch[0].origin + inputPatch[0].size / 2.0) / g_HeightFieldSize;
	texcoord0to1.y = 1 - texcoord0to1.y;

	// conservative frustum culling
	float3 patch_center = float3(inputPatch[0].origin.x + inputPatch[0].size.x*0.5, g_TerrainBeingRendered*g_HeightfieldTexture.SampleLevel(SamplerLinearWrap, texcoord0to1, 0).w, inputPatch[0].origin.y + inputPatch[0].size.y*0.5);
	float3 camera_to_patch_vector = patch_center - g_CameraPosition;
	float3 patch_to_camera_direction_vector = g_CameraDirection*dot(camera_to_patch_vector, g_CameraDirection) - camera_to_patch_vector;
	float3 patch_center_realigned = patch_center + normalize(patch_to_camera_direction_vector)*min(2 * inputPatch[0].size.x, length(patch_to_camera_direction_vector));
	float4 patch_screenspace_center = mul(float4(patch_center_realigned, 1.0), g_ModelViewProjectionMatrix);

	if (((patch_screenspace_center.x / patch_screenspace_center.w>-1.0) && (patch_screenspace_center.x / patch_screenspace_center.w<1.0)
		&& (patch_screenspace_center.y / patch_screenspace_center.w>-1.0) && (patch_screenspace_center.y / patch_screenspace_center.w<1.0)
		&& (patch_screenspace_center.w>0)) || (length(patch_center - g_CameraPosition)<2 * inputPatch[0].size.x))
	{
		in_frustum = 1;
	}

	if ((in_frustum) || (g_FrustumCullInHS == 0))
	{
		distance_to_camera = length(g_CameraPosition.xz - inputPatch[0].origin - float2(0, inputPatch[0].size.y*0.5));
		tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[0] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;

		distance_to_camera = length(g_CameraPosition.xz - inputPatch[0].origin - float2(inputPatch[0].size.x*0.5, 0));
		tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[1] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;

		distance_to_camera = length(g_CameraPosition.xz - inputPatch[0].origin - float2(inputPatch[0].size.x, inputPatch[0].size.y*0.5));
		tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[2] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;

		distance_to_camera = length(g_CameraPosition.xz - inputPatch[0].origin - float2(inputPatch[0].size.x*0.5, inputPatch[0].size.y));
		tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[3] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;
		output.Inside[0] = output.Inside[1] = inside_tessellation_factor*0.25;
	}
	else
	{
		output.Edges[0] = -1;
		output.Edges[1] = -1;
		output.Edges[2] = -1;
		output.Edges[3] = -1;
		output.Inside[0] = -1;
		output.Inside[1] = -1;
	}

	return output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(1)]
[patchconstantfunc("PatchConstantHS")]
DUMMY PatchHS(InputPatch<HSIn_Heightfield, 1> inputPatch)
{
	return (DUMMY)0;
}

[domain("quad")]
PSIn_Diffuse HeightFieldPatchDS(PatchData input,
	float2 uv : SV_DomainLocation,
	OutputPatch<DUMMY, 1> inputPatch)
{
	PSIn_Diffuse output;
	float3 vertexPosition;
	float4 base_texvalue;
	float2 texcoord0to1 = (input.origin + uv * input.size) / g_HeightFieldSize;
	float3 base_normal;
	float3 detail_normal;
	float3 detail_normal_rotated;
	float4 detail_texvalue;
	float detail_height;
	float3x3 normal_rotation_matrix;
	float4 layerdef;
	float distance_to_camera;
	float detailmap_miplevel;
	texcoord0to1.y = 1 - texcoord0to1.y;

	// fetching base heightmap,normal and moving vertices along y axis
	base_texvalue = g_HeightfieldTexture.SampleLevel(SamplerLinearWrap, texcoord0to1, 0);
	base_normal = base_texvalue.xyz;
	base_normal.z = -base_normal.z;
	vertexPosition.xz = input.origin + uv * input.size;
	vertexPosition.y = base_texvalue.w;

	// calculating MIP level for detail texture fetches
	distance_to_camera = length(g_CameraPosition - vertexPosition);
	detailmap_miplevel = CalculateMIPLevelForDisplacementTextures(distance_to_camera);//log2(1+distance_to_camera*3000/(g_HeightFieldSize*g_TessFactor));

																					  // fetching layer definition texture
	layerdef = g_LayerdefTexture.SampleLevel(SamplerLinearWrap, texcoord0to1, 0);

	// default detail texture
	detail_texvalue = g_SandBumpTexture.SampleLevel(SamplerLinearWrap, texcoord0to1*SandBumpTexcoordScale, detailmap_miplevel).rbga;
	detail_normal = normalize(2 * detail_texvalue.xyz - float3(1, 0, 1));
	detail_height = (detail_texvalue.w - 0.5)*SandBumpHeightScale;

	// rock detail texture
	detail_texvalue = g_RockBumpTexture.SampleLevel(SamplerLinearWrap, texcoord0to1*RockBumpTexcoordScale, detailmap_miplevel).rbga;
	detail_normal = lerp(detail_normal, normalize(2 * detail_texvalue.xyz - float3(1, 1.4, 1)), layerdef.w);
	detail_height = lerp(detail_height, (detail_texvalue.w - 0.5)*RockBumpHeightScale, layerdef.w);

	// moving vertices by detail height along base normal
	vertexPosition += base_normal*detail_height;

	//calculating base normal rotation matrix
	normal_rotation_matrix[1] = base_normal;
	normal_rotation_matrix[2] = normalize(cross(float3(-1.0, 0.0, 0.0), normal_rotation_matrix[1]));
	normal_rotation_matrix[0] = normalize(cross(normal_rotation_matrix[2], normal_rotation_matrix[1]));

	//applying base rotation matrix to detail normal
	detail_normal_rotated = mul(detail_normal, normal_rotation_matrix);

	//adding refraction caustics
	float cc = 0;

	//if ((g_SkipCausticsCalculation == 0) && (g_RenderCaustics>0)) // doing it only for main
	//{
	//	cc = CalculateWaterCausticIntensity(vertexPosition.xyz);
	//}
	//
	//// fading caustics out at distance
	//cc *= (200.0 / (200.0 + distance_to_camera));
	//
	//// fading caustics out as we're getting closer to water surface
	//cc *= min(1, max(0, -g_WaterHeightBumpScale - vertexPosition.y));

	// writing output params
	output.position = mul(float4(vertexPosition, 1.0), g_ModelViewProjectionMatrix);
	output.texcoord = texcoord0to1*DiffuseTexcoordScale;
	output.normal = detail_normal_rotated;
	output.positionWS = vertexPosition;
	output.positionVS = mul(float4(vertexPosition, 1.f), g_matView).xyz;
	output.layerdef = layerdef;
	output.depthmap_scaler = float4(1.0, 1.0, detail_height, cc);

	return output;
}

struct PS_OUTPUT
{
	float4 normals : SV_Target0;
	float4 colors : SV_Target1;
	float4 disneyBRDF : SV_Target2;
};

PS_OUTPUT HeightFieldPatchPS(PSIn_Diffuse input)
{
	float4 color = 0.f;
	float3 pixel_to_light_vector = normalize(g_LightPosition - input.positionWS);
	float3 pixel_to_eye_vector = normalize(g_CameraPosition - input.positionWS);
	float3 microbump_normal = 0.f;

	float3x3 normal_rotation_matrix = 0.f;

	// culling halfspace if needed
	//clip(g_HalfSpaceCullSign*(input.positionWS.y - g_HalfSpaceCullPosition));

	// fetching default microbump normal
	microbump_normal = normalize(2 * g_SandMicroBumpTexture.Sample(SamplerAnisotropicWrap, input.texcoord).rbg - float3 (1.0, 1.0, 1.0));
	microbump_normal = normalize(lerp(microbump_normal, 2 * g_RockMicroBumpTexture.Sample(SamplerAnisotropicWrap, input.texcoord).rbg - float3 (1.0, 1.0, 1.0), input.layerdef.w));

	//calculating base normal rotation matrix
	normal_rotation_matrix[1] = input.normal;
	normal_rotation_matrix[2] = normalize(cross(float3(-1.0, 0.0, 0.0), normal_rotation_matrix[1]));
	normal_rotation_matrix[0] = normalize(cross(normal_rotation_matrix[2], normal_rotation_matrix[1]));
	microbump_normal = mul(microbump_normal, normal_rotation_matrix);

	// getting diffuse color
	color = g_SlopeDiffuseTexture.Sample(SamplerAnisotropicWrap, input.texcoord);
	color = lerp(color, g_SandDiffuseTexture.Sample(SamplerAnisotropicWrap, input.texcoord), input.layerdef.g*input.layerdef.g);
	color = lerp(color, g_RockDiffuseTexture.Sample(SamplerAnisotropicWrap, input.texcoord), input.layerdef.w*input.layerdef.w);
	color = lerp(color, g_GrassDiffuseTexture.Sample(SamplerAnisotropicWrap, input.texcoord), input.layerdef.b);

	// adding per-vertex lighting defined by displacement of vertex 
	//color *= 0.5 + 0.5*min(1.0, max(0.0, input.depthmap_scaler.b / 3.0f + 0.5f));

	// calculating pixel position in light view space
	float4 positionLS = mul(float4(input.positionWS, 1), g_LightModelViewProjectionMatrix);
	positionLS.xyz /= positionLS.w;
	positionLS.x = (positionLS.x + 1)*0.5;
	positionLS.y = (1 - positionLS.y)*0.5;


	// fetching shadowmap and shading
	float dsf = 0.75f / 4096.0f;
	float shadow_factor = 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy, positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(dsf, dsf), positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(-dsf, dsf), positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(dsf, -dsf), positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(-dsf, -dsf), positionLS.z* 0.995f).r;
	//color.rgb *= max(0, dot(pixel_to_light_vector, microbump_normal))*shadow_factor + 0.2;


	// adding light from the sky
	//color.rgb += (0.0 + 0.2*max(0, (dot(float3(0, 1, 0), microbump_normal))))*float3(0.2, 0.2, 0.3);

	// making all a bit brighter, simultaneously pretending the wet surface is darker than normal;
	color.rgb *= 0.5 + 0.8*max(0, min(1, input.positionWS.y*0.5 + 0.5));



	// applying refraction caustics
	//color.rgb *= (1.0 + max(0, 0.4 + 0.6*dot(pixel_to_light_vector, microbump_normal))*input.depthmap_scaler.a*(0.4 + 0.6*shadow_factor));

	// applying fog
	//color.rgb = lerp(CalculateFogColor(pixel_to_light_vector, pixel_to_eye_vector).rgb, color.rgb, min(1, exp(-length(g_CameraPosition - input.positionWS)*FogDensity)));
	//color.a = length(g_CameraPosition - input.positionWS);

	color.rgb = saturate(color.rgb);

	PS_OUTPUT output = (PS_OUTPUT)0.f;
	output.normals.xy = CompressNormal(microbump_normal);
	output.normals.zw = CompressNormal(CalcTangent(microbump_normal));

	float3 RM = float3(1.f, 0.f, 0.f);
	float3 specular = lerp(0.03f, color.rgb, RM.y);

	output.colors.x = Pack3PNForFP32(color.rgb);
	output.colors.y = Pack3PNForFP32(specular);

	output.disneyBRDF.x = Pack3PNForFP32(RM);

	return output;
}

float4 ColorPS(uniform float4 color) : SV_Target
{
	return color;
}

technique11 RenderHeightfield
{
	pass Solid
	{
		SetVertexShader(CompileShader(vs_5_0, PassThroughVS()));
		SetHullShader(CompileShader(hs_5_0, PatchHS()));
		SetDomainShader(CompileShader(ds_5_0, HeightFieldPatchDS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, HeightFieldPatchPS()));
	}

	pass DepthOnly
	{
		SetRasterizerState(CullBackMS);
		SetDepthStencilState(DepthNormal, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);

		SetVertexShader(CompileShader(vs_5_0, PassThroughVS()));
		SetHullShader(CompileShader(hs_5_0, PatchHS()));
		SetDomainShader(CompileShader(ds_5_0, HeightFieldPatchDS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, ColorPS(float4(1.0f, 1.0f, 1.0f, 1.0f))));
	}
}

#endif
