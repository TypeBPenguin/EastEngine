#pragma once

#include "Light.h"

namespace EastEngine
{
	namespace Graphics
	{
		class PointLight : public IPointLight
		{
		private:
			PointLight();

		public:
			virtual ~PointLight();

			static PointLight* Create(const String::StringID& strName, const Math::Vector3& f3Position, const Math::Color& color, float fIntensity, float fAmbientIntensity = 0.f, float fReflectionIntensity = 0.f, ShadowConfig* pShadowConfig = nullptr);

		public:
			virtual void Update(float fElapsedTime) override;

		public:
			virtual const String::StringID& GetName() const override { return m_strName; }

			virtual float GetIntensity() const override { return m_fIntensity; }
			virtual void SetIntensity(float fIntensity) override { m_fIntensity = fIntensity; }

			virtual float GetAmbientIntensity() const override { return m_fAmbientIntensity; }
			virtual void SetAmbientIntensity(float fAmbientIntensity) override { m_fAmbientIntensity = fAmbientIntensity; }

			virtual float GetReflectionIntensity() const override { return m_fReflectionIntensity; }
			virtual void SetReflectionIntensity(float fReflectionIntensity) override { m_fReflectionIntensity = fReflectionIntensity;; }

			virtual const Math::Color& GetColor() const override { return m_color; }
			virtual void SetColor(const Math::Color& color) override { m_color = color; }

			virtual bool IsEnableShadow() const override { return m_isEnableShadow; }
			virtual void SetEnableShadow(bool isEnableShadow) override { m_isEnableShadow = isEnableShadow; }

		public:
			virtual IShadowCubeMap* GetShadowCubeMap() const { return m_pShadowCubeMap; }

			virtual const Math::Vector3& GetPosition() const override { return m_f3Pos; }
			virtual void SetPosition(const Math::Vector3& vPos) override { m_f3Pos = vPos; }

		protected:
			String::StringID m_strName;
			bool m_isEnableShadow;

			Math::Vector3 m_f3Pos;

			float m_fIntensity;
			float m_fAmbientIntensity;
			float m_fReflectionIntensity;

			Math::Color m_color;

			ShadowConfig m_shadowConfig;
			IShadowCubeMap* m_pShadowCubeMap;
		};
	}
}