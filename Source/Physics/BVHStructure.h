#pragma once

#include "PhysicsInterface.h"

namespace est
{
	namespace physics
	{
		class BVHStructure : public IBVHStructure
		{
		public:
			BVHStructure(physx::PxBVHStructure* pBVHStructure);
			virtual ~BVHStructure();

		public:
			virtual uint32_t Raycast(const math::float3& origin, const math::float3& unitDir, float maxDist, uint32_t maxHits, uint32_t* __restrict rayHits) const override;
			virtual uint32_t Sweep(const Bounds& aabb, const math::float3& unitDir, float maxDist, uint32_t maxHits, uint32_t* __restrict sweepHits) const override;
			virtual uint32_t Overlap(const Bounds& aabb, uint32_t maxHits, uint32_t* __restrict overlapHits) const override;

			virtual const Bounds* GetBounds() const override;
			virtual uint32_t GetNumBounds() const override;

		public:
			physx::PxBVHStructure* GetInterface() const { return m_pBVHStructure; }

		private:
			physx::PxBVHStructure* m_pBVHStructure{ nullptr };
		};
	}
}