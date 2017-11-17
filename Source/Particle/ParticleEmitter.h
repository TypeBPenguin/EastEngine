#pragma once

#include "ParticleInterface.h"
#include "../DirectX/Vertex.h"

namespace EastEngine
{
	namespace Graphics
	{
		namespace EmParticleEmitter
		{
			enum ClassifyPoint
			{
				eFront = 0,
				eBack,
				eOnPlane,
			};

			enum CollisionResults
			{
				eBounce = 0,
				eStick,
				eRecycle,
			};
		}

		struct BouncePlane
		{
			Math::Vector3 f3Normal;
			Math::Vector3 f3Point;
			float fBounceFactor = 0.f;
			EmParticleEmitter::CollisionResults emResult = EmParticleEmitter::eBounce;
			String::StringID strName;
		};

		class ParticleEmitter : public IParticleEmitter
		{
		public:
			ParticleEmitter();
			virtual~ParticleEmitter();

		public:
			bool Init(const ParticleEmitterAttributes& attributes);

			virtual void Update(float fElapsedTime, const Math::Matrix& matView, const Math::Matrix& matViewProjection, const Collision::Frustum& frustum) override;

		private:
			float m_fTime;
			float m_fLastUpdate;

			std::list<ParticleInstanceInfo*> m_listAliveParticle;
			std::list<BouncePlane> m_listPlane;

			std::vector<VertexPosTexCol> m_vecVertices;

			ParticleEmitterAttributes m_attributes;

			std::shared_ptr<ITexture> m_pTexture;

			struct EmitterVertex
			{
				VertexPosTexCol vertex[4];
				Math::Vector3 f3Pos;
			};

			struct ParticleSortor
			{
				bool operator() (const EmitterVertex& a, const EmitterVertex& b)
				{
					return a.f3Pos.z > b.f3Pos.z;
				}
			};

			std::priority_queue<EmitterVertex, std::vector<EmitterVertex>, ParticleSortor> m_queueParticle;
		};
	}
}