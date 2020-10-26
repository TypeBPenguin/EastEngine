#pragma once

#include "ParticleInterface.h"

namespace est
{
	namespace graphics
	{
		class ParticleEmitter : public IParticleEmitter
		{
		public:
			enum ClassifyPoint
			{
				eFront = 0,
				eBack,
				eOnPlane,
			};

			enum CollisionReaction
			{
				eBounce = 0,
				eStick,
				eRecycle,
			};

			struct BouncePlane
			{
				math::float3 normal{ 0.f, 1.f, 0.f };
				math::float3 position;
				float bounceFactor{ 0.f };
				CollisionReaction emReaction{ CollisionReaction::eBounce };

				ClassifyPoint Classify(const math::float3& particlePosition);
			};

		public:
			ParticleEmitter(const ParticleEmitterAttributes& attributes, const TexturePtr& pTexture);
			virtual~ParticleEmitter();

		public:
			virtual void Update(float elapsedTime, const math::Matrix& matView, const math::Matrix& matViewProjection, const collision::Frustum& frustum) override;

		private:
			ParticleEmitterAttributes m_attributes;
			TexturePtr m_pTexture{ nullptr };

			std::vector<ParticleInstance> m_instancePool;
			size_t m_aliveCount{ 0 };

			float m_updateTime{ 0.f };
			float m_lastUpdateTim{ 0.f };

			std::vector<BouncePlane> m_bouncePlanes;

			std::vector<VertexPosTexCol> m_vertices;

			struct EmitterVertices
			{
				VertexPosTexCol vertex[4];
				float zDepth{ 0.f };
			};

			struct VerticesSort
			{
				bool operator() (const EmitterVertices& a, const EmitterVertices& b)
				{
					return a.zDepth > b.zDepth;
				}
			};
			std::priority_queue<EmitterVertices, std::vector<EmitterVertices>, VerticesSort> m_queueParticle;
		};
	}
}