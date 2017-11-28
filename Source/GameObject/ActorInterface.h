#pragma once

#include "GameObject.h"
#include "ComponentInterface.h"

namespace EastEngine
{
	namespace GameObject
	{
		class IActor : public IGameObject
		{
		protected:
			IActor();
			virtual ~IActor() = 0;

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
	}
}