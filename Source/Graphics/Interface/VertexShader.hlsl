cbuffer ConstantBuffer : register(b0)
{
	float4x4 matWVP;
}

void main( in float4 inPos : POSITION, in float2 inTexCoord : TEXCOORD,
	out float4 outPos : SV_POSITION, out float2 outTexCoord : TEXCOORD )
{
	inPos.w = 1.f;

	outPos = mul(inPos, matWVP);
	outTexCoord = inTexCoord;
}