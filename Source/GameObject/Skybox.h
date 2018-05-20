#pragma once

#include "GameObject.h"

namespace eastengine
{
	namespace gameobject
	{
		class Skybox : public ISkybox
		{
		public:
			Skybox();
			virtual ~Skybox();

		public:
			virtual void Update(float fElapsedTime) override;

		public:
			virtual const String::StringID& GetName() const override { return m_strName; }
			virtual void SetName(const String::StringID& strName) override { m_strName = strName; }

			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }
			virtual bool IsVisible() const override { return m_isVisible; }

			virtual std::shared_ptr<graphics::ITexture> GetTexture() const override { return m_pTexture; }
			virtual void SetTexture(const std::shared_ptr<graphics::ITexture>& pTexture) override { m_pTexture = pTexture; }

		public:
			void SetDestroy(bool isDestroy) { m_isDestroy = isDestroy; }
			bool IsDestroy() const { return m_isDestroy; }

		public:
			void Init(const SkyboxProperty& property);

		private:
			String::StringID m_strName;

			bool m_isDestroy;
			bool m_isVisible;

			SkyboxProperty m_property;

			graphics::IVertexBuffer* m_pVertexBuffer;
			graphics::IIndexBuffer* m_pIndexBuffer;

			std::shared_ptr<graphics::ITexture> m_pTexture;
		};
	}
}