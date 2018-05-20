#pragma once

#include "Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		struct ParticleSortor
		{
			bool operator() (const RenderSubsetParticleEmitter& a, const RenderSubsetParticleEmitter& b)
			{
				return a.f3Pos.z < b.f3Pos.z;
			}
		};

		class ParticleRenderer : public IRenderer
		{
		public:
			ParticleRenderer();
			virtual ~ParticleRenderer();

			enum Group
			{
				eEmitter = 1 << 0,
				eDecal = 1 << 1,
				eDecalAlpha = 1 << 2,
			};

			enum
			{
				eEmitterCapacity = 8192 * 8,
			};

		public:
			virtual void AddRender(const RenderSubsetParticleEmitter& renderSubset) override;
			virtual void AddRender(const RenderSubsetParticleDecal& renderSubset) override;

			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}