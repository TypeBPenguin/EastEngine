#include "stdafx.h"
#include "ModelLoader.h"

#include "CommonLib/FileUtil.h"

#include "ModelInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		namespace EmModelLoader
		{
			GeometryType GetGeometryType(const char* strType)
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

			const char* GetGeometryTypeToSTring(GeometryType emType)
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
		}

		ModelLoader::ModelLoader(std::function<void(bool)> funcCallback)
			: m_isEnableThreadLoad(false)
			, m_f3Scale(Math::Vector3::One)
			, m_funcCallback(funcCallback)
			, m_fScaleFactor(0.f)
			, m_emLoadModelType(EmModelLoader::eGeometry)
			, m_emLoadGeometryType(EmModelLoader::eCustomStaticModel)
			, m_isRhcoords(false)
			, m_isInvertn(false)
			, m_nLodMax(0)
		{
		}

		ModelLoader::~ModelLoader()
		{
		}

		void ModelLoader::InitFBX(const String::StringID& strModelName, const char* strFilePath, float fScaleFactor, uint32_t nLodMax, const LODReductionRate& reductionRate)
		{
			m_strModelName = strModelName;
			m_strFilePath = strFilePath;
			m_fScaleFactor = 1.f / fScaleFactor;

			m_nLodMax = nLodMax;
			m_lodReductionRate = reductionRate;

			m_emLoadModelType = EmModelLoader::eFbx;
		}

		void ModelLoader::InitObj(const String::StringID& strModelName, const char* strFilePath, float fScaleFactor, uint32_t nLodMax, const LODReductionRate& reductionRate)
		{
			m_strModelName = strModelName;
			m_strFilePath = strFilePath;
			m_fScaleFactor = fScaleFactor;

			m_nLodMax = nLodMax;
			m_lodReductionRate = reductionRate;

			m_emLoadModelType = EmModelLoader::eObj;
		}

		void ModelLoader::InitEast(const String::StringID& strModelName, const char* strFilePath)
		{
			m_strModelName = strModelName;
			m_strFilePath = strFilePath;
			m_fScaleFactor = 1.f;

			m_emLoadModelType = EmModelLoader::eEast;
		}

		void ModelLoader::InitCube(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			m_info.cube.fSize = fSize;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eCube;
		}

		void ModelLoader::InitBox(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			const Math::Vector3& size, bool rhcoords, bool invertn)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;
			m_isInvertn = invertn;

			m_info.box.f3Size = size;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eBox;
		}

		void ModelLoader::InitSphere(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float diameter, uint32_t nTessellation, bool rhcoords, bool invertn)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;
			m_isInvertn = invertn;

			m_info.sphere.fDiameter = diameter;
			m_info.sphere.nTessellation = nTessellation;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eSphere;
		}

		void ModelLoader::InitGeoSphere(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float diameter, uint32_t nTessellation, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			m_info.geoSphere.fDiameter = diameter;
			m_info.geoSphere.nTessellation = nTessellation;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eGeoSphere;
		}

		void ModelLoader::InitCylinder(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float height, float diameter, uint32_t nTessellation, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			m_info.cylinder.fHeight = height;
			m_info.cylinder.fDiameter = diameter;
			m_info.cylinder.nTessellation = nTessellation;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eCylinder;
		}

		void ModelLoader::InitCone(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float diameter, float height, uint32_t nTessellation, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			m_info.cone.fDiameter = diameter;
			m_info.cone.fHeight = height;
			m_info.cone.nTessellation = nTessellation;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eCone;
		}

		void ModelLoader::InitTorus(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float diameter, float thickness, uint32_t nTessellation, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			m_info.torus.fDiameter = diameter;
			m_info.torus.fThickness = thickness;
			m_info.torus.nTessellation = nTessellation;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eTorus;
		}

		void ModelLoader::InitTetrahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			m_info.tetrahedron.fSize = fSize;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eTetrahedron;
		}

		void ModelLoader::InitOctahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			m_info.octahedron.fSize = fSize;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eOctahedron;
		}

		void ModelLoader::InitDodecahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			m_info.dodecahedron.fSize = fSize;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eDodecahedron;
		}

		void ModelLoader::InitIcosahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			m_info.icosahedron.fSize = fSize;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eIcosahedron;
		}

		void ModelLoader::InitTeapot(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float fSize, uint32_t nTessellation, bool rhcoords)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();
			m_isRhcoords = rhcoords;

			m_info.teapot.fSize = fSize;
			m_info.teapot.nTessellation = nTessellation;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eTeapot;
		}

		void ModelLoader::InitHexagon(const String::StringID& strModelName, MaterialInfo* pMaterialInfo,
			float radius)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();

			m_info.hexagon.fRadius = radius;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eHexagon;
		}

		void ModelLoader::InitCapsule(const String::StringID& strModelName, MaterialInfo* pMaterialInfo, float fRadius, float fHeight, int nSubdivisionHeight, int nSegments)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();

			m_info.capsule.fRadius = fRadius;
			m_info.capsule.fHeight = fHeight;
			m_info.capsule.nSubdivisionHeight = nSubdivisionHeight;
			m_info.capsule.nSegments = nSegments;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eCapsule;
		}

		void ModelLoader::InitGrid(const String::StringID& strModelName, float fGridSizeX, float fGridSizeZ,
			uint32_t nBlockCountWidth, uint32_t nBlockCountLength, MaterialInfo* pMaterialInfo)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();

			m_info.grid.fGridSizeX = fGridSizeX;
			m_info.grid.fGridSizeZ = fGridSizeZ;
			m_info.grid.nBlockCountWidth = nBlockCountWidth;
			m_info.grid.nBlockCountLength = nBlockCountLength;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eGrid;
		}

		void ModelLoader::InitPlane(const String::StringID& strModelName, float fEachLengthX, float fEachLengthZ,
			int nTotalCountX, int nTotalCountZ, MaterialInfo* pMaterialInfo)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();

			m_info.plane.fEachLengthX = fEachLengthX;
			m_info.plane.fEachLengthZ = fEachLengthZ;
			m_info.plane.nTotalCountX = nTotalCountX;
			m_info.plane.nTotalCountZ = nTotalCountZ;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::ePlane;
		}

		void ModelLoader::InitCustomStaticModel(const String::StringID& strModelName, const String::StringID& strNodeName,
			IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, MaterialInfo* pMaterialInfo)
		{
			m_strModelName = strModelName;
			m_materialInfo = pMaterialInfo != nullptr ? *pMaterialInfo : MaterialInfo();

			m_info.customStaticModel.strNodeName = strNodeName;
			m_info.customStaticModel.pVertexBuffer = pVertexBuffer;
			m_info.customStaticModel.pIndexBuffer = pIndexBuffer;

			m_emLoadModelType = EmModelLoader::eGeometry;
			m_emLoadGeometryType = EmModelLoader::eCustomStaticModel;
		}

		void ModelLoader::AddAnimInfoByFBXFolder(const char* strAnimPath)
		{
			auto vecFiles = File::GetFilesInFolder(strAnimPath, "*.fbx", true);

			std::copy(vecFiles.begin(), vecFiles.end(), std::back_inserter(m_listAnimationList));
		}
	}
}