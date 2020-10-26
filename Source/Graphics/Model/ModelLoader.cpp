#include "stdafx.h"
#include "ModelLoader.h"

#include "CommonLib/FileUtil.h"

namespace est
{
	namespace graphics
	{
		ModelLoader::GeometryType ModelLoader::GetGeometryType(const wchar_t* strType)
		{
			if (string::IsEquals(strType, L"CustomStaticModel"))
				return eCustomStaticModel;
			if (string::IsEquals(strType, L"Cube"))
				return eCube;
			if (string::IsEquals(strType, L"Box"))
				return eBox;
			if (string::IsEquals(strType, L"Sphere"))
				return eSphere;
			if (string::IsEquals(strType, L"GeoSphere"))
				return eGeoSphere;
			if (string::IsEquals(strType, L"Cylinder"))
				return eCylinder;
			if (string::IsEquals(strType, L"Cone"))
				return eCone;
			if (string::IsEquals(strType, L"Torus"))
				return eTorus;
			if (string::IsEquals(strType, L"Tetrahedron"))
				return eTetrahedron;
			if (string::IsEquals(strType, L"Octahedron"))
				return eOctahedron;
			if (string::IsEquals(strType, L"Dodecahedron"))
				return eDodecahedron;
			if (string::IsEquals(strType, L"Icosahedron"))
				return eIcosahedron;
			if (string::IsEquals(strType, L"Teapot"))
				return eTeapot;
			if (string::IsEquals(strType, L"Hexagon"))
				return eHexagon;
			if (string::IsEquals(strType, L"Capsule"))
				return eCapsule;
			if (string::IsEquals(strType, L"Grid"))
				return eGrid;
			if (string::IsEquals(strType, L"Plane"))
				return ePlane;

			return eInvalid;
		}

		const wchar_t* ModelLoader::GetGeometryTypeToString(GeometryType emType)
		{
			switch (emType)
			{
			case eCustomStaticModel:
				return L"CustomStaticModel";
			case eCube:
				return L"Cube";
			case eBox:
				return L"Box";
			case eSphere:
				return L"Sphere";
			case eGeoSphere:
				return L"GeoSphere";
			case eCylinder:
				return L"Cylinder";
			case eCone:
				return L"Cone";
			case eTorus:
				return L"Torus";
			case eTetrahedron:
				return L"Tetrahedron";
			case eOctahedron:
				return L"Octahedron";
			case eDodecahedron:
				return L"Dodecahedron";
			case eIcosahedron:
				return L"Icosahedron";
			case eTeapot:
				return L"Teapot";
			case eHexagon:
				return L"Hexagon";
			case eCapsule:
				return L"Capsule";
			case eGrid:
				return L"Grid";
			case ePlane:
				return L"Plane";
			}

			return L"Invalid";
		}

		ModelLoader::ModelLoader(std::function<void(bool)> funcCallback)
			: m_funcCallback(funcCallback)
		{
		}

		ModelLoader::~ModelLoader()
		{
		}

		void ModelLoader::InitFBX(const string::StringID& modelName, const wchar_t* filePath, float scaleFactor, bool isFlipZ)
		{
			m_modelName = modelName;
			m_filePath = filePath;
			m_scaleFactor = scaleFactor;
			m_isFlipZ = isFlipZ;

			m_emLoadModelType = ModelLoader::eFbx;
		}

		void ModelLoader::InitObj(const string::StringID& modelName, const wchar_t* filePath, float scaleFactor)
		{
			m_modelName = modelName;
			m_filePath = filePath;
			m_scaleFactor = scaleFactor;

			m_emLoadModelType = ModelLoader::eObj;
		}

		void ModelLoader::InitXPS(const string::StringID& modelName, const wchar_t* filePath)
		{
			m_modelName = modelName;
			m_filePath = filePath;

			m_emLoadModelType = ModelLoader::eXps;
		}

		void ModelLoader::InitEast(const string::StringID& modelName, const wchar_t* filePath)
		{
			m_modelName = modelName;
			m_filePath = filePath;
			m_scaleFactor = 1.f;

			m_emLoadModelType = ModelLoader::eEast;
		}

		void ModelLoader::InitCube(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float size, bool rhcoords)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;

			LoadInfoCube& loadInfo = m_loadGeometryInfo.emplace<LoadInfoCube>();
			loadInfo.size = size;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eCube;
		}

		void ModelLoader::InitBox(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			const math::float3& halfExtents, bool rhcoords, bool invertn)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;
			m_isInvertn = invertn;

