#ifndef _SHADOW_INTERFACE_
#define _SHADOW_INTERFACE_

#define PI 3.14159265359f

Texture2D g_texShadowMap : register(t1);
TextureCube g_texShadowCubeMap : register(t2);
SamplerComparisonState g_samShadow : register(s1);
SamplerState g_samShadow2 : register(s2);

struct CascadedShadow
{
	int2 n2PCFBlurSize;
	float2 f2TexelOffset;

	int nCascadeLevel;
	float fDepthBias;
	float2 padding;

	float4x4 matCascadeViewProj[8];
	float4 f2SplitDpeths[8];
};

cbuffer cbCascadedShadow
{
	CascadedShadow g_cascadeShadow;
};

struct ShadowMap
{
	int2 n2PCFBlurSize;
	float2 f2TexelOffset;

	float fDepthBias;
	float3 f3LightPos;

	float3 f3LightDir;
	float fLightAngle;

	float fLightIntensity;
	float3 padding;
	
	float4x4 matViewProj;
};

struct ShadowCubeMap
{
	int2 n2PCFBlurSize;
	float2 f2TexelOffset;

	float fDepthBias;
	float3 f3LightPos;

	float fLightIntensity;
	float fFarPlane;
	float2 padding;
};

cbuffer cbShadowMap
{
	ShadowMap g_shadowMap;
	ShadowCubeMap g_shadowCubeMap;
};

int ComputeCascadeIndex(in float3 f3PosVS)
{
	for (int i = 0; i < g_cascadeShadow.nCascadeLevel; ++i)
	{
		if (g_cascadeShadow.f2SplitDpeths[i].x <= f3PosVS.z && f3PosVS.z < g_cascadeShadow.f2SplitDpeths[i].y)
			return i;
	}

	return -1;
}

// Calculates the shadow occlusion using bilinear PCF
float CalcShadowTermPCF(in float fDepth, in float2 f2ShadowTexCoord, in int2 n2PCFBlurSize, in float2 f2TexelOffset, in float fDepthBias)
{
	float fPercentLit = 0.f;
	// This loop could be unrolled, and texture immediate offsets could be used if the kernel size were fixed.
	// This would be performance improvment.

	// A very simple solution to the depth bias problems of PCF is to use an offset.
	// Unfortunately, too much offset can lead to Peter-panning (shadows near the base of object disappear )
	// Too little offset can lead to shadow acne ( objects that should not be in shadow are partially self shadowed ).
	float depthcompare = fDepth - fDepthBias;

	for (int x = n2PCFBlurSize.x; x < n2PCFBlurSize.y; ++x)
	{
		for (int y = n2PCFBlurSize.x; y < n2PCFBlurSize.y; ++y)
		{
			float fOffsetX = (float)x;
			float fOffsetY = (float)y;

			float2 uv = float2(f2ShadowTexCoord.x + (fOffsetX * f2TexelOffset.x),
				f2ShadowTexCoord.y + (fOffsetY * f2TexelOffset.y));

			// Compare the transformed pixel depth to the depth read from the map.
			fPercentLit += g_texShadowMap.SampleCmpLevelZero(g_samShadow, uv, depthcompare);
		}
	}

	int iBlurRowSize = n2PCFBlurSize.y - n2PCFBlurSize.x;
	iBlurRowSize *= iBlurRowSize;
	fPercentLit /= (float)(iBlurRowSize);

	return fPercentLit;
}

float CalculateCascadedShadow(in float3 f3PositionW, in float3 f3PositionWV)
{
	float fShadowLighting = 1.f;

	int nCascadeLevel = ComputeCascadeIndex(f3PositionWV);
	if (nCascadeLevel >= 0)
	{
		float4 f4PositionLightCS = mul(float4(f3PositionW, 1.f), g_cascadeShadow.matCascadeViewProj[nCascadeLevel]);

		float fDepth = f4PositionLightCS.z / f4PositionLightCS.w;

		// Transform from light space to shadow map texture space.
		float2 f2ShadowTexCoord = f4PositionLightCS.xy / f4PositionLightCS.w * 0.5f + float2(0.5f, 0.5f);
		f2ShadowTexCoord.x = f2ShadowTexCoord.x / (float)g_cascadeShadow.nCascadeLevel + ((float)nCascadeLevel / (float)g_cascadeShadow.nCascadeLevel);
		f2ShadowTexCoord.y = 1.f - f2ShadowTexCoord.y;

		fShadowLighting = 1.f - CalcShadowTermPCF(fDepth, f2ShadowTexCoord, g_cascadeShadow.n2PCFBlurSize, g_cascadeShadow.f2TexelOffset, g_cascadeShadow.fDepthBias);
	}

	return fShadowLighting;
}

