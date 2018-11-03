#pragma once

#include "GraphicsInterface/RenderJob.h"
#include "GraphicsInterface/Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			class ModelRenderer : public IRenderer
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
				virtual Type GetType() const { return IRenderer::eModel; }

			public:
				void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera, Group emGroup);
				void Cleanup();

			public:
				void PushJob(const RenderJobStatic& job);
				void PushJob(const RenderJobSkinned& job);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}