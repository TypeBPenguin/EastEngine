#pragma once

#include "Light.h"

namespace est
{
	namespace graphics
	{
		class SpotLight : public ISpotLight
		{
		public:
			SpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData);
			virtual ~SpotLight();

		public:
			void Update(float elapsedTime);

		public:
			virtual const string::StringID& GetName() const override { return m_name; }

			virtual float GetIntensity() const override { return m_lightData.lightIntensity; }
			virtual void SetIntensity(float fIntensity) override { m_lightData.lightIntensity = fIntensity; }

			virtual float GetAmbientIntensity() const override { return m_lightData.ambientIntensity; }
			virtual void SetAmbientIntensity(float fAmbientIntensity) override { m_lightData.ambientIntensity= fAmbientIntensity; }

			virtual float GetReflectionIntensity() const override { return m_lightData.reflectionIntensity; }
			virtual void SetReflectionIntensity(float fReflectionIntensity) override { m_lightData.reflectionIntensity = fReflectionIntensity;; }

			virtual const math::float3& GetColor() const override { return m_lightData.color; }
			virtual void SetColor(const math::float3& color) override { m_lightData.color = color; }

			virtual bool IsEnableShadow() const override { return m_isEnableShadow; }
			virtual void SetEnableShadow(bool isEnableShadow) override;

		public:
			virtual const math::float3& GetPosition() const override { return m_lightData.position; }
			virtual void SetPosition(const math::float3& vPos) override { m_lightData.position = vPos; }

			virtual const math::float3& GetDirection() const override { return m_lightData.direction; }
			virtual void SetDirection(const math::float3& f3Direction) override { m_lightData.direction = f3Direction; }

			virtual float GetAngle() const override { return m_lightData.angle; }
			virtual void SetAngle(float fAngle) override { m_lightData.angle = fAngle; }

			virtual const SpotLightData& GetData() const override { return m_lightData; }

		protected:
			string::StringID m_name;
			bool m_isEnableShadow{ false };

			SpotLightData m_lightData;
		};
	}
}