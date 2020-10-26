#include "stdafx.h"
#include "Skybox.h"

#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/Camera.h"

#include "Graphics/Model/GeometryModel.h"

namespace est
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
			graphics::ReleaseResource(m_pVertexBuffer);
			graphics::ReleaseResource(m_pIndexBuffer);
			graphics::ReleaseResource(m_pTexture);
		}

		void Skybox::Initialize(const SkyboxProperty& skyProperty)
		{
			m_property = skyProperty;

			std::vector<graphics::VertexPosTexNor> vertices;
			std::vector<uint32_t> indices;

			if (graphics::geometry::CreateBox(vertices, indices, math::float3(skyProperty.boxSize), false, true) == false)
			{
				assert(false);
				return;
			}

			if (vertices.empty() == true || indices.empty() == true)
			{
				assert(false);
				return;
			}

			m_pVertexBuffer = graphics::CreateVertexBuffer(reinterpret_cast<uint8_t*>(vertices.data()), static_cast<uint32_t>(vertices.size()), sizeof(graphics::VertexPosTexNor), false);
			m_pIndexBuffer = graphics::CreateIndexBuffer(reinterpret_cast<uint8_t*>(indices.data()), static_cast<uint32_t>(indices.size()), sizeof(uint32_t), false);

			m_pTexture = graphics::CreateTexture(skyProperty.texSkymap.c_str());
		}

		void Skybox::Update(float elapsedTime)
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

		void Skybox::SetTexture(const graphics::TexturePtr& pTexture)
		{
			graphics::ReleaseResource(m_pTexture);
			m_pTexture = pTexture;
		}
	}
}