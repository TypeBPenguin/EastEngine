#include "stdafx.h"
#include "RenderJob.h"

namespace eastengine
{
	namespace graphics
	{
		RenderJobStatic::RenderJobStatic(const void* pKey, const IVertexBuffer* pVertexBuffer, const IIndexBuffer* pIndexBuffer, const IMaterial* pMaterial,
			const math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount,
			float fDepth, const Collision::Sphere& boundingSphere)
			: pKey(pKey), pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMaterial(pMaterial)
			, matWorld(matWorld), nStartIndex(nStartIndex), nIndexCount(nIndexCount)
			, fDepth(fDepth), boundingSphere(boundingSphere)
		{
		}

		RenderJobStatic::RenderJobStatic(RenderJobStatic&& source) noexcept
			: pKey(std::move(source.pKey))
			, pVertexBuffer(std::move(source.pVertexBuffer))
			, pIndexBuffer(std::move(source.pIndexBuffer))
			, pMaterial(std::move(source.pMaterial))
			, matWorld(std::move(source.matWorld))
			, nStartIndex(std::move(source.nStartIndex))
			, nIndexCount(std::move(source.nIndexCount))
			, fDepth(std::move(source.fDepth))
			, boundingSphere(std::move(source.boundingSphere))
		{
		}

		RenderJobSkinned::RenderJobSkinned(const void* pKey, const IVertexBuffer* pVertexBuffer, const IIndexBuffer* pIndexBuffer, const IMaterial* pMaterial,
			const math::Matrix& matWorld, uint32_t nStartIndex, uint32_t nIndexCount, uint32_t nVTFID,
			float fDepth)
			: pKey(pKey), pVertexBuffer(pVertexBuffer), pIndexBuffer(pIndexBuffer), pMaterial(pMaterial)
			, matWorld(matWorld), nStartIndex(nStartIndex), nIndexCount(nIndexCount)
			, nVTFID(nVTFID)
			, fDepth(fDepth)
		{
		}

		RenderJobSkinned::RenderJobSkinned(RenderJobSkinned&& source) noexcept
			: pKey(std::move(source.pKey))
			, pVertexBuffer(std::move(source.pVertexBuffer))
			, pIndexBuffer(std::move(source.pIndexBuffer))
			, pMaterial(std::move(source.pMaterial))
			, matWorld(std::move(source.matWorld))
			, nStartIndex(std::move(source.nStartIndex))
			, nIndexCount(std::move(source.nIndexCount))
			, nVTFID(std::move(source.nVTFID))
			, fDepth(std::move(source.fDepth))
		{
		}
	}
}