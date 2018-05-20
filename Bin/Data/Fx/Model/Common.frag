#define PI 3.14159265359f

#ifdef Vulkan
#define lerp(a, b, t) mix(a, b, t)
#define saturate(a) clamp(a, 0.0, 1.0)
#define mad(a, b, c) (a * b + c)
#define mul(v, m) (m * v)
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define float4x4 mat4x4
#define asfloat uintBitsToFloat
#define asuint floatBitsToUint
#endif

#include "../Converter.hlsl"
#include "BRDF.hlsl"
#include "Light.hlsl"

#define TexDiffuseHDR(normal)		textureLod(g_texDiffuseHDR, normal, 0)
#define TexSpecularHDR(reflectVector, mipIndex)		textureLod(g_texSpecularHDR, reflectVector, mipIndex)
#define TexSpecularBRDF(normal)		textureLod(g_texSpecularBRDF, normal, 0)

layout(set = 0, binding = 4) uniform samplerCube g_texDiffuseHDR;
layout(set = 0, binding = 5) uniform samplerCube g_texSpecularHDR;
layout(set = 0, binding = 6) uniform sampler2D g_texSpecularBRDF;

layout(set = 1, binding = 0) uniform ubLightContents
{
	uint nDirectionalLightCount;
	uint nPointLightCount;
	uint nSpotLightCount;
	uint padding;

	//DirectionalLight lightDirectional[eMaxDirectionalLightCount];
	//PointLight lightPoint[eMaxPointLightCount];
	//SpotLight lightSpot[eMaxSpotLightCount];
} LightContents;

layout(set = 2, binding = 0) uniform ubCommonContents
{
	float4x4 matInvView;
	float4x4 matInvProj;

	float3 f3CameraPos;
	int nEnableShadowCount;
} CommonContents;

