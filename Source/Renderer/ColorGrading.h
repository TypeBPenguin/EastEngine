#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IEffect;
		class IEffectTech;

		class ISamplerState;

		class ColorGrading : public Singleton<ColorGrading>
		{
			friend Singleton<ColorGrading>;
		private:
			ColorGrading();
			virtual ~ColorGrading();

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

			ISamplerState* m_pSamplerState;
		};
	}
}