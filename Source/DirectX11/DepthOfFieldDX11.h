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

			class DepthOfField
			{
			public:
				DepthOfField();
				~DepthOfField();

			public:
				void Apply(Camera* pCamera, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}