#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class Camera;
		class IEffect;
		class IEffectTech;

		class ModelRenderer : public IRenderer
		{
		public:
			ModelRenderer();
			virtual ~ModelRenderer();

		public:
			virtual bool Init(const Math::Viewport& viewport) override;

			virtual void Render(uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		public:
			virtual void AddRender(const RenderSubsetStatic& renderSubset) override
			{
				if (m_nStaticIndex >= m_vecStaticSubsets.size())
				{
					m_vecStaticSubsets.resize(m_vecStaticSubsets.size() * 2);
				}

				m_vecStaticSubsets[m_nStaticIndex].Set(renderSubset);
				++m_nStaticIndex;
			}

			virtual void AddRender(const RenderSubsetSkinned& renderSubset) override
			{
				if (m_nSkinnedIndex >= m_vecSkinnedSubsets.size())
				{
					m_vecSkinnedSubsets.resize(m_vecSkinnedSubsets.size() * 2);
				}

				m_vecSkinnedSubsets[m_nSkinnedIndex].Set(renderSubset);
				++m_nSkinnedIndex;
			}
		private:
			void renderStaticModel(IDevice* pDevice, Camera* pCamera);
			void renderSkinnedModel(IDevice* pDevice, Camera* pCamera);

			void renderStaticModel_Shadow(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, const Math::Matrix* pMatView, const Math::Matrix& matProj, const Collision::Frustum& frustum, bool isRenderCubeMap);
			void renderSkinnedModel_Shadow(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, const Math::Matrix* pMatView, const Math::Matrix& matProj, const Collision::Frustum& frustum, bool isRenderCubeMap);

		private:
			struct StaticSubset
			{
				std::pair<const void*, IMaterial*> pairKey;
				RenderSubsetStatic data;
				bool isCulling = false;

				void Set(const RenderSubsetStatic& source)
				{
					pairKey = std::make_pair(source.pKey, source.pMaterial);
					data = source;
					isCulling = false;
				}
			};
			std::vector<StaticSubset> m_vecStaticSubsets;
			size_t m_nStaticIndex;

			struct SkinnedSubset
			{
				std::pair<const void*, IMaterial*> pairKey;
				RenderSubsetSkinned data;
				bool isCulling = false;

				void Set(const RenderSubsetSkinned& source)
				{
					pairKey = std::make_pair(source.pKey, source.pMaterial);
					data = source;;
				}
			};
			std::vector<SkinnedSubset> m_vecSkinnedSubsets;
			size_t m_nSkinnedIndex;

			struct RenderSubsetStaticBatch
			{
				const StaticSubset* pSubset = nullptr;
				std::vector<InstStaticData> vecInstData;

				RenderSubsetStaticBatch(const StaticSubset* pSubset, const Math::Matrix& matWorld)
					: pSubset(pSubset)
				{
					vecInstData.emplace_back(matWorld);
				}
			};

			struct RenderSubsetSkinnedBatch
			{
				const SkinnedSubset* pSubset = nullptr;
				std::vector<InstSkinnedData> vecInstData;

				RenderSubsetSkinnedBatch(const SkinnedSubset* pSubset, const Math::Matrix& matWorld, uint32_t nVTFID)
					: pSubset(pSubset)
				{
					vecInstData.emplace_back(matWorld, nVTFID);
				}
			};
		};
	}
}