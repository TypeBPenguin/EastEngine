#ifndef _MODEL_DEFERRED_
#define _MODEL_DEFERRED_

#define PI 3.14159265359f

#include "../Converter.fx"
#include "BRDF.fx"
#include "Light.fx"
//#include "Shadow.fx"

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
};

Texture2D<float> g_texDepth;
Texture2D g_texNormal;
Texture2D g_texAlbedoSpecular;
Texture2D g_texDisneyBRDF;

Texture2D<float2> g_texShadowMap;

TextureCube g_texIBLMap;
TextureCube g_texIrradianceMap;

SamplerState g_samPoint : register(s0);
SamplerState g_samLinearWrap : register(s1);

cbuffer cbContents
{
	float3 g_f3CameraPos;
	int g_nEnableShadowCount;

	float4x4 g_matInvView;
	float4x4 g_matInvProj;
};

PS_INPUT VS(uint id : SV_VertexID)
{
	PS_INPUT output;
	output.tex = float2(((id << 1) & 2) != 0, (id & 2) != 0);
	output.pos = float4(output.tex * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);

	return output;
}

float4 PS(PS_INPUT input) : SV_Target0
{
	float depth = g_texDepth.Sample(g_samPoint, input.tex);
	clip((1.f - 1e-5f) - depth);

	float4 colors = g_texAlbedoSpecular.Sample(g_samPoint, input.tex);
	float4 normals = g_texNormal.Sample(g_samPoint, input.tex);
	float4 disneyBRDF = g_texDisneyBRDF.Sample(g_samPoint, input.tex);

	float4 posWV = 0.f;
	float3 posW = CalcWorldSpacePosFromDepth(depth, input.tex, g_matInvView, g_matInvProj, posWV);

	float3 normal = DeCompressNormal(normals.xy);
	float3 tangent = DeCompressNormal(normals.zw);
	float3 binormal = CalcBinormal(normal, tangent);

	float3 albedo = Unpack3PNFromFP32(colors.x);
	float3 specular = Unpack3PNFromFP32(colors.y);
	float3 emissive = Unpack3PNFromFP32(colors.z);
	float3 emissiveIntensity = colors.w;

	float3 RM = Unpack3PNFromFP32(disneyBRDF.x);
	float3 SST = Unpack3PNFromFP32(disneyBRDF.y);
	float3 AST = Unpack3PNFromFP32(disneyBRDF.z);
	float3 CG = Unpack3PNFromFP32(disneyBRDF.w);

	// Lerp with metallic value to find the good diffuse and specular.
	float3 realAlbedo = albedo - albedo * RM.y;

	float3 viewDir = normalize(g_f3CameraPos - posW);
	float3 reflectVector = reflect(-viewDir, normal);

	float mipIndex = RM.x * RM.x * 8.0f;

	float3 envColor = pow(abs(g_texIBLMap.SampleLevel(g_samLinearWrap, reflectVector, mipIndex)), 2.2f).xyz;

	//float3 irradiance = pow(abs(g_texIrradianceMap.Sample(g_samLinearWrap, normal)), 2.2f).xyz;
	float3 irradiance = g_texIrradianceMap.Sample(g_samLinearWrap, normal).xyz;

	float3 envFresnel = Specular_F_Roughness(specular, RM.x * RM.x, normal, viewDir);

	float3 color = (emissive * emissiveIntensity) * realAlbedo;

	float3 diffuse = 0.f;
	float3 ambient = 0.f;
	float sumReflectionIntensity = 0.f;

	uint i = 0;

	[loop]
	for (i = 0; i < g_nDirectionalLightCount; ++i)
	{
		float3 lightColor = g_lightDirectional[i].GetColor();
		float3 lightDir = -g_lightDirectional[i].GetDir();
	
		float lightIntensity = g_lightDirectional[i].GetIntensity();
		float ambientIntensity = g_lightDirectional[i].GetAmbientIntensity();
		float reflectionIntensity = g_lightDirectional[i].GetReflectionIntensity();
	
		float attenuation = 0.001f * 0.1f;
	
		float3 albedo = float3(0.f, 0.f, 0.f);
		float3 diffuseColor = float3(0.f, 0.f, 0.f);
	
		CalcBRDF(realAlbedo, specular, normal,
			RM.x, RM.y,
			SST.x, SST.y, SST.z, AST.x,
			AST.y, AST.z, CG.x, CG.y,
			lightColor, lightDir, viewDir, tangent, binormal, albedo, diffuseColor);
	
		diffuse += diffuseColor * attenuation * lightIntensity;
		sumReflectionIntensity += reflectionIntensity;
		ambient += albedo * ambientIntensity;
	}

	[loop]
	for (i = 0; i < g_nPointLightCount; ++i)
	{
		float3 lightColor = g_lightPoint[i].GetColor();
		float3 lightDir = g_lightPoint[i].GetPos() - posW;
		float lightDist = length(lightDir);
		lightDir /= lightDist;

		float lightIntensity = g_lightPoint[i].GetIntensity();
		float attenuation = (PI / (lightDist * lightDist)) * lightIntensity;
		if (attenuation > PI)
		{
			float ambientIntensity = g_lightPoint[i].GetAmbientIntensity();
			float reflectionIntensity = g_lightPoint[i].GetReflectionIntensity();

			float3 albedo = float3(0.f, 0.f, 0.f);
			float3 diffuseColor = float3(0.f, 0.f, 0.f);

			// ´õ ±×·²½ÎÇÑ ¹æ¹ý ¾ø´Ì?
			attenuation -= PI + 1e-5f;
			attenuation *= 0.1f;

			CalcBRDF(realAlbedo, specular, normal,
				RM.x, RM.y,
				SST.x, SST.y, SST.z, AST.x,
				AST.y, AST.z, CG.x, CG.y,
				lightColor, lightDir, viewDir, tangent, binormal, albedo, diffuseColor);

			diffuse += diffuseColor * attenuation;
			sumReflectionIntensity += reflectionIntensity * attenuation;
			ambient += albedo * ambientIntensity * attenuation;
		}
	}

	[loop]
	for (i = 0; i < g_nSpotLightCount; ++i)
	{
		float3 lightDir = g_lightSpot[i].GetPos() - posW;
		float lightDist = length(lightDir);
		lightDir /= lightDist;

		float minCos = cos(radians(g_lightSpot[i].GetAngle()));
		float maxCos = lerp(minCos, 1.f, 0.5f);

		float cosAngle = dot(-g_lightSpot[i].GetDir(), lightDir);

		float spotIntensity = smoothstep(minCos, maxCos, cosAngle);
		if (spotIntensity > 0.001f)
		{
			spotIntensity -= 0.001f + 1e-5f;

			float lightIntensity = g_lightSpot[i].GetIntensity();
			float attenuation = (PI / (lightDist * lightDist)) * lightIntensity;
			attenuation *= 0.1f;

			//attenuation = smoothstep(0.01f, 1.f, attenuation);

			float3 lightColor = g_lightSpot[i].GetColor();

			float ambientIntensity = g_lightSpot[i].GetAmbientIntensity();
			float reflectionIntensity = g_lightSpot[i].GetReflectionIntensity();

			float3 albedo = float3(0.f, 0.f, 0.f);
			float3 diffuseColor = float3(0.f, 0.f, 0.f);

			CalcBRDF(realAlbedo, specular, normal,
				RM.x, RM.y,
				SST.x, SST.y, SST.z, AST.x,
				AST.y, AST.z, CG.x, CG.y,
				lightColor, lightDir, viewDir, tangent, binormal, albedo, diffuseColor);

			diffuse += diffuseColor * attenuation * spotIntensity;
			sumReflectionIntensity += reflectionIntensity * attenuation * spotIntensity;
			ambient += albedo * ambientIntensity * attenuation * spotIntensity;
		}
	}

	float shadow = 1.f;
	if (g_nEnableShadowCount > 0)
	{
		float2 shadowData = g_texShadowMap.Sample(g_samPoint, input.tex);
		float fMask = shadowData.y * 100.f;
		if (fMask > 0.f)
		{
			shadow = shadowData.x / fMask;
			shadow = 1.f - shadow;
		}
		else
		{
			shadow = 0.f;
		}
		//
		//shadow = 1.f - shadow;

		//shadow = g_texShadowMap.Sample(g_samPoint, input.tex);
		//shadow = saturate(shadow);
		//shadow = g_texShadowMap.Sample(g_samPoint, input.tex) / (float)g_nEnableShadowCount;
		//shadow = smoothstep(0.25f, 0.75f, shadow);

		//shadow = CalculateShadow(posW, posWV.xyz);
		/*if (-1.f <= shadow && shadow < 0.f)
		{
			return float4(0.f, 0.f, 0.f, 1.f);
		}
		else if (0.f <= shadow && shadow < 1.f)
		{
			return float4(1.f, 0.f, 0.f, 1.f);
		}
		else if (1.f <= shadow && shadow < 2.f)
		{
			return float4(0.f, 1.f, 0.f, 1.f);
		}
		else if (2.f <= shadow && shadow < 3.f)
		{
			return float4(0.f, 0.f, 1.f, 1.f);
		}
		else
		{
			return float4(1.f, 1.f, 1.f, 1.f);
		}*/
	}

	color += ((diffuse + (envFresnel * envColor * sumReflectionIntensity)) * shadow) + (irradiance * ambient);

	return float4(pow(abs(color), 1.f / 2.2f), SST.x);
}

technique11 Deferred
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

#endif