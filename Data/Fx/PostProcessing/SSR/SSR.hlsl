#include "../../Converter.hlsl"

#define TexColor(uv)	g_texColor.SampleLevel(SamplerLinearClamp, uv, 0)
#define TexNormal(uv)	g_texNormal.Sample(SamplerPointClamp, uv)
#define TexDisneyBRDF(uv)	g_texDisneyBRDF.Sample(SamplerPointClamp, uv)
#define TexDepth(uv)	g_texDepth.SampleLevel(SamplerPointClamp, uv, 0).r
#define TexNoise(uv)	g_texNoise.Sample(SamplerPointClamp, uv)
#define TexSSR(uv)	g_texSSR.Sample(SamplerPointClamp, uv)

Texture2D g_texColor : register(t0);
Texture2D g_texNormal : register(t1);
Texture2D g_texDisneyBRDF : register(t2);
Texture2D<float> g_texDepth : register(t3);
Texture2D g_texNoise : register(t4);

Texture2D g_texSSR : register(t1);

SamplerState SamplerLinearClamp : register(s0);
SamplerState SamplerPointClamp : register(s1);

cbuffer cbSSRContents : register(b0)
{
	float4x4 g_matInvView;
	float4x4 g_matInvProj;

	float4x4 g_matView;
	float4x4 g_matProj;

	float2 g_resolution;
	int Samples;
	float padding;
};

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
};

PS_INPUT VS(uint id : SV_VertexID)
{
	PS_INPUT output;
	output.tex = float2(((id << 1) & 2) != 0, (id & 2) != 0);
	output.pos = float4(output.tex * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);

	return output;
}

float3 randomNormal(float2 tex)
{
	tex = frac(tex);
	float noiseX = (frac(sin(dot(tex, float2(15.8989f, 76.132f) * 1.0f)) * 46336.23745f)) * 2 - 1;
	float noiseY = (frac(sin(dot(tex, float2(11.9899f, 62.223f) * 2.0f)) * 34748.34744f)) * 2 - 1;
	float noiseZ = (frac(sin(dot(tex, float2(13.3238f, 63.122f) * 3.0f)) * 59998.47362f)) * 2 - 1;
	return normalize(float3(noiseX, noiseY, noiseZ));
}

