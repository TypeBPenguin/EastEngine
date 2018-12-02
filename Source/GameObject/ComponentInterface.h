#pragma once

namespace eastengine
{
	namespace file
	{
		class Stream;
	}

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

			static const char* ToString(Type emType);
			static Type GetType(const char* strType);

		public:
			IComponent(IActor* pOwner, Type emCompType);
			virtual ~IComponent() = 0;

		public:
			virtual void Update(float elapsedTime) = 0;

			virtual bool LoadFile(file::Stream& file) { return true; }
			virtual bool SaveFile(file::Stream& file) { return true; }

		public:
			IComponent* AddComponent(IComponent* pComponent);
			IComponent* GetComponent(Type emComponentType);
			void DelComponent(Type emComponentType);

		public:
			IActor* GetOwner() const { return m_pOwner; }
			Type GetComponentType() const { return m_emCompType; }

		protected:
			IActor* m_pOwner;

			std::unordered_map<Type, IComponent*> m_umapChild;

		private:
			Type m_emCompType;
		};
	}
}