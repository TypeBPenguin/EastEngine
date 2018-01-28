#ifndef _BRDF_INTERFACE_
#define _BRDF_INTERFACE_

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

float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
	return 1 / (NdotV + sqrt(sqr(VdotX * ax) + sqr(VdotY * ay) + sqr(NdotV)));
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

void CalcBRDF(in float3 albedoColor, in float3 normal, in float3 tangent, in float3 binormal,
	in float3 lightDir, in float3 viewDir,
	float roughness, float metallic,
	float subsurface, float specular, float specularTint, float anisotropic,
	float sheen, float sheenTint, float clearcoat, float clearcoatGloss,
	out float3 albedo, out float3 diffuse)
{
	// Compute some useful values.
	float NdL = saturate(dot(normal, lightDir));
	float NdV = saturate(dot(normal, viewDir));

	float3 h = normalize(lightDir + viewDir);
	float NdH = saturate(dot(normal, h));
	float LdH = saturate(dot(lightDir, h));

	// luminance approx.
	const float3 LUM_CONVERT = float3(0.299f, 0.587f, 0.114f);
	float luminance = dot(LUM_CONVERT, albedoColor);

	float3 CTint = luminance > 0.0f ? albedoColor / luminance : 1.f.xxx; // Normalize luminance to isolate hue+sat.
	float3 CSpec0 = lerp(specular * 0.08f * lerp(1.f.xxx, CTint, specularTint), albedoColor, metallic);
	float3 CSheen = lerp(1.f.xxx, CTint, sheenTint);

	// Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
	// and mix in diffuse retro-reflection based on roughness
	float FL = Fresnel_Schlick(NdL);
	float FV = Fresnel_Schlick(NdV);
	float Fd90 = 0.5f + 2.0f * LdH * LdH * roughness;
	float Fd = lerp(1.f, Fd90, FL) * lerp(1.f, Fd90, FV);

	// Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
	// 1.25 scale is used to (roughly) preserve albedo
	// Fss90 used to "flatten" retroreflection based on roughness
	float Fss90 = LdH * LdH * roughness;
	float Fss = lerp(1.f, Fss90, FL) * lerp(1.f, Fss90, FV);
	float ss = 1.25f * (Fss * (1.f / (NdL + NdV + 1e-5f) - 0.5f) + 0.5f);

	// Specular
	float aspect = sqrt(1.f - anisotropic * 0.9f);
	float ax = max(0.001f, sqr(roughness) / aspect);
	float ay = max(0.001f, sqr(roughness) * aspect);
	float Ds = GTR2_aniso(NdH, dot(h, tangent), dot(h, binormal), ax, ay);
	float FH = Fresnel_Schlick(LdH);
	float3 Fs = lerp(CSpec0, 1.f.xxx, FH);
	float Gs = smithG_GGX_aniso(NdL, dot(lightDir, tangent), dot(lightDir, binormal), ax, ay);
	Gs *= smithG_GGX_aniso(NdV, dot(viewDir, tangent), dot(viewDir, binormal), ax, ay);

	// Sheen
	float3 Fsheen = FH * sheen * CSheen;

	// Clearcoat (ior = 1.5 -> F0 = 0.04)
	float Dr = GTR1(NdH, lerp(0.1f, 0.001f, clearcoatGloss));
	float Fr = lerp(0.04f, 1.f, FH);
	float Gr = smithG_GGX(NdL, 0.25f) * smithG_GGX(NdV, 0.25f);
	albedo = (INV_PI * lerp(Fd, ss, subsurface) * albedoColor + Fsheen) * (1.f - metallic);

	diffuse = (albedo + Gs*Fs*Ds + 0.25f * clearcoat * Gr * Fr * Dr) * NdL;
}

#endif