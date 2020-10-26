#pragma once

namespace est
{
	namespace graphics
	{
#define GraphicsResource(ClassType) friend std::default_delete<ClassType>;

		class IResource
		{
			GraphicsResource(IResource);
		protected:
			IResource() = default;
			virtual ~IResource() = default;

		public:
			virtual const string::StringID& GetResourceType() const = 0;

			enum State
			{
				eReady = 0,
				eLoading,
				eComplete,
				eInvalid,
			};

		public:
			State GetState() const { return m_emLoadState; }
			void SetState(State emLoadState) { m_emLoadState = emLoadState; }

		private:
			std::atomic<State> m_emLoadState{ State::eReady };
		};
	}
}