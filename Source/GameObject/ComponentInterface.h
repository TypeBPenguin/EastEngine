#pragma once

namespace EastEngine
{
	namespace File
	{
		class FileStream;
	}

	namespace GameObject
	{
		namespace EmComponent
		{
			enum Type
			{
				eTimer = 0,
				eActionState,
				ePhysics,
				eModel,
				eCamera,
				eLight,

				TypeCount,
			};

			const char* ToString(Type emType);
			Type GetType(const char* strType);
		}

		class IActor;

		class IComponent
		{
		public:
			IComponent(IActor* pOwner, EmComponent::Type emCompType);
			virtual ~IComponent() = 0;

		public:
			virtual void Update(float fElapsedTime) = 0;

			virtual bool LoadToFile(File::FileStream& file) { return true; }
			virtual bool SaveToFile(File::FileStream& file) { return true; }

		public:
			IComponent* AddComponent(IComponent* pComponent);
			IComponent* GetComponent(EmComponent::Type emComponentType);
			void DelComponent(EmComponent::Type emComponentType);

		public:
			IActor* GetOwner() const { return m_pOwner; }
			EmComponent::Type GetComponentType() { return m_emCompType; }

		protected:
			IActor* m_pOwner;

			std::unordered_map<EmComponent::Type, IComponent*> m_umapChild;

		private:
			EmComponent::Type m_emCompType;
		};
	}
}