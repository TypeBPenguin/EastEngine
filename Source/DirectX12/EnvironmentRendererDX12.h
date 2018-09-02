#pragma once

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class EnvironmentRenderer
			{
			public:
				EnvironmentRenderer();
				~EnvironmentRenderer();

			public:
				void Render(Camera* pCamera);
				void Flush();

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}