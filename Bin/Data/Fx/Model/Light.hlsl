//float ComputeOrenNayarLighting_Fakey(float lightDot, float viewDot, float roughness)
//{
//	float result = saturate(lightDot);
//	float soft_rim = saturate(1.f - viewDot * 0.5f);
//
//	float fakey = pow(1.f - result * soft_rim, 2);
//	float fakey_magic = 0.62f;
//	fakey = fakey_magic - fakey * fakey_magic;
//
//	return lerp(result, fakey, roughness);
//}
//
//float4 ComputeOrenNayarLighting(float3 diffuse, float3 normal, float3 lightDir, float3 viewDir, float roughness)
//{
//	float lightDot = dot(lightDir, normal);
//	float viewDot = dot(viewDir, normal);
//
//	float gamma = dot(viewDir - normal * dot(viewDir, normal), lightDot - normal * dot(lightDir, normal));
//	float rough_sq = roughness * roughness;
//
//	// Why A, B, C!!!!?
//	float A = 1.f - 0.5f * (rough_sq / (rough_sq + 0.33f));
//	float B = 0.45f * (rough_sq / (rough_sq + 0.09f));
//	float C = ComputeOrenNayarLighting_Fakey(lightDot, viewDot, roughness);
//
//	float3 final = (A + B * max(0.f, gamma) * C);
//
//	return float4(diffuse * max(0.f, dot(normal, lightDir)) * final, 1.f);
//}

struct DirectionalLight
{
	float3 color;
	float lightIntensity;

	float3 direction;
	float ambientIntensity;

	float3 padding;
	float reflectionIntensity;
};

struct PointLight
{
	float3 color;
	float lightIntensity;

	float3 position;
	float ambientIntensity;

	float3 padding;
	float reflectionIntensity;
};

struct SpotLight
{
	float3 color;
	float lightIntensity;

	float3 position;
	float ambientIntensity;

	float3 direction;
	float reflectionIntensity;

	float3 padding;
	float fAngle;
};

#define eMaxDirectionalLightCount 32
#define eMaxPointLightCount 1024
#define eMaxSpotLightCount 128