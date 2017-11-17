#ifndef _CONVERTER_
#define _CONVERTER_

#define STEREOSCALE 1.77777f
#define INVSTEREOSCALE (1.f / STEREOSCALE)

float3 CalcWorldSpacePosFromDepth(in float depth, in float2 uv, in float4x4 matInvView, in float4x4 matInvProj)
{
	float x = uv.x * 2.f - 1.f;
	float y = (1.f - uv.y) * 2.f - 1.f;
	float z = depth;

	float4 clipSpacePosition = float4(x, y, z, 1.f);
	float4 viewSpacePosition = mul(clipSpacePosition, matInvProj);
	viewSpacePosition /= viewSpacePosition.w;

	float4 worldSpacePosition = mul(viewSpacePosition, matInvView);

	return worldSpacePosition.xyz;
}

float3 CalcWorldSpacePosFromDepth(in float depth, in float2 uv, in float4x4 matInvView, in float4x4 matInvProj, out float4 viewSpacePosition_out)
{
	float x = uv.x * 2.f - 1.f;
	float y = (1.f - uv.y) * 2.f - 1.f;
	float z = depth;

	float4 clipSpacePosition = float4(x, y, z, 1.f);
	viewSpacePosition_out = mul(clipSpacePosition, matInvProj);
	viewSpacePosition_out /= viewSpacePosition_out.w;

	float4 worldSpacePosition = mul(viewSpacePosition_out, matInvView);

	return worldSpacePosition.xyz;
}

float3 CalcViewSpcaePosFromDepth(in float depth, in float2 uv, in float4x4 matInvProj)
{
	float x = uv.x * 2.f - 1.f;
	float y = (1.f - uv.y) * 2.f - 1.f;
	float z = depth;

	float4 clipSpacePosition = float4(x, y, z, 1.f);
	float4 viewSpacePosition = mul(clipSpacePosition, matInvProj);
	viewSpacePosition /= viewSpacePosition.w;

	return viewSpacePosition.xyz;
}

float2 CompressNormal(in float3 normal)
{
	normal = normalize(-(normal + 1e-5f));
	float2 n = (normal.xy / (normal.z + 1.f)) * INVSTEREOSCALE;

	return n;
}

float3 DeCompressNormal(in float2 normal)
{
	float3 nn;
	nn.xy = (normal + 1e-5f) * STEREOSCALE;
	nn.z = 1.f;

	float g = 2.f / dot(nn, nn);

	float3 n;
	n.xy = g * nn.xy;
	n.z = g - 1.f;

	return normalize(-n);
}

half2 FloatToHalf2(in float f1)
{
	uint n = asuint(f1);

	half2 h;
	h.x = f16tof32(n & 0x0000ffff);
	h.y = f16tof32(n >> 16);

	return h;
}

float Half2ToFloat(in float h1, in float h2)
{
	uint n1 = f32tof16(h1);
	uint n2 = f32tof16(h2);

	uint n;
	n = n1;
	n |= n2 << 16;

	return asfloat(n);
}

float Pack3PNForFP32(float3 channel)
{
	// layout of a 32-bit fp register
	// SEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM
	// 1 sign bit; 8 bits for the exponent and 23 bits for the mantissa
	uint uValue;

	// pack x
	uValue = ((uint)(channel.x * 65535.0 + 0.5)); // goes from bit 0 to 15

	// pack y in EMMMMMMM
	uValue |= ((uint)(channel.y * 255.0 + 0.5)) << 16;

	// pack z in SEEEEEEE
	// the last E will never be 1b because the upper value is 254
	// max value is 11111110 == 254
	// this prevents the bits of the exponents to become all 1
	// range is 1.. 254
	// to prevent an exponent that is 0 we add 1.0
	uValue |= ((uint)(channel.z * 253.0 + 1.5)) << 24;

	return asfloat(uValue);
}

float3 Unpack3PNFromFP32(float fFloatFromFP32)
{
	float a, b, c, d;
	uint uValue;

	uint uInputFloat = asuint(fFloatFromFP32);

	// unpack a
	// mask out all the stuff above 16-bit with 0xFFFF
	a = ((uInputFloat) & 0xFFFF) / 65535.0;

	b = ((uInputFloat >> 16) & 0xFF) / 255.0;

	// extract the 1..254 value range and subtract 1
	// ending up with 0..253
	c = (((uInputFloat >> 24) & 0xFF) - 1.0) / 253.0;

	return float3(a, b, c);
}

float3 CalcTangent(in float3 normal)
{
	float3 tangent;
	float3 c1 = cross(normal, float3(0.f, 0.f, 1.f));
	float3 c2 = cross(normal, float3(0.f, 1.f, 0.f));

	if (length(c1) > length(c2))
	{
		tangent = c1;
	}
	else
	{
		tangent = c2;
	}

	tangent = normalize(-tangent);

	return tangent;
}

float3 CalcBinormal(in float3 normal, in float3 tangent)
{
	float3 binormal = cross(normal, tangent);
	binormal = normalize(-binormal);

	return binormal;
}

#endif