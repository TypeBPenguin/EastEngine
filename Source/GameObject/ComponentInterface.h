#pragma once

namespace est
{
	namespace gameobject
	{
		class IActor;

		class IComponent
		{
		public:
			enum Type
			{
				eTimer = 0,
				eBehaviorTree,
				eFiniteStateMachine,
				ePhysics,
				eModel,
				eCamera,
				eLight,

				TypeCount,
			};

			static const wchar_t* ToString(Type emType);
			static Type GetType(const wchar_t* strType);

		public:
			IComponent(IActor* pOwner, Type emCompType);
			virtual ~IComponent() = 0;

		public:
			virtual void Update(float elapsedTime) = 0;

		public:
			IActor* GetOwner() const { return m_pOwner; }
			Type GetComponentType() const { return m_emCompType; }

		protected:
			IActor* m_pOwner{ nullptr };

		private:
			Type m_emCompType;
		};
	}
}