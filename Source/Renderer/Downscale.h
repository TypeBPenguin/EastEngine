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

		class Downscale : public Singleton<Downscale>
		{
			friend Singleton<Downscale>;
		private:
			Downscale();
			virtual ~Downscale();

		public:
			bool Init();
			void Release();

		public:
			bool Apply4SW(IRenderTarget* pResult, IRenderTarget* pSource, bool isLuminance = false);
			bool Apply16SW(IRenderTarget* pResult, IRenderTarget* pSource);

			bool ApplyHW(IRenderTarget* pResult, IRenderTarget* pSource);
			bool Apply16HW(IRenderTarget* pResult, IRenderTarget* pSource);

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		private:
			bool m_isInit;

			IEffect* m_pEffect;

			ISamplerState* m_pSamplerPoint;
			ISamplerState* m_pSamplerLinear;
		};
	}
}