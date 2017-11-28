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

#define terrain_gridpoints					512
#define terrain_numpatches_1d				64
#define terrain_geometry_scale				1.0f
#define terrain_maxheight					30.0f 
#define terrain_minheight					-30.0f 
#define terrain_fractalfactor				0.68f;
#define terrain_fractalinitialvalue			100.0f
#define terrain_smoothfactor1				0.99f
#define terrain_smoothfactor2				0.10f
#define terrain_rockfactor					0.95f
#define terrain_smoothsteps					40

#define terrain_height_underwater_start		-100.0f
#define terrain_height_underwater_end		-8.0f
#define terrain_height_sand_start			-30.0f
#define terrain_height_sand_end				1.7f
#define terrain_height_grass_start			1.7f
#define terrain_height_grass_end			30.0f
#define terrain_height_rocks_start			-2.0f
#define terrain_height_trees_start			4.0f
#define terrain_height_trees_end			30.0f
#define terrain_slope_grass_start			0.96f
#define terrain_slope_rocks_start			0.85f

#define terrain_far_range terrain_gridpoints*terrain_geometry_scale

#define shadowmap_resource_buffer_size_xy				4096
#define water_normalmap_resource_buffer_size_xy			2048
#define terrain_layerdef_map_texture_size				1024
#define terrain_depth_shadow_map_texture_size			512

#define sky_gridpoints						10
#define sky_texture_angle					0.425f

#define main_buffer_size_multiplier			1.1f
#define reflection_buffer_size_multiplier   1.1f
#define refraction_buffer_size_multiplier   1.1f

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
			void Init();

			void Update(float fElapsedTime);

		private:
			float m_height[terrain_gridpoints + 1][terrain_gridpoints + 1];
			Math::Vector3 m_normal[terrain_gridpoints + 1][terrain_gridpoints + 1];

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