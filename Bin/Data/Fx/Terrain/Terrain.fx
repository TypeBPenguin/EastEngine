#ifndef _TERRAIN_
#define _TERRAIN_

#include "../Converter.fx"

Texture2D g_texHeightField;
Texture2D g_texColor;

Texture2D g_texDetail;
Texture2D g_texDetailNormal;

SamplerState SamplerLinearWrap
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState SamplerLinearBorder
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Border;
	AddressV = Border;
};

SamplerState SamplerAnisotropicBorder
{
	Filter = ANISOTROPIC;
	AddressU = Border;
	AddressV = Border;
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
	float g_UseDynamicLOD;
	float g_FrustumCullInHS;
	float g_DynamicTessFactor;
	float g_StaticTessFactor;

	float4x4 g_ModelViewProjectionMatrix;

	float3 g_CameraPosition;
	float3 g_CameraDirection;

	float2 g_f2PatchSize;
	float2 g_f2HeightFieldSize;
};

struct DUMMY
{
	float Dummmy	: DUMMY;
};

struct HSIn_Heightfield
{
	float2 origin	: ORIGIN;
};

struct PatchData
{
	float Edges[4]	: SV_TessFactor;
	float Inside[2]	: SV_InsideTessFactor;

	float2 origin	: ORIGIN;
};

struct PSIn_Diffuse
{
	float4 position					: SV_Position;
	centroid float2 texcoord		: TEXCOORD0;
	centroid float3 normal			: NORMAL;
	centroid float3 tangent			: TANGENT;
	centroid float2 texcoord_detail	: TEXCOORD2;
};

// calculating tessellation factor. It is either constant or hyperbolic depending on g_UseDynamicLOD switch
float CalculateTessellationFactor(float distance)
{
	return lerp(g_StaticTessFactor, g_DynamicTessFactor * (1.f / (0.015f * distance)), g_UseDynamicLOD);
}

// to avoid vertex swimming while tessellation varies, one can use mipmapping for displacement maps
// it's not always the best choice, but it effificiently suppresses high frequencies at zero cost
float CalculateMIPLevelForDisplacementTextures(float distance)
{
	return log2(128 / CalculateTessellationFactor(distance));
}

HSIn_Heightfield PassThroughVS(float4 PatchParams : POSITION)
{
	HSIn_Heightfield output;
	output.origin = PatchParams.xy;

	return output;
}

PatchData PatchConstantHS(InputPatch<HSIn_Heightfield, 1> inputPatch)
{
	PatchData output;

	output.origin = inputPatch[0].origin;

	float2 texcoord0to1 = (inputPatch[0].origin + g_f2PatchSize / 2.0) / g_f2HeightFieldSize;
	texcoord0to1.y = 1 - texcoord0to1.y;

	// conservative frustum culling
	float3 patch_center = 0.f;
	patch_center.x = inputPatch[0].origin.x + g_f2PatchSize.x * 0.5f;
	patch_center.y = g_texHeightField.SampleLevel(SamplerLinearBorder, texcoord0to1, 0).w;
	patch_center.z = inputPatch[0].origin.y + g_f2PatchSize.y * 0.5f;

	float3 camera_to_patch_vector = patch_center - g_CameraPosition;
	float3 patch_to_camera_direction_vector = g_CameraDirection * dot(camera_to_patch_vector, g_CameraDirection) - camera_to_patch_vector;
	float3 patch_center_realigned = patch_center + normalize(patch_to_camera_direction_vector) * min(2 * g_f2PatchSize.x, length(patch_to_camera_direction_vector));
	float4 patch_screenspace_center = mul(float4(patch_center_realigned, 1.0), g_ModelViewProjectionMatrix);

	float in_frustum = 0;
	if (((patch_screenspace_center.x / patch_screenspace_center.w>-1.0) && (patch_screenspace_center.x / patch_screenspace_center.w<1.0)
		&& (patch_screenspace_center.y / patch_screenspace_center.w>-1.0) && (patch_screenspace_center.y / patch_screenspace_center.w<1.0)
		&& (patch_screenspace_center.w>0)) || (length(patch_center - g_CameraPosition)<2 * g_f2PatchSize.x))
	{
		in_frustum = 1;
	}

	if ((in_frustum) || (g_FrustumCullInHS == 0))
	{
		float inside_tessellation_factor = 0.f;

		float3 pos;
		pos.x = inputPatch[0].origin.x;
		pos.y = patch_center.y;
		pos.z = inputPatch[0].origin.y + g_f2PatchSize.y * 0.5f;

		float distance_to_camera = length(g_CameraPosition - pos);
		float tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[0] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;

		pos.x = inputPatch[0].origin.x + g_f2PatchSize.x * 0.5f;
		pos.z = inputPatch[0].origin.y;

		distance_to_camera = length(g_CameraPosition - pos);
		tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[1] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;

		pos.x = inputPatch[0].origin.x + g_f2PatchSize.x;
		pos.z = inputPatch[0].origin.y + g_f2PatchSize.y * 0.5f;

		distance_to_camera = length(g_CameraPosition - pos);
		tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[2] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;

		pos.x = inputPatch[0].origin.x + g_f2PatchSize.x * 0.5f;
		pos.z = inputPatch[0].origin.y + g_f2PatchSize.y;

		distance_to_camera = length(g_CameraPosition - pos);
		tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[3] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;
		output.Inside[0] = output.Inside[1] = inside_tessellation_factor * 0.25;
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
	float2 texcoord0to1 = (input.origin + uv * g_f2PatchSize) / g_f2HeightFieldSize;
	texcoord0to1.y = 1 - texcoord0to1.y;

	// fetching base heightmap,normal and moving vertices along y axis
	float4 base_texvalue = g_texHeightField.SampleLevel(SamplerLinearBorder, texcoord0to1, 0);
	float3 base_normal = base_texvalue.xyz;
	base_normal.z = -base_normal.z;

	float3 vertexPosition = 0.f;
	vertexPosition.xz = input.origin + uv * g_f2PatchSize;
	vertexPosition.y = base_texvalue.w;

	// writing output params
	output.position = mul(float4(vertexPosition, 1.0), g_ModelViewProjectionMatrix);
	output.texcoord = texcoord0to1;
	output.texcoord_detail = texcoord0to1 * g_f2HeightFieldSize;
	output.normal = base_normal;
	output.tangent = normalize(cross(float3(-1.0, 0.0, 0.0), base_normal));

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
	float3 albedo = saturate(pow(abs(g_texDetail.Sample(SamplerLinearWrap, input.texcoord_detail).rgb), 2.2f) * 2.f);
	albedo = saturate(albedo  * g_texColor.Sample(SamplerAnisotropicBorder, input.texcoord).rgb);

	float3 binormal = normalize(cross(input.tangent, input.normal));

	float4 detailNormal = g_texDetailNormal.Sample(SamplerLinearWrap, input.texcoord_detail);
	float3 normal = normalize(2.f * detailNormal.xyz - 1.f);
	normal = normalize((normal.x * input.tangent) + (normal.y * binormal) + input.normal);

	PS_OUTPUT output = (PS_OUTPUT)0.f;
	output.normals.xy = CompressNormal(normal);
	output.normals.zw = CompressNormal(input.tangent);

	float3 RM = float3(1.f, 0.f, 0.f);
	float3 specular = lerp(0.03f, albedo, RM.y);

	output.colors.x = Pack3PNForFP32(albedo);
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
