#pragma once

#include "CommonLib/Singleton.h"

namespace eastengine
{
	namespace graphics
	{
		class SSS : public Singleton<SSS>
		{
			friend Singleton<SSS>;
		private:
			SSS();
			virtual ~SSS();

		public:
			bool Init();
			void Release();

		public:
			bool Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pResult, IRenderTarget* pSource, const std::shared_ptr<ITexture>& pDepth);

			void SetSSSWidth(float fSSSWidth) { m_fSSSWidth = fSSSWidth; }
			float GetSSSWidth() const { return m_fSSSWidth; }

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		private:
			bool m_isInit;

			IEffect* m_pEffect;

			float m_fSSSWidth;
		};
	}
}