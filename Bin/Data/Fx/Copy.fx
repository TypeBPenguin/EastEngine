#ifndef _Copy_
#define _Copy_

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
};

sampler g_sampler;
Texture2D g_texture;

PS_INPUT VS(uint id : SV_VertexID)
{
	PS_INPUT output;
	output.tex = float2(((id << 1) & 2) != 0, (id & 2) != 0);
	output.pos = float4(output.tex * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);
	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return g_texture.Sample(g_sampler, input.tex);
}

technique11 Copy
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

#endif