#pragma once

#include "Physics/RigidBody.h"
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
			Math::Int2 n2Size = Math::Int2(1024, 1024);

			Math::Int2 n2Patches = Math::Int2(64, 64);

			float fHeightScale = 300.f;

			std::string strTexHeightMap;
			std::string strTexColorMap;

			// 터레인 디테일맵 여러개 사용할 수 있는 구조로 바꿔야함
			std::string strTexDetailMap;
			std::string strTexDetailNormalMap;

			float fRoughness = 1.f;
			float fMetallic = 0.f;

			Physics::RigidBodyProperty rigidBodyProperty;
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