#pragma once

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			class RenderTarget;

			class DownScale
			{
			public:
				DownScale();
				~DownScale();

			public:
				void Apply4SW(const RenderTarget* pSource, RenderTarget* pResult, bool isLuminance = false);
				void Apply16SW(const RenderTarget* pSource, RenderTarget* pResult);

				void ApplyHW(const RenderTarget* pSource, RenderTarget* pResult);
				void Apply16HW(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}