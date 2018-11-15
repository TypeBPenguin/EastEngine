#pragma once

namespace eastengine
{
	namespace physics
	{
		class DebugTriangleDrawCallback : public btTriangleCallback
		{
		public:
			DebugTriangleDrawCallback(std::vector<math::float3>* pTriangles);
			virtual ~DebugTriangleDrawCallback() = default;

			void processTriangle(btVector3* triangle, int partId, int triangleIndex);

		private:
			std::vector<math::float3>* m_pTriangles;
		};
	}
}