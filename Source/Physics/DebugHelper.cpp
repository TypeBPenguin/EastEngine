#include "stdafx.h"
#include "DebugHelper.h"

#include "MathConvertor.h"

namespace eastengine
{
	namespace physics
	{
		DebugTriangleDrawCallback::DebugTriangleDrawCallback(std::vector<math::float3>* pTriangles)
			: m_pTriangles(pTriangles)
		{
		}

		void DebugTriangleDrawCallback::processTriangle(btVector3* triangle, int partId, int triangleIndex)
		{
			m_pTriangles->emplace_back(math::Convert(*triangle));
			m_pTriangles->emplace_back(math::Convert(*(triangle + 2)));
			m_pTriangles->emplace_back(math::Convert(*(triangle + 1)));
		}
	}
}