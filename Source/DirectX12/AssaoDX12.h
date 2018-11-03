#pragma once

#include "RendererDX12.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class RenderTarget;
			class DepthStencil;

			class Assao : public IRendererDX12
			{
			public:
				Assao();
				virtual ~Assao();

			public:
				virtual Type GetType() const override { return IRenderer::eAssao; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Apply(const Camera* pCamera, const RenderTarget* pNormalMap, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}