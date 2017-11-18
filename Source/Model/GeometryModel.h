#pragma once

#include "DirectX/Vertex.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IVertexBuffer;
		class IIndexBuffer;

		namespace GeometryModel
		{
			void CalculateTangentBinormal(const VertexPosTex& vertex1, const VertexPosTex& vertex2, const VertexPosTex& vertex3, Math::Vector3& tangent, Math::Vector3& binormal);
			void CalculateTangentBinormal(const Math::Vector3& normal, Math::Vector3& tangent, Math::Vector3& binormal);
			void CalculateNormal(const Math::Vector3& vPos0, const Math::Vector3& vPos1, const Math::Vector3& vPos2, Math::Vector3& vNormal);

			bool CreateCube(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size = 1.f, bool rhcoords = false);
			bool CreateBox(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, const Math::Vector3& size = Math::Vector3(1.f, 1.f, 1.f), bool rhcoords = false, bool invertn = false);
			bool CreateSphere(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float diameter = 1.f, uint32_t tessellation = 16, bool rhcoords = false, bool invertn = false);
			bool CreateGeoSphere(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float diameter = 1.f, uint32_t tessellation = 3, bool rhcoords = false);
			bool CreateCylinder(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float height = 1.f, float diameter = 1.f, uint32_t tessellation = 32, bool rhcoords = false);
			bool CreateCone(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float diameter = 1.f, float height = 1.f, uint32_t tessellation = 32, bool rhcoords = false);
			bool CreateTorus(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float diameter = 1.f, float thickness = 0.333f, uint32_t tessellation = 32, bool rhcoords = false);
			bool CreateTetrahedron(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size = 1.f, bool rhcoords = false);
			bool CreateOctahedron(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size = 1.f, bool rhcoords = false);
			bool CreateDodecahedron(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size = 1.f, bool rhcoords = false);
			bool CreateIcosahedron(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size = 1.f, bool rhcoords = false);
			bool CreateTeapot(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float size = 1.f, uint32_t tessellation = 8, bool rhcoords = false);
			bool CreateHexagon(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float radius, bool rhcoords = false);
			bool CreateCapsule(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float fRadius, float fHeight, int nSubdivisionHeight, int nSegments);
			bool CreateGrid(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float fGridSizeX, float fGridSizeZ, uint32_t nBlockCountWidth, uint32_t nBlockCountLength);
			bool CreatePlane(IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer, float fEachLengthX, float fEachLengthZ, int nTotalCountX, int nTotalCountZ);

			namespace Simplify
			{
				bool GenerateSimplificationMesh(const std::vector<VertexPosTexNor>& in_vecVertices, const std::vector<uint32_t>& in_vecIndices, std::vector<VertexPosTexNor>& out_vecVertices, std::vector<uint32_t>& out_vecIndices, float fReduceFraction, double dAgressiveness = 7.0);
			}
		}
	}
}