#pragma once

#include "GameObject.h"

namespace eastengine
{
	namespace gameobject
	{
		class Terrain : public ITerrain
		{
		public:
			Terrain(const Handle& handle);
			virtual ~Terrain();

		public:
			virtual void Update(float elapsedTime) override;

		public:
			virtual const string::StringID& GetName() const override { return m_strName; }
			virtual void SetName(const string::StringID& strName) override { m_strName = strName; }

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

			std::optional<float> CheckHeightOfTriangle(float x, float z, const math::float3& v0, const math::float3& v1, const math::float3& v2) const;

		private:
			string::StringID m_strName;

			math::Matrix m_matWorld;

			bool m_isDestroy{ false };
			bool m_isVisible{ true };
			bool m_isBuildComplete{ false };

			TerrainProperty m_property;

			struct HeightMapVertex
			{
				math::float3 pos;
				math::float3 normal;
			};
			std::vector<HeightMapVertex> m_vecHeightMap;

			graphics::IVertexBuffer* m_pHeightField{ nullptr };

			graphics::ITexture* m_pTexHeightMap{ nullptr };
			graphics::ITexture* m_pTexColorMap{ nullptr };
			graphics::ITexture* m_pTexDetailMap{ nullptr };
			graphics::ITexture* m_pTexDetailNormalMap{ nullptr };

			// �Ʒ��� ���� ������ RigidBody�� �����Ǳ� ������ ������ �����ؾ���
			// �ƴϸ� ��۸� ���ٹ�
			// RigidBody ���ο��� �����ؼ� ������� �ʴ� ������ ������ ���� ������
			// ���� RigidBody�� ����� ����ϴ� ��찡 �ֱ� ������
			struct RigidBodyData
			{
				std::vector<math::float3> vecVertices;
				std::vector<uint32_t> vecIndices;
			};
			RigidBodyData m_rigidBodyData;
			physics::RigidBody* m_pPhysics{ nullptr };

			float m_fHeightMax{ std::numeric_limits<float>::max() };
			float m_fHeightMin{ std::numeric_limits<float>::min() };
		};
	}
}