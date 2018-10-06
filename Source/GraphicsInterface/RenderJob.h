#pragma once

#include "GraphicsInterface.h"

namespace eastengine
{
	namespace graphics
	{
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
			Collision::Sphere boundingSphere;

			RenderJobStatic() = default;
			RenderJobStatic(const void* pKey, const IVertexBuffer* pVertexBuffer, const IIndexBuffer* pIndexBuffer, const IMaterial* pMaterial,
				const math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount,
				float fDepth, const Collision::Sphere& boundingSphere);

			RenderJobStatic(const RenderJobStatic& source) = default;
			RenderJobStatic(RenderJobStatic&& source) noexcept;

			void operator = (const RenderJobStatic& source)
			{
				pKey = source.pKey;
				pVertexBuffer = source.pVertexBuffer;
				pIndexBuffer = source.pIndexBuffer;
				pMaterial = source.pMaterial;
				matWorld = source.matWorld;
				nStartIndex = source.nStartIndex;
				nIndexCount = source.nIndexCount;
				fDepth = source.fDepth;
				boundingSphere = source.boundingSphere;
			}
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

			RenderJobSkinned() = default;
			RenderJobSkinned(const void* pKey, const IVertexBuffer* pVertexBuffer, const IIndexBuffer* pIndexBuffer, const IMaterial* pMaterial,
				const math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount, uint32_t nVTFID,
				float fDepth);

			RenderJobSkinned(const RenderJobSkinned& source) = default;
			RenderJobSkinned(RenderJobSkinned&& source) noexcept;

			void operator = (const RenderJobSkinned& source)
			{
				pKey = source.pKey;
				pVertexBuffer = source.pVertexBuffer;
				pIndexBuffer = source.pIndexBuffer;
				pMaterial = source.pMaterial;
				matWorld = source.matWorld;
				nStartIndex = source.nStartIndex;
				nIndexCount = source.nIndexCount;
				nVTFID = source.nVTFID;
				fDepth = source.fDepth;
			}
		};

		struct RenderJobTerrain
		{
			IVertexBuffer* pVertexBuffer{ nullptr };

			math::Vector2 f2PatchSize;
			math::Vector2 f2HeightFieldSize;

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
	}
}