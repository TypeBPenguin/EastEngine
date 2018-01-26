#ifndef _MODEL_COMMON_
#define _MODEL_COMMON_

#define PI 3.14159265359f

#include "../Converter.fx"
#include "BRDF.fx"
#include "Light.fx"

TextureCube g_texIBLMap;
TextureCube g_texIrradianceMap;

SamplerState g_samPoint : register(s0);
SamplerState g_samLinearWrap : register(s1);

Texture2D<float2> g_texShadowMap;

cbuffer cbCommonContents
{
	float3 g_f3CameraPos;
	int g_nEnableShadowCount = 0;
};

float4 CalcColor(in float3 posW, 
	in float3 normal, in float3 tangent, in float3 binormal, 
	in float3 albedo, in float3 specularColor, in float3 emissive, in float emissiveIntensity, 
	in float roughness, in float metallic, 
	in float subsourface, in float specular, in float specularTint,
	in float anisotropic, in float sheen, in float sheenTint,
	in float clearcoat, in float clearcoatGloss,
	in float2 shadowTexUV)
{
	// Lerp with metallic value to find the good diffuse and specular.
	float3 realAlbedo = albedo - albedo * metallic;

	float3 viewDir = normalize(g_f3CameraPos - posW);
	float3 reflectVector = reflect(-viewDir, normal);

	float mipIndex = roughness * roughness * 8.0f;

	float3 envColor = pow(abs(g_texIBLMap.SampleLevel(g_samLinearWrap, reflectVector, mipIndex)), 2.2f).xyz;

	//float3 irradiance = pow(abs(g_texIrradianceMap.Sample(g_samLinearWrap, normal)), 2.2f).xyz;
	float3 irradiance = g_texIrradianceMap.Sample(g_samLinearWrap, normal).xyz;

	float3 envFresnel = Specular_F_Roughness(specularColor, roughness * roughness, normal, viewDir);

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
	
		CalcBRDF(realAlbedo, normal, tangent, binormal,
			lightDir, viewDir,
			roughness, metallic,
			subsourface, specular, specularTint, anisotropic,
			sheen, sheenTint, clearcoat, clearcoatGloss,
			albedo, diffuseColor);
	
		diffuse += diffuseColor * attenuation * lightIntensity;
		sumReflectionIntensity += reflectionIntensity * 0.1f;
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

			CalcBRDF(realAlbedo, normal, tangent, binormal,
				lightDir, viewDir,
				roughness, metallic,
				subsourface, specular, specularTint, anisotropic,
				sheen, sheenTint, clearcoat, clearcoatGloss,
				albedo, diffuseColor);

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

			CalcBRDF(realAlbedo, normal, tangent, binormal,
				lightDir, viewDir,
				roughness, metallic,
				subsourface, specular, specularTint, anisotropic,
				sheen, sheenTint, clearcoat, clearcoatGloss,
				albedo, diffuseColor);

			diffuse += diffuseColor * attenuation * spotIntensity;
			sumReflectionIntensity += reflectionIntensity * attenuation * spotIntensity;
			ambient += albedo * ambientIntensity * attenuation * spotIntensity;
		}
	}

	float shadow = 1.f;
	if (g_nEnableShadowCount > 0)
	{
		float2 shadowData = g_texShadowMap.Sample(g_samPoint, shadowTexUV);
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
	}

	color += ((diffuse + (envFresnel * envColor * sumReflectionIntensity)) * shadow) + (irradiance * ambient);

	return float4(pow(abs(color), 1.f / 2.2f), subsourface);
}

#endif