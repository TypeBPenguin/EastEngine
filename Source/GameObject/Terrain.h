#pragma once

#include "GameObject.h"

namespace EastEngine
{
	namespace Thread
	{
		class Thread;
	}

	namespace GameObject
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

			virtual const Math::Matrix* GetWorldMatrixPtr() const override { return &m_matWorld; }
			virtual const Math::Matrix& GetWorldMatrix() const override { return m_matWorld; }
			virtual const Math::Matrix& CalcWorldMatrix() override { m_isDirtyWorldMatrix = false; m_matWorld = Math::Matrix::Compose(m_f3Scale, m_quatRotation, m_f3Pos); return m_matWorld; }

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

			std::optional<float> CheckHeightOfTriangle(float x, float z, const Math::Vector3& v0, const Math::Vector3& v1, const Math::Vector3& v2) const;

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
			bool m_isBuildComplete;

			TerrainProperty m_property;

			struct HeightMapVertex
			{
				Math::Vector3 pos;
				Math::Vector3 normal;
			};
			std::vector<HeightMapVertex> m_vecHeightMap;

			Graphics::IVertexBuffer* m_pHeightField;

			std::shared_ptr<Graphics::ITexture> m_pTexHeightMap;
			std::shared_ptr<Graphics::ITexture> m_pTexColorMap;
			std::shared_ptr<Graphics::ITexture> m_pTexDetailMap;
			std::shared_ptr<Graphics::ITexture> m_pTexDetailNormalMap;

			Physics::RigidBody* m_pPhysics;

			std::optional<std::vector<Math::Vector3>> m_optVertices;
			float m_fHeightMax;
			float m_fHeightMin;
		};
	}
}