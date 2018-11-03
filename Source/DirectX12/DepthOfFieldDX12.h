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

			class DepthOfField : public IRendererDX12
			{
			public:
				DepthOfField();
				virtual ~DepthOfField();

			public:
				virtual Type GetType() const override { return IRenderer::eDepthOfField; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Apply(Camera* pCamera, RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}