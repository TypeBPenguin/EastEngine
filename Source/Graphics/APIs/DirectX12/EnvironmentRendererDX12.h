#pragma once

#include "RendererDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			struct RenderElement;

			class EnvironmentRenderer : public IRendererDX12
			{
			public:
				EnvironmentRenderer();
				virtual ~EnvironmentRenderer();

			public:
				virtual Type GetType() const override { return IRenderer::eEnvironment; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Render(const RenderElement& renderElement);
				void Cleanup();

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}