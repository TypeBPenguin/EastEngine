#pragma once

#include "GraphicsInterface.h"

namespace eastengine
{
	namespace graphics
	{
		struct OcclusionCullingData
		{
			collision::AABB aabb;
			const VertexPos* pVertices{ nullptr };
			const uint32_t* pIndices{ nullptr };
			size_t indexCount{ 0 };
		};

		struct RenderJobStatic
		{
			const void* pKey{ nullptr };
			const IVertexBuffer* pVertexBuffer{ nullptr };
			const IIndexBuffer* pIndexBuffer{ nullptr };
			const IMaterial* pMaterial{ nullptr };
			math::Matrix matWorld;
			uint32_t nStartIndex{ 0 };
			uint32_t nIndexCount{ 0 };
			float fDepth{ 0.f };
			OcclusionCullingData occlusionCullingData;

			RenderJobStatic() = default;
			RenderJobStatic(const void* pKey, const IVertexBuffer* pVertexBuffer, const IIndexBuffer* pIndexBuffer, const IMaterial* pMaterial,
				const math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount,
				float fDepth, const OcclusionCullingData& occlusionCullingData);

			RenderJobStatic(const RenderJobStatic& source);
			RenderJobStatic(RenderJobStatic&& source) noexcept;

			RenderJobStatic& operator = (const RenderJobStatic& source);
			RenderJobStatic& operator = (RenderJobStatic&& source) noexcept;
		};

		struct RenderJobSkinned
		{
			const void* pKey{ nullptr };
			const IVertexBuffer* pVertexBuffer{ nullptr };
			const IIndexBuffer* pIndexBuffer{ nullptr };
			const IMaterial* pMaterial{ nullptr };
			math::Matrix matWorld;
			uint32_t nStartIndex{ 0 };
			uint32_t nIndexCount{ 0 };
			uint32_t nVTFID{ 0 };
			float fDepth{ 0.f };
			OcclusionCullingData occlusionCullingData;

			RenderJobSkinned() = default;
			RenderJobSkinned(const void* pKey, const IVertexBuffer* pVertexBuffer, const IIndexBuffer* pIndexBuffer, const IMaterial* pMaterial,
				const math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount, uint32_t nVTFID,
				float fDepth, const OcclusionCullingData& occlusionCullingData);

			RenderJobSkinned(const RenderJobSkinned& source);
			RenderJobSkinned(RenderJobSkinned&& source) noexcept;

			RenderJobSkinned& operator = (const RenderJobSkinned& source);
			RenderJobSkinned& operator = (RenderJobSkinned&& source) noexcept;
		};

		struct RenderJobTerrain
		{
			IVertexBuffer* pVertexBuffer{ nullptr };

			math::float2 f2PatchSize;
			math::float2 f2HeightFieldSize;

			bool isEnableDynamicLOD{ true };
			bool isEnableFrustumCullInHS{ true };

			float fDynamicTessFactor{ 50.f };
			float fStaticTessFactor{ 12.f };

			math::Matrix matWorld;

			ITexture* pTexHeightField{ nullptr };
			ITexture* pTexColorMap{ nullptr };

			ITexture* pTexDetailMap{ nullptr };
			ITexture* pTexDetailNormalMap{ nullptr };
		};

		struct RenderJobVertex
		{
			const IVertexBuffer* pVertexBuffer{ nullptr };
			const IIndexBuffer* pIndexBuffer{ nullptr };

			math::Matrix matWorld;
			math::Color color;

			RenderJobVertex() = default;
			RenderJobVertex(const IVertexBuffer* pVertexBuffer, const IIndexBuffer* pIndexBuffer, const math::Matrix& matWorld, const math::Color& color);

			RenderJobVertex(const RenderJobVertex& source);
			RenderJobVertex(RenderJobVertex&& source) noexcept;

			RenderJobVertex& operator = (const RenderJobVertex& source);
			RenderJobVertex& operator = (RenderJobVertex&& source) noexcept;
		};
	}
}