#pragma once

//#define NEW_TERRAIN

#include "GameObject.h"

#ifndef NEW_TERRAIN
#include "TerrainCells.h"
#endif

namespace EastEngine
{
#ifndef NEW_TERRAIN
	namespace GameObject
	{
		class Terrain : public ITerrain
		{
		public:
			Terrain();
			virtual ~Terrain();

		public:
			virtual const String::StringID& GetName() const override { return m_strName; }
			virtual void SetName(const String::StringID& strName) override { m_strName = strName; }

			virtual const Math::Matrix* GetWorldMatrixPtr() const override { return &m_matWorld; }
			virtual const Math::Matrix& GetWorldMatrix() const override { return m_matWorld; }
			virtual const Math::Matrix& CalcWorldMatrix() override;

			virtual const Math::Vector3& GetPosition() const override { return m_f3Pos; }
			virtual void SetPosition(const Math::Vector3& f3Pos) override { m_isDirtyWorldMatrix = true;  m_f3PrevPos = m_f3Pos; m_f3Pos = f3Pos; }
			virtual const Math::Vector3& GetPrevPosition() const override { return m_f3PrevPos; }

			virtual const Math::Vector3& GetScale() const override { return m_f3Scale; }
			virtual void SetScale(const Math::Vector3& f3Scale) override { m_isDirtyWorldMatrix = true; m_f3PrevScale = m_f3Scale; m_f3Scale = f3Scale; }
			virtual const Math::Vector3& GetPrevScale() const override { return m_f3PrevScale; }

			virtual const Math::Quaternion& GetRotation() const override { return m_quatRotation; }
			virtual void SetRotation(const Math::Quaternion& quat) override { m_isDirtyWorldMatrix = true; m_quatPrevRotation = m_quatRotation; m_quatRotation = quat; }
			virtual const Math::Quaternion& GetPrevRotation() const override { return m_quatPrevRotation; }

			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }
			virtual bool IsVisible() const override { return m_isVisible; }

		public:
			void SetDestroy(bool isDestroy) { m_isDestroy = isDestroy; }
			bool IsDestroy() const { return m_isDestroy; }

		public:
			bool Init(const TerrainProperty* pTerrainProperty, bool isEnableThreadLoad);

			void Update(float fElapsedTime);

		public:
			bool IsEnableShowCellLine() const { return m_isShowCellLine; }
			void SetEnableShowCellLine(bool isShowCellLine) { m_isShowCellLine = isShowCellLine; }

			float GetHeight(float fPosX, float fPosZ) const;

			bool IsLoadComplete() const { return m_isLoadComplete; }

		private:
			bool init();
			bool loadHeightMap(const char* strFilePath);
			bool loadColorMap(const char* strFilePath);
			bool loadRawHeightmap(const char* strFilePath);
			bool initTerrain();
			bool initTerrainCell(const std::vector<Graphics::VertexPosTexNorCol>& vecModel);

			std::optional<float> checkHeightOfTriangle(float x, float z, const Math::Vector3& v0, const Math::Vector3& v1, const Math::Vector3& v2);

		private:
			String::StringID m_strName;

			Math::Matrix m_matWorld;
			Math::Vector3 m_f3Pos;
			Math::Vector3 m_f3PrevPos;
			Math::Vector3 m_f3Scale;
			Math::Vector3 m_f3PrevScale;
			Math::Quaternion m_quatRotation;
			Math::Quaternion m_quatPrevRotation;

			bool m_isDestroy;
			bool m_isVisible;
			bool m_isDirtyWorldMatrix;

			struct HeightMapVertex
			{
				Math::Vector3 pos;
				Math::Vector3 normal;
				Math::Color color = Math::Color::White;
			};

			std::vector<float> m_vecHeight;
			std::vector<HeightMapVertex> m_vecHeightMap;

			TerrainProperty m_property;

			Math::Vector3 m_f3DefaultPos;

			std::vector<Graphics::IMaterial*> m_vecMaterial;

			uint32_t m_nCellCount;

			std::vector<TerrainCells> m_veTerrainCells;

			Physics::RigidBody* m_pPhysics;

			std::optional<std::vector<Math::Vector3>> m_optVertices;
			std::optional<Concurrency::task<bool>> m_optLoadTask;

			uint32_t m_nLastCell;

			bool m_isShowCellLine;
			bool m_isLoadComplete;
		};
	}

#else

	namespace Physics
	{
		class RigidBody;
	}

	namespace GameObject
	{
		class Terrain : public ITerrain
		{
		public:
			Terrain();
			virtual ~Terrain();

		public:
			void Init(const TerrainProperty* pTerrainProperty);

		public:
			void Update(float fElapsedTime);
			void PhysicsUpdate(float fElapsedTime);

		public:
			virtual const String::StringID& GetName() const override { return m_strName; }
			virtual void SetName(const String::StringID& strName) override { m_strName = strName; }

			virtual const Math::Matrix* GetWorldMatrixPtr() const override { return &m_matWorld; }
			virtual const Math::Matrix& GetWorldMatrix() const override { return m_matWorld; }
			virtual const Math::Matrix& CalcWorldMatrix() override;

			virtual const Math::Vector3& GetPosition() const override { return m_f3Pos; }
			virtual void SetPosition(const Math::Vector3& f3Pos) override { m_isDirtyWorldMatrix = true;  m_f3PrevPos = m_f3Pos; m_f3Pos = f3Pos; }
			virtual const Math::Vector3& GetPrevPosition() const override { return m_f3PrevPos; }

			virtual const Math::Vector3& GetScale() const override { return m_f3Scale; }
			virtual void SetScale(const Math::Vector3& f3Scale) override { m_isDirtyWorldMatrix = true; m_f3PrevScale = m_f3Scale; m_f3Scale = f3Scale; }
			virtual const Math::Vector3& GetPrevScale() const override { return m_f3PrevScale; }

			virtual const Math::Quaternion& GetRotation() const override { return m_quatRotation; }
			virtual void SetRotation(const Math::Quaternion& quat) override { m_isDirtyWorldMatrix = true; m_quatPrevRotation = m_quatRotation; m_quatRotation = quat; }
			virtual const Math::Quaternion& GetPrevRotation() const override { return m_quatPrevRotation; }

			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }
			virtual bool IsVisible() const override { return m_isVisible; }

		public:
			void SetDestroy(bool isDestroy) { m_isDestroy = isDestroy; }
			bool IsDestroy() const { return m_isDestroy; }

		private:
			void PhysicsDebugDrawCallback(const Math::Vector3* pTriangles, const uint32_t nCount);

		private:
			String::StringID m_strName;

			Math::Matrix m_matWorld;
			Math::Vector3 m_f3Pos;
			Math::Vector3 m_f3PrevPos;
			Math::Vector3 m_f3Scale;
			Math::Vector3 m_f3PrevScale;
			Math::Quaternion m_quatRotation;
			Math::Quaternion m_quatPrevRotation;

			bool m_isDestroy;
			bool m_isVisible;
			bool m_isDirtyWorldMatrix;

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

			Physics::RigidBody* m_pRigidBody;
			Graphics::IVertexBuffer* m_pDebugTriangles;
			Graphics::IIndexBuffer* m_pDebugTrianglesIB;
		};
	}
#endif
}