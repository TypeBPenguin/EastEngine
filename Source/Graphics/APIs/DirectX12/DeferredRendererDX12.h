#pragma once

#include "RendererDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			struct RenderElement;
			class DeferredRenderer : public IRendererDX12
			{
			public:
				DeferredRenderer();
				virtual ~DeferredRenderer();

			public:
				virtual Type GetType() const override { return IRenderer::eDeferred; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Render(const RenderElement& renderElement);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}