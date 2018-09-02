#pragma once

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class RenderTarget;

			class BloomFilter
			{
			public:
				BloomFilter();
				~BloomFilter();;

			public:
				void Apply(RenderTarget* pSource);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}