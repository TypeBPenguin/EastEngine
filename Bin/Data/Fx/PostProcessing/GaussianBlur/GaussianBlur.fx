cbuffer cbContents
{
	float g_fSigma = 0.5f;
	float2 g_f2SourceDimensions;
};

Texture2D g_texColor;
sampler g_samplerPoint;

Texture2D<float> g_texDepth;

float CalcGaussianWeight(int nSamplePoint)
{
	float g = 1.f / sqrt(2.f * 3.14159f * g_fSigma * g_fSigma);
	return (g * exp(-(nSamplePoint * nSamplePoint) / (2.f * g_fSigma * g_fSigma)));
}

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

float4 GaussianBlurH_PS(PS_INPUT input, uniform int nRadius) : SV_Target0
{
	float4 f4Color = 0.f;
	float2 f2Tex = input.tex;

	for (int i = -nRadius; i < nRadius; ++i)
	{
		float fWeight = CalcGaussianWeight(i);
		f2Tex.x = input.tex.x + (i / g_f2SourceDimensions.x);

		float4 f4Sample = g_texColor.Sample(g_samplerPoint, f2Tex);
		f4Color += f4Sample * fWeight;
	}

	return f4Color;
}

float4 GaussianBlurV_PS(PS_INPUT input, uniform int nRadius) : SV_Target0
{
	float4 f4Color = 0.f;
	float2 f2Tex = input.tex;

	for (int i = -nRadius; i < nRadius; ++i)
	{
		float fWeight = CalcGaussianWeight(i);
		f2Tex.y = input.tex.y + (i / g_f2SourceDimensions.y);

		float4 f4Sample = g_texColor.Sample(g_samplerPoint, f2Tex);
		f4Color += f4Sample * fWeight;
	}

	return f4Color;
}

float4 GaussianDepthBlurH_PS(PS_INPUT input, uniform int nRadius) : SV_Target0
{
	float4 f4Color = 0.f;
	float2 f2Tex = input.tex;
	float4 f4CenterColor = g_texColor.Sample(g_samplerPoint, input.tex);
	float fCenterDepth = g_texDepth.Sample(g_samplerPoint, input.tex).x;

	{
		[unroll]
		for (int i = -nRadius; i < 0; ++i)
		{
			f2Tex.x = input.tex.x + (i / g_f2SourceDimensions.x);
			float fDepth = g_texDepth.Sample(g_samplerPoint, f2Tex).x;
			float fWeight = CalcGaussianWeight(i);

			if (fDepth >= fCenterDepth)
			{
				float4 f4Sample = g_texColor.Sample(g_samplerPoint, f2Tex);
				f4Color += f4Sample * fWeight;
			}
			else
			{
				f4Color += f4CenterColor * fWeight;
			}
		}
	}

	{
		[unroll]
		for (int i = 1; i < nRadius; ++i)
		{
			f2Tex.x = input.tex.x + (i / g_f2SourceDimensions.x);
			float fDepth = g_texDepth.Sample(g_samplerPoint, f2Tex).x;
			float fWeight = CalcGaussianWeight(i);

			if (fDepth >= fCenterDepth)
			{
				float4 f4Sample = g_texColor.Sample(g_samplerPoint, f2Tex);
				f4Color += f4Sample * fWeight;
			}
			else
			{
				f4Color += f4CenterColor * fWeight;
			}
		}
	}

	f4Color += f4CenterColor * CalcGaussianWeight(0);

	return f4Color;
}

float4 GaussianDepthBlurV_PS(PS_INPUT input, uniform int nRadius) : SV_Target0
{
	float4 f4Color = 0.f;
	float2 f2Tex = input.tex;
	float4 f4CenterColor = g_texColor.Sample(g_samplerPoint, input.tex);
	float fCenterDepth = g_texDepth.Sample(g_samplerPoint, input.tex).x;

	{
		[unroll]
		for (int i = -nRadius; i < 0; ++i)
		{
			f2Tex.y = input.tex.y + (i / g_f2SourceDimensions.y);
			float fDepth = g_texDepth.Sample(g_samplerPoint, f2Tex).x;
			float fWeight = CalcGaussianWeight(i);
			
			if (fDepth >= fCenterDepth)
			{
				float4 f4Sample = g_texColor.Sample(g_samplerPoint, f2Tex);
				f4Color += f4Sample * fWeight;
			}
			else
			{
				f4Color += f4CenterColor * fWeight;
			}
		}
	}

	{
		[unroll]
		for (int i = 1; i < nRadius; ++i)
		{
			f2Tex.y = input.tex.x + (i / g_f2SourceDimensions.y);
			float fDepth = g_texDepth.Sample(g_samplerPoint, f2Tex).x;
			float fWeight = CalcGaussianWeight(i);

			if (fDepth >= fCenterDepth)
			{
				float4 f4Sample = g_texColor.Sample(g_samplerPoint, f2Tex);
				f4Color += f4Sample * fWeight;
			}
			else
			{
				f4Color += f4CenterColor * fWeight;
			}
		}
	}

	f4Color += f4CenterColor * CalcGaussianWeight(0);

	return f4Color;
}

technique11 GaussianBlurH
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, GaussianBlurH_PS(6)));
	}
}

technique11 GaussianBlurV
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, GaussianBlurV_PS(6)));
	}
}

technique11 GaussianDepthBlurH
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, GaussianDepthBlurH_PS(6)));
	}
}

technique11 GaussianDepthBlurV
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, GaussianDepthBlurV_PS(6)));
	}
}