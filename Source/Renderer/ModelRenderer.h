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

			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		public:
			virtual void AddRender(const RenderSubsetStatic& renderSubset) override;
			virtual void AddRender(const RenderSubsetSkinned& renderSubset) override;

		private:
			void OcclusionCulling(Camera* pCamera, uint32_t nRenderGroupFlag);

			void RenderStaticModel(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag, uint32_t nRenderTypeFlag);
			void RenderSkinnedModel(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag, uint32_t nRenderTypeFlag);

			void RenderStaticModel_Shadow(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag, const Math::Matrix* pMatView, const Math::Matrix& matProj, const Collision::Frustum& frustum, bool isRenderCubeMap);
			void RenderSkinnedModel_Shadow(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag, const Math::Matrix* pMatView, const Math::Matrix& matProj, const Collision::Frustum& frustum, bool isRenderCubeMap);

		private:
			struct StaticSubset
			{
				std::pair<const void*, IMaterial*> pairKey;
				RenderSubsetStatic data;
				bool isCulling = false;
				//std::vector<VertexClipSpace> vecVertexClipSpace;

				void Set(const RenderSubsetStatic& source)
				{
					pairKey = std::make_pair(source.pKey, source.pMaterial);
					data = source;
					isCulling = false;
				}
			};
			std::array<std::array<std::vector<StaticSubset>, GroupCount>, ThreadCount> m_vecStaticSubsets;
			std::array<std::array<size_t, GroupCount>, ThreadCount> m_nStaticIndex;

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
			std::array<std::array<std::vector<SkinnedSubset>, GroupCount>, 2> m_vecSkinnedSubsets;
			std::array<std::array<size_t, GroupCount>, 2> m_nSkinnedIndex;

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