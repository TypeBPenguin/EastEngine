#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class HDRFilter : public Singleton<HDRFilter>
		{
			friend Singleton<HDRFilter>;
		private:
			HDRFilter();
			virtual ~HDRFilter();

		public:
			enum ToneMappingType
			{
				eNone = 0,
				eLogarithmic,
				eExponential,
				eDragoLogarithmic,
				eReinhard,
				eReinhardModified,
				eFilmicALU,
				eFilmicUncharted,
				eACES,

				NumToneMappingTypes,
			};

			enum AutoExposureType
			{
				eManual = 0,
				eGeometricMean = 1,
				eGeometricMeanAutoKey = 2,

				NumAutoExposureTypes,
			};

			struct Settings
			{
				float BloomThreshold{ 2.f };		// 0.f ~ 10.f
				float BloomMagnitude{ 0.f };		// 0.f ~ 2.f
				float BloomBlurSigma{ 0.8f };		// 0.5f ~ 1.5f
				float Tau{ 1.25f };					// 0.f ~ 4.f
				float TimeDelta{};
				float ToneMapTechnique{};
				float Exposure{ 0.f };				// -10.f ~ 10.f
				float KeyValue{ 0.18f };			// 0.f ~ 1.f
				float AutoExposure{};
				float WhiteLevel{ 5.f };			// 0.f ~ 25.f
				float ShoulderStrength{ 0.22f };	// 0.f ~ 2.f
				float LinearStrength{ 0.3f };		// 0.f ~ 5.f
				float LinearAngle{ 0.1f };			// 0.f ~ 1.f
				float ToeStrength{ 0.2f };			// 0.f ~ 2.f
				float ToeNumerator{ 0.01f };		// 0.f ~ 0.5f
				float ToeDenominator{ 0.3f };		// 0.f ~ 2.f
				float LinearWhite{ 11.2f };			// 0.f ~ 20.f
				float LuminanceSaturation{ 1.f };	// 0.f ~ 4.f
				float LumMapMipLevel{ 10.f };		// 0 ~ 10
				float Bias{ 0.5f };					// 0.f ~ 1.f
			};

		public:
			bool Apply(IRenderTarget* pResult, IRenderTarget* pSource);

		public:
			Settings& GetSettings();
			void SetSettings(const Settings& settings);

			ToneMappingType GetToneMappingType() const;
			void SetToneMappingType(ToneMappingType emToneMappingType);

			AutoExposureType GetAutoExposureType() const;
			void SetAutoExposureType(AutoExposureType emAutoExposureType);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}