#include "stdafx.h"
#include "Skybox.h"

#include "CommonLib/FileUtil.h"

#include "DirectX/Camera.h"
#include "Model/GeometryModel.h"

namespace eastengine
{
	namespace gameobject
	{
		Skybox::Skybox()
			: m_isDestroy(false)
			, m_isVisible(true)
			, m_pVertexBuffer(nullptr)
			, m_pIndexBuffer(nullptr)
		{
		}

		Skybox::~Skybox()
		{
			SafeDelete(m_pVertexBuffer);
			SafeDelete(m_pIndexBuffer);
		}

		void Skybox::Init(const SkyboxProperty& property)
		{
			m_property = property;

			graphics::GeometryModel::CreateBox(&m_pVertexBuffer, &m_pIndexBuffer, math::Vector3(property.fBoxSize, property.fBoxSize, property.fBoxSize), false, true);

			m_pTexture = graphics::ITexture::Create(property.strTexSky.c_str());
		}

		void Skybox::Update(float fElapsedTime)
		{
			if (IsVisible() == true)
			{
				if (m_pTexture->GetState() == graphics::EmLoadState::eComplete)
				{
					graphics::Camera* pCamera = graphics::Camera::GetInstance();

					//math::Matrix matWorld = math::Matrix::CreateTranslation(pCamera->GetPosition());
					math::Matrix matWorld = math::Matrix::CreateTranslation(pCamera->GetViewMatrix(graphics::GetThreadID(graphics::eUpdate)).Invert().Translation());

					graphics::RenderSubsetSkybox subset;
					subset.pVertexBuffer = m_pVertexBuffer;
					subset.pIndexBuffer = m_pIndexBuffer;

					subset.pTexSkyCubemap = m_pTexture;
					subset.matWorld = matWorld;

					graphics::RendererManager::GetInstance()->AddRender(subset);
				}
			}
		}
	}
}