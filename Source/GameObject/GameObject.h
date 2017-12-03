#pragma once

#include "ComponentInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;
		class IVertexBuffer;
	}

	namespace GameObject
	{
		enum EmObjectType
		{
			eActor = 0,
			eTerrain,
			eSky,
			eCloud,
		};

		class IGameObject
		{
		protected:
			IGameObject() = default;
			virtual ~IGameObject() = default;

		public:
			virtual EmObjectType GetType() const = 0;
		};

		class IActor : public IGameObject
		{
		protected:
			IActor() = default;
			virtual ~IActor() = default;

		public:
			virtual EmObjectType GetType() const override { return EmObjectType::eActor; }

		public:
			static IActor* CreateByFile(const char* strFilePath);
			static IActor* Create(const String::StringID& strName);
			static void Destroy(IActor** ppActor);

			static bool SaveToFile(IActor* pActor, const char* strFilePath);

		public:
			virtual void Update(float fElapsedTime) = 0;

			virtual IComponent* CreateComponent(EmComponent::Type emComponentType) = 0;
			virtual void DestroyComponent(EmComponent::Type emComponentType) = 0;

			virtual IComponent* GetComponent(EmComponent::Type emComponentType) = 0;

		public:
			virtual uint32_t GetActorID() const = 0;

			virtual const String::StringID& GetName() const = 0;
			virtual void SetName(const String::StringID& strActorName) = 0;

			virtual const Math::Matrix* GetWorldMatrixPtr() const = 0;
			virtual const Math::Matrix& GetWorldMatrix() const = 0;
			virtual const Math::Matrix& CalcWorldMatrix() = 0;

			virtual const Math::Vector3& GetPosition() const = 0;
			virtual void SetPosition(const Math::Vector3& f3Pos) = 0;
			virtual const Math::Vector3& GetPrevPosition() const = 0;

			virtual const Math::Vector3& GetScale() const = 0;
			virtual void SetScale(const Math::Vector3& f3Scale) = 0;
			virtual const Math::Vector3& GetPrevScale() const = 0;

			virtual const Math::Quaternion& GetRotation() const = 0;
			virtual void SetRotation(const Math::Quaternion& quat) = 0;
			virtual const Math::Quaternion& GetPrevRotation() const = 0;

			virtual const Math::Vector3& GetVelocity() const = 0;
			virtual void SetVelocity(const Math::Vector3& f3Velocity) = 0;

			virtual void SetVisible(bool bVisible) = 0;
			virtual bool IsVisible() const = 0;
		};

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

		class ITerrain : public IGameObject
		{
		protected:
			ITerrain() = default;
			virtual ~ITerrain() = default;

		public:
			static ITerrain* Create(const String::StringID& strName, const TerrainProperty* pTerrainProperty);
			static void Destroy(ITerrain** ppTerrain);

		public:
			virtual EmObjectType GetType() const override { return EmObjectType::eTerrain; }

		public:
			virtual void Update(float fElapsedTime) = 0;

		public:
			virtual const String::StringID& GetName() const = 0;
			virtual void SetName(const String::StringID& strActorName) = 0;

			virtual const Math::Matrix* GetWorldMatrixPtr() const = 0;
			virtual const Math::Matrix& GetWorldMatrix() const = 0;
			virtual const Math::Matrix& CalcWorldMatrix() = 0;

			virtual const Math::Vector3& GetPosition() const = 0;
			virtual void SetPosition(const Math::Vector3& f3Pos) = 0;
			virtual const Math::Vector3& GetPrevPosition() const = 0;

			virtual const Math::Vector3& GetScale() const = 0;
			virtual void SetScale(const Math::Vector3& f3Scale) = 0;
			virtual const Math::Vector3& GetPrevScale() const = 0;

			virtual const Math::Quaternion& GetRotation() const = 0;
			virtual void SetRotation(const Math::Quaternion& quat) = 0;
			virtual const Math::Quaternion& GetPrevRotation() const = 0;

			virtual void SetVisible(bool bVisible) = 0;
			virtual bool IsVisible() const = 0;
		};
	}
}