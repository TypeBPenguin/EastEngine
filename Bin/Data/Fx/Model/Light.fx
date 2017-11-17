#ifndef _LIGHT_INTERFACE_
#define _LIGHT_INTERFACE_

float ComputeOrenNayarLighting_Fakey(float lightDot, float viewDot, float roughness)
{
	float result = saturate(lightDot);
	float soft_rim = saturate(1.f - viewDot * 0.5f);

	float fakey = pow(1.f - result * soft_rim, 2);
	float fakey_magic = 0.62f;
	fakey = fakey_magic - fakey * fakey_magic;

	return lerp(result, fakey, roughness);
}

float4 ComputeOrenNayarLighting(float3 diffuse, float3 normal, float3 lightDir, float3 viewDir, float roughness)
{
	float lightDot = dot(lightDir, normal);
	float viewDot = dot(viewDir, normal);

	float gamma = dot(viewDir - normal * dot(viewDir, normal), lightDot - normal * dot(lightDir, normal));
	float rough_sq = roughness * roughness;

	// Why A, B, C!!!!?
	float A = 1.f - 0.5f * (rough_sq / (rough_sq + 0.33f));
	float B = 0.45f * (rough_sq / (rough_sq + 0.09f));
	float C = ComputeOrenNayarLighting_Fakey(lightDot, viewDot, roughness);

	float3 final = (A + B * max(0.f, gamma) * C);

	return float4(diffuse * max(0.f, dot(normal, lightDir)) * final, 1.f);
}

struct DirectionalLight
{
	float3 f3Color;
	float fLightIntensity;

	float3 f3Dir;
	float fAmbientIntensity;

	float3 padding;
	float fReflectionIntensity;

	float GetIntensity() { return fLightIntensity; }
	float GetAmbientIntensity() { return fAmbientIntensity; }
	float GetReflectionIntensity() { return fReflectionIntensity; }
	float3 GetDir() { return f3Dir; }
	float3 GetColor() { return f3Color; }
};

struct PointLight
{
	float3 f3Color;
	float fLightIntensity;

	float3 f3Pos;
	float fAmbientIntensity;

	float3 padding;
	float fReflectionIntensity;

	float GetIntensity() { return fLightIntensity; }
	float GetAmbientIntensity() { return fAmbientIntensity; }
	float GetReflectionIntensity() { return fReflectionIntensity; }
	float3 GetPos() { return f3Pos; }
	float3 GetColor() { return f3Color; }
};

struct SpotLight
{
	float3 f3Color;
	float fLightIntensity;

	float3 f3Pos;
	float fAmbientIntensity;

	float3 f3Dir;
	float fReflectionIntensity;

	float3 padding;
	float fAngle;

	float GetIntensity() { return fLightIntensity; }
	float GetAmbientIntensity() { return fAmbientIntensity; }
	float GetReflectionIntensity() { return fReflectionIntensity; }
	float3 GetPos() { return f3Pos; }
	float3 GetColor() { return f3Color; }

	float3 GetDir() { return f3Dir; }
	float GetAngle() { return fAngle; }
};

cbuffer cbLights
{
	uint g_nDirectionalLightCount = 0;
	uint g_nPointLightCount = 0;
	uint g_nSpotLightCount = 0;
};

StructuredBuffer<DirectionalLight> g_lightDirectional;
StructuredBuffer<PointLight> g_lightPoint;
StructuredBuffer<SpotLight> g_lightSpot;

#endif