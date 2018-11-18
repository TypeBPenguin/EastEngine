#include "stdafx.h"
#include "RenderJob.h"

namespace eastengine
{
	namespace graphics
	{
		RenderJobStatic::RenderJobStatic(const void* pKey, const IVertexBuffer* pVertexBuffer, const IIndexBuffer* pIndexBuffer, const IMaterial* pMaterial,
			const math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount,
			float fDepth, const OcclusionCullingData& occlusionCullingData)
			: pKey(pKey), pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMaterial(pMaterial)
			, matWorld(matWorld), nStartIndex(nStartIndex), nIndexCount(nIndexCount)
			, fDepth(fDepth), occlusionCullingData(occlusionCullingData)
		{
		}

		RenderJobStatic::RenderJobStatic(const RenderJobStatic& source)
		{
			*this = source;
		}

		RenderJobStatic::RenderJobStatic(RenderJobStatic&& source) noexcept
		{
			*this = std::move(source);
		}

		RenderJobStatic& RenderJobStatic::operator = (const RenderJobStatic& source)
		{
			pKey = source.pKey;
			pVertexBuffer = source.pVertexBuffer;
			pIndexBuffer = source.pIndexBuffer;
			pMaterial = source.pMaterial;
			matWorld = source.matWorld;
			nStartIndex = source.nStartIndex;
			nIndexCount = source.nIndexCount;
			fDepth = source.fDepth;
			occlusionCullingData = source.occlusionCullingData;

			return *this;
		}

		RenderJobStatic& RenderJobStatic::operator = (RenderJobStatic&& source) noexcept
		{
			pKey = std::move(source.pKey);
			pVertexBuffer = std::move(source.pVertexBuffer);
			pIndexBuffer = std::move(source.pIndexBuffer);
			pMaterial = std::move(source.pMaterial);
			matWorld = std::move(source.matWorld);
			nStartIndex = std::move(source.nStartIndex);
			nIndexCount = std::move(source.nIndexCount);
			fDepth = std::move(source.fDepth);
			occlusionCullingData = std::move(source.occlusionCullingData);

			return *this;
		}

		RenderJobSkinned::RenderJobSkinned(const void* pKey, const IVertexBuffer* pVertexBuffer, const IIndexBuffer* pIndexBuffer, const IMaterial* pMaterial,
			const math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount, uint32_t nVTFID,
			float fDepth, const OcclusionCullingData& occlusionCullingData)
			: pKey(pKey), pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMaterial(pMaterial)
			, matWorld(matWorld), nStartIndex(nStartIndex), nIndexCount(nIndexCount), nVTFID(nVTFID)
			, fDepth(fDepth), occlusionCullingData(occlusionCullingData)
		{
		}

		RenderJobSkinned::RenderJobSkinned(const RenderJobSkinned& source)
		{
			*this = source;
		}

		RenderJobSkinned::RenderJobSkinned(RenderJobSkinned&& source) noexcept
		{
			*this = std::move(source);
		}

		RenderJobSkinned& RenderJobSkinned::operator = (const RenderJobSkinned& source)
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
			occlusionCullingData = source.occlusionCullingData;

			return *this;
		}

		RenderJobSkinned& RenderJobSkinned::operator = (RenderJobSkinned&& source) noexcept
		{
			pKey = std::move(source.pKey);
			pVertexBuffer = std::move(source.pVertexBuffer);
			pIndexBuffer = std::move(source.pIndexBuffer);
			pMaterial = std::move(source.pMaterial);
			matWorld = std::move(source.matWorld);
			nStartIndex = std::move(source.nStartIndex);
			nIndexCount = std::move(source.nIndexCount);
			nVTFID = std::move(source.nVTFID);
			fDepth = std::move(source.fDepth);
			occlusionCullingData = std::move(source.occlusionCullingData);

			return *this;
		}

		RenderJobVertex::RenderJobVertex(const IVertexBuffer* pVertexBuffer, const IIndexBuffer* pIndexBuffer, const math::Matrix& matWorld, const math::Color& color)
			: pVertexBuffer(pVertexBuffer)
			, pIndexBuffer(pIndexBuffer)
			, matWorld(matWorld)
			, color(color)
		{
		}

		RenderJobVertex::RenderJobVertex(const RenderJobVertex& source)
		{
			*this = source;
		}

		RenderJobVertex::RenderJobVertex(RenderJobVertex&& source) noexcept
		{
			*this = std::move(source);
		}

		RenderJobVertex& RenderJobVertex::operator = (const RenderJobVertex& source)
		{
			pVertexBuffer = source.pVertexBuffer;
			pIndexBuffer = source.pIndexBuffer;
			matWorld = source.matWorld;
			color = source.color;
			return *this;
		}

		RenderJobVertex& RenderJobVertex::operator = (RenderJobVertex&& source) noexcept
		{
			pVertexBuffer = std::move(source.pVertexBuffer);
			pIndexBuffer = std::move(source.pIndexBuffer);
			matWorld = std::move(source.matWorld);
			color = std::move(source.color);
			return *this;
		}
	}
}