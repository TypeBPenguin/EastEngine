#define eMaxCascades 5

struct CascadedShadow
{
	int2 pcfBlurSize;
	float2 texelOffset;

	uint numCascades;
	float depthBias;
	float2 padding;

	float4 viewPos[eMaxCascades];
	float4 splitDepths[eMaxCascades];
	float4x4 cascadeViewProjectionMatrix[eMaxCascades];
	float4x4 cascadeViewLinearProjectionMatrix[eMaxCascades];
};

int ComputeCascadeIndex(in CascadedShadow cascadedShadow, in float3 f3PosVS)
{
	for (uint i = 0; i < cascadedShadow.numCascades; ++i)
	{
		if (cascadedShadow.splitDepths[i].x <= f3PosVS.z && f3PosVS.z < cascadedShadow.splitDepths[i].y)
			return i;
	}
	return -1;
}

// Calculates the shadow occlusion using bilinear PCF
float CalcShadowTermPCF(in Texture2D cascadeShadowMap, in SamplerComparisonState samplerShadow, in float depth, in float2 shadowTexCoord, in int2 pcfBlurSize, in float2 texelOffset, in float depthBias, in float4 projCoords)
{
	float percentLit = 0.f;
	// This loop could be unrolled, and texture immediate offsets could be used if the kernel size were fixed.
	// This would be performance improvment.

	// A very simple solution to the depth bias problems of PCF is to use an offset.
	// Unfortunately, too much offset can lead to Peter-panning (shadows near the base of object disappear )
	// Too little offset can lead to shadow acne ( objects that should not be in shadow are partially self shadowed ).
	//float depthcompare = depth - depthBias;

	float4 duvdist_dx = ddx(projCoords);
	float4 duvdist_dy = ddy(projCoords);
	
	float invDet = 1 / ((duvdist_dx.x * duvdist_dy.y) - (duvdist_dx.y * duvdist_dy.x));

	//Top row of 2x2
	float2 ddist_duv = 0.f;
	ddist_duv.x = duvdist_dy.y * duvdist_dx.w; // invJtrans[0][0] * ddist_dx
	ddist_duv.x -= duvdist_dx.y * duvdist_dy.w; // invJtrans[0][1] * ddist_dy
	//Bottom row of 2x2
	ddist_duv.y = duvdist_dx.x * duvdist_dy.w; // invJtrans[1][1] * ddist_dy
	ddist_duv.y -= duvdist_dy.x * duvdist_dx.w; // invJtrans[1][0] * ddist_dx
	ddist_duv *= invDet;

	int blurSize = pcfBlurSize.y - pcfBlurSize.x;
	float halfBlurSize = blurSize == 1 ? 0.f : (float)blurSize * -0.5f;

	float biasOffset = 1.f + (float)blurSize * 0.1f;

	for (int x = 0; x < blurSize; ++x)
	{
		for (int y = 0; y < blurSize; ++y)
		{
			float2 texCoordOffset = 0.f;
			texCoordOffset.x = (halfBlurSize + float(x)) * texelOffset.x;
			texCoordOffset.y = (halfBlurSize + float(y)) * texelOffset.y;

			float2 uv = float2(shadowTexCoord.x + texCoordOffset.x, shadowTexCoord.y + texCoordOffset.y);

			float depthcompare = depth - (biasOffset * (depthBias + ((ddist_duv.x * texCoordOffset.x) + (ddist_duv.y * texCoordOffset.y))));
			percentLit += cascadeShadowMap.SampleCmpLevelZero(samplerShadow, uv, depthcompare);
		}
	}

	//for (int x = pcfBlurSize.x; x < pcfBlurSize.y; ++x)
	//{
	//	for (int y = pcfBlurSize.x; y < pcfBlurSize.y; ++y)
	//	{
	//		float fOffsetX = (float)x;
	//		float fOffsetY = (float)y;
	//
	//		float2 uv = float2(shadowTexCoord.x + (fOffsetX * texelOffset.x),
	//			shadowTexCoord.y + (fOffsetY * texelOffset.y));
	//
	//		// Compare the transformed pixel depth to the depth read from the map.
	//		percentLit += cascadeShadowMap.SampleCmpLevelZero(samplerShadow, uv, depthcompare);
	//	}
	//}

	int iBlurRowSize = pcfBlurSize.y - pcfBlurSize.x;
	iBlurRowSize *= iBlurRowSize;
	percentLit /= (float)(iBlurRowSize);

	return percentLit;
}

