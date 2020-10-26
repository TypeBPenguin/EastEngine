#pragma once

#include "Light.h"

namespace est
{
	namespace graphics
	{
		class DirectionalLight : public IDirectionalLight
		{
		public:
			DirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData);
			virtual ~DirectionalLight();

		public:
			void Update(float elapsedTime);

		public:
			virtual const string::StringID& GetName() const override { return m_name; }

			virtual float GetIntensity() const override { return m_lightData.lightIntensity; }
			virtual void SetIntensity(float fIntensity) override { m_lightData.lightIntensity = fIntensity; }

			virtual float GetAmbientIntensity() const override { return m_lightData.ambientIntensity; }
			virtual void SetAmbientIntensity(float fAmbientIntensity) override { m_lightData.ambientIntensity = fAmbientIntensity; }

			virtual float GetReflectionIntensity() const override { return m_lightData.reflectionIntensity; }
			virtual void SetReflectionIntensity(float fReflectionIntensity) override { m_lightData.reflectionIntensity = fReflectionIntensity;; }

			virtual const math::float3& GetColor() const override { return m_lightData.color; }
			virtual void SetColor(const math::float3& color) override { m_lightData.color = color; }

			virtual bool IsEnableShadow() const override { return m_isEnableShadow; }
			virtual void SetEnableShadow(bool isEnableShadow) override;

		public:
			virtual const math::float3& GetDirection() const override { return m_lightData.direction; }
			virtual void SetDirection(const math::float3& f3Direction) override { m_lightData.direction = f3Direction; }

			virtual const DirectionalLightData& GetData() const override { return m_lightData; }
			virtual CascadedShadows& GetCascadedShadows() override { return m_cascadedShadows; }

			virtual void SetDepthMapResource(void* pResource) override { m_depthMapResource = pResource; }
			virtual void* GetDepthMapResource() const override { return m_depthMapResource; }

		protected:
			string::StringID m_name;
			bool m_isEnableShadow{ false };

			DirectionalLightData m_lightData;
			CascadedShadows m_cascadedShadows;

			void* m_depthMapResource{ nullptr };
		};
	}
}