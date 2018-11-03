#pragma once

#include "GameObject.h"

namespace eastengine
{
	namespace gameobject
	{
		class Skybox : public ISkybox
		{
		public:
			Skybox(const Handle& handle);
			virtual ~Skybox();

		public:
			virtual void Update(float fElapsedTime) override;

		public:
			virtual const string::StringID& GetName() const override { return m_strName; }
			virtual void SetName(const string::StringID& strName) override { m_strName = strName; }

			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }
			virtual bool IsVisible() const override { return m_isVisible; }

			virtual graphics::ITexture* GetTexture() const override { return m_pTexture; }
			virtual void SetTexture(graphics::ITexture* pTexture) override;

		public:
			void SetDestroy(bool isDestroy) { m_isDestroy = isDestroy; }
			bool IsDestroy() const { return m_isDestroy; }

		public:
			void Init(const SkyboxProperty& property);

		private:
			string::StringID m_strName;

			bool m_isDestroy{ false };
			bool m_isVisible{ true };

			SkyboxProperty m_property;

			graphics::IVertexBuffer* m_pVertexBuffer{ nullptr };
			graphics::IIndexBuffer* m_pIndexBuffer{ nullptr };

			graphics::ITexture* m_pTexture{ nullptr };
		};
	}
}