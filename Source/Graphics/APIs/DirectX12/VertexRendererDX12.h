#pragma once

#include "Graphics/Interface/RenderJob.h"
#include "RendererDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			struct RenderElement;

			class VertexRenderer : public IRendererDX12
			{
			public:
				VertexRenderer();
				virtual ~VertexRenderer();

			public:
				virtual Type GetType() const override { return IRenderer::eVertex; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Render(const RenderElement& renderElement);
				void Cleanup();

			public:
				void PushJob(const RenderJobVertex& renderJob);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}