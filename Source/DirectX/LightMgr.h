#pragma once

#include "../CommonLib/Singleton.h"

#include "Light.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IStructuredBuffer;

		class LightManager : public Singleton<LightManager>
		{
			friend Singleton<LightManager>;
		private:
			LightManager();
			virtual ~LightManager();

		public:
			bool Init();
			void Release();

			void Update(float fElapsedTime);

			bool AddLight(ILight* pLight);
			void Remove(ILight* pLight);
			void Remove(EmLight::Type emType, uint32_t nIndex);
			void RemoveAll();
			ILight* GetLight(EmLight::Type emType, uint32_t nIndex);
			uint32_t GetLightCount(EmLight::Type emType);

		public:
			IStructuredBuffer* GetLightBuffer(EmLight::Type emType) { return m_pLightBuffers[emType]; }
			uint32_t GetLightCountInView(EmLight::Type emType) { return m_nLightCountInView[emType]; }

		private:
			void updateLightBuffer();

		private:
			bool m_isInit;

			std::array<IStructuredBuffer*, EmLight::eCount> m_pLightBuffers;
			std::array<uint32_t, EmLight::eCount> m_nLightCountInView;

			std::vector<IDirectionalLight*> m_vecDirectionalLights;
			std::vector<ISpotLight*> m_vecSpotLights;
			std::vector<IPointLight*> m_vecPointLights;

			struct DirectionalLightData
			{
				Math::Vector3 f3Color;
				float fLightIntensity = 0.f;

				Math::Vector3 f3Dir;
				float fAmbientIntensity = 0.f;

				Math::Vector3 padding;
				float fReflectionIntensity = 0.f;

				void Set(const Math::Color& color, const Math::Vector3& direction, float lightIntensity, float ambientIntensity, float reflectionIntensity)
				{
					f3Color.x = color.r;
					f3Color.y = color.g;
					f3Color.z = color.b;

					f3Dir = direction;
					fLightIntensity = lightIntensity;
					fAmbientIntensity = ambientIntensity;
					fReflectionIntensity = reflectionIntensity;
				}
			};
			std::array<DirectionalLightData, EmLight::eMaxDirectionalLightCount> m_directionalLightData;

			struct PointLightData
			{
				Math::Vector3 f3Color;
				float fLightIntensity = 0.f;

				Math::Vector3 f3Pos;
				float fAmbientIntensity = 0.f;

				Math::Vector3 padding;
				float fReflectionIntensity = 0.f;

				void Set(const Math::Color& color, const Math::Vector3& pos, float lightIntensity, float ambientIntensity, float reflectionIntensity)
				{
					f3Color.x = color.r;
					f3Color.y = color.g;
					f3Color.z = color.b;

					f3Pos = pos;
					fLightIntensity = lightIntensity;
					fAmbientIntensity = ambientIntensity;
					fReflectionIntensity = reflectionIntensity;
				}
			};
			std::array<PointLightData, EmLight::eMaxPointLightCount> m_pointLightData;

			struct SpotLightData
			{
				Math::Vector3 f3Color;
				float fLightIntensity = 0.f;

				Math::Vector3 f3Pos;
				float fAmbientIntensity = 0.f;

				Math::Vector3 f3Dir;
				float fReflectionIntensity = 0.f;

				Math::Vector3 padding;
				float fAngle = 0.f;

				void Set(const Math::Color& color, const Math::Vector3& position, const Math::Vector3& direction, float lightIntensity, float ambientIntensity, float reflectionIntensity, float angle)
				{
					f3Color.x = color.r;
					f3Color.y = color.g;
					f3Color.z = color.b;

					f3Pos = position;
					f3Dir = direction;
					fLightIntensity = lightIntensity;
					fAmbientIntensity = ambientIntensity;
					fReflectionIntensity = reflectionIntensity;
					fAngle = angle;
				}
			};
			std::array<SpotLightData, EmLight::eMaxSpotLightCount> m_spotLightData;
		};
	}
}