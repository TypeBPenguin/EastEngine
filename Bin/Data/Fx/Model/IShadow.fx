#ifndef _ISHADOW_INTERFACE_
#define _ISHADOW_INTERFACE_

#ifndef USE_SHADOW_MAP_COUNT
#define USE_SHADOW_MAP_COUNT 1
#endif

Texture2D<float> g_texShadowMaps[USE_SHADOW_MAP_COUNT];
SamplerComparisonState g_samShadow;

interface IShadow
{
	float CalcShadow(in uint nIndex, in float3 f3PositionW, in float3 f3Normal);
};

class CCascadeShadow : IShadow
{
	float4x4 m_matCameraView;
	float4x4 m_matCascadeViewProj[8];

	float3 m_f3CascadeAreas[8][2];

	float m_nCascadeLevel;
	float2 m_n2PCFBlurSize;
	float m_fDepthBias;
	float2 m_f2TexelOffset;

	float CalcShadow(in uint nIndex, in float3 f3PositionW, in float3 f3Normal)
	{
		float4 f4PositionWV = mul(float4(f3PositionW, 1.f), m_matCameraView);

		float fShadowLighting = 0.f;

		int nCascadeLevel = ComputeCascadeIndex(f4PositionWV.xyz);
		if (nCascadeLevel >= 0)
		{
			float4 f4PositionLightCS = mul(float4(f3PositionW, 1.f), m_matCascadeViewProj[nCascadeLevel]);

			float fLightDepth = f4PositionLightCS.z / f4PositionLightCS.w;

			// Transform from light space to shadow map texture space.
			float2 f2ShadowTexCoord = f4PositionLightCS.xy / f4PositionLightCS.w * 0.5f + float2(0.5f, 0.5f);
			f2ShadowTexCoord.x = f2ShadowTexCoord.x / m_nCascadeLevel + (nCascadeLevel / (float)m_nCascadeLevel);
			f2ShadowTexCoord.y = 1.f - f2ShadowTexCoord.y;

			// Offset the coordinate by half a texel so we sample it correctly
			f2ShadowTexCoord += 0.5f / 1024 * m_nCascadeLevel;

			float fPercentLit = CalcShadowTermPCF(nIndex, fLightDepth, f2ShadowTexCoord, nCascadeLevel);

			//float fPercentLit_blend = 0.f;
			//if (nCascadeLevel < m_nCascadeLevel)
			//{
			//	int nNextCascadeLevel = nCascadeLevel + 1;
			//	// Transform from light space to shadow map texture space.
			//	float2 f2ShadowTexCoord = f4PositionLightCS.xy / f4PositionLightCS.w * 0.5f + float2(0.5f, 0.5f);
			//	f2ShadowTexCoord.x = f2ShadowTexCoord.x / m_nCascadeLevel + (nNextCascadeLevel / (float)m_nCascadeLevel);
			//	f2ShadowTexCoord.y = 1.f - f2ShadowTexCoord.y;

			//	// Offset the coordinate by half a texel so we sample it correctly
			//	f2ShadowTexCoord += 0.5f / 1024 * m_nCascadeLevel;

			//	float fBlendBetweenCascadesAmount = 1.f;
			//	float fCurrentPixelsBlendBandLocation = 1.f;

			//	CalculateBlendAmountForMap(f2ShadowTexCoord, fCurrentPixelsBlendBandLocation, fBlendBetweenCascadesAmount);
			//	if (fCurrentPixelsBlendBandLocation < 0.005f)
			//	{
			//		int iBlurRowSize = m_nPCFBlurForLoop.y - m_nPCFBlurForLoop.x;
			//		iBlurRowSize *= iBlurRowSize;
			//		float fBlurRowSize = (float)iBlurRowSize;
			//		float fPercentLit_blend = CalcShadowTermPCF(fLightDepth, f2ShadowTexCoord, fBlurRowSize, nNextCascadeLevel);
			//		fPercentLit = lerp(fPercentLit_blend, fPercentLit, fBlendBetweenCascadesAmount);
			//	}
			//}

			const float3 f3LightDir1 = normalize(float3(-1.f, 1.f, -1.f));
			const float3 f3LightDir2 = normalize(float3(1.f, 1.f, -1.f));
			const float3 f3LightDir3 = normalize(float3(0.f, -1.f, 0.f));
			const float3 f3LightDir4 = normalize(float3(1.f, 1.f, 1.f));
			// Some ambient-like lighting.
			float fLighting = saturate(dot(f3LightDir1, f3Normal)) * 0.05f +
				saturate(dot(f3LightDir2, f3Normal)) * 0.05f +
				saturate(dot(f3LightDir3, f3Normal)) * 0.05f +
				saturate(dot(f3LightDir4, f3Normal)) * 0.05f;

			fShadowLighting = fLighting * 0.5f;
			fShadowLighting = lerp(fShadowLighting, fLighting, fPercentLit);
		}

		return fShadowLighting;
	}

