#include "stdafx.h"
#include "Skybox.h"

#include "CommonLib/FileUtil.h"

#include "DirectX/CameraManager.h"
#include "Model/GeometryModel.h"

namespace EastEngine
{
	namespace GameObject
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

			Graphics::GeometryModel::CreateBox(&m_pVertexBuffer, &m_pIndexBuffer, Math::Vector3(property.fBoxSize, property.fBoxSize, property.fBoxSize), false, true);

			m_pTexture = Graphics::ITexture::Create(File::GetFileName(property.strTexSky).c_str(), property.strTexSky.c_str());
		}

		void Skybox::Update(float fElapsedTime)
		{
			if (IsVisible() == true)
			{
				if (m_pTexture->GetLoadState() == Graphics::EmLoadState::eComplete)
				{
					Graphics::Camera* pCamera = Graphics::CameraManager::GetInstance()->GetMainCamera();

					//Math::Matrix matWorld = Math::Matrix::CreateTranslation(pCamera->GetPosition());
					Math::Matrix matWorld = Math::Matrix::CreateTranslation(pCamera->GetViewMatrix().Invert().Translation());

					Graphics::RenderSubsetSkybox subset;
					subset.pVertexBuffer = m_pVertexBuffer;
					subset.pIndexBuffer = m_pIndexBuffer;

					subset.pTexSkyCubemap = m_pTexture;
					subset.matWorld = matWorld;

					Graphics::RendererManager::GetInstance()->AddRender(subset);
				}
			}
		}
	}
}