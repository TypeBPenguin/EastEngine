#pragma once

#include "../CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IEffect;
		class IEffectTech;

		class IRenderTarget;
		class ISamplerState;

		class HDRFilter : public Singleton<HDRFilter>
		{
			friend Singleton<HDRFilter>;
		private:
			HDRFilter();
			virtual ~HDRFilter();

		public:
			bool Init();
			void Release();

		public:
			float GetBloomThreshold() const { return m_fBloomThreshold; }
			void SetBloomThreshold(float fBloomThreshold) { m_fBloomThreshold = fBloomThreshold; }

			float GetBloomMultiplier() const { return m_fBloomMultiplier; }
			void SetBloomMultiplier(float fBloomMultiplier) { m_fBloomMultiplier = fBloomMultiplier; }

			float GetToneMapKey() const { return m_fToneMapKey; }
			void SetToneMapKey(float fToneMapKey) { m_fToneMapKey = fToneMapKey; }

			float GetMaxLuminance() const { return m_fMaxLuminance; }
			void SetMaxLuminance(float fMaxLuminance) { m_fMaxLuminance = fMaxLuminance;; }

			float GetBlurSigma() const { return m_fBlurSigma; }
			void SetBlurSigma(float fBlurSigma) { m_fBlurSigma = fBlurSigma;; }

			bool IsEnableLensFlare() { return m_isEnableLensFlare; }
			void SetEnableLensFlare(bool isEnableLensFlare) { m_isEnableLensFlare = isEnableLensFlare; }

		public:
			bool ToneMap(IRenderTarget* pResult, IRenderTarget* pSource, float fElapsedTime);

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech);

		private:
			bool m_isInit;
			bool m_isEnableLensFlare;

			IEffect* m_pEffect;

			float m_fBloomThreshold;
			float m_fToneMapKey;
			float m_fMaxLuminance;
			float m_fBloomMultiplier;
			float m_fBlurSigma;

			std::vector<IRenderTarget*> m_vecLuminanceChain;
			IRenderTarget* m_pLuminanceCurrent;
			IRenderTarget* m_pLuminanceLast;
			IRenderTarget* m_pAdaptedLuminance;

			ISamplerState* m_pSamplerPoint;
			ISamplerState* m_pSamplerLinear;
		};
	}
}