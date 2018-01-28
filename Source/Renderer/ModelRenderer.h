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

			enum Group
			{
				eDeferred = 0,
				eAlphaBlend,

				GroupCount,
			};

		public:
			virtual bool Init(const Math::Viewport& viewport) override;

			virtual void Render(uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		public:
			virtual void AddRender(const RenderSubsetStatic& renderSubset) override;
			virtual void AddRender(const RenderSubsetSkinned& renderSubset) override;

		private:
			void renderStaticModel(IDevice* pDevice, Camera* pCamera, uint32_t nRenderGroupFlag, uint32_t nRenderTypeFlag);
			void renderSkinnedModel(IDevice* pDevice, Camera* pCamera, uint32_t nRenderGroupFlag, uint32_t nRenderTypeFlag);

			void renderStaticModel_Shadow(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag, const Math::Matrix* pMatView, const Math::Matrix& matProj, const Collision::Frustum& frustum, bool isRenderCubeMap);
			void renderSkinnedModel_Shadow(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag, const Math::Matrix* pMatView, const Math::Matrix& matProj, const Collision::Frustum& frustum, bool isRenderCubeMap);

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
			std::array<std::vector<StaticSubset>, GroupCount> m_vecStaticSubsets;
			std::array<size_t, GroupCount> m_nStaticIndex;

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
			std::array<std::vector<SkinnedSubset>, GroupCount> m_vecSkinnedSubsets;
			std::array<size_t, GroupCount> m_nSkinnedIndex;

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