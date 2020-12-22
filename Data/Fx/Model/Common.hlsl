#define PI 3.14159265359f

#include "../Converter.hlsl"
#include "BRDF.hlsl"
#include "Light.hlsl"
#include "Shadow.hlsl"

#ifdef DX11

#define TexDiffuseHDR(normal)	g_texDiffuseHDR.SampleLevel(SamplerLinearClamp, normal, 0)
#define TexSpecularHDR(reflectVector, mipIndex)	g_texSpecularHDR.SampleLevel(SamplerLinearClamp, reflectVector, mipIndex)
#define TexSpecularBRDF(normal)	g_texSpecularBRDF.SampleLevel(SamplerLinearClamp, normal, 0)

TextureCube g_texDiffuseHDR : register(t17);
TextureCube g_texSpecularHDR : register(t18);
Texture2D g_texSpecularBRDF : register(t19);

Texture2D g_texCascadeShadowMap[8] : register(t20);
SamplerComparisonState SamplerShadow : register(s3);

SamplerState SamplerPointClamp : register(s1);
SamplerState SamplerLinearClamp : register(s2);

cbuffer cbCommonContents : register(b5)
{
	float3 g_f3CameraPos;
	float g_farClip;

	uint g_nDirectionalLightCount;
	uint g_nPointLightCount;
	uint g_nSpotLightCount;
	uint g_cascadeShadowCount;

	DirectionalLight g_lightDirectional[eMaxDirectionalLightCount];
	PointLight g_lightPoint[eMaxPointLightCount];
	SpotLight g_lightSpot[eMaxSpotLightCount];

	CascadedShadow g_cascadedShadow[eMaxDirectionalLightCount];
};

#elif DX12

#define TexDiffuseHDR(normal)	TexCubeTable[g_nTexDiffuseHDRIndex].SampleLevel(SamplerLinearClamp, normal, 0)
#define TexSpecularHDR(reflectVector, mipIndex)	TexCubeTable[g_nTexSpecularHDRIndex].SampleLevel(SamplerLinearClamp, reflectVector, mipIndex)
#define TexSpecularBRDF(normal)	Tex2DTable[g_nTexSpecularBRDFIndex].SampleLevel(SamplerLinearClamp, normal, 0)

SamplerState SamplerPointClamp : register(s0, space100);
SamplerState SamplerLinearClamp : register(s1, space100);

cbuffer cbCommonContents : register(b5)
{
	float3 g_f3CameraPos;
	int g_nEnableShadowCount = 0;

	uint g_nTexDiffuseHDRIndex;
	uint g_nTexSpecularHDRIndex;
	uint g_nTexSpecularBRDFIndex;
	uint g_nTexShadowMapIndex;

	uint g_nDirectionalLightCount = 0;
	uint g_nPointLightCount = 0;
	uint g_nSpotLightCount = 0;
	uint padding = 0;

	DirectionalLight g_lightDirectional[eMaxDirectionalLightCount];
	PointLight g_lightPoint[eMaxPointLightCount];
	SpotLight g_lightSpot[eMaxSpotLightCount];
};

#endif

