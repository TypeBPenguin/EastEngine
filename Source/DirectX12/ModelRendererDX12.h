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
				void Render(Camera* pCamera, Group emGroup);
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