#pragma once

#include "GameObject.h"

namespace est
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
			bool Initialize(const TerrainProperty& terrainProperty, bool isEnableThreadLoad);

		public:
			virtual float GetHeight(float fPosX, float fPosZ) const override;
			virtual float GetHeightMin() const override { return m_heightMin; }
			virtual float GetHeightMax() const override { return m_heightMax; }

			virtual bool IsBuildComplete() const override { return m_isBuildComplete; }

		private:
			bool init();
			bool loadHeightMap(const wchar_t* strFilePath);
			bool loadColorMap(const wchar_t* strFilePath);
			bool loadRawHeightmap(const wchar_t* strFilePath);
			bool initTerrain();

			std::optional<float> CheckHeightOfTriangle(float x, float z, const math::float3& v0, const math::float3& v1, const math::float3& v2) const;

		private:
			string::StringID m_strName;

			math::Matrix m_matWorld;
			math::Matrix m_matPrevWorld;

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

			graphics::VertexBufferPtr m_pHeightField{ nullptr };

			graphics::TexturePtr m_pTexHeightMap{ nullptr };
			graphics::TexturePtr m_pTexColorMap{ nullptr };
			graphics::TexturePtr m_pTexDetailMap{ nullptr };
			graphics::TexturePtr m_pTexDetailNormalMap{ nullptr };

			// 아래의 정점 정보는 RigidBody가 삭제되기 전까지 내용을 유지해야함
			// 아니면 댕글링 빠바방
			// RigidBody 내부에서 복사해서 사용하지 않는 이유는 동일한 정점 정보로
			// 여러 RigidBody를 만들어 사용하는 경우가 있기 때문에
			struct RigidBodyData
			{
				std::vector<math::float3> vertices;
				std::vector<uint32_t> indices;
			};
			RigidBodyData m_rigidBodyData;
			std::unique_ptr<physics::IRigidActor> m_pPhysics;

			float m_heightMax{ std::numeric_limits<float>::max() };
			float m_heightMin{ std::numeric_limits<float>::min() };
		};
	}
}