float4 CalcColor(in float3 posW, in float4 posWV,
	in float3 normal, in float3 tangent, in float3 binormal, 
	in float3 albedo, in float3 emissive, in float emissiveIntensity, 
	in float roughness, in float metallic, 
	in float subsourface, in float specular, in float specularTint,
	in float anisotropic, in float sheen, in float sheenTint,
	in float clearcoat, in float clearcoatGloss)
{
	// Lerp with metallic value to find the good diffuse and specular.
	float3 realAlbedo = albedo - albedo * metallic;

	float3 viewDir = normalize(g_f3CameraPos - posW);

	float3 color = (emissive * emissiveIntensity) * realAlbedo;

	float3 diffuse = 0.f;
	float3 ambient = realAlbedo;
	float3 finalSpecular = 0.f;
	float sumReflectionIntensity = 0.f;

	uint i = 0;

	[loop]
	for (i = 0; i < g_nDirectionalLightCount; ++i)
	{
		float3 lightColor = g_lightDirectional[i].color;
		float3 lightDir = normalize(-g_lightDirectional[i].direction);
	
		float lightIntensity = g_lightDirectional[i].lightIntensity;
		float ambientIntensity = g_lightDirectional[i].ambientIntensity;
		float reflectionIntensity = g_lightDirectional[i].reflectionIntensity;
	
		float attenuation = 1.f;
	
		float3 albedo = float3(0.f, 0.f, 0.f);
		float3 diffuseColor = float3(0.f, 0.f, 0.f);
	
		CalcBRDF(realAlbedo, normal, tangent, binormal,
			lightColor, lightDir, viewDir,
			roughness, metallic,
			subsourface, specular, specularTint, anisotropic,
			sheen, sheenTint, clearcoat, clearcoatGloss,
			albedo, diffuseColor);
	
		diffuse += diffuseColor * attenuation * lightIntensity;
		sumReflectionIntensity += reflectionIntensity;
		ambient += albedo * ambientIntensity;
	}

	[loop]
	for (i = 0; i < g_nPointLightCount; ++i)
	{
		float3 lightColor = g_lightPoint[i].color;
		float3 lightDir = g_lightPoint[i].position - posW;
		float lightDist = length(lightDir);
		lightDir /= lightDist;

		float lightIntensity = g_lightPoint[i].lightIntensity;
		float attenuation = (PI / (lightDist * lightDist)) * lightIntensity;
		if (attenuation > PI)
		{
			float ambientIntensity = g_lightPoint[i].ambientIntensity;
			float reflectionIntensity = g_lightPoint[i].reflectionIntensity;

			float3 albedo = float3(0.f, 0.f, 0.f);
			float3 diffuseColor = float3(0.f, 0.f, 0.f);

			// ´õ ±×·²½ÎÇÑ ¹æ¹ý ¾ø´Ì?
			attenuation -= PI + 1e-5f;
			attenuation *= 0.1f;

			CalcBRDF(realAlbedo, normal, tangent, binormal,
				lightColor, lightDir, viewDir,
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
		float3 lightDir = g_lightSpot[i].position - posW;
		float lightDist = length(lightDir);
		lightDir /= lightDist;

		float minCos = cos(radians(g_lightSpot[i].fAngle));
		float maxCos = lerp(minCos, 1.f, 0.5f);

		float cosAngle = dot(-g_lightSpot[i].direction, lightDir);

		float spotIntensity = smoothstep(minCos, maxCos, cosAngle);
		if (spotIntensity > 0.001f)
		{
			spotIntensity -= 0.001f + 1e-5f;

			float lightIntensity = g_lightSpot[i].lightIntensity;
			float attenuation = (PI / (lightDist * lightDist)) * lightIntensity;
			attenuation *= 0.1f;

			//attenuation = smoothstep(0.01f, 1.f, attenuation);

			float3 lightColor = g_lightSpot[i].color;

			float ambientIntensity = g_lightSpot[i].ambientIntensity;
			float reflectionIntensity = g_lightSpot[i].reflectionIntensity;

			float3 albedo = float3(0.f, 0.f, 0.f);
			float3 diffuseColor = float3(0.f, 0.f, 0.f);

			CalcBRDF(realAlbedo, normal, tangent, binormal,
				lightColor, lightDir, viewDir,
				roughness, metallic,
				subsourface, specular, specularTint, anisotropic,
				sheen, sheenTint, clearcoat, clearcoatGloss,
				albedo, diffuseColor);

			diffuse += diffuseColor * attenuation * spotIntensity;
			sumReflectionIntensity += reflectionIntensity * attenuation * spotIntensity;
			ambient += albedo * ambientIntensity * attenuation * spotIntensity;
		}
	}

#ifdef DX11
	[unroll]
	for (i = 0; i < g_cascadeShadowCount; ++i)
	{
		float shadowIntensity = CalculateCascadedShadow(g_texCascadeShadowMap[i], g_cascadedShadow[i], SamplerShadow, posW, posWV.xyz);
		diffuse *= shadowIntensity;
	}
#endif

	//float shadow = 1.f;
	//if (g_nEnableShadowCount > 0)
	//{
	//	float2 shadowData = g_texShadowMap.Sample(SamplerLinearClamp, shadowTexUV);
	//	float fMask = shadowData.y * 100.f;
	//	if (fMask > 0.f)
	//	{
	//		shadow = shadowData.x / fMask;
	//		shadow = 1.f - shadow;
	//	}
	//	else
	//	{
	//		shadow = 0.f;
	//	}
	//}

	float3 reflectVector = normalize(reflect(-viewDir, normal));

	float4 diffuseIBL = TexDiffuseHDR(normal) / PI;

	float mipIndex = roughness * roughness * 8.0f;
	float3 specularIBL = TexSpecularHDR(reflectVector, mipIndex).xyz;

	float vdotn = dot(viewDir, normal);
	float4 brdfTerm = TexSpecularBRDF(float2(vdotn, 1.f - roughness));
	float3 metalSpecularIBL = specularIBL.rgb;

	float3 specColor = lerp(0.03f.xxx, albedo, metallic);

	float3 envFresnel = Specular_F_Roughness(specColor, roughness * roughness, normal, viewDir);

	color = (diffuse + (envFresnel * metalSpecularIBL * (specColor * brdfTerm.x + (brdfTerm.y))) + (ambient * diffuseIBL.rgb));

	return float4(pow(abs(color), 1.f / 2.2f), subsourface);
}