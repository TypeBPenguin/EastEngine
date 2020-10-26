#pragma once

#include "GameObject.h"

namespace est
{
	namespace gameobject
	{
		class Skybox : public ISkybox
		{
		public:
			Skybox(const Handle& handle);
			virtual ~Skybox();

		public:
			virtual void Update(float elapsedTime) override;

		public:
			virtual const string::StringID& GetName() const override { return m_strName; }
			virtual void SetName(const string::StringID& strName) override { m_strName = strName; }

			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }
			virtual bool IsVisible() const override { return m_isVisible; }

			virtual graphics::TexturePtr GetTexture() const override { return m_pTexture; }
			virtual void SetTexture(const graphics::TexturePtr& pTexture) override;

		public:
			void SetDestroy(bool isDestroy) { m_isDestroy = isDestroy; }
			bool IsDestroy() const { return m_isDestroy; }

		public:
			void Initialize(const SkyboxProperty& skyProperty);

		private:
			string::StringID m_strName;

			bool m_isDestroy{ false };
			bool m_isVisible{ true };

			SkyboxProperty m_property;

			graphics::VertexBufferPtr m_pVertexBuffer{ nullptr };
			graphics::IndexBufferPtr m_pIndexBuffer{ nullptr };

			graphics::TexturePtr m_pTexture{ nullptr };
		};
	}
}