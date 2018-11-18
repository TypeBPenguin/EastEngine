#pragma once

#include "GraphicsInterface/Vertex.h"

namespace eastengine
{
	namespace graphics
	{
		class IVertexBuffer;
		class IIndexBuffer;

		namespace geometry
		{
			enum DebugModelType
			{
				eBox,
				eSphere,
				eCount,
			};

			bool Initialize();
			void Release();

			void GetDebugModel(DebugModelType emType, IVertexBuffer** ppVertexBuffer, IIndexBuffer** ppIndexBuffer);

			void CalculateTangentBinormal(const VertexPosTex& vertex1, const VertexPosTex& vertex2, const VertexPosTex& vertex3, math::float3& tangent, math::float3& binormal);
			void CalculateTangentBinormal(const math::float3& normal, math::float3& tangent, math::float3& binormal);
			void CalculateNormal(const math::float3& vPos0, const math::float3& vPos1, const math::float3& vPos2, math::float3& vNormal);

			bool CreateCube(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size = 1.f, bool rhcoords = false);
			bool CreateBox(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, const math::float3& size = math::float3(1.f, 1.f, 1.f), bool rhcoords = false, bool invertn = false);
			bool CreateSphere(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float diameter = 1.f, uint32_t tessellation = 16, bool rhcoords = false, bool invertn = false);
			bool CreateGeoSphere(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float diameter = 1.f, uint32_t tessellation = 3, bool rhcoords = false);
			bool CreateCylinder(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float height = 1.f, float diameter = 1.f, uint32_t tessellation = 32, bool rhcoords = false);
			bool CreateCone(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float diameter = 1.f, float height = 1.f, uint32_t tessellation = 32, bool rhcoords = false);
			bool CreateTorus(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float diameter = 1.f, float thickness = 0.333f, uint32_t tessellation = 32, bool rhcoords = false);
			bool CreateTetrahedron(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size = 1.f, bool rhcoords = false);
			bool CreateOctahedron(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size = 1.f, bool rhcoords = false);
			bool CreateDodecahedron(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size = 1.f, bool rhcoords = false);
			bool CreateIcosahedron(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size = 1.f, bool rhcoords = false);
			bool CreateTeapot(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float size = 1.f, uint32_t tessellation = 8, bool rhcoords = false);
			bool CreateHexagon(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float radius, bool rhcoords = false);
			bool CreateCapsule(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float fRadius, float fHeight, int nSubdivisionHeight, int nSegments);
			bool CreateGrid(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float fGridSizeX, float fGridSizeZ, uint32_t nBlockCountWidth, uint32_t nBlockCountLength);
			bool CreatePlane(std::vector<VertexPosTexNor>& vertices, std::vector<uint32_t>& indices, float fEachLengthX, float fEachLengthZ, int nTotalCountX, int nTotalCountZ);

			namespace Simplify
			{
				bool GenerateSimplificationMesh(const std::vector<VertexPosTexNor>& in_vecVertices, const std::vector<uint32_t>& in_vecIndices, std::vector<VertexPosTexNor>& out_vecVertices, std::vector<uint32_t>& out_vecIndices, float fReduceFraction, float dAgressiveness = 7.0);
			}
		}
	}
}