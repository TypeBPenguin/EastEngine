#pragma once

#include "GameObject.h"

namespace eastengine
{
	namespace gameobject
	{
		class Terrain : public ITerrain
		{
		public:
			Terrain();
			virtual ~Terrain();

		public:
			virtual void Update(float fElapsedTime) override;

		public:
			virtual const String::StringID& GetName() const override { return m_strName; }
			virtual void SetName(const String::StringID& strName) override { m_strName = strName; }

			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }
			virtual bool IsVisible() const override { return m_isVisible; }

		public:
			void SetDestroy(bool isDestroy) { m_isDestroy = isDestroy; }
			bool IsDestroy() const { return m_isDestroy; }

		public:
			bool Init(const TerrainProperty& terrainProperty, bool isEnableThreadLoad);

		public:
			virtual float GetHeight(float fPosX, float fPosZ) const override;
			virtual float GetHeightMin() const override { return m_fHeightMin; }
			virtual float GetHeightMax() const override { return m_fHeightMax; }

			virtual bool IsBuildComplete() const override { return m_isBuildComplete; }

		private:
			bool init();
			bool loadHeightMap(const char* strFilePath);
			bool loadColorMap(const char* strFilePath);
			bool loadRawHeightmap(const char* strFilePath);
			bool initTerrain();

			std::optional<float> CheckHeightOfTriangle(float x, float z, const math::Vector3& v0, const math::Vector3& v1, const math::Vector3& v2) const;

		private:
			String::StringID m_strName;

			math::Matrix m_matWorld;

			bool m_isDestroy;
			bool m_isVisible;
			bool m_isBuildComplete;

			TerrainProperty m_property;

			struct HeightMapVertex
			{
				math::Vector3 pos;
				math::Vector3 normal;
			};
			std::vector<HeightMapVertex> m_vecHeightMap;

			graphics::IVertexBuffer* m_pHeightField;

			std::shared_ptr<graphics::ITexture> m_pTexHeightMap;
			std::shared_ptr<graphics::ITexture> m_pTexColorMap;
			std::shared_ptr<graphics::ITexture> m_pTexDetailMap;
			std::shared_ptr<graphics::ITexture> m_pTexDetailNormalMap;

			// �Ʒ��� ���� ������ RigidBody�� �����Ǳ� ������ ������ �����ؾ���
			// �ƴϸ� ��۸� ���ٹ�
			// RigidBody ���ο��� �����ؼ� ������� �ʴ� ������ ������ ���� ������
			// ���� RigidBody�� ����� ����ϴ� ��찡 �ֱ� ������
			struct RigidBodyData
			{
				std::vector<math::Vector3> vecVertices;
				std::vector<uint32_t> vecIndices;
			};
			RigidBodyData m_rigidBodyData;
			Physics::RigidBody* m_pPhysics;

			float m_fHeightMax;
			float m_fHeightMin;
		};
	}
}