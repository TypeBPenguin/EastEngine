#include "stdafx.h"
#include "ModelLoader.h"

#include "CommonLib/FileUtil.h"

#include "ModelInterface.h"

namespace eastengine
{
	namespace graphics
	{
		ModelLoader::GeometryType ModelLoader::GetGeometryType(const char* strType)
		{
			if (String::IsEquals(strType, "CustomStaticModel"))
				return eCustomStaticModel;
			if (String::IsEquals(strType, "Cube"))
				return eCube;
			if (String::IsEquals(strType, "Box"))
				return eBox;
			if (String::IsEquals(strType, "Sphere"))
				return eSphere;
			if (String::IsEquals(strType, "GeoSphere"))
				return eGeoSphere;
			if (String::IsEquals(strType, "Cylinder"))
				return eCylinder;
			if (String::IsEquals(strType, "Cone"))
				return eCone;
			if (String::IsEquals(strType, "Torus"))
				return eTorus;
			if (String::IsEquals(strType, "Tetrahedron"))
				return eTetrahedron;
			if (String::IsEquals(strType, "Octahedron"))
				return eOctahedron;
			if (String::IsEquals(strType, "Dodecahedron"))
				return eDodecahedron;
			if (String::IsEquals(strType, "Icosahedron"))
				return eIcosahedron;
			if (String::IsEquals(strType, "Teapot"))
				return eTeapot;
			if (String::IsEquals(strType, "Hexagon"))
				return eHexagon;
			if (String::IsEquals(strType, "Capsule"))
				return eCapsule;
			if (String::IsEquals(strType, "Grid"))
				return eGrid;
			if (String::IsEquals(strType, "Plane"))
				return ePlane;

			return eInvalid;
		}

		const char* ModelLoader::GetGeometryTypeToSTring(GeometryType emType)
		{
			switch (emType)
			{
			case eCustomStaticModel:
				return "CustomStaticModel";
			case eCube:
				return "Cube";
			case eBox:
				return "Box";
			case eSphere:
				return "Sphere";
			case eGeoSphere:
				return "GeoSphere";
			case eCylinder:
				return "Cylinder";
			case eCone:
				return "Cone";
			case eTorus:
				return "Torus";
			case eTetrahedron:
				return "Tetrahedron";
			case eOctahedron:
				return "Octahedron";
			case eDodecahedron:
				return "Dodecahedron";
			case eIcosahedron:
				return "Icosahedron";
			case eTeapot:
				return "Teapot";
			case eHexagon:
				return "Hexagon";
			case eCapsule:
				return "Capsule";
			case eGrid:
				return "Grid";
			case ePlane:
				return "Plane";
			}

			return "None";
		}

		ModelLoader::ModelLoader(std::function<void(bool)> funcCallback)
			: m_isEnableThreadLoad(false)
			, m_f3Scale(math::Vector3::One)
			, m_funcCallback(funcCallback)
			, m_fScaleFactor(0.f)
			, m_isFlipZ(true)
			, m_emLoadModelType(ModelLoader::eGeometry)
			, m_emLoadGeometryType(ModelLoader::eCustomStaticModel)
			, m_isRhcoords(false)
			, m_isInvertn(false)
			, m_nLodMax(0)
		{
		}

		ModelLoader::~ModelLoader()
		{
		}

		void ModelLoader::InitFBX(const String::StringID& strModelName, const char* strFilePath, float fScaleFactor, bool isFlipZ, uint32_t nLodMax, const LODReductionRate& reductionRate)
		{
			m_strModelName = strModelName;
			m_strFilePath = strFilePath;
			m_fScaleFactor = fScaleFactor;
			m_isFlipZ = isFlipZ;

			m_nLodMax = nLodMax;
			m_lodReductionRate = reductionRate;

			m_emLoadModelType = ModelLoader::eFbx;
		}

		void ModelLoader::InitObj(const String::StringID& strModelName, const char* strFilePath, float fScaleFactor, uint32_t nLodMax, const LODReductionRate& reductionRate)
		{
			m_strModelName = strModelName;
			m_strFilePath = strFilePath;
			m_fScaleFactor = fScaleFactor;

			m_nLodMax = nLodMax;
			m_lodReductionRate = reductionRate;

			m_emLoadModelType = ModelLoader::eObj;
		}

		void ModelLoader::InitXPS(const String::StringID& strModelName, const char* strFilePath)
		{
			m_strModelName = strModelName;
			m_strFilePath = strFilePath;

			m_emLoadModelType = ModelLoader::eXps;
		}

		void ModelLoader::InitEast(const String::StringID& strModelName, const char* strFilePath)
		{
			m_strModelName = strModelName;
			m_strFilePath = strFilePath;
			m_fScaleFactor = 1.f;

			m_emLoadModelType = ModelLoader::eEast;
		}

		void ModelLoader::InitCube(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			LoadInfoCube& loadInfo = m_loadGeometryInfo.emplace<LoadInfoCube>();
			loadInfo.fSize = fSize;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eCube;
		}

		void ModelLoader::InitBox(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			const math::Vector3& size, bool rhcoords, bool invertn)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;
			m_isInvertn = invertn;

			LoadInfoBox& loadInfo = m_loadGeometryInfo.emplace<LoadInfoBox>();
			loadInfo.f3Size = size;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eBox;
		}

		void ModelLoader::InitSphere(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float diameter, uint32_t nTessellation, bool rhcoords, bool invertn)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;
			m_isInvertn = invertn;

