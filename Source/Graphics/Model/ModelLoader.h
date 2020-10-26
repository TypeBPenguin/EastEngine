#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

namespace est
{
	namespace graphics
	{
		struct LoadInfoCube
		{
			float size;
		};

		struct LoadInfoBox
		{
			math::float3 halfExtents;
		};

		struct LoadInfoSphere
		{
			float diameter;
			uint32_t tessellation;
		};

		struct LoadInfoGeoSphere
		{
			float diameter;
			uint32_t tessellation;
		};

		struct LoadInfoCylinder
		{
			float height;
			float diameter;
			uint32_t tessellation;
		};

		struct LoadInfoCone
		{
			float diameter;
			float height;
			uint32_t tessellation;
		};

		struct LoadInfoTorus
		{
			float diameter;
			float thickness;
			uint32_t tessellation;
		};

		struct LoadInfoTetrahedron
		{
			float size;
		};

		struct LoadInfoOctahedron
		{
			float size;
		};

		struct LoadInfoDodecahedron
		{
			float size;
		};

		struct LoadInfoIcosahedron
		{
			float size;
		};

		struct LoadInfoTeapot
		{
			float size;
			uint32_t tessellation;
		};

		struct LoadInfoHexagon
		{
			float radius;
		};

		struct LoadInfoCapsule
		{
			float radius;
			float height;
			int subdivisionHeight;
			int segments;
		};

		struct LoadInfoGrid
		{
			float gridSizeX;
			float gridSizeZ;
			uint32_t blockCountWidth;
			uint32_t blockCountLength;
		};

		struct LoadInfoPlane
		{
			float eachLengthX;
			float eachLengthZ;
			int totalCountX;
			int totalCountZ;
		};

		struct LoadInfoCustomStaticModel
		{
			string::StringID nodeName;
			VertexBufferPtr pVertexBuffer;
			IndexBufferPtr pIndexBuffer;
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

			static GeometryType GetGeometryType(const wchar_t* strType);
			static const wchar_t* GetGeometryTypeToString(GeometryType emType);

		public:
			ModelLoader(std::function<void(bool)> funcCallback = nullptr);
			~ModelLoader();

		public:
			void InitFBX(const string::StringID& modelName, const wchar_t* filePath, float scaleFactor = 1.f, bool isFlipZ = true);
			void InitObj(const string::StringID& modelName, const wchar_t* filePath, float scaleFactor = 1.f);
			void InitXPS(const string::StringID& modelName, const wchar_t* filePath);
			void InitEast(const string::StringID& modelName, const wchar_t* filePath);

			void InitCube(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float size = 1.f, bool rhcoords = false);
			void InitBox(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, const math::float3& halfExtents = math::float3(1.f, 1.f, 1.f), bool rhcoords = false, bool invertn = false);
			void InitSphere(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float diameter = 1.f, uint32_t tessellation = 16, bool rhcoords = false, bool invertn = false);
			void InitGeoSphere(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float diameter = 1.f, uint32_t tessellation = 3, bool rhcoords = false);
			void InitCylinder(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float height = 1.f, float diameter = 1.f, uint32_t tessellation = 32, bool rhcoords = false);
			void InitCone(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float diameter = 1.f, float height = 1.f, uint32_t tessellation = 32, bool rhcoords = false);
			void InitTorus(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float diameter = 1.f, float thickness = 0.333f, uint32_t tessellation = 32, bool rhcoords = false);
			void InitTetrahedron(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float size = 1.f, bool rhcoords = false);
			void InitOctahedron(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float size = 1.f, bool rhcoords = false);
			void InitDodecahedron(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float size = 1.f, bool rhcoords = false);
			void InitIcosahedron(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float size = 1.f, bool rhcoords = false);
			void InitTeapot(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float size = 1.f, uint32_t tessellation = 8, bool rhcoords = false);
			void InitHexagon(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float radius = 1.f);
			void InitCapsule(const string::StringID& modelName, const IMaterial::Data* pMaterialData = nullptr, float radius = 0.5f, float height = 1.f, int subdivisionHeight = 12, int segments = 12);
			void InitGrid(const string::StringID& modelName, float gridSizeX, float gridSizeZ, uint32_t blockCountWidth, uint32_t blockCountLength, const IMaterial::Data* pMaterialData = nullptr);
			void InitPlane(const string::StringID& modelName, float eachLengthX, float eachLengthZ, int totalCountX, int totalCountZ, const IMaterial::Data* pMaterialData = nullptr);
			void InitCustomStaticModel(const string::StringID& modelName, const string::StringID& nodeName, const VertexBufferPtr& pVertexBuffer, const IndexBufferPtr& pIndexBuffer, const IMaterial::Data* pMaterialData = nullptr);

			void AddAnimInfoByFBXFolder(const wchar_t* animPath);
			void AddAnimInfoByFBX(const std::wstring& animPath) { m_vecAnimationList.emplace_back(animPath); }
			const std::vector<std::wstring>& GetAnimList() const { return m_vecAnimationList; }

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

			const std::wstring& GetFilePath() const { return m_filePath; }
			const string::StringID& GetModelName() const { return m_modelName; }

			const IMaterial::Data& GetMaterial() const { return m_materialData; }

			float GetScaleFactor() const { return m_scaleFactor; }
			bool IsFlipZ() const { return m_isFlipZ; }

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
			bool m_isEnableThreadLoad{ false };

			math::float3 m_f3Pos;
			math::float3 m_f3Scale{ math::float3::One };
			math::Quaternion m_quatRotation;

			std::function<void(bool)> m_funcCallback;
			LoadType m_emLoadModelType{ ModelLoader::eGeometry };
			GeometryType m_emLoadGeometryType{ ModelLoader::eCustomStaticModel };

			std::vector<std::wstring> m_vecAnimationList;
			std::vector<std::string> m_vecDevideByKeywords;

			std::wstring m_filePath;
			string::StringID m_modelName;

			IMaterial::Data m_materialData;

			float m_scaleFactor{ 1.f };
			bool m_isFlipZ{ true };

			bool m_isRhcoords{ false };
			bool m_isInvertn{ false };

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