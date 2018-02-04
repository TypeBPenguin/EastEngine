#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class BloomFilter : public Singleton<BloomFilter>
		{
			friend Singleton<BloomFilter>;
		private:
			BloomFilter();
			virtual ~BloomFilter();

		public:
			enum Presets
			{
				eWide = 0,
				eFocussed,
				eSmall,
				eSuperWide,
				eCheap,
				eOne,

				PresetCount,
			};

			struct Settings
			{
				Presets emPreset = Presets::eWide;
				float fThreshold = 0.2f;
				float fStrengthMultiplier = 1.f;
				bool isEnableLuminance = false;
			};

		public:
			bool Init();
			void Release();

		public:
			bool Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pSource);

			Settings& GetSettings() { return m_settings; }
			void SetSettings(const Settings& settings) { m_settings = settings; }

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);
			void SetBloomPreset(Presets emPreset);

		private:
			bool m_isInit;

			IEffect* m_pEffect;

			std::array<float, 5> m_fRadius;
			std::array<float, 5> m_fStrengths;
			float m_fRadiusMultiplier;
			float m_fStreakLength;
			int m_nDownsamplePasses;

			Settings m_settings;
		};
	}
}