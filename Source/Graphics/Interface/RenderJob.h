#pragma once

#include "GraphicsInterface.h"

namespace est
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
			VertexBufferPtr pVertexBuffer{ nullptr };
			IndexBufferPtr pIndexBuffer{ nullptr };
			MaterialPtr pMaterial{ nullptr };
			math::Matrix worldMatrix;
			math::Matrix prevWorldMatrix;
			uint32_t startIndex{ 0 };
			uint32_t indexCount{ 0 };
			float depth{ 0.f };
			OcclusionCullingData occlusionCullingData;

			RenderJobStatic() = default;
			RenderJobStatic(const void* pKey, const VertexBufferPtr& pVertexBuffer, const IndexBufferPtr& pIndexBuffer, const MaterialPtr& pMaterial,
				const math::Matrix& worldMatrix, const math::Matrix& prevWorldMatrix,
				uint32_t startIndex, uint32_t indexCount,
				float depth, const OcclusionCullingData& occlusionCullingData);

			RenderJobStatic(const RenderJobStatic& source);
			RenderJobStatic(RenderJobStatic&& source) noexcept;

			RenderJobStatic& operator = (const RenderJobStatic& source);
			RenderJobStatic& operator = (RenderJobStatic&& source) noexcept;
		};

		struct RenderJobSkinned
		{
			const void* pKey{ nullptr };
			VertexBufferPtr pVertexBuffer{ nullptr };
			IndexBufferPtr pIndexBuffer{ nullptr };
			MaterialPtr pMaterial{ nullptr };
			math::Matrix worldMatrix;
			math::Matrix prevWorldMatrix;
			uint32_t startIndex{ 0 };
			uint32_t indexCount{ 0 };
			uint32_t VTFID{ 0 };
			uint32_t PrevVTFID{ 0 };
			float depth{ 0.f };
			OcclusionCullingData occlusionCullingData;

			RenderJobSkinned() = default;
			RenderJobSkinned(const void* pKey, const VertexBufferPtr& pVertexBuffer, const IndexBufferPtr& pIndexBuffer, const MaterialPtr& pMaterial,
				const math::Matrix& worldMatrix, const math::Matrix& prevWorldMatrix,
				uint32_t startIndex, uint32_t indexCount, uint32_t VTFID, uint32_t PrevVTFID,
				float depth, const OcclusionCullingData& occlusionCullingData);

			RenderJobSkinned(const RenderJobSkinned& source);
			RenderJobSkinned(RenderJobSkinned&& source) noexcept;

			RenderJobSkinned& operator = (const RenderJobSkinned& source);
			RenderJobSkinned& operator = (RenderJobSkinned&& source) noexcept;
		};

		struct RenderJobTerrain
		{
			VertexBufferPtr pVertexBuffer{ nullptr };

			math::float2 f2PatchSize;
			math::float2 f2HeightFieldSize;

			bool isEnableDynamicLOD{ true };
			bool isEnableFrustumCullInHS{ true };

			float fDynamicTessFactor{ 50.f };
			float fStaticTessFactor{ 12.f };

			math::Matrix worldMatrix;
			math::Matrix prevWorldMatrix;

			TexturePtr pTexHeightField;
			TexturePtr pTexColorMap;

			TexturePtr pTexDetailMap;
			TexturePtr pTexDetailNormalMap;
		};

		struct RenderJobVertex
		{
			VertexBufferPtr pVertexBuffer{ nullptr };
			IndexBufferPtr pIndexBuffer{ nullptr };

			math::Matrix worldMatrix;
			math::Color color;

			RenderJobVertex() = default;
			RenderJobVertex(const VertexBufferPtr& pVertexBuffer, const IndexBufferPtr& pIndexBuffer, const math::Matrix& worldMatrix, const math::Color& color);

			RenderJobVertex(const RenderJobVertex& source);
			RenderJobVertex(RenderJobVertex&& source) noexcept;

			RenderJobVertex& operator = (const RenderJobVertex& source);
			RenderJobVertex& operator = (RenderJobVertex&& source) noexcept;
		};
	}
}