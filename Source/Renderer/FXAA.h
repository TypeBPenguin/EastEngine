#pragma once

#include "CommonLib/Singleton.h"

namespace eastengine
{
	namespace graphics
	{
		class IEffect;
		class IEffectTech;

		class FXAA : public Singleton<FXAA>
		{
			friend Singleton<FXAA>;
		private:
			FXAA();
			virtual ~FXAA();

		public:
			bool Init();
			void Release();

			bool Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pResult, IRenderTarget* pSource);

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		private:
			bool m_isInit;

			IEffect* m_pEffect;

			ISamplerState* m_pSamplerAnisotropic;
		};
	}
}