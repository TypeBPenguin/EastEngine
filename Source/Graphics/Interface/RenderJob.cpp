#include "stdafx.h"
#include "RenderJob.h"

namespace est
{
	namespace graphics
	{
		RenderJobStatic::RenderJobStatic(const void* pKey, const VertexBufferPtr& pVertexBuffer, const IndexBufferPtr& pIndexBuffer, const MaterialPtr& pMaterial,
			const math::Matrix& worldMatrix, const math::Matrix& prevWorldMatrix, uint32_t startIndex, uint32_t indexCount,
			float depth, const OcclusionCullingData& occlusionCullingData)
			: pKey(pKey), pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMaterial(pMaterial)
			, worldMatrix(worldMatrix), prevWorldMatrix(prevWorldMatrix), startIndex(startIndex), indexCount(indexCount)
			, depth(depth), occlusionCullingData(occlusionCullingData)
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
			worldMatrix = source.worldMatrix;
			prevWorldMatrix = source.prevWorldMatrix;
			startIndex = source.startIndex;
			indexCount = source.indexCount;
			depth = source.depth;
			occlusionCullingData = source.occlusionCullingData;

			return *this;
		}

		RenderJobStatic& RenderJobStatic::operator = (RenderJobStatic&& source) noexcept
		{
			pKey = std::move(source.pKey);
			pVertexBuffer = std::move(source.pVertexBuffer);
			pIndexBuffer = std::move(source.pIndexBuffer);
			pMaterial = std::move(source.pMaterial);
			worldMatrix = std::move(source.worldMatrix);
			prevWorldMatrix = std::move(source.prevWorldMatrix);
			startIndex = std::move(source.startIndex);
			indexCount = std::move(source.indexCount);
			depth = std::move(source.depth);
			occlusionCullingData = std::move(source.occlusionCullingData);

			return *this;
		}

		RenderJobSkinned::RenderJobSkinned(const void* pKey, const VertexBufferPtr& pVertexBuffer, const IndexBufferPtr& pIndexBuffer, const MaterialPtr& pMaterial,
			const math::Matrix& worldMatrix, const math::Matrix& prevWorldMatrix,
			uint32_t startIndex, uint32_t indexCount, uint32_t VTFID, uint32_t PrevVTFID,
			float depth, const OcclusionCullingData& occlusionCullingData)
			: pKey(pKey), pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMaterial(pMaterial)
			, worldMatrix(worldMatrix), prevWorldMatrix(prevWorldMatrix), startIndex(startIndex), indexCount(indexCount), VTFID(VTFID), PrevVTFID(PrevVTFID)
			, depth(depth), occlusionCullingData(occlusionCullingData)
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
			worldMatrix = source.worldMatrix;
			prevWorldMatrix = source.prevWorldMatrix;
			startIndex = source.startIndex;
			indexCount = source.indexCount;
			VTFID = source.VTFID;
			PrevVTFID = source.PrevVTFID;
			depth = source.depth;
			occlusionCullingData = source.occlusionCullingData;

			return *this;
		}

		RenderJobSkinned& RenderJobSkinned::operator = (RenderJobSkinned&& source) noexcept
		{
			pKey = std::move(source.pKey);
			pVertexBuffer = std::move(source.pVertexBuffer);
			pIndexBuffer = std::move(source.pIndexBuffer);
			pMaterial = std::move(source.pMaterial);
			worldMatrix = std::move(source.worldMatrix);
			prevWorldMatrix = std::move(source.prevWorldMatrix);
			startIndex = std::move(source.startIndex);
			indexCount = std::move(source.indexCount);
			VTFID = std::move(source.VTFID);
			PrevVTFID = std::move(source.PrevVTFID);
			depth = std::move(source.depth);
			occlusionCullingData = std::move(source.occlusionCullingData);

			return *this;
		}

		RenderJobVertex::RenderJobVertex(const VertexBufferPtr& pVertexBuffer, const IndexBufferPtr& pIndexBuffer, const math::Matrix& worldMatrix, const math::Color& color)
			: pVertexBuffer(pVertexBuffer)
			, pIndexBuffer(pIndexBuffer)
			, worldMatrix(worldMatrix)
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
			worldMatrix = source.worldMatrix;
			color = source.color;
			return *this;
		}

		RenderJobVertex& RenderJobVertex::operator = (RenderJobVertex&& source) noexcept
		{
			pVertexBuffer = std::move(source.pVertexBuffer);
			pIndexBuffer = std::move(source.pIndexBuffer);
			worldMatrix = std::move(source.worldMatrix);
			color = std::move(source.color);
			return *this;
		}
	}
}