#pragma once

namespace est
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
				eVertex,
				eFxaa,
				eDownScale,
				eGaussianBlur,
				eDepthOfField,
				eAssao,
				eColorGrading,
				eBloomFilter,
				eScreenSpaceShadow,
				eSSS,
				eSSR,
				eHDR,
				eMotionBlur,
				eCopy,

				TypeCount,
			};

		public:
			virtual Type GetType() const = 0;
		};
	}
}