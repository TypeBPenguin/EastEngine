#ifndef _BRDF_INTERFACE_
#define _BRDF_INTERFACE_

static const float3 LUM_CONVERT = float3(0.299f, 0.587f, 0.114f);

#define PI			3.14159265359f
#define INV_PI		0.31830988618f

float sqr(float x)
{
	return x * x;
}

float GTR2_aniso(float NdH, float HdX, float HdY, float ax, float ay)
{
	return 1.f / (PI * ax*ay * sqr(sqr(HdX / ax) + sqr(HdY / ay) + NdH*NdH));
}

float smithG_GGX(float NdV, float alphaG)
{
	float a = alphaG*alphaG;
	float b = NdV*NdV;
	return 1.f / (NdV + sqrt(a + b - a*b));
}

float GTR1(float NdH, float a)
{
	if (a >= 1.f)
	{
		return INV_PI;
	}
	else
	{
		float a2 = a * a;
		float t = 1.f + (a2 - 1.f) * NdH * NdH;
		return (a2 - 1.f) / (PI * log(a2) * t);
	}
}

float3 Diffuse(float3 pAlbedo)
{
	return pAlbedo / PI;
}

float NormalDistribution_GGX(float a, float NdH)
{
	// Isotropic ggx.
	float a2 = a * a;
	float NdH2 = NdH * NdH;

	float denominator = NdH2 * (a2 - 1.f) + 1.f;
	denominator *= denominator;
	denominator *= PI;

	return a2 / denominator;
}

float Geometric_Smith_Schlick_GGX(float a, float NdV, float NdL)
{
	// Smith schlick-GGX.
	float k = a * 0.5f;
	float GV = NdV / (NdV * (1 - k) + k);
	float GL = NdL / (NdL * (1 - k) + k);

	return GV * GL;
}

float Fresnel_Schlick(float u)
{
	float m = saturate(1.f - u);
	float m2 = m * m;
	return m2 * m2 * m;
}

float3 Fresnel_Schlick(float3 specularColor, float3 h, float3 v)
{
	return (specularColor + (1.f - specularColor) * pow((1.f - saturate(dot(v, h))), 5));
}

float Specular_D(float a, float NdH)
{
	return NormalDistribution_GGX(a, NdH);
}

float3 Specular_F(float3 specularColor, float3 h, float3 v)
{
	return Fresnel_Schlick(specularColor, h, v);
}

float3 Specular_F_Roughness(float3 specularColor, float a, float3 h, float3 v)
{
	// Sclick using roughness to attenuate fresnel.
	return (specularColor + (max(1.f - a, specularColor) - specularColor) * pow((1 - saturate(dot(v, h))), 5));
}

float Specular_G(float a, float NdV, float NdL, float NdH, float VdH, float LdV)
{
	return Geometric_Smith_Schlick_GGX(a, NdV, NdL);
}

float3 Specular(float3 specularColor, float3 h, float3 v, float3 l, float a, float NdL, float NdV, float NdH, float VdH, float LdV)
{
	return ((Specular_D(a, NdH) * Specular_G(a, NdV, NdL, NdH, VdH, LdV)) * Specular_F(specularColor, v, h)) / (4.0f * NdL * NdV + 0.0001f);
}

//class CBaseBRDF : IBRDF
//{
//	float3 CalcBRDF(half3 albedoColor, half3 specularColor, float3 normal,
//		half2 f2RoughMet, half4 f4SurSpecTintAniso, half4 f4SheenTintClearcoatGloss,
//		float3 lightColor, float3 lightDir, float3 viewDir, float3 X, float3 Y, inout float3 diffuse)
//	{
//		// Compute some useful values.
//		float NdL = saturate(dot(normal, lightDir));
//		float NdV = saturate(dot(normal, viewDir));
//		float3 h = normalize(lightDir + viewDir);
//		float NdH = saturate(dot(normal, h));
//		float VdH = saturate(dot(viewDir, h));
//		float LdV = saturate(dot(lightDir, viewDir));
//		float a = max(0.001f, roughness * roughness);
//
//		float3 cDiff = Diffuse(albedoColor);
//		float3 cSpec = Specular(specularColor, h, viewDir, lightDir, a, NdL, NdV, NdH, VdH, LdV);
//
//		return lightColor * NdL * (cDiff * (1.f - cSpec) + cSpec);
//	}
//};

