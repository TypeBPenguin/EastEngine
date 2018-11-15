#pragma once

#include <variant>

#include "ModelInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class IModel;
		class IVertexBuffer;
		class IIndexBuffer;

		struct MaterialInfo;

		struct LoadInfoCube
		{
			float fSize;
		};

		struct LoadInfoBox
		{
			math::float3 f3Size;
		};

		struct LoadInfoSphere
		{
			float fDiameter;
			uint32_t nTessellation;
		};

		struct LoadInfoGeoSphere
		{
			float fDiameter;
			uint32_t nTessellation;
		};

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
			string::StringID strNodeName;
			IVertexBuffer* pVertexBuffer = nullptr;
			IIndexBuffer* pIndexBuffer = nullptr;
		};

		class ModelLoader
		{
		public:
			enum LoadType
			{
				eGeometry = 0,
				eFbx,
				eObj,
				eXps,
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

			static GeometryType GetGeometryType(const char* strType);
			static const char* GetGeometryTypeToString(GeometryType emType);

		public:
			ModelLoader(std::function<void(bool)> funcCallback = nullptr);
			~ModelLoader();

		public:
			void InitFBX(const string::StringID& strModelName, const char* strFilePath, float fScaleFactor = 1.f, bool isFlipZ = true, uint32_t nLodMax = 0, const LODReductionRate& reductionRate = LODReductionRate());
			void InitObj(const string::StringID& strModelName, const char* strFilePath, float fScaleFactor = 1.f, uint32_t nLodMax = 0, const LODReductionRate& reductionRate = LODReductionRate());
			void InitXPS(const string::StringID& strModelName, const char* strFilePath);
			void InitEast(const string::StringID& strModelName, const char* strFilePath);

			void InitCube(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, bool rhcoords = false);
			void InitBox(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, const math::float3& size = math::float3(1.f, 1.f, 1.f), bool rhcoords = false, bool invertn = false);
			void InitSphere(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float diameter = 1.f, uint32_t nTessellation = 16, bool rhcoords = false, bool invertn = false);
			void InitGeoSphere(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float diameter = 1.f, uint32_t nTessellation = 3, bool rhcoords = false);
			void InitCylinder(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float height = 1.f, float diameter = 1.f, uint32_t nTessellation = 32, bool rhcoords = false);
			void InitCone(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float diameter = 1.f, float height = 1.f, uint32_t nTessellation = 32, bool rhcoords = false);
			void InitTorus(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float diameter = 1.f, float thickness = 0.333f, uint32_t nTessellation = 32, bool rhcoords = false);
			void InitTetrahedron(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, bool rhcoords = false);
			void InitOctahedron(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, bool rhcoords = false);
			void InitDodecahedron(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, bool rhcoords = false);
			void InitIcosahedron(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, bool rhcoords = false);
			void InitTeapot(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fSize = 1.f, uint32_t nTessellation = 8, bool rhcoords = false);
			void InitHexagon(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float radius = 1.f);
			void InitCapsule(const string::StringID& strModelName, MaterialInfo* pMaterialInfo = nullptr, float fRadius = 0.5f, float fHeight = 1.f, int nSubdivisionHeight = 12, int nSegments = 12);
			void InitGrid(const string::StringID& strModelName, float fGridSizeX, float fGridSizeZ, uint32_t nBlockCountWidth, uint32_t nBlockCountLength, MaterialInfo* pMaterialInfo = nullptr);
			void InitPlane(const string::StringID& strModelName, float fEachLengthX, float fEachLengthZ, int nTotalCountX, int nTotalCountZ, MaterialInfo* pMaterialInfo = nullptr);
			void InitCustomStaticModel(const string::StringID& strModelName, const string::StringID& strNodeName, IVertexBuffer* pVertexBuffer, IIndexBuffer* pIndexBuffer, MaterialInfo* pMaterialInfo = nullptr);

			void AddAnimInfoByFBXFolder(const char* strAnimPath);
			void AddAnimInfoByFBX(const std::string& strAnimPath) { m_vecAnimationList.emplace_back(strAnimPath); }
			const std::vector<std::string>& GetAnimList() const { return m_vecAnimationList; }

			void AddDevideByKeywordByXPS(const char* strKeyword) { m_vecDevideByKeywords.emplace_back(strKeyword); }
			const std::string* GetDevideKeywords() const { return m_vecDevideByKeywords.data(); }
			size_t GetDevideKeywordCount() const { return m_vecDevideByKeywords.size(); }

			std::function<void(bool)> GetCallbackFunc() const { return m_funcCallback; }

		public:
			bool IsEnableThreadLoad() const { return m_isEnableThreadLoad; }
			void SetEnableThreadLoad(bool isEnableThreadLoad) { m_isEnableThreadLoad = isEnableThreadLoad; }

			const math::float3& GetLocalPosition() const { return m_f3Pos; }
			void SetLocalPosition(const math::float3& f3Pos) { m_f3Pos = f3Pos; }
			const math::float3& GetLocalScale() const { return m_f3Scale; }
			void SetLocalScale(const math::float3& f3Scale) { m_f3Scale = f3Scale; }
			const math::Quaternion& GetLocalRotation() const { return m_quatRotation; }
			void SetLocalRotation(const math::Quaternion& quatRotation) { m_quatRotation = quatRotation; }

			LoadType GetLoadModelType() const { return m_emLoadModelType; }
			GeometryType GetLoadGeometryType() const { return m_emLoadGeometryType; }

			const std::string& GetFilePath() const { return m_strFilePath; }
			const string::StringID& GetModelName() const { return m_strModelName; }

			const MaterialInfo& GetMaterial() const { return m_materialInfo; }

			float GetScaleFactor() const { return m_fScaleFactor; }
			bool IsFlipZ() const { return m_isFlipZ; }

			uint32_t GetLodMax() const { return m_nLodMax; }
			const LODReductionRate& GetLODReductionRate() const { return m_lodReductionRate; }

			bool IsRightHandCoords() const { return m_isRhcoords; }
			bool IsInvertNormal() const { return m_isInvertn; }

			const LoadInfoCube* GetCubeInfo() const { return std::get_if<LoadInfoCube>(&m_loadGeometryInfo); }
			const LoadInfoBox* GetBoxInfo() const { return std::get_if<LoadInfoBox>(&m_loadGeometryInfo); }
			const LoadInfoSphere* GetSphereInfo() const { return std::get_if<LoadInfoSphere>(&m_loadGeometryInfo);; }
			const LoadInfoGeoSphere* GetGeoSphereInfo() const { return std::get_if<LoadInfoGeoSphere>(&m_loadGeometryInfo);; }
			const LoadInfoCylinder* GetCylinderInfo() const { return std::get_if<LoadInfoCylinder>(&m_loadGeometryInfo);; }
			const LoadInfoCone* GetConeInfo() const { return std::get_if<LoadInfoCone>(&m_loadGeometryInfo);; }
			const LoadInfoTorus* GetTorusInfo() const { return std::get_if<LoadInfoTorus>(&m_loadGeometryInfo);; }
			const LoadInfoTetrahedron* GetTetrahedronInfo() const { return std::get_if<LoadInfoTetrahedron>(&m_loadGeometryInfo);; }
			const LoadInfoOctahedron* GetOctahedronInfo() const { return std::get_if<LoadInfoOctahedron>(&m_loadGeometryInfo);; }
			const LoadInfoDodecahedron* GetDodecahedronInfo() const { return std::get_if<LoadInfoDodecahedron>(&m_loadGeometryInfo);; }
			const LoadInfoIcosahedron* GetIcosahedronInfo() const { return std::get_if<LoadInfoIcosahedron>(&m_loadGeometryInfo);; }
			const LoadInfoTeapot* GetTeapotInfo() const { return std::get_if<LoadInfoTeapot>(&m_loadGeometryInfo);; }
			const LoadInfoHexagon* GetHexagonInfo() const { return std::get_if<LoadInfoHexagon>(&m_loadGeometryInfo);; }
			const LoadInfoCapsule* GetCapsuleInfo() const { return std::get_if<LoadInfoCapsule>(&m_loadGeometryInfo);; }
			const LoadInfoGrid* GetGridInfo() const { return std::get_if<LoadInfoGrid>(&m_loadGeometryInfo);; }
			const LoadInfoPlane* GetPlaneInfo() const { return std::get_if<LoadInfoPlane>(&m_loadGeometryInfo);; }
			const LoadInfoCustomStaticModel* GetCustomStaticModelInfo() const { return std::get_if<LoadInfoCustomStaticModel>(&m_loadGeometryInfo);; }

		private:
			bool m_isEnableThreadLoad;

			math::float3 m_f3Pos;
			math::float3 m_f3Scale;
			math::Quaternion m_quatRotation;

			std::function<void(bool)> m_funcCallback;
			LoadType m_emLoadModelType;
			GeometryType m_emLoadGeometryType;

			std::vector<std::string> m_vecAnimationList;
			std::vector<std::string> m_vecDevideByKeywords;

			std::string m_strFilePath;
			string::StringID m_strModelName;

			MaterialInfo m_materialInfo;

			float m_fScaleFactor;
			bool m_isFlipZ;

			uint32_t m_nLodMax;
			LODReductionRate m_lodReductionRate;

			bool m_isRhcoords;
			bool m_isInvertn;

			std::variant<LoadInfoCube,
				LoadInfoBox,
				LoadInfoSphere,
				LoadInfoGeoSphere,
				LoadInfoCylinder,
				LoadInfoCone,
				LoadInfoTorus,
				LoadInfoTetrahedron,
				LoadInfoOctahedron,
				LoadInfoDodecahedron,
				LoadInfoIcosahedron,
				LoadInfoTeapot,
				LoadInfoHexagon,
				LoadInfoCapsule,
				LoadInfoGrid,
				LoadInfoPlane,
				LoadInfoCustomStaticModel> m_loadGeometryInfo;
		};
	}
}