float CalculateCascadedShadow(in Texture2D cascadeShadowMap, in CascadedShadow cascadedShadow, in SamplerComparisonState samplerShadow, in float3 positionW, in float3 positionWV)
{
	float shadowIntensity = 0.f;

	int cascadeLevel = ComputeCascadeIndex(cascadedShadow, positionWV);
	if (0 <= cascadeLevel && cascadeLevel < eMaxCascades)
	{
		float4 positionLightCS = mul(float4(positionW, 1.f), cascadedShadow.cascadeViewProjectionMatrix[cascadeLevel]);
		float4 positionLightLS = mul(float4(positionW, 1.f), cascadedShadow.cascadeViewLinearProjectionMatrix[cascadeLevel]);

		//float depth = positionLightCS.z / positionLightCS.w;
		float depth = positionLightLS.z * positionLightLS.w;

		// Transform from light space to shadow map texture space.
		float2 shadowTexCoord = positionLightCS.xy / positionLightCS.w * 0.5f + float2(0.5f, 0.5f);
		shadowTexCoord.x = shadowTexCoord.x / (float)cascadedShadow.numCascades + ((float)cascadeLevel / (float)cascadedShadow.numCascades);
		shadowTexCoord.y = 1.f - shadowTexCoord.y;

		shadowIntensity = CalcShadowTermPCF(cascadeShadowMap, samplerShadow, depth, shadowTexCoord, cascadedShadow.pcfBlurSize, cascadedShadow.texelOffset, cascadedShadow.depthBias, positionLightCS);
	}

	return shadowIntensity;
}

