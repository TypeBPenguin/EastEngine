#pragma once

namespace eastengine
{
	namespace graphics
	{
		class IResource
		{
		protected:
			IResource() = default;

		public:
			virtual ~IResource() = default;

			virtual const string::StringID& GetResourceType() const = 0;

			enum State
			{
				eReady = 0,
				eLoading,
				eComplete,
				eInvalid,
			};

			enum
			{
				eLife = 10,
			};

		public:
			int GetReferenceCount() const { return m_nReferenceCount; }
			int IncreaseReference() { return ++m_nReferenceCount; }
			int DecreaseReference() { return --m_nReferenceCount; }

		public:
			bool IsAlive() const { return m_nLife > 0; }
			void SetAlive(bool isAlive) { m_nLife = isAlive == true ? eLife : 0; }
			void SubtractLife() { --m_nLife; }

			State GetState() const { return m_emLoadState; }
			void SetState(State emLoadState) { m_emLoadState = emLoadState; }

		private:
			std::atomic<int> m_nLife{ 0 };
			std::atomic<int> m_nReferenceCount{ 0 };
			State m_emLoadState{ State::eReady };
		};
	}
}