	int ComputeCascadeIndex(float3 f3Pos)
	{
		for (int i = 0; i < m_nCascadeLevel; ++i)
		{
			if (f3Pos.x >= m_f3CascadeAreas[i][0].x &&
				f3Pos.x < m_f3CascadeAreas[i][1].x &&
				f3Pos.y >= m_f3CascadeAreas[i][0].y &&
				f3Pos.y < m_f3CascadeAreas[i][1].y &&
				f3Pos.z >= m_f3CascadeAreas[i][0].z &&
				f3Pos.z < m_f3CascadeAreas[i][1].z)
			{
				return i;
			}
		}

		return -1;
	}

	// Calculates the shadow occlusion using bilinear PCF
	float CalcShadowTermPCF(in uint nIndex, in float fLightDepth, in float2 f2ShadowTexCoord, in int nCascadeLevel)
	{
		float fPercentLit = 0.f;
		// This loop could be unrolled, and texture immediate offsets could be used if the kernel size were fixed.
		// This would be performance improvment.

		float depthcompare = fLightDepth;
		// A very simple solution to the depth bias problems of PCF is to use an offset.
		// Unfortunately, too much offset can lead to Peter-panning (shadows near the base of object disappear )
		// Too little offset can lead to shadow acne ( objects that should not be in shadow are partially self shadowed ).
		depthcompare -= m_fDepthBias;

		for (int x = m_n2PCFBlurSize.x; x < m_n2PCFBlurSize.y; ++x)
		{
			for (int y = m_n2PCFBlurSize.x; y < m_n2PCFBlurSize.y; ++y)
			{
				// Compare the transformed pixel depth to the depth read from the map.
				fPercentLit += g_texShadowMaps[nIndex].SampleCmpLevelZero(g_samShadow,
					float2(f2ShadowTexCoord.x + (((float)x) * m_f2TexelOffset.x),
						f2ShadowTexCoord.y + (((float)y) * m_f2TexelOffset.y)),
					depthcompare);
			}
		}

		int iBlurRowSize = m_n2PCFBlurSize.y - m_n2PCFBlurSize.x;
		iBlurRowSize *= iBlurRowSize;
		fPercentLit /= (float)(iBlurRowSize);

		return fPercentLit;
	}

	void CalculateBlendAmountForMap(in float2 vShadowMapTextureCoord,
		in out float fCurrentPixelsBlendBandLocation,
		out float fBlendBetweenCascadesAmount)
	{
		// Calcaulte the blend band for the map based selection.
		float2 distanceToOne = float2 (1.f - vShadowMapTextureCoord.x, 1.f - vShadowMapTextureCoord.y);
		fCurrentPixelsBlendBandLocation = min(vShadowMapTextureCoord.x, vShadowMapTextureCoord.y);
		float fCurrentPixelsBlendBandLocation2 = min(distanceToOne.x, distanceToOne.y);
		fCurrentPixelsBlendBandLocation =
			min(fCurrentPixelsBlendBandLocation, fCurrentPixelsBlendBandLocation2);
		fBlendBetweenCascadesAmount = fCurrentPixelsBlendBandLocation / 0.005f;
	}
};

IShadow g_abstractShadow;

cbuffer cbShadow
{
	CCascadeShadow g_cascadeShadow;
};

#endif