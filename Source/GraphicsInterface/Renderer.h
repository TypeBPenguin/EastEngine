#pragma once

namespace eastengine
{
	namespace graphics
	{
		class IRenderer
		{
		public:
			IRenderer();
			virtual ~IRenderer() = 0;

			enum Type
			{
				eModel = 0,
				eDeferred,
				eEnvironment,
				eTerrain,
				eFxaa,
				eDownScale,
				eGaussianBlur,
				eDepthOfField,
				eAssao,
				eColorGrading,
				eBloomFilter,
				eSSS,
				eHDR,

				TypeCount,
			};

		public:
			virtual Type GetType() const = 0;
		};
	}
}