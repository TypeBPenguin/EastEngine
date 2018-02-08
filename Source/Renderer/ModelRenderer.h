#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
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
			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		public:
			virtual void AddRender(const RenderSubsetStatic& renderSubset) override;
			virtual void AddRender(const RenderSubsetSkinned& renderSubset) override;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}