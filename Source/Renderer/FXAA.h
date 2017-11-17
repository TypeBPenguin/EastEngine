#pragma once

#include "../CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
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

			bool Apply(IRenderTarget* pResult, IRenderTarget* pSource);
			void Flush();

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		private:
			bool m_isInit;

			IEffect* m_pEffect;

			ISamplerState* m_pSamplerAnisotropic;
		};
	}
}