float2 CalculateShadowMap(in float3 f3PositionW)
{
	float3 lightDir = g_shadowMap.f3LightPos - f3PositionW;
	float lightDist = length(lightDir);
	lightDir /= lightDist;

	float minCos = cos(radians(g_shadowMap.fLightAngle));
	float maxCos = lerp(minCos, 1.f, 0.5f);

	float cosAngle = dot(-g_shadowMap.f3LightDir, lightDir);

	float fShadowLighting = 0.f;
	float fMask = 0.f;

	float spotIntensity = smoothstep(minCos, maxCos, cosAngle);
	if (spotIntensity > 0.001f)
	{
		spotIntensity -= 0.001f + 1e-5f;

		// SpotLight 특성상 라이트에서 멀어질수록 빛이 약해지기 때문에 그림자도 약해지도록 처리하려고 했음
		// 하지만 빛의 영역과 아닌 영역간의 경계가 너무 뚜렷하게 보이는 문제가 발생
		// 이런저런 처리를 더 해보려 했으나, 그림자 연산이 아닌 라이팅 연산을 하는 것만 같아 일단 보류
		// 더 좋은 방식이 있는지 생각해보자
		float attenuation = (PI / (lightDist * lightDist)) * g_shadowMap.fLightIntensity;
		attenuation = spotIntensity * attenuation * 0.1f;
		//if (attenuation > 0.01f)
		{
			attenuation = smoothstep(0.01f, 1.f, attenuation);
		
			float4 f4PositionLightCS = mul(float4(f3PositionW, 1.f), g_shadowMap.matViewProj);
		
			float fDepth = f4PositionLightCS.z / f4PositionLightCS.w;
		
			// Transform from light space to shadow map texture space.
			float2 f2ShadowTexCoord = f4PositionLightCS.xy / f4PositionLightCS.w * 0.5f + float2(0.5f, 0.5f);
			f2ShadowTexCoord.y = 1.f - f2ShadowTexCoord.y;
		
			fShadowLighting = CalcShadowTermPCF(fDepth, f2ShadowTexCoord, g_shadowMap.n2PCFBlurSize, g_shadowMap.f2TexelOffset, g_shadowMap.fDepthBias);
			fShadowLighting = (1.f - fShadowLighting) * attenuation;
		
			//fShadowLighting = 1.f - fShadowLighting;
		
			fMask = 0.01f;
		}
	}
	
	return float2(fShadowLighting, fMask);
}

float GetDepthByLightDir(in float3 positionToLight, in float texSize)
{
	float3 absVec = abs(positionToLight);
	float localZComp = max(absVec.x, max(absVec.y, absVec.z));

	const float f = texSize;
	const float n = 1.f;
	float normalZComp = (f + n) / (f - n) - (2.f * f * n) / (f - n) / localZComp;
	return (normalZComp + 1.f) * 0.5f;
}

float2 CalculateShadowCubeMap(in float3 f3PositionW)
{
	float3 lightDir = f3PositionW - g_shadowCubeMap.f3LightPos;
	//float fDepth = GetDepthByLightDir(lightDir, g_shadowCubeMap.fFarPlane);
	float lightDist = length(lightDir);
	//lightDir /= lightDist;
	lightDir = normalize(lightDir);

	float fShadowLighting = 0.f;
	float fMask = 0.f;

	float attenuation = (PI / (lightDist * lightDist)) * g_shadowCubeMap.fLightIntensity;
	if (attenuation > PI)
	{
		// 더 그럴싸한 방법 없늬?
		attenuation -= PI + 1e-5f;
		attenuation *= 0.1f;

		// Compare the transformed pixel depth to the depth read from the map.
		//float depthcompare = fDepth - g_shadowCubeMap.fDepthBias;
		//fShadowLighting = g_texShadowCubeMap.SampleCmpLevelZero(g_samShadow, lightDir, depthcompare);
		fShadowLighting = g_texShadowCubeMap.Sample(g_samShadow2, lightDir);

		//if (lightDist < fShadowLighting * g_shadowCubeMap.fFarPlane + g_shadowCubeMap.fDepthBias)
		//{
		//	fShadowLighting = 1.f;
		//}

		//fShadowLighting = (1.f - fShadowLighting) * attenuation;
		fShadowLighting = (1.f - fShadowLighting);

		//fShadowLighting = 1.f - fShadowLighting;

		fMask = 0.01f;
	}

	return float2(fShadowLighting, fMask);
}

#endif
