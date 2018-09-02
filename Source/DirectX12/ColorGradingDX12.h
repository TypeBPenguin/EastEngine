#pragma once

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class RenderTarget;

			class ColorGrading
			{
			public:
				ColorGrading();
				~ColorGrading();

			public:
				void Apply(Camera* pCamera, const RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}