#pragma once

#include "Renderer.h"

#include <queue>

namespace EastEngine
{
	namespace Graphics
	{
		class IEffect;
		class IEffectTech;

		namespace EmParticleGroup
		{
			enum Type
			{
				eEmitter = 1 << 0,
				eDecal = 1 << 1,
				eDecalAlpha = 1 << 2,
			};
		}

		enum
		{
			eEmitterCapacity = 8192 * 8,
		};

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

		public:
			virtual bool Init(const Math::Viewport& viewport) override;

			virtual void AddRender(const RenderSubsetParticleEmitter& renderSubset) override
			{
				if (m_nEmitterVertexCount + renderSubset.nVertexCount > eEmitterCapacity)
					return;

				m_nEmitterVertexCount += renderSubset.nVertexCount;

				m_queueEmitter.emplace(renderSubset);
			}

			virtual void AddRender(const RenderSubsetParticleDecal& renderSubset) override { m_listDecal.emplace_back(renderSubset); }

			virtual void Render(uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			void renderEmitter();
			void renderDecal();

		private:
			IEffect* m_pEffect;

			std::priority_queue<RenderSubsetParticleEmitter, std::vector<RenderSubsetParticleEmitter>, ParticleSortor> m_queueEmitter;
			uint32_t m_nEmitterVertexCount;

			std::list<RenderSubsetParticleDecal> m_listDecal;

			IVertexBuffer* m_pEmitterVB;
			IIndexBuffer* m_pEmitterIB;

			IVertexBuffer* m_pDecalVB;
			IIndexBuffer* m_pDecalIB;
		};
	}
}