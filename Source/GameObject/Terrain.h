#pragma once

#include "GameObject.h"
#include "TerrainCells.h"

#define NEW_TERRAIN

namespace EastEngine
{
#ifndef NEW_TERRAIN
	namespace GameObject
	{
		class Terrain
		{
		public:
			Terrain();
			~Terrain();

			bool Init(const char* strSetupFile);

			void Update(float fElapsedTime, const Math::Vector3* pPlayerPosition = nullptr);

		public:
			bool GetShowCellLine() { return m_isShowCellLine; }
			void SetShowCellLine(bool isShowCellLine) { m_isShowCellLine = isShowCellLine; }

			float GetHeightAtPosition(float fPosX, float fPosZ);

			bool IsLoadComplete() { return m_isLoadComplete; }

		private:
			bool initByThread(std::string strSetupFile);
			bool loadSetup(const char* strSetupFile, Physics::RigidBodyProperty& rigidBodyProperty);
			bool loadHeightMap(const char* strHeightMapFile, const char* strPath);
			bool loadColorMap(const char* strColorMapFile, const char* strPath);
			bool loadRawHeightmap(const char* strRawHeightMapFile, const char* strPath);
			bool initTerrain();
			bool initTerrainCell(std::vector<Graphics::VertexPosTexNorCol>& vecModel);

			bool checkHeightOfTriangle(float x, float z, float& height, Math::Vector3& v0, Math::Vector3& v1, Math::Vector3& v2);

		private:
			struct HeightMapVertex
			{
				Math::Vector3 pos;
				Math::Vector3 normal;
				Math::Color color = Math::Color::White;
			};

			std::vector<float> m_vecHeight;
			std::vector<HeightMapVertex> m_vecHeightMap;

			uint32_t m_nTerrainWidth;
			uint32_t m_nTerrainHeight;

			Math::Vector2 m_f2MinMaxHeight;

			Math::Vector3 m_f3Scaling;
			Math::Vector3 m_f3DefaultPos;

			std::vector<Graphics::IMaterial*> m_vecMaterial;

			uint32_t m_nCellWidth;
			uint32_t m_nCellHeight;

			uint32_t m_nCellCount;

			std::vector<TerrainCells> m_veTerrainCells;

			Physics::RigidBody* m_pPhysics;

			Graphics::VertexCollector<Graphics::VertexPos> m_vecVertices;
			Graphics::IndexCollector<uint32_t>	m_vecIndices;

			uint32_t m_nLastCell;

			bool m_isShowCellLine;
			bool m_isLoadComplete;
		};
	}
#else

	struct TerrainProperty
	{
		int nGridPoints = 512;
		int nPatches = 64;

		float fGeometryScale = 1.f;
		float fMaxHeight = 30.f;
		float fMinHeight = -30.f;
		float fFractalFactor = 0.68f;
		float fFractalInitialValue = 100.f;
		float fSmoothFactor1 = 0.99f;
		float fSmoothFactor2 = 0.1f;
		float fRockfactor = 0.95f;
		int nSmoothSteps = 40;

		float fHeightUnderWaterStart = -100.f;
		float fHeightUnderWaterEnd = -8.f;

		float fHeightSandStart = -30.f;
		float fHeightSandEnd = 1.7f;

		float fHeightGrassStart = 1.7f;
		float fHeightGrassEnd = 30.f;

		float fHeightRocksStart = -2.f;

		float fHeightTreesStart = 4.f;
		float fHeightTressEnd = 30.f;

		float fSlopeGrassStart = 0.96f;
		float fSlopeRocksStart = 0.85f;

		int nLayerDefMapSize = 1024;
		int nDepthShadowMapSize = 512;

		int nSkyGridPoints = 10;
		float fSkyTextureAngle = 0.425f;

		float fHalfSpaceCullHeight = -0.6f;
		bool isHalfSpaceCullSign = true;

		std::string strTexRockBumpFile;
		std::string strTexRockMicroFile;
		std::string strTexRockDiffuseFile;

		std::string strTexSandBumpFile;
		std::string strTexSandMicroFile;
		std::string strTexSandDiffuseFile;

		std::string strTexGrassDiffuse;
		std::string strTexSlopeDiffuse;
		std::string strTexWaterBump;
		std::string strTexSky;
	};

	namespace Graphics
	{
		class ITexture;
		class IVertexBuffer;
	}

	namespace GameObject
	{
		class Terrain : public IGameObject
		{
		public:
			Terrain();
			virtual ~Terrain();

		public:
			virtual EmObjectType GetType() const override { return EmObjectType::eTerrain; }

		public:
			void Init(TerrainProperty* pTerrainProperty);

			void Update(float fElapsedTime);

		private:
			TerrainProperty m_property;

			std::vector<std::vector<float>> m_vecHeights;
			std::vector<std::vector<Math::Vector3>> m_vecNormals;

			std::shared_ptr<Graphics::ITexture> m_pTexRockBump;
			std::shared_ptr<Graphics::ITexture> m_pTexRockMicroBump;
			std::shared_ptr<Graphics::ITexture> m_pTexRockDiffuse;

			std::shared_ptr<Graphics::ITexture> m_pTexSandBump;
			std::shared_ptr<Graphics::ITexture> m_pTexSandMicroBump;
			std::shared_ptr<Graphics::ITexture> m_pTexSandDIffuse;

			std::shared_ptr<Graphics::ITexture> m_pTexGrassDiffuse;
			std::shared_ptr<Graphics::ITexture> m_pTexSlopeDiffuse;
			std::shared_ptr<Graphics::ITexture> m_pTexWaterBump;
			std::shared_ptr<Graphics::ITexture> m_pTexSky;

			std::shared_ptr<Graphics::ITexture> m_pTexLayerdef;
			std::shared_ptr<Graphics::ITexture> m_pTexHeightMap;
			std::shared_ptr<Graphics::ITexture> m_pTexDepthMap;

			Graphics::IVertexBuffer* m_pHeightField;
			Graphics::IVertexBuffer* m_pSky;
		};
#endif
	}
}