float4 CalcColor(in float3 posW, 
	in float3 normal, in float3 tangent, in float3 binormal, 
	in float3 albedo, in float3 emissive, in float emissiveIntensity, 
	in float roughness, in float metallic, 
	in float subsourface, in float specular, in float specularTint,
	in float anisotropic, in float sheen, in float sheenTint,
	in float clearcoat, in float clearcoatGloss,
	in float2 shadowTexUV)
{
	// Lerp with metallic value to find the good diffuse and specular.
	float3 realAlbedo = albedo - albedo * metallic;

	float3 viewDir = normalize(CommonContents.f3CameraPos - posW);

	float3 color = (emissive * emissiveIntensity) * realAlbedo;

	float3 diffuse = float3(0.f);
	float3 ambient = realAlbedo;
	float3 finalSpecular = float3(0.f);
	float sumReflectionIntensity = 0.f;

	uint i = 0;
	
	//for (i = 0; i < LightContents.nDirectionalLightCount; ++i)
	//{
	//	float3 lightColor = LightContents.lightDirectional[i].f3Color;
	//	float3 lightDir = normalize(-LightContents.lightDirectional[i].GetDir());
	//
	//	float lightIntensity = LightContents.lightDirectional[i].fLightIntensity;
	//	float ambientIntensity = LightContents.lightDirectional[i].fAmbientIntensity;
	//	float reflectionIntensity = LightContents.lightDirectional[i].fReflectionIntensity;
	//
	//	float attenuation = 1.f;
	//
	//	float3 albedo = float3(0.f, 0.f, 0.f);
	//	float3 diffuseColor = float3(0.f, 0.f, 0.f);
	//
	//	CalcBRDF(realAlbedo, normal, tangent, binormal,
	//		lightDir, viewDir,
	//		roughness, metallic,
	//		subsourface, specular, specularTint, anisotropic,
	//		sheen, sheenTint, clearcoat, clearcoatGloss,
	//		albedo, diffuseColor);
	//
	//	diffuse += diffuseColor * attenuation * lightIntensity;
	//	sumReflectionIntensity += reflectionIntensity;
	//	ambient += albedo * ambientIntensity;
	//}
	//
	//for (i = 0; i < LightContents.nPointLightCount; ++i)
	//{
	//	float3 lightColor = LightContents.lightPoint[i].f3Color;
	//	float3 lightDir = LightContents.lightPoint[i].f3Pos - posW;
	//	float lightDist = length(lightDir);
	//	lightDir /= lightDist;
	//
	//	float lightIntensity = LightContents.lightPoint[i].fLightIntensity;
	//	float attenuation = (PI / (lightDist * lightDist)) * lightIntensity;
	//	if (attenuation > PI)
	//	{
	//		float ambientIntensity = LightContents.lightPoint[i].fAmbientIntensity;
	//		float reflectionIntensity = LightContents.lightPoint[i].fReflectionIntensity;
	//
	//		float3 albedo = float3(0.f, 0.f, 0.f);
	//		float3 diffuseColor = float3(0.f, 0.f, 0.f);
	//
	//		// ´õ ±×·²½ÎÇÑ ¹æ¹ý ¾ø´Ì?
	//		attenuation -= PI + 1e-5f;
	//		attenuation *= 0.1f;
	//
	//		CalcBRDF(realAlbedo, normal, tangent, binormal,
	//			lightDir, viewDir,
	//			roughness, metallic,
	//			subsourface, specular, specularTint, anisotropic,
	//			sheen, sheenTint, clearcoat, clearcoatGloss,
	//			albedo, diffuseColor);
	//
	//		diffuse += diffuseColor * attenuation;
	//		sumReflectionIntensity += reflectionIntensity * attenuation;
	//		ambient += albedo * ambientIntensity * attenuation;
	//	}
	//}
	//
	//for (i = 0; i < LightContents.nSpotLightCount; ++i)
	//{
	//	float3 lightDir = LightContents.lightSpot[i].f3Pos - posW;
	//	float lightDist = length(lightDir);
	//	lightDir /= lightDist;
	//
	//	float minCos = cos(radians(LightContents.lightSpot[i].GetAngle()));
	//	float maxCos = lerp(minCos, 1.f, 0.5f);
	//
	//	float cosAngle = dot(-LightContents.lightSpot[i].GetDir(), lightDir);
	//
	//	float spotIntensity = smoothstep(minCos, maxCos, cosAngle);
	//	if (spotIntensity > 0.001f)
	//	{
	//		spotIntensity -= 0.001f + 1e-5f;
	//
	//		float lightIntensity = LightContents.lightSpot[i].fLightIntensity;
	//		float attenuation = (PI / (lightDist * lightDist)) * lightIntensity;
	//		attenuation *= 0.1f;
	//
	//		//attenuation = smoothstep(0.01f, 1.f, attenuation);
	//
	//		float3 lightColor = LightContents.lightSpot[i].f3Color;
	//
	//		float ambientIntensity = LightContents.lightSpot[i].fAmbientIntensity;
	//		float reflectionIntensity = LightContents.lightSpot[i].fReflectionIntensity;
	//
	//		float3 albedo = float3(0.f, 0.f, 0.f);
	//		float3 diffuseColor = float3(0.f, 0.f, 0.f);
	//
	//		CalcBRDF(realAlbedo, normal, tangent, binormal,
	//			lightDir, viewDir,
	//			roughness, metallic,
	//			subsourface, specular, specularTint, anisotropic,
	//			sheen, sheenTint, clearcoat, clearcoatGloss,
	//			albedo, diffuseColor);
	//
	//		diffuse += diffuseColor * attenuation * spotIntensity;
	//		sumReflectionIntensity += reflectionIntensity * attenuation * spotIntensity;
	//		ambient += albedo * ambientIntensity * attenuation * spotIntensity;
	//	}
	//}

	//float shadow = 1.f;
	//if (CommonContents.nEnableShadowCount > 0)
	//{
	//	float2 shadowData = texShadowMap.Sample(SamplerClamp, shadowTexUV);
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

	//float4 diffuseIBL = TexDiffuseHDR(normal) / float4(PI);
	float4 diffuseIBL = textureLod(g_texDiffuseHDR, normal, 0) / float4(PI);
	
	float mipIndex = roughness * roughness * 8.0f;
	float3 specularIBL = TexSpecularHDR(reflectVector, mipIndex).xyz;

	float vdotn = dot(viewDir, normal);
	float4 brdfTerm = TexSpecularBRDF(float2(vdotn, 1.f - roughness));
	float3 metalSpecularIBL = specularIBL.rgb;

	float3 specColor = lerp(0.03f.xxx, albedo, metallic);

	float3 envFresnel = Specular_F_Roughness(specColor, roughness * roughness, normal, viewDir);

	color = (diffuse + (envFresnel * metalSpecularIBL * (specColor * brdfTerm.x + (brdfTerm.y))) + (ambient * diffuseIBL.rgb));

	return float4(pow(abs(color), float3(1.f / 2.2f)), subsourface);
}