void CalcBRDF(float3 albedoColor, float3 specularColor, float3 normal,
	float roughness, float metallic,
	float subsurface, float specular, float specularTint, float anisotropic,
	float sheen, float sheenTint, float clearcoat, float clearcoatGloss,
	float3 lightColor, float3 lightDir, float3 viewDir, float3 X, float3 Y,
	out float3 albedo, out float3 diffuse)
{
	// Compute some useful values.
	float NdL = saturate(dot(normal, lightDir));
	float NdV = saturate(dot(normal, viewDir));

	float3 h = normalize(lightDir + viewDir);
	float NdH = saturate(dot(normal, h));
	float VdH = saturate(dot(viewDir, h));
	float LdV = saturate(dot(lightDir, viewDir));
	float LdH = saturate(dot(lightDir, h));
	float roughness_sqr = max(0.001f, roughness * roughness);

	float luminance = dot(LUM_CONVERT, albedoColor);

	float3 tint = luminance > 0.0f ? albedoColor / luminance : 1.f.xxx; // Normalize luminance to isolate hue+sat.
	specularColor = lerp(specular * 0.08f * lerp(1.f.xxx, tint, specularTint), albedoColor, metallic);
	float3 CSheen = lerp(1.f.xxx, tint, sheenTint);

	// Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
	// and mix in diffuse retro-reflection based on roughness
	float FL = Fresnel_Schlick(NdL);
	float FV = Fresnel_Schlick(NdV);
	float Fd90 = 0.5f + 2.0f * LdH * LdH * roughness_sqr;
	float Fd = lerp(1.f, Fd90, FL) * lerp(1.f, Fd90, FV);

	// Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
	// 1.25 scale is used to (roughly) preserve albedo
	// Fss90 used to "flatten" retroreflection based on roughness
	float Fss90 = LdH * LdH * roughness_sqr;
	float Fss = lerp(1.f, Fss90, FL) * lerp(1.f, Fss90, FV);
	float ss = 1.25f * (Fss * (1.f / (NdL + NdV + 0.0001f) - 0.5f) + 0.5f);

	// Specular
	float aspect = sqrt(1.f - anisotropic * 0.9f);
	float ax = max(0.001f, sqr(roughness_sqr) / aspect);
	float ay = max(0.001f, sqr(roughness_sqr) * aspect);
	float Ds = GTR2_aniso(NdH, dot(h, X), dot(h, Y), ax, ay);
	Ds = isnan(Ds) ? 1e-8 : Ds;
	float FH = Fresnel_Schlick(LdH);
	float3 Fs = lerp(specularColor, 1.f.xxx, FH);
	float roughg = sqr(roughness_sqr * 0.5f + 0.5f);
	float Gs = smithG_GGX(NdL, roughg) * smithG_GGX(NdV, roughg);

	// Sheen
	float3 Fsheen = FH * sheen * CSheen;

	// Clearcoat (ior = 1.5 -> F0 = 0.04)
	float Dr = GTR1(NdH, lerp(0.1f, 0.001f, clearcoatGloss));
	float Fr = lerp(0.04f, 1.f, FH);
	float Gr = smithG_GGX(NdL, 0.25f) * smithG_GGX(NdV, 0.25f);
	albedo = (INV_PI * lerp(Fd, ss, subsurface) * albedoColor + Fsheen) * (1.f - metallic);

	diffuse = (lightColor * albedo + Gs*Fs*Ds + 0.25f * clearcoat * Gr * Fr * Dr) * NdL;
}

#endif