//#ifndef _SHADOW_INTERFACE_
//#define _SHADOW_INTERFACE_
//
//#define PI 3.14159265359f
//
//Texture2D g_texShadowMap : register(t1);
//TextureCube g_texShadowCubeMap : register(t2);
//SamplerComparisonState g_samShadow : register(s1);
//SamplerState g_samShadow2 : register(s2);
//
//struct CascadedShadow
//{
//	int2 pcfBlurSize;
//	float2 texelOffset;
//
//	int cascadeLevel;
//	float depthBias;
//	float2 padding;
//
//	float4x4 matCascadeViewProj[8];
//	float4 splitDepths[8];
//};
//
//cbuffer cbCascadedShadow
//{
//	CascadedShadow g_cascadeShadow;
//};
//
//struct ShadowMap
//{
//	int2 pcfBlurSize;
//	float2 texelOffset;
//
//	float depthBias;
//	float3 lightPosition;
//
//	float3 lighthDirection;
//	float lightAngle;
//
//	float lightIntensity;
//	float3 padding;
//	
//	float4x4 matViewProj;
//};
//
//struct ShadowCubeMap
//{
//	int2 pcfBlurSize;
//	float2 texelOffset;
//
//	float depthBias;
//	float3 lightPosition;
//
//	float lightIntensity;
//	float farPlane;
//	float2 padding;
//};
//
//cbuffer cbShadowMap
//{
//	ShadowMap g_shadowMap;
//	ShadowCubeMap g_shadowCubeMap;
//};
//
//int ComputeCascadeIndex(in CascadedShadow cascadedShadow, in float3 f3PosVS)
//{
//	for (int i = 0; i < cascadedShadow.cascadeLevel; ++i)
//	{
//		if (cascadedShadow.splitDepths[i].x <= f3PosVS.z && f3PosVS.z < cascadedShadow.splitDepths[i].y)
//			return i;
//	}
//	return -1;
//}
//
//// Calculates the shadow occlusion using bilinear PCF
//float CalcShadowTermPCF(in float depth, in float2 shadowTexCoord, in int2 pcfBlurSize, in float2 texelOffset, in float depthBias)
//{
//	float percentLit = 0.f;
//	// This loop could be unrolled, and texture immediate offsets could be used if the kernel size were fixed.
//	// This would be performance improvment.
//
//	// A very simple solution to the depth bias problems of PCF is to use an offset.
//	// Unfortunately, too much offset can lead to Peter-panning (shadows near the base of object disappear )
//	// Too little offset can lead to shadow acne ( objects that should not be in shadow are partially self shadowed ).
//	float depthcompare = depth - depthBias;
//
//	for (int x = pcfBlurSize.x; x < pcfBlurSize.y; ++x)
//	{
//		for (int y = pcfBlurSize.x; y < pcfBlurSize.y; ++y)
//		{
//			float fOffsetX = (float)x;
//			float fOffsetY = (float)y;
//
//			float2 uv = float2(shadowTexCoord.x + (fOffsetX * texelOffset.x),
//				shadowTexCoord.y + (fOffsetY * texelOffset.y));
//
//			// Compare the transformed pixel depth to the depth read from the map.
//			percentLit += g_texShadowMap.SampleCmpLevelZero(g_samShadow, uv, depthcompare);
//		}
//	}
//
//	int iBlurRowSize = pcfBlurSize.y - pcfBlurSize.x;
//	iBlurRowSize *= iBlurRowSize;
//	percentLit /= (float)(iBlurRowSize);
//
//	return percentLit;
//}
//
//float CalculateCascadedShadow(in CascadedShadow cascadedShadow, in float3 positionW, in float3 positionWV)
//{
//	float shadowLighting = 1.f;
//
//	int cascadeLevel = ComputeCascadeIndex(cascadedShadow, positionWV);
//	if (0 <= cascadeLevel && cascadeLevel < eMaxCascades)
//	{
//		float4 positionLightCS = mul(float4(positionW, 1.f), cascadedShadow.matCascadeViewProj[cascadeLevel]);
//
//		float depth = positionLightCS.z / positionLightCS.w;
//
//		// Transform from light space to shadow map texture space.
//		float2 shadowTexCoord = positionLightCS.xy / positionLightCS.w * 0.5f + float2(0.5f, 0.5f);
//		shadowTexCoord.x = shadowTexCoord.x / (float)cascadedShadow.cascadeLevel + ((float)cascadeLevel / (float)cascadedShadow.cascadeLevel);
//		shadowTexCoord.y = 1.f - shadowTexCoord.y;
//
//		shadowLighting = 1.f - CalcShadowTermPCF(depth, shadowTexCoord, cascadedShadow.pcfBlurSize, cascadedShadow.texelOffset, cascadedShadow.depthBias);
//	}
//
//	return shadowLighting;
//}
//
//float2 CalculateShadowMap(in float3 positionW)
//{
//	float3 lightDir = g_shadowMap.lightPosition - positionW;
//	float lightDist = length(lightDir);
//	lightDir /= lightDist;
//
//	float minCos = cos(radians(g_shadowMap.lightAngle));
//	float maxCos = lerp(minCos, 1.f, 0.5f);
//
//	float cosAngle = dot(-g_shadowMap.lighthDirection, lightDir);
//
//	float shadowLighting = 0.f;
//	float mask = 0.f;
//
//	float spotIntensity = smoothstep(minCos, maxCos, cosAngle);
//	if (spotIntensity > 0.001f)
//	{
//		spotIntensity -= 0.001f + 1e-5f;
//
//		// SpotLight 특성상 라이트에서 멀어질수록 빛이 약해지기 때문에 그림자도 약해지도록 처리하려고 했음
//		// 하지만 빛의 영역과 아닌 영역간의 경계가 너무 뚜렷하게 보이는 문제가 발생
//		// 이런저런 처리를 더 해보려 했으나, 그림자 연산이 아닌 라이팅 연산을 하는 것만 같아 일단 보류
//		// 더 좋은 방식이 있는지 생각해보자
//		float attenuation = (PI / (lightDist * lightDist)) * g_shadowMap.lightIntensity;
//		attenuation = spotIntensity * attenuation * 0.1f;
//		//if (attenuation > 0.01f)
//		{
//			attenuation = smoothstep(0.01f, 1.f, attenuation);
//		
//			float4 positionLightCS = mul(float4(positionW, 1.f), g_shadowMap.matViewProj);
//		
//			float depth = positionLightCS.z / positionLightCS.w;
//		
//			// Transform from light space to shadow map texture space.
//			float2 shadowTexCoord = positionLightCS.xy / positionLightCS.w * 0.5f + float2(0.5f, 0.5f);
//			shadowTexCoord.y = 1.f - shadowTexCoord.y;
//		
//			shadowLighting = CalcShadowTermPCF(depth, shadowTexCoord, g_shadowMap.pcfBlurSize, g_shadowMap.texelOffset, g_shadowMap.depthBias);
//			shadowLighting = (1.f - shadowLighting) * attenuation;
//		
//			//shadowLighting = 1.f - shadowLighting;
//		
//			mask = 0.01f;
//		}
//	}
//	
//	return float2(shadowLighting, mask);
//}
//
//float GetDepthByLightDir(in float3 positionToLight, in float texSize)
//{
//	float3 absVec = abs(positionToLight);
//	float localZComp = max(absVec.x, max(absVec.y, absVec.z));
//
//	const float f = texSize;
//	const float n = 1.f;
//	float normalZComp = (f + n) / (f - n) - (2.f * f * n) / (f - n) / localZComp;
//	return (normalZComp + 1.f) * 0.5f;
//}
//
//float2 CalculateShadowCubeMap(in float3 positionW)
//{
//	float3 lightDir = positionW - g_shadowCubeMap.lightPosition;
//	//float depth = GetDepthByLightDir(lightDir, g_shadowCubeMap.farPlane);
//	float lightDist = length(lightDir);
//	//lightDir /= lightDist;
//	lightDir = normalize(lightDir);
//
//	float shadowLighting = 0.f;
//	float mask = 0.f;
//
//	float attenuation = (PI / (lightDist * lightDist)) * g_shadowCubeMap.lightIntensity;
//	if (attenuation > PI)
//	{
//		// 더 그럴싸한 방법 없늬?
//		attenuation -= PI + 1e-5f;
//		attenuation *= 0.1f;
//
//		// Compare the transformed pixel depth to the depth read from the map.
//		//float depthcompare = depth - g_shadowCubeMap.depthBias;
//		//shadowLighting = g_texShadowCubeMap.SampleCmpLevelZero(g_samShadow, lightDir, depthcompare);
//		shadowLighting = g_texShadowCubeMap.Sample(g_samShadow2, lightDir);
//
//		//if (lightDist < shadowLighting * g_shadowCubeMap.farPlane + g_shadowCubeMap.depthBias)
//		//{
//		//	shadowLighting = 1.f;
//		//}
//
//		//shadowLighting = (1.f - shadowLighting) * attenuation;
//		shadowLighting = (1.f - shadowLighting);
//
//		//shadowLighting = 1.f - shadowLighting;
//
//		mask = 0.01f;
//	}
//
//	return float2(shadowLighting, mask);
//}
//
//#endif
