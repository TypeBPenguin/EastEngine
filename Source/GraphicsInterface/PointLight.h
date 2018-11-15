#pragma once

#include "Light.h"

namespace eastengine
{
	namespace graphics
	{
		class PointLight : public IPointLight
		{
		private:
			PointLight();

		public:
			virtual ~PointLight();

			static PointLight* Create(const string::StringID& strName, const math::float3& f3Position, const math::Color& color, float fIntensity, float fAmbientIntensity = 0.f, float fReflectionIntensity = 0.f);

		public:
			virtual void Update(float fElapsedTime) override;

		public:
			virtual const string::StringID& GetName() const override { return m_strName; }

			virtual float GetIntensity() const override { return m_fIntensity; }
			virtual void SetIntensity(float fIntensity) override { m_fIntensity = fIntensity; }

			virtual float GetAmbientIntensity() const override { return m_fAmbientIntensity; }
			virtual void SetAmbientIntensity(float fAmbientIntensity) override { m_fAmbientIntensity = fAmbientIntensity; }

			virtual float GetReflectionIntensity() const override { return m_fReflectionIntensity; }
			virtual void SetReflectionIntensity(float fReflectionIntensity) override { m_fReflectionIntensity = fReflectionIntensity;; }

			virtual const math::Color& GetColor() const override { return m_color; }
			virtual void SetColor(const math::Color& color) override { m_color = color; }

			virtual bool IsEnableShadow() const override { return m_isEnableShadow; }
			virtual void SetEnableShadow(bool isEnableShadow) override { m_isEnableShadow = isEnableShadow; }

		public:
			virtual const math::float3& GetPosition() const override { return m_f3Pos; }
			virtual void SetPosition(const math::float3& vPos) override { m_f3Pos = vPos; }

		protected:
			string::StringID m_strName;
			bool m_isEnableShadow;

			math::float3 m_f3Pos;

			float m_fIntensity;
			float m_fAmbientIntensity;
			float m_fReflectionIntensity;

			math::Color m_color;
		};
	}
}