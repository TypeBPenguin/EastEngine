#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IEffect;
		class IEffectTech;

		class IRenderTarget;
		class ISamplerState;

		class GaussianBlur : public Singleton<GaussianBlur>
		{
			friend Singleton<GaussianBlur>;
		private:
			GaussianBlur();
			virtual ~GaussianBlur();

		public:
			bool Init();
			void Release();

		public:
			bool Apply(IRenderTarget* pResult, IRenderTarget* pSource, float fSigma);
			bool Apply(IRenderTarget* pResult, IRenderTarget* pSource, IRenderTarget* pDepth, float fSigma);

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		private:
			bool m_isInit;

			IEffect* m_pEffect;

			ISamplerState* m_pSamplerPoint;
		};
	}
}