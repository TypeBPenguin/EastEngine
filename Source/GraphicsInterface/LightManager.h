#pragma once

#include "CommonLib/Singleton.h"

#include "Light.h"

namespace eastengine
{
	namespace graphics
	{
		class LightManager : public Singleton<LightManager>
		{
			friend Singleton<LightManager>;
		private:
			LightManager();
			virtual ~LightManager();

		public:
			void Update(float elapsedTime);

		public:
			bool AddLight(ILight* pLight);
			void Remove(ILight* pLight);
			void Remove(ILight::Type emType, size_t nIndex);
			void RemoveAll();
			ILight* GetLight(ILight::Type emType, size_t nIndex);
			size_t GetLightCount(ILight::Type emType);

		public:
			uint32_t GetLightCountInView(ILight::Type emType);
			void GetDirectionalLightData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const;
			void GetPointLightData(const PointLightData** ppPointLightData, uint32_t* pSize) const;
			void GetSpotLightData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;

		};
	}
}