#pragma once

#include "CommonLib/Singleton.h"

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IEffect;
		class IEffectTech;

		namespace EmRenderer
		{
			enum Type
			{
				ePreProcessing = 0,
				eSky,
				eTerrain,
				eModel,
				eDeferred,
				eWater,
				eVertex,
				ePostProcessing,
				eParticle,
				eUI,

				TypeCount,
			};
		};

		class RendererManager : public Singleton<RendererManager>
		{
			friend Singleton<RendererManager>;
		private:
			RendererManager();
			virtual ~RendererManager();

		public:
			void Render();
			void Flush();

		public:
			template <typename T>
			void AddRender(const T& renderSubset);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}