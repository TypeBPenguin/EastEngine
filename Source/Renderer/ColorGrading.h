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

			bool Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pResult, IRenderTarget* pSource);

		public:
			void SetColorGuide(const Math::Vector3& f3ColorGuide) { m_f3ColorGuide = f3ColorGuide; }
			const Math::Vector3& GetColorGuide() const { return m_f3ColorGuide; }

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		private:
			bool m_isInit;

			Math::Vector3 m_f3ColorGuide;

			IEffect* m_pEffect;
			ISamplerState* m_pSamplerState;
		};
	}
}