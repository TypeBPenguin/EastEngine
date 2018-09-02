#pragma once

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			class RenderTarget;

			class Fxaa
			{
			public:
				Fxaa();
				~Fxaa();

			public:
				void Apply(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}