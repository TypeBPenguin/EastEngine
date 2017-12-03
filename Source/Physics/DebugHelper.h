#pragma once

namespace EastEngine
{
	namespace Physics
	{
		class DebugTriangleDrawCallback : public btTriangleCallback
		{
		public:
			DebugTriangleDrawCallback(std::vector<Math::Vector3>* pTriangles);
			virtual ~DebugTriangleDrawCallback() = default;

			void processTriangle(btVector3* triangle, int partId, int triangleIndex);

		private:
			std::vector<Math::Vector3>* m_pTriangles;
		};
	}
}