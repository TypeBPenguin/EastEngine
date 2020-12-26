#pragma once

#include "Graphics/Interface/Light.h"

namespace est
{
	namespace graphics
	{
		class LightManager;

		namespace dx12
		{
			class Device;
			class DepthStencil;

			class LightResourceManager
			{
			public:
				LightResourceManager(LightManager* pLightManager);
				~LightResourceManager();

			public:
				LightPtr GetLight(ILight::Type emType, size_t index) const;
				size_t GetLightCount(ILight::Type emType) const;

			public:
				uint32_t GetLightCountInView(ILight::Type emType) const;
				void GetDirectionalLightRenderData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const;
				void GetPointLightRenderData(const PointLightData** ppPointLightData, uint32_t* pSize) const;
				void GetSpotLightRenderData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const;

				uint32_t GetShadowCount(ILight::Type type) const;

			public:
				void Cleanup();
				DepthStencil* GetDepthStencil(Device* pDeviceInstance, IDirectionalLight* pDirectionalLight);

			private:
				LightManager* m_pLightManager{ nullptr };
				std::vector<DepthStencil*> m_depthStencil;
			};
		}
	}
}