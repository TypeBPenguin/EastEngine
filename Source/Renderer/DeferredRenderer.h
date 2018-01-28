#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IDevice;
		class IDeviceContext;
		class IEffect;
		class IEffectTech;
		class ISamplerState;
		class IBlendState;
		class IRenderTarget;

		class DeferredRenderer : public IRenderer
		{
		public:
			DeferredRenderer();
			virtual ~DeferredRenderer();

		public:
			virtual bool Init(const Math::Viewport& viewport) override;

			virtual void Render(uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			int RenderShadowMap(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pRenderTarget);
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);
			void ClearEffect_Shadow(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		private:
			IEffect* m_pEffect;
			IEffect* m_pEffectShadow;
		};
	}
}