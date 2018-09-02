#pragma once

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			class RenderTarget;
			class DepthStencil;

			class Assao
			{
			public:
				Assao();
				~Assao();

			public:
				void Apply(const Camera* pCamera, const RenderTarget* pNormalMap, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}