			LoadInfoSphere& loadInfo = m_loadGeometryInfo.emplace<LoadInfoSphere>();
			loadInfo.fDiameter = diameter;
			loadInfo.nTessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eSphere;
		}

		void ModelLoader::InitGeoSphere(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float diameter, uint32_t nTessellation, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			LoadInfoGeoSphere& loadInfo = m_loadGeometryInfo.emplace<LoadInfoGeoSphere>();
			loadInfo.fDiameter = diameter;
			loadInfo.nTessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eGeoSphere;
		}

		void ModelLoader::InitCylinder(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float height, float diameter, uint32_t nTessellation, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			LoadInfoCylinder& loadInfo = m_loadGeometryInfo.emplace<LoadInfoCylinder>();
			loadInfo.fHeight = height;
			loadInfo.fDiameter = diameter;
			loadInfo.nTessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eCylinder;
		}

		void ModelLoader::InitCone(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float diameter, float height, uint32_t nTessellation, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			LoadInfoCone& loadInfo = m_loadGeometryInfo.emplace<LoadInfoCone>();
			loadInfo.fDiameter = diameter;
			loadInfo.fHeight = height;
			loadInfo.nTessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eCone;
		}

		void ModelLoader::InitTorus(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float diameter, float thickness, uint32_t nTessellation, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			LoadInfoTorus& loadInfo = m_loadGeometryInfo.emplace<LoadInfoTorus>();
			loadInfo.fDiameter = diameter;
			loadInfo.fThickness = thickness;
			loadInfo.nTessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eTorus;
		}

		void ModelLoader::InitTetrahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			LoadInfoTetrahedron& loadInfo = m_loadGeometryInfo.emplace<LoadInfoTetrahedron>();
			loadInfo.fSize = fSize;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eTetrahedron;
		}

		void ModelLoader::InitOctahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			LoadInfoOctahedron& loadInfo = m_loadGeometryInfo.emplace<LoadInfoOctahedron>();
			loadInfo.fSize = fSize;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eOctahedron;
		}

		void ModelLoader::InitDodecahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			LoadInfoDodecahedron& loadInfo = m_loadGeometryInfo.emplace<LoadInfoDodecahedron>();
			loadInfo.fSize = fSize;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eDodecahedron;
		}

		void ModelLoader::InitIcosahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			LoadInfoIcosahedron& loadInfo = m_loadGeometryInfo.emplace<LoadInfoIcosahedron>();
			loadInfo.fSize = fSize;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eIcosahedron;
		}

		void ModelLoader::InitTeapot(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, uint32_t nTessellation, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			LoadInfoTeapot& loadInfo = m_loadGeometryInfo.emplace<LoadInfoTeapot>();
			loadInfo.fSize = fSize;
			loadInfo.nTessellation = nTessellation;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eTeapot;
		}

		void ModelLoader::InitHexagon(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float radius)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();

			LoadInfoHexagon& loadInfo = m_loadGeometryInfo.emplace<LoadInfoHexagon>();
			loadInfo.fRadius = radius;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eHexagon;
		}

		void ModelLoader::InitCapsule(const String::StringID& strModelName, MaterialInfo* pMaterialInfo, float fRadius, float fHeight, int nSubdivisionHeight, int nSegments)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();

			LoadInfoCapsule& loadInfo = m_loadGeometryInfo.emplace<LoadInfoCapsule>();
			loadInfo.fRadius = fRadius;
			loadInfo.fHeight = fHeight;
			loadInfo.nSubdivisionHeight = nSubdivisionHeight;
			loadInfo.nSegments = nSegments;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eCapsule;
		}

		void ModelLoader::InitGrid(const String::StringID& strModelName, float fGridSizeX, float fGridSizeZ,
			uint32_t nBlockCountWidth, uint32_t nBlockCountLength, MaterialInfo* pMaterialInfo)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();

			LoadInfoGrid& loadInfo = m_loadGeometryInfo.emplace<LoadInfoGrid>();
			loadInfo.fGridSizeX = fGridSizeX;
			loadInfo.fGridSizeZ = fGridSizeZ;
			loadInfo.nBlockCountWidth = nBlockCountWidth;
			loadInfo.nBlockCountLength = nBlockCountLength;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eGrid;
		}

		void ModelLoader::InitPlane(const String::StringID& strModelName, float fEachLengthX, float fEachLengthZ,
			int nTotalCountX, int nTotalCountZ, MaterialInfo* pMaterialInfo)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();

			LoadInfoPlane& loadInfo = m_loadGeometryInfo.emplace<LoadInfoPlane>();
			loadInfo.fEachLengthX = fEachLengthX;
			loadInfo.fEachLengthZ = fEachLengthZ;
			loadInfo.nTotalCountX = nTotalCountX;
			loadInfo.nTotalCountZ = nTotalCountZ;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::ePlane;
		}

		void ModelLoader::InitCustomStaticModel(const String::StringID& strModelName, const String::StringID& strNodeName,
			IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, MaterialInfo* pMaterialInfo)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();

			LoadInfoCustomStaticModel& loadInfo = m_loadGeometryInfo.emplace<LoadInfoCustomStaticModel>();
			loadInfo.strNodeName = strNodeName;
			loadInfo.pVertexBuffer = pVertexBuffer;
			loadInfo.pIndexBuffer = pIndexBuffer;

			m_emLoadModelType = ModelLoader::eGeometry;
			m_emLoadGeometryType = ModelLoader::eCustomStaticModel;
		}

		void ModelLoader::AddAnimInfoByFBXFolder(const char* strAnimPath)
		{
			auto vecFiles = file::GetFilesInFolder(strAnimPath, "*.fbx", true);

			std::copy(vecFiles.begin(), vecFiles.end(), std::back_inserter(m_listAnimationList));
		}
	}
}