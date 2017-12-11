#pragma once

#include "GameObject.h"

namespace EastEngine
{
	namespace GameObject
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

			Graphics::IVertexBuffer* m_pVertexBuffer;
			Graphics::IIndexBuffer* m_pIndexBuffer;

			std::shared_ptr<Graphics::ITexture> m_pTexture;
		};
	}
}