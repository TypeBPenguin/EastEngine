#pragma once

#include "Graphics/Interface/Light.h"

struct ID3D11DeviceContext;

namespace est
{
	namespace graphics
	{
		class LightManager;

		namespace dx11
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
				void GetDirectionalLightData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const;
				void GetPointLightData(const PointLightData** ppPointLightData, uint32_t* pSize) const;
				void GetSpotLightData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const;

				uint32_t GetShadowCount(ILight::Type type) const;

			public:
				void Cleanup();
				DepthStencil* GetDepthStencil(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext, IDirectionalLight* pDirectionalLight);

			private:
				LightManager* m_pLightManager{ nullptr };
				std::vector<DepthStencil*> m_depthStencil;
			};
		}
	}
}