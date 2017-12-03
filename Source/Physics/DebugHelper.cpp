#include "stdafx.h"
#include "DebugHelper.h"

#include "MathConvertor.h"

namespace EastEngine
{
	namespace Physics
	{
		DebugTriangleDrawCallback::DebugTriangleDrawCallback(std::vector<Math::Vector3>* pTriangles)
			: m_pTriangles(pTriangles)
		{
		}

		void DebugTriangleDrawCallback::processTriangle(btVector3* triangle, int partId, int triangleIndex)
		{
			m_pTriangles->emplace_back(Math::Convert(*triangle));
			m_pTriangles->emplace_back(Math::Convert(*(triangle + 2)));
			m_pTriangles->emplace_back(Math::Convert(*(triangle + 1)));
		}
	}
}