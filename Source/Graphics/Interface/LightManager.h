#pragma once

#include "CommonLib/Singleton.h"

#include "Light.h"

namespace est
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
			DirectionalLightPtr CreateDirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData);
			PointLightPtr CreatePointLight(const string::StringID& name, bool isEnableShadow, const PointLightData& lightData);
			SpotLightPtr CreateSpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData);

			void Remove(const LightPtr& pLight);
			void Remove(ILight::Type emType, size_t index);
			void RemoveAll();
			LightPtr GetLight(ILight::Type emType, size_t index) const;
			size_t GetLightCount(ILight::Type emType) const;

		public:
			uint32_t GetLightCountInView(ILight::Type emType) const;
			void GetDirectionalLightData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const;
			void GetPointLightData(const PointLightData** ppPointLightData, uint32_t* pSize) const;
			void GetSpotLightData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const;

		public:
			uint32_t GetShadowCount(ILight::Type type) const;
			void IncreaseShadowCount(ILight::Type type);
			void DecreaseShadowCount(ILight::Type type);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}