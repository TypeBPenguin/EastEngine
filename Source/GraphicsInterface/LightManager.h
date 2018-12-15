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
			IDirectionalLight* CreateDirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData);
			IPointLight* CreatePointLight(const string::StringID& name, bool isEnableShadow, const PointLightData& lightData);
			ISpotLight* CreateSpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData);

			void Remove(ILight* pLight);
			void Remove(ILight::Type emType, size_t index);
			void RemoveAll();
			ILight* GetLight(ILight::Type emType, size_t index) const;
			size_t GetLightCount(ILight::Type emType) const;

		public:
			uint32_t GetLightCountInView(ILight::Type emType) const;
			void GetDirectionalLightData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const;
			void GetPointLightData(const PointLightData** ppPointLightData, uint32_t* pSize) const;
			void GetSpotLightData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}