#include "stdafx.h"
#include "BVHStructure.h"

#include "PhysicsUtil.h"

namespace est
{
	namespace physics
	{
		BVHStructure::BVHStructure(physx::PxBVHStructure* pBVHStructure)
			: m_pBVHStructure(pBVHStructure)
		{
		}

		BVHStructure::~BVHStructure()
		{
			m_pBVHStructure->release();
			m_pBVHStructure = nullptr;
		}

		uint32_t BVHStructure::Raycast(const math::float3& origin, const math::float3& unitDir, float maxDist, uint32_t maxHits, uint32_t* __restrict rayHits) const
		{
			return m_pBVHStructure->raycast(
				Convert<const physx::PxVec3>(origin),
				Convert<const physx::PxVec3>(unitDir),
				maxDist,
				maxHits,
				rayHits);
		}

		uint32_t BVHStructure::Sweep(const Bounds& aabb, const math::float3& unitDir, float maxDist, uint32_t maxHits, uint32_t* __restrict sweepHits) const
		{
			return m_pBVHStructure->sweep(
				Convert<const physx::PxBounds3>(aabb),
				Convert<const physx::PxVec3>(unitDir),
				maxDist,
				maxHits,
				sweepHits);
		}

		uint32_t BVHStructure::Overlap(const Bounds& aabb, uint32_t maxHits, uint32_t* __restrict overlapHits) const
		{
			return m_pBVHStructure->overlap(
				Convert<const physx::PxBounds3>(aabb),
				maxHits, overlapHits);
		}

		const Bounds* BVHStructure::GetBounds() const
		{
			return reinterpret_cast<const Bounds*>(m_pBVHStructure->getBounds());
		}

		uint32_t BVHStructure::GetNumBounds() const
		{
			return m_pBVHStructure->getNbBounds();
		}
	}
}