#ifdef DX12
#include "../DescriptorTablesDX12.hlsl"
#endif

#ifdef DX11

#define TexEnvironmentMap(uv)	g_texEnvironmentMap.Sample(g_samplerAnisotropic, uv)
TextureCube g_texEnvironmentMap : register(t0);
SamplerState g_samplerAnisotropic : register(s0);

#elif DX12

#define TexEnvironmentMap(uv)	TexCubeTable[g_nTexEnvironmentMapIndex].Sample(g_samplerAnisotropic, uv)
SamplerState g_samplerAnisotropic : register(s0, space100);

#endif

cbuffer cbEnvironmentContents : register(b0)
{
	float4x4 g_matInvView;
	float4x4 g_matProjection;
	float g_textureGamma;

#ifdef DX11
	float3 padding;
#elif DX12
	uint g_nTexEnvironmentMapIndex;
	float2 padding;
#endif
};

struct VS_INPUT
{
	float4 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float3 normal : TEXCOORD1;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	input.position.w = 1.f;

	output.position = mul(input.position, g_matProjection);
	output.normal = normalize(mul(input.normal, (float3x3)g_matInvView));

	return output;
}

float4 PS(PS_INPUT input) : SV_Target0
{
	float4 textureGammaColor = TexEnvironmentMap(input.normal);

	float4 diffuseColor = float4(pow(abs(textureGammaColor), g_textureGamma).rgb, textureGammaColor.a);

	// alpha 값은 subsourface 로 사용된다.
	// 그래서 배경은 0.f
	return float4(diffuseColor.xyz, 0.f);
}