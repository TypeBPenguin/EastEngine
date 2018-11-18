#pragma once

#include "GraphicsInterface/RenderJob.h"
#include "RendererDX12.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class VertexRenderer : public IRendererDX12
			{
			public:
				VertexRenderer();
				virtual ~VertexRenderer();

			public:
				virtual Type GetType() const override { return IRenderer::eVertex; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Render(Camera* pCamera);
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