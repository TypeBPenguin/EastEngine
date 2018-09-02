#pragma once

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class RenderTarget;
			class DepthStencil;

			class SSS
			{
			public:
				SSS();
				~SSS();

			public:
				void Apply(ID3D12GraphicsCommandList2* pCommandList, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}