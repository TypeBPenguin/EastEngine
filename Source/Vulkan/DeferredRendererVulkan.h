#pragma once

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace vulkan
		{
			class DeferredRenderer
			{
			public:
				DeferredRenderer();
				~DeferredRenderer();

			public:
				void Render(Camera* pCamera);
				void Cleanup();

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}