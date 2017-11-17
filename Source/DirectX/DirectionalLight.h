#pragma once

#include "Light.h"

namespace EastEngine
{
	namespace Graphics
	{
		class DirectionalLight : public IDirectionalLight
		{
		private:
			DirectionalLight();

		public:
			virtual ~DirectionalLight();

			static DirectionalLight* Create(const String::StringID& strName, const Math::Vector3& f3Direction, const Math::Color& color, float fIntensity, float fAmbientIntensity = 0.f, float fReflectionIntensity = 0.f, CascadedShadowsConfig* pCascadeConfig = nullptr);

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
			virtual ICascadedShadows* GetCascadedShadow() const override { return m_pCascadedShadows; }

			virtual const Math::Vector3& GetDirection() const override { return m_f3Direction; }
			virtual void SetDirection(const Math::Vector3& f3Direction) override { m_f3Direction = f3Direction; }

		protected:
			String::StringID m_strName;
			bool m_isEnableShadow;

			Math::Vector3 m_f3Direction;

			float m_fIntensity;
			float m_fAmbientIntensity;
			float m_fReflectionIntensity;

			Math::Color m_color;

			CascadedShadowsConfig m_cascadeConfig;
			ICascadedShadows* m_pCascadedShadows;
		};
	}
}