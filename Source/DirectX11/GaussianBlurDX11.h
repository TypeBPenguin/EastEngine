#pragma once

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			class RenderTarget;
			class DepthStencil;

			class GaussianBlur
			{
			public:
				GaussianBlur();
				~GaussianBlur();

			public:
				void Apply(const RenderTarget* pSource, RenderTarget* pResult, float fSigma);
				void Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult, float fSigma);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}