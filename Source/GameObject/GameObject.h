#pragma once

#include "CommonLib/PhantomType.h"

#include "Graphics/Interface/GraphicsInterface.h"
#include "Physics/PhysicsInterface.h"

#include "ComponentInterface.h"

namespace est
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
		public:
			enum : uint64_t
			{
				eInvalidHandle = std::numeric_limits<uint64_t>::max(),
			};
			struct tHandle { static constexpr uint64_t DefaultValue() { return eInvalidHandle; } };
			using Handle = PhantomType<tHandle, uint64_t>;

			inline static const Handle InvalidHandle{ eInvalidHandle };

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

		struct GameObjectDeleter { void operator()(IGameObject* pGameObject); };

		class IActor : public IGameObject
		{
		protected:
			IActor(const Handle& handle);
			virtual ~IActor() = default;

		public:
			virtual ObjectType GetType() const override { return ObjectType::eActor; }

		public:
			virtual IComponent* CreateComponent(IComponent::Type emComponentType) = 0;
			virtual void DestroyComponent(IComponent::Type emComponentType) = 0;

			virtual IComponent* GetComponent(IComponent::Type emComponentType) = 0;

		public:
			virtual const string::StringID& GetName() const = 0;
			virtual void SetName(const string::StringID& actorName) = 0;

			virtual const math::Matrix& GetWorldMatrix() = 0;

			virtual const math::float3& GetPosition() const = 0;
			virtual void SetPosition(const math::float3& position) = 0;
			virtual const math::float3& GetPrevPosition() const = 0;

			virtual const math::float3& GetScale() const = 0;
			virtual void SetScale(const math::float3& scale) = 0;
			virtual const math::float3& GetPrevScale() const = 0;

			virtual const math::Quaternion& GetRotation() const = 0;
			virtual void SetRotation(const math::Quaternion& rotation) = 0;
			virtual const math::Quaternion& GetPrevRotation() const = 0;

			virtual void SetVisible(bool bVisible) = 0;
			virtual bool IsVisible() const = 0;
		};
		using ActorPtr = std::unique_ptr<IActor, GameObjectDeleter>;

		struct TerrainProperty
		{
			math::int2 size{ 1024, 1024 };

			math::int2 patches{ 64, 64 };

			float heightScale{ 300.f };

			math::Transform transform;

			std::wstring texHeightMap;
			std::wstring texColorMap;

			// 터레인 디테일맵 여러개 사용할 수 있는 구조로 바꿔야함
			std::wstring texDetailMap;
			std::wstring texDetailNormalMap;

			float roughness{ 1.f };
			float metallic{ 0.f };

			physics::RigidActorProperty rigidActorProperty;
		};

		class ITerrain : public IGameObject
		{
		protected:
			ITerrain(const Handle& handle);
			virtual ~ITerrain() = default;

		public:
			virtual ObjectType GetType() const override { return ObjectType::eTerrain; }

		public:
			virtual void Update(float elapsedTime) = 0;

		public:
			virtual const string::StringID& GetName() const = 0;
			virtual void SetName(const string::StringID& actorName) = 0;

			virtual void SetVisible(bool bVisible) = 0;
			virtual bool IsVisible() const = 0;

		public:
			virtual float GetHeight(float fPosX, float fPosZ) const = 0;
			virtual float GetHeightMin() const = 0;
			virtual float GetHeightMax() const = 0;

			virtual bool IsBuildComplete() const = 0;
		};
		using TerrainPtr = std::unique_ptr<ITerrain, GameObjectDeleter>;

		struct SkyboxProperty
		{
			std::wstring texSkymap;
			float boxSize{ 1000.f };
		};

		class ISkybox : public IGameObject
		{
		protected:
			ISkybox(const Handle& handle);
			virtual ~ISkybox() = default;

		public:
			virtual ObjectType GetType() const override { return ObjectType::eSky; }

		public:
			virtual void Update(float elapsedTime) = 0;

		public:
			virtual const string::StringID& GetName() const = 0;
			virtual void SetName(const string::StringID& actorName) = 0;

			virtual void SetVisible(bool bVisible) = 0;
			virtual bool IsVisible() const = 0;

			virtual graphics::TexturePtr GetTexture() const = 0;
			virtual void SetTexture(const graphics::TexturePtr& pTexture) = 0;
		};
		using SkyboxPtr = std::unique_ptr<ISkybox, GameObjectDeleter>;

		ActorPtr CreateActor(const string::StringID& name);
		SkyboxPtr CreateSkybox(const string::StringID& name, const SkyboxProperty& skyProperty);
		TerrainPtr CreateTerrain(const string::StringID& name, const TerrainProperty& terrainProperty);
		TerrainPtr CreateTerrainAsync(const string::StringID& name, const TerrainProperty& terrainProperty);
	}
}

namespace std
{
	template <>
	struct hash<est::gameobject::IGameObject::Handle>
	{
		uint64_t operator()(const est::gameobject::IGameObject::Handle& key) const
		{
			return key;
		}
	};
}