float4 PS_SSR(PS_INPUT input) : SV_Target0
{
	const float depth = TexDepth(input.tex);
	clip((1.f - 1e-5f) - depth);

	const float2 texCoord = float2(input.tex);

	const float4 normals = TexNormal(texCoord);
	float3 normal = DeCompressNormal(normals.xy);
	normal = mul(float4(normal, 0.f), g_matView).xyz;

	[branch]
	if (normal.x + normal.y < 0.001f)
	{
		discard;
	}

	const float4 disneyBRDF = TexDisneyBRDF(texCoord);
	const float3 RM = Unpack3PNFromFP32(disneyBRDF.x);
	const float roughness = RM.x;

	[branch]
	if (roughness > 0.9f)
	{
		discard;
	}

	float3 randNor = randomNormal(texCoord); // randomNormal(frac(mul(input.TexCoord, noise).xy)) * -randomNormal(frac(mul(1 - input.TexCoord, noise).xy)); //

	//hemisphere
	[branch]
	if (dot(randNor, normal) < 0.f)
	{
		randNor *= -1.f;
	}

	//Jitter the normal based on roughness to simulate microfacets. This should be updated to correctly map to lobes with some BRDF.
	normal = normalize(lerp(normal, randNor, roughness * roughness));

	static const float border = 0.1f;
	const float border2 = 1.f - border;
	const float bordermulti = 1.f / border;

	float4 positionVS = 0.f;
	const float3 positionWS = CalcWorldSpacePosFromDepth(depth, texCoord, g_matInvView, g_matInvProj, positionVS);

	const float3 incident = normalize(positionVS.xyz);
	const float3 reflectVector = reflect(incident, normal);

	float4 reflectVectorVPS = mul(float4(positionVS.xyz + reflectVector, 1.f), g_matProj);
	reflectVectorVPS.xyz /= reflectVectorVPS.w;

	const float2 reflectVectorUV = 0.5f * (float2(reflectVectorVPS.x, -reflectVectorVPS.y) + float2(1.f, 1.f));

	const float3 rayOrigin = float3(texCoord, depth);
	float3 rayStep = float3(reflectVectorUV - texCoord, reflectVectorVPS.z - depth);

	float xMultiplier = (rayStep.x > 0.f ? (1.f - texCoord.x) : -texCoord.x) / rayStep.x;
	float yMultiplier = (rayStep.y > 0.f ? (1.f - texCoord.y) : -texCoord.y) / rayStep.y;
	float multiplier = min(xMultiplier, yMultiplier) / Samples;
	rayStep *= multiplier;

	//Add some noise
	const float noise = TexNoise(frac((texCoord * g_resolution) / 64)).r;
	const float roughnessOffset = noise * max(0.1f, roughness * roughness) * 15.f;

	//Some variables we need later when precising the hit point
	float startingDepth = rayOrigin.z;
	float3 hitPosition = 0.f;
	float2 hitTexCoord = 0.f;

	float4 output = 0.f;

	[loop]
	for (int i = 1; i <= Samples; ++i)
	{
		//March a step
		float3 rayPosition = rayOrigin + (i - 0.5f + roughnessOffset) * rayStep;

		//We don't consider rays coming out of the screeen
		[branch]
		if (rayPosition.z < 0.f || rayPosition.z > 1.f)
			break;

		//Get the depth at our new position
		float sampleDepth = TexDepth(rayPosition.xy);
		const float depthDifference = sampleDepth - rayPosition.z;

		//needs normal looking to it!

		//Coming towards us, let's go back to linear depth!
		[branch]
		if (rayStep.z < 0.f && depthDifference < 0.f)
		{
			//Where are we currently in linDepth, note - in VS is + in VPS
			const float depthMinusThickness = sampleDepth - 0.001f;
			if (depthMinusThickness < rayPosition.z)
				continue;
		}

		//March backwards, idea -> binary searcH?
		[branch]
		if (depthDifference <= 0.f && sampleDepth >= startingDepth - rayStep.z * 0.5f) //sample < rayPosition.z
		{
			hitPosition = rayPosition;

			bool hit = false;

			float sampleDepthFirstHit = sampleDepth;
			float3 rayPositionFirstHit = rayPosition;

			static const int SecondarySamples = 4;

			//March backwards until we are outside again
			[loop]
			for (int j = 1; j <= SecondarySamples; ++j)
			{
				rayPosition = hitPosition - rayStep * j / (SecondarySamples);

				sampleDepth = TexDepth(rayPosition.xy) + 0.001f;

				//Looks like we don't hit anything any more?
				[branch]
				if (sampleDepth >= rayPosition.z)
				{
					//only z is relevant
					float origin = rayPositionFirstHit.z;

					//should be smaller
					float r = rayPosition.z - origin;
					float a = sampleDepthFirstHit - origin;
					float b = sampleDepth - origin;
					float x = (a) / (r - b + a);

					float sampleDepthLerped = lerp(sampleDepth, sampleDepthFirstHit, x);

					hitTexCoord = lerp(rayPosition.xy, rayPositionFirstHit.xy, x);

					hit = true;

					break;
				}

				sampleDepthFirstHit = sampleDepth;
				rayPositionFirstHit = rayPosition;
			}

			//We haven't hit anything we can travel further
			[branch]
			if (hit == false ||
				hitTexCoord.x < 0 || hitTexCoord.x > 1.f ||
				hitTexCoord.y < 0 || hitTexCoord.y > 1.f)
				continue;

			output.rgb = TexColor(hitTexCoord.xy).rgb;
			output.a = 1.f;

			//Fade out to the edges
			[branch]
			if (rayPosition.y > border2)
			{
				output.a = lerp(1.f, 0.f, (hitTexCoord.y - border2) * bordermulti);
			}
			else if (rayPosition.y < border)
			{
				output.a = lerp(0.f, 1.f, hitTexCoord.y * bordermulti);
			}

			[branch]
			if (rayPosition.x > border2)
			{
				output.a *= lerp(1.f, 0.f, (hitTexCoord.x - border2) * bordermulti);
			}
			else if (rayPosition.x < border)
			{
				output.a *= lerp(0.f, 1.f, hitTexCoord.x * bordermulti);
			}

			//Fade out to the front
			//float fade = saturate(1.f - reflectVector.z);
			//output.a *= (1.f - roughness) * fade;
			output.a *= (1.f - roughness);

			break;
		}

		startingDepth = rayPosition.z;
	}

	return output;
}

float4 PS_SSR_Color(PS_INPUT input) : SV_Target0
{
	float4 color = TexColor(input.tex);
	float4 ssr = TexSSR(input.tex);

	color.rgb = lerp(color.rgb, ssr.rgb, ssr.a);
	return color;
}