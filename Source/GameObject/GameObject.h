#pragma once

#include "CommonLib/PhantomType.h"

#include "Physics/RigidBody.h"
#include "ComponentInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class ITexture;
	}

	namespace gameobject
	{
		enum ObjectType
		{
			eActor = 0,
			eTerrain,
			eSky,

			eTypeCount,
		};

		class IGameObject
		{
		private:
			struct tHandle {};

		public:
			using Handle = PhantomType<tHandle, const uint64_t>;

		protected:
			IGameObject(const Handle& handle);
			virtual ~IGameObject() = default;

		public:
			const Handle& GetHandle() const { return m_handle; }

		public:
			virtual ObjectType GetType() const = 0;

		private:
			const Handle m_handle;
		};

		class IActor : public IGameObject
		{
		protected:
			IActor(const Handle& handle);
			virtual ~IActor() = default;

		public:
			virtual ObjectType GetType() const override { return ObjectType::eActor; }

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
			virtual const String::StringID& GetName() const = 0;
			virtual void SetName(const String::StringID& strActorName) = 0;

			virtual const math::Matrix* GetWorldMatrixPtr() const = 0;
			virtual const math::Matrix& GetWorldMatrix() const = 0;
			virtual const math::Matrix& CalcWorldMatrix() = 0;

			virtual const math::Vector3& GetPosition() const = 0;
			virtual void SetPosition(const math::Vector3& f3Pos) = 0;
			virtual const math::Vector3& GetPrevPosition() const = 0;

			virtual const math::Vector3& GetScale() const = 0;
			virtual void SetScale(const math::Vector3& f3Scale) = 0;
			virtual const math::Vector3& GetPrevScale() const = 0;

			virtual const math::Quaternion& GetRotation() const = 0;
			virtual void SetRotation(const math::Quaternion& quat) = 0;
			virtual const math::Quaternion& GetPrevRotation() const = 0;

			virtual const math::Vector3& GetVelocity() const = 0;
			virtual void SetVelocity(const math::Vector3& f3Velocity) = 0;

			virtual void SetVisible(bool bVisible) = 0;
			virtual bool IsVisible() const = 0;
		};

		struct TerrainProperty
		{
			math::Int2 n2Size{ 1024, 1024 };

			math::Int2 n2Patches{ 64, 64 };

			float fHeightScale{ 300.f };

			math::Transform transform;

			std::string strTexHeightMap;
			std::string strTexColorMap;

			// 터레인 디테일맵 여러개 사용할 수 있는 구조로 바꿔야함
			std::string strTexDetailMap;
			std::string strTexDetailNormalMap;

			float fRoughness{ 1.f };
			float fMetallic{ 0.f };

			Physics::RigidBodyProperty rigidBodyProperty;
		};

		class ITerrain : public IGameObject
		{
		protected:
			ITerrain(const Handle& handle);
			virtual ~ITerrain() = default;

		public:
			static ITerrain* Create(const String::StringID& strName, const TerrainProperty& terrainProperty);
			static ITerrain* CreateAsync(const String::StringID& strName, const TerrainProperty& terrainProperty);
			static void Destroy(ITerrain** ppTerrain);

		public:
			virtual ObjectType GetType() const override { return ObjectType::eTerrain; }

		public:
			virtual void Update(float fElapsedTime) = 0;

		public:
			virtual const String::StringID& GetName() const = 0;
			virtual void SetName(const String::StringID& strActorName) = 0;

			virtual void SetVisible(bool bVisible) = 0;
			virtual bool IsVisible() const = 0;

		public:
			virtual float GetHeight(float fPosX, float fPosZ) const = 0;
			virtual float GetHeightMin() const = 0;
			virtual float GetHeightMax() const = 0;

			virtual bool IsBuildComplete() const = 0;
		};

		struct SkyboxProperty
		{
			std::string strTexSky;

			float fBoxSize = 1000.f;
		};

		class ISkybox : public IGameObject
		{
		protected:
			ISkybox(const Handle& handle);
			virtual ~ISkybox() = default;

		public:
			static ISkybox* Create(const String::StringID& strName, const SkyboxProperty& property);
			static void Destroy(ISkybox** ppSkybox);

		public:
			virtual ObjectType GetType() const override { return ObjectType::eSky; }

		public:
			virtual void Update(float fElapsedTime) = 0;

		public:
			virtual const String::StringID& GetName() const = 0;
			virtual void SetName(const String::StringID& strActorName) = 0;

			virtual void SetVisible(bool bVisible) = 0;
			virtual bool IsVisible() const = 0;

			virtual graphics::ITexture* GetTexture() const = 0;
			virtual void SetTexture(graphics::ITexture* pTexture) = 0;
		};
	}
}

namespace std
{
	template <>
	struct hash<eastengine::gameobject::IGameObject::Handle>
	{
		uint64_t operator()(const eastengine::gameobject::IGameObject::Handle& key) const
		{
			return key.value;
		}
	};
}