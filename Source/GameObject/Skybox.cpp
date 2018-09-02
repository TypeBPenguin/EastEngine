#include "stdafx.h"
#include "Skybox.h"

#include "CommonLib/FileUtil.h"

#include "GraphicsInterface/Camera.h"

#include "Model/GeometryModel.h"

namespace eastengine
{
	namespace gameobject
	{
		Skybox::Skybox(const Handle& handle)
			: ISkybox(handle)
			, m_isDestroy(false)
			, m_isVisible(true)
		{
		}

		Skybox::~Skybox()
		{
			graphics::ReleaseResource(&m_pVertexBuffer);
			graphics::ReleaseResource(&m_pIndexBuffer);
			graphics::ReleaseResource(&m_pTexture);
		}

		void Skybox::Init(const SkyboxProperty& property)
		{
			m_property = property;

			graphics::geometry::CreateBox(&m_pVertexBuffer, &m_pIndexBuffer, math::Vector3(property.fBoxSize, property.fBoxSize, property.fBoxSize), false, true);

			m_pTexture = graphics::CreateTexture(property.strTexSky.c_str());
		}

		void Skybox::Update(float fElapsedTime)
		{
			if (IsVisible() == true)
			{
				if (m_pTexture->GetState() == graphics::IResource::eComplete)
				{
					graphics::Camera* pCamera = graphics::Camera::GetInstance();

					//math::Matrix matWorld = math::Matrix::CreateTranslation(pCamera->GetPosition());
					math::Matrix matWorld = math::Matrix::CreateTranslation(pCamera->GetViewMatrix().Invert().Translation());

					// 스카이 렌더러 고치고 사용하시오.
					assert(false);

					//graphics::RenderSubsetSkybox subset;
					//subset.pVertexBuffer = m_pVertexBuffer;
					//subset.pIndexBuffer = m_pIndexBuffer;
					//
					//subset.pTexSkyCubemap = m_pTexture;
					//subset.matWorld = matWorld;
					//
					//graphics::RendererManager::GetInstance()->AddRender(subset);
				}
			}
		}

		void Skybox::SetTexture(graphics::ITexture* pTexture)
		{
			if (m_pTexture != nullptr)
			{
				graphics::ReleaseResource(&m_pTexture);
			}

			m_pTexture = pTexture;

			if (m_pTexture != nullptr)
			{
				m_pTexture->IncreaseReference();
			}
		}
	}
}