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

			class ModelRenderer : public IRendererDX12
			{
			public:
				ModelRenderer();
				virtual ~ModelRenderer();

				enum Group
				{
					eDeferred = 0,
					eAlphaBlend,

					GroupCount,
				};

			public:
				virtual Type GetType() const override { return IRenderer::eModel; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Render(const RenderElement& renderElement, Group emGroup, const math::Matrix& matPrevViewProjection);
				void Cleanup();

			public:
				void PushJob(const RenderJobStatic& renderJob);
				void PushJob(const RenderJobSkinned& renderJob);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}