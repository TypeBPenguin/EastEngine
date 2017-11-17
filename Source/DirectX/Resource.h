#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		namespace EmLoadState
		{
			enum Type
			{
				eReady = 0,
				eLoading,
				eComplete,
				eInvalid,
			};
		};

		class Resource
		{
		public:
			Resource();
			virtual ~Resource();

		public:
			bool IsAlive() { return m_nLife > 0; }
			void SetAlive(bool isAlive) { m_nLife = isAlive == true ? 10 : 0; }
			void SubtractLife() { --m_nLife; }

			EmLoadState::Type GetLoadState() const { return m_emLoadState; }
			void SetLoadState(EmLoadState::Type emLoadState) { m_emLoadState = emLoadState; }

		private:
			int m_nLife;

			EmLoadState::Type m_emLoadState;
		};
	}
}