#pragma once

#include "ModelInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IModel;
		class IVertexBuffer;
		class IIndexBuffer;

		struct MaterialInfo;

		namespace EmModelLoader
		{
			enum LoadType
			{
				eGeometry = 0,
				eFbx,
				eObj,
				eEast,
			};

			enum GeometryType
			{
				eCustomStaticModel = 0,
				eCube,
				eBox,
				eSphere,
				eGeoSphere,
				eCylinder,
				eCone,
				eTorus,
				eTetrahedron,
				eOctahedron,
				eDodecahedron,
				eIcosahedron,
				eTeapot,
				eHexagon,
				eCapsule,
				eGrid,
				ePlane,

				eInvalid,
				GeometryCount = eInvalid,
			};

			GeometryType GetGeometryType(const char* strType);
			const char* GetGeometryTypeToSTring(GeometryType emType);
		}

		struct LoadInfoCube
		{
			float fSize;
		};

		struct LoadInfoBox
		{
			Math::Vector3 f3Size;
		};

		struct LoadInfoSphere
		{
			float fDiameter;
			uint32_t nTessellation;
		};
		typedef LoadInfoSphere LoadInfoGeoSphere;

		struct LoadInfoCylinder
		{
			float fHeight;
			float fDiameter;
			uint32_t nTessellation;
		};

		struct LoadInfoCone
		{
			float fDiameter;
			float fHeight;
			uint32_t nTessellation;
		};

		struct LoadInfoTorus
		{
			float fDiameter;
			float fThickness;
			uint32_t nTessellation;
		};

		struct LoadInfoTetrahedron
		{
			float fSize;
		};

		struct LoadInfoOctahedron
		{
			float fSize;
		};

		struct LoadInfoDodecahedron
		{
			float fSize;
		};

		struct LoadInfoIcosahedron
		{
			float fSize;
		};

		struct LoadInfoTeapot
		{
			float fSize;
			uint32_t nTessellation;
		};

		struct LoadInfoHexagon
		{
			float fRadius;
		};

		struct LoadInfoCapsule
		{
			float fRadius;
			float fHeight;
			int nSubdivisionHeight;
			int nSegments;
		};

		struct LoadInfoGrid
		{
			float fGridSizeX;
			float fGridSizeZ;
			uint32_t nBlockCountWidth;
			uint32_t nBlockCountLength;
		};

		struct LoadInfoPlane
		{
			float fEachLengthX;
			float fEachLengthZ;
			int nTotalCountX;
			int nTotalCountZ;
		};

		struct LoadInfoCustomStaticModel
		{
			String::StringID strNodeName;
			IVertexBuffer* pVertexBuffer = nullptr;
			IIndexBuffer* pIndexBuffer = nullptr;
		};

		class ModelLoader
		{
		public:
			ModelLoader(std::function<void(bool)> funcCallback = nullptr);
			~ModelLoader();

		public:
			void InitFBX(const String::StringID& strModelName, const char* strFilePath, float fScaleFactor = 1.f, uint32_t nLodMax = 0, const LODReductionRate& reductionRate = LODReductionRate());
			void InitObj(const String::StringID& strModelName, const char* strFilePath, float fScaleFactor = 1.f, uint32_t nLodMax = 0, const LODReductionRate& reductionRate = LODReductionRate());
			void InitEast(const String::StringID& strModelName, const char* strFilePath);

			void InitCube(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, bool rhcoords = false);
			void InitBox(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, const Math::Vector3& size = Math::Vector3(1.f, 1.f, 1.f), bool rhcoords = false, bool invertn = false);
			void InitSphere(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float diameter = 1.f, uint32_t nTessellation = 16, bool rhcoords = false, bool invertn = false);
			void InitGeoSphere(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float diameter = 1.f, uint32_t nTessellation = 3, bool rhcoords = false);
			void InitCylinder(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float height = 1.f, float diameter = 1.f, uint32_t nTessellation = 32, bool rhcoords = false);
			void InitCone(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float diameter = 1.f, float height = 1.f, uint32_t nTessellation = 32, bool rhcoords = false);
			void InitTorus(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float diameter = 1.f, float thickness = 0.333f, uint32_t nTessellation = 32, bool rhcoords = false);
			void InitTetrahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, bool rhcoords = false);
			void InitOctahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, bool rhcoords = false);
			void InitDodecahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, bool rhcoords = false);
			void InitIcosahedron(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, bool rhcoords = false);
			void InitTeapot(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, uint32_t nTessellation = 8, bool rhcoords = false);
			void InitHexagon(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float radius = 1.f);
			void InitCapsule(const String::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fRadius = 0.5f, float fHeight = 1.f, int nSubdivisionHeight = 12, int nSegments = 12);
			void InitGrid(const String::StringID& strModelName, float fGridSizeX, float fGridSizeZ, uint32_t nBlockCountWidth, uint32_t nBlockCountLength, MaterialInfo* pMaterialInfo = nullptr);
			void InitPlane(const String::StringID& strModelName, float fEachLengthX, float fEachLengthZ, int nTotalCountX, int nTotalCountZ, MaterialInfo* pMaterialInfo = nullptr);
			void InitCustomStaticModel(const String::StringID& strModelName, const String::StringID& strNodeName, IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, MaterialInfo* pMaterialInfo = nullptr);

			void AddAnimInfoByFBXFolder(const char* strAnimPath);
			void AddAnimInfoByFBX(std::string strAnimPath) { m_listAnimationList.emplace_back(strAnimPath); }
			const std::list<std::string>& GetAnimInfo() const { return m_listAnimationList; }

			std::function<void(bool)> GetCallbackFunc() const { return m_funcCallback; }

		public:
			bool IsEnableThreadLoad() const { return m_isEnableThreadLoad; }
			void SetEnableThreadLoad(bool isEnableThreadLoad) { m_isEnableThreadLoad = isEnableThreadLoad; }

			const Math::Vector3& GetLocalPosition() const { return m_f3Pos; }
			void SetLocalPosition(const Math::Vector3& f3Pos) { m_f3Pos = f3Pos; }
			const Math::Vector3& GetLocalScale() const { return m_f3Scale; }
			void SetLocalScale(const Math::Vector3& f3Scale) { m_f3Scale = f3Scale; }
			const Math::Quaternion& GetLocalRotation() const { return m_quatRotation; }
			void SetLocalRotation(const Math::Quaternion& quatRotation) { m_quatRotation = quatRotation; }

			EmModelLoader::LoadType GetLoadModelType() const { return m_emLoadModelType; }
			EmModelLoader::GeometryType GetLoadGeometryType() const { return m_emLoadGeometryType; }

			const std::string& GetFilePath() const { return m_strFilePath; }
			const String::StringID& GetModelName() const { return m_strModelName; }

			const MaterialInfo& GetMaterial() const { return m_materialInfo; }

			float GetScaleFactor() const { return m_fScaleFactor; }

			uint32_t GetLodMax() const { return m_nLodMax; }
			const LODReductionRate& GetLODReductionRate() const { return m_lodReductionRate; }

			bool IsRightHandCoords() const { return m_isRhcoords; }
			bool IsInvertNormal() const { return m_isInvertn; }

			const LoadInfoCube& GetCubeInfo() const { return m_info.cube; }
			const LoadInfoBox& GetBoxInfo() const { return m_info.box; }
			const LoadInfoSphere& GetSphereInfo() const { return m_info.sphere; }
			const LoadInfoGeoSphere& GetGeoSphereInfo() const { return m_info.geoSphere; }
			const LoadInfoCylinder& GetCylinderInfo() const { return m_info.cylinder; }
			const LoadInfoCone& GetConeInfo() const { return m_info.cone; }
			const LoadInfoTorus& GetTorusInfo() const { return m_info.torus; }
			const LoadInfoTetrahedron& GetTetrahedronInfo() const { return m_info.tetrahedron; }
			const LoadInfoOctahedron& GetOctahedronInfo() const { return m_info.octahedron; }
			const LoadInfoDodecahedron& GetDodecahedronInfo() const { return m_info.dodecahedron; }
			const LoadInfoIcosahedron& GetIcosahedronInfo() const { return m_info.icosahedron; }
			const LoadInfoTeapot& GetTeapotInfo() const { return m_info.teapot; }
			const LoadInfoHexagon& GetHexagonInfo() const { return m_info.hexagon; }
			const LoadInfoCapsule& GetCapsuleInfo() const { return m_info.capsule; }
			const LoadInfoGrid& GetGridInfo() const { return m_info.grid; }
			const LoadInfoPlane& GetPlaneInfo() const { return m_info.plane; }
			const LoadInfoCustomStaticModel& GetCustomStaticModelInfo() const { return m_info.customStaticModel; }

		private:
			bool m_isEnableThreadLoad;

			Math::Vector3 m_f3Pos;
			Math::Vector3 m_f3Scale;
			Math::Quaternion m_quatRotation;

			std::function<void(bool)> m_funcCallback;
			EmModelLoader::LoadType m_emLoadModelType;
			EmModelLoader::GeometryType m_emLoadGeometryType;

			std::list<std::string> m_listAnimationList;

			std::string m_strFilePath;
			String::StringID m_strModelName;

			MaterialInfo m_materialInfo;

			float m_fScaleFactor;

			uint32_t m_nLodMax;
			LODReductionRate m_lodReductionRate;

			bool m_isRhcoords;
			bool m_isInvertn;

			union LoadInfo
			{
				LoadInfoCube cube;
				LoadInfoBox box;
				LoadInfoSphere sphere;
				LoadInfoGeoSphere geoSphere;
				LoadInfoCylinder cylinder;
				LoadInfoCone cone;
				LoadInfoTorus torus;
				LoadInfoTetrahedron tetrahedron;
				LoadInfoOctahedron octahedron;
				LoadInfoDodecahedron dodecahedron;
				LoadInfoIcosahedron icosahedron;
				LoadInfoTeapot teapot;
				LoadInfoHexagon hexagon;
				LoadInfoCapsule capsule;
				LoadInfoGrid grid;
				LoadInfoPlane plane;
				LoadInfoCustomStaticModel customStaticModel;

				LoadInfo() {}
				LoadInfo(const LoadInfo& source)
				{
					*this = source;
				}
				~LoadInfo() {}

				LoadInfo* operator=(const LoadInfo& source)
				{
					cube = source.cube;
					box = source.box;
					sphere = source.sphere;
					geoSphere = source.geoSphere;
					cylinder = source.cylinder;
					cone = source.cone;
					torus = source.torus;
					tetrahedron = source.tetrahedron;
					octahedron = source.octahedron;
					dodecahedron = source.dodecahedron;
					icosahedron = source.icosahedron;
					teapot = source.teapot;
					hexagon = source.hexagon;
					grid = source.grid;
					plane = source.plane;
					customStaticModel = source.customStaticModel;

					return this;
				}
			} m_info;
		};
	}
}