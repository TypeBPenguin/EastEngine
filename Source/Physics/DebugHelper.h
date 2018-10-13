#pragma once

namespace eastengine
{
	namespace physics
	{
		class DebugTriangleDrawCallback : public btTriangleCallback
		{
		public:
			DebugTriangleDrawCallback(std::vector<math::Vector3>* pTriangles);
			virtual ~DebugTriangleDrawCallback() = default;

			void processTriangle(btVector3* triangle, int partId, int triangleIndex);

		private:
			std::vector<math::Vector3>* m_pTriangles;
		};
	}
}