			LoadInfoBox& loadInfo = m_loadGeometryInfo.emplace<LoadInfoBox>();
			loadInfo.halfExtents = halfExtents;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eBox;
		}

		void ModelLoader::InitSphere(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float diameter, uint32_t nTessellation, bool rhcoords, bool invertn)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;
			m_isInvertn = invertn;

			LoadInfoSphere& loadInfo = m_loadGeometryInfo.emplace<LoadInfoSphere>();
			loadInfo.diameter = diameter;
			loadInfo.tessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eSphere;
		}

		void ModelLoader::InitGeoSphere(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float diameter, uint32_t nTessellation, bool rhcoords)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;

			LoadInfoGeoSphere& loadInfo = m_loadGeometryInfo.emplace<LoadInfoGeoSphere>();
			loadInfo.diameter = diameter;
			loadInfo.tessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eGeoSphere;
		}

		void ModelLoader::InitCylinder(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float height, float diameter, uint32_t nTessellation, bool rhcoords)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;

			LoadInfoCylinder& loadInfo = m_loadGeometryInfo.emplace<LoadInfoCylinder>();
			loadInfo.height = height;
			loadInfo.diameter = diameter;
			loadInfo.tessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eCylinder;
		}

		void ModelLoader::InitCone(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float diameter, float height, uint32_t nTessellation, bool rhcoords)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;

			LoadInfoCone& loadInfo = m_loadGeometryInfo.emplace<LoadInfoCone>();
			loadInfo.diameter = diameter;
			loadInfo.height = height;
			loadInfo.tessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eCone;
		}

		void ModelLoader::InitTorus(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float diameter, float thickness, uint32_t nTessellation, bool rhcoords)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;

			LoadInfoTorus& loadInfo = m_loadGeometryInfo.emplace<LoadInfoTorus>();
			loadInfo.diameter = diameter;
			loadInfo.thickness = thickness;
			loadInfo.tessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eTorus;
		}

		void ModelLoader::InitTetrahedron(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float size, bool rhcoords)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;

			LoadInfoTetrahedron& loadInfo = m_loadGeometryInfo.emplace<LoadInfoTetrahedron>();
			loadInfo.size = size;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eTetrahedron;
		}

		void ModelLoader::InitOctahedron(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float size, bool rhcoords)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;

			LoadInfoOctahedron& loadInfo = m_loadGeometryInfo.emplace<LoadInfoOctahedron>();
			loadInfo.size = size;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eOctahedron;
		}

		void ModelLoader::InitDodecahedron(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float size, bool rhcoords)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;

			LoadInfoDodecahedron& loadInfo = m_loadGeometryInfo.emplace<LoadInfoDodecahedron>();
			loadInfo.size = size;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eDodecahedron;
		}

		void ModelLoader::InitIcosahedron(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float size, bool rhcoords)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;

			LoadInfoIcosahedron& loadInfo = m_loadGeometryInfo.emplace<LoadInfoIcosahedron>();
			loadInfo.size = size;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eIcosahedron;
		}

		void ModelLoader::InitTeapot(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float size, uint32_t nTessellation, bool rhcoords)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();
			m_isRhcoords = rhcoords;

			LoadInfoTeapot& loadInfo = m_loadGeometryInfo.emplace<LoadInfoTeapot>();
			loadInfo.size = size;
			loadInfo.tessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eTeapot;
		}

		void ModelLoader::InitHexagon(const string::StringID& modelName, const IMaterial::Data* pMaterialData,
			float radius)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();

			LoadInfoHexagon& loadInfo = m_loadGeometryInfo.emplace<LoadInfoHexagon>();
			loadInfo.radius = radius;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eHexagon;
		}

		void ModelLoader::InitCapsule(const string::StringID& modelName, const IMaterial::Data* pMaterialData, float radius, float height, int nSubdivisionHeight, int nSegments)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();

			LoadInfoCapsule& loadInfo = m_loadGeometryInfo.emplace<LoadInfoCapsule>();
			loadInfo.radius = radius;
			loadInfo.height = height;
			loadInfo.subdivisionHeight = nSubdivisionHeight;
			loadInfo.segments = nSegments;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eCapsule;
		}

		void ModelLoader::InitGrid(const string::StringID& modelName, float gridSizeX, float gridSizeZ,
			uint32_t nBlockCountWidth, uint32_t nBlockCountLength, const IMaterial::Data* pMaterialData)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();

			LoadInfoGrid& loadInfo = m_loadGeometryInfo.emplace<LoadInfoGrid>();
			loadInfo.gridSizeX = gridSizeX;
			loadInfo.gridSizeZ = gridSizeZ;
			loadInfo.blockCountWidth = nBlockCountWidth;
			loadInfo.blockCountLength = nBlockCountLength;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eGrid;
		}

		void ModelLoader::InitPlane(const string::StringID& modelName, float eachLengthX, float eachLengthZ,
			int totalCountX, int totalCountZ, const IMaterial::Data* pMaterialData)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();

			LoadInfoPlane& loadInfo = m_loadGeometryInfo.emplace<LoadInfoPlane>();
			loadInfo.eachLengthX = eachLengthX;
			loadInfo.eachLengthZ = eachLengthZ;
			loadInfo.totalCountX = totalCountX;
			loadInfo.totalCountZ = totalCountZ;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::ePlane;
		}

		void ModelLoader::InitCustomStaticModel(const string::StringID& modelName, const string::StringID& nodeName,
			const VertexBufferPtr& pVertexBuffer, const IndexBufferPtr& pIndexBuffer, const IMaterial::Data* pMaterialData)
		{
			m_modelName = modelName;
			m_materialData = pMaterialData != nullptr ? *pMaterialData : IMaterial::Data();

			LoadInfoCustomStaticModel& loadInfo = m_loadGeometryInfo.emplace<LoadInfoCustomStaticModel>();
			loadInfo.nodeName = nodeName;
			loadInfo.pVertexBuffer = pVertexBuffer;
			loadInfo.pIndexBuffer = pIndexBuffer;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eCustomStaticModel;
		}

		void ModelLoader::AddAnimInfoByFBXFolder(const wchar_t* animPath)
		{
			std::vector<std::wstring> vecFiles;
			file::GetFiles(animPath, L".fbx", vecFiles);

			std::copy(vecFiles.begin(), vecFiles.end(), std::back_inserter(m_vecAnimationList));
		}
	}
}