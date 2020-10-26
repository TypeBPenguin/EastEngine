#include "stdafx.h"
#include "ParticleEmitter.h"

namespace est
{
	namespace graphics
	{
		enum QuatPoint
		{
			eBottomLeft = 0,
			eBottomRight,
			eTopRight,
			eTopLeft,
		};

		inline const math::float3& QuadVertex(QuatPoint emQuatPoint)
		{
			static const math::float3 point[] =
			{
				{ -1.f, -1.f, 0.f },	// Bottom Left
				{ -1.f, 1.f, 0.f },		// Bottom Right
				{ 1.f, 1.f, 0.f },		// Top Right
				{ 1.f, -1.f, 0.f },		// Top Left
			};
			return point[emQuatPoint];
		};

		inline ParticleEmitter::ClassifyPoint ParticleEmitter::BouncePlane::Classify(const math::float3& particlePosition)
		{
			const math::float3 direction = position - particlePosition;
			const float result = direction.Dot(normal);

			if (result < -0.001f)
				return ParticleEmitter::eFront;

			if (result > 0.001f)
				return ParticleEmitter::eBack;

			return ParticleEmitter::eOnPlane;
		}

		ParticleEmitter::ParticleEmitter(const ParticleEmitterAttributes& attributes, const TexturePtr& pTexture)
			: m_attributes(attributes)
			, m_pTexture(pTexture)
			, m_instancePool(attributes.maxParticles)
			, m_vertices(attributes.maxParticles * 4)
		{
		}

		ParticleEmitter::~ParticleEmitter()
		{
			ReleaseResource(m_pTexture);
		}

		void ParticleEmitter::Update(float elapsedTime, const math::Matrix& matInvView, const math::Matrix& matViewProjection, const collision::Frustum& frustum)
		{
			if (GetState() != State::eRunning)
				return;

			m_updateTime += elapsedTime;

			math::Matrix matBilboardView = matInvView;
			matBilboardView.m[3][0] = m_attributes.position.x;
			matBilboardView.m[3][1] = m_attributes.position.y;
			matBilboardView.m[3][2] = m_attributes.position.z;
			matBilboardView.m[3][3] = 1.f;

			const math::Matrix matBilboardViewProjection = matBilboardView * matViewProjection;

			const math::float3 quadVertices[] =
			{
				QuadVertex(eBottomLeft) * m_attributes.size,
				QuadVertex(eBottomRight) * m_attributes.size,
				QuadVertex(eTopRight) * m_attributes.size,
				QuadVertex(eTopLeft) * m_attributes.size
			};

			const math::float2 quadUV[] =
			{
				{ 0.f, 1.f },
				{ 0.f, 0.f },
				{ 1.f, 0.f },
				{ 1.f, 1.f }
			};

			uint32_t vertexCount = 0;
			std::vector<ParticleInstance*> idleInstances;
			idleInstances.reserve(m_attributes.numToRelease);

			if (m_aliveCount > 0)
			{
				for (auto& instance : m_instancePool)
				{
					if (instance.isAlive == false)
					{
						if (idleInstances.size() < m_attributes.numToRelease)
						{
							idleInstances.emplace_back(&instance);
						}
					}
					else
					{
						const float timePassed = m_updateTime - instance.creationTime;
						if (timePassed >= m_attributes.lifeCycle)
						{
							instance.isAlive = false;
							--m_aliveCount;
						}
						else
						{
							instance.velocity += m_attributes.gravity * elapsedTime;

							if (m_attributes.isAirResistance == true)
							{
								instance.velocity += (m_attributes.wind - instance.velocity) * elapsedTime;
							}

							const math::float3 oldPosition = instance.position;
							instance.position += instance.velocity * elapsedTime;

							for (auto& bouncePlane : m_bouncePlanes)
							{
								const ClassifyPoint emClassifyPoint = bouncePlane.Classify(instance.position);
								if (emClassifyPoint == ClassifyPoint::eBack)
								{
									switch (bouncePlane.emReaction)
									{
									case CollisionReaction::eBounce:
									{
										instance.position = oldPosition;

										const math::float3 n = bouncePlane.normal.Dot(instance.velocity) * bouncePlane.normal;
										const math::float3 t = instance.velocity - n;

										instance.velocity = t - bouncePlane.bounceFactor * n;
									}
									break;
									case CollisionReaction::eRecycle:
									{
										instance.creationTime -= m_attributes.lifeCycle;
									}
									break;
									case CollisionReaction::eStick:
									{
										instance.position = oldPosition;
										instance.velocity = math::float3::Zero;
									}
									break;
									}
								}
							}

							if (m_pTexture->GetState() == IResource::State::eComplete)
							{
								const math::float3 quadPositions[] =
								{
									math::float3::Transform(quadVertices[eBottomLeft] + instance.position, matBilboardViewProjection),
									math::float3::Transform(quadVertices[eBottomRight] + instance.position, matBilboardViewProjection),
									math::float3::Transform(quadVertices[eTopRight] + instance.position, matBilboardViewProjection),
									math::float3::Transform(quadVertices[eTopLeft] + instance.position, matBilboardViewProjection)
								};

								for (auto& quadPosition : quadPositions)
								{
									if (frustum.Contains(quadPosition) != collision::EmContainment::eDisjoint)
									{
										EmitterVertices particle;

										particle.vertex[0].pos = quadPositions[eBottomLeft];
										particle.vertex[0].uv = quadUV[eBottomLeft];

										particle.vertex[1].pos = quadPositions[eBottomRight];
										particle.vertex[1].uv = quadUV[eBottomRight];

										particle.vertex[2].pos = quadPositions[eTopRight];
										particle.vertex[2].uv = quadUV[eTopRight];

										particle.vertex[3].pos = quadPositions[eTopLeft];
										particle.vertex[3].uv = quadUV[eTopLeft];

										particle.vertex[0].color =
											particle.vertex[1].color =
											particle.vertex[2].color =
											particle.vertex[3].color = instance.color;

										particle.zDepth = instance.position.z;

										m_queueParticle.emplace(particle);

										vertexCount += 4;
										break;
									}
								}
							}
						}
					}
				}
			}

			if (vertexCount > 0)
			{
				const size_t copySize = sizeof(VertexPosTexCol) * 4;
				size_t index = 0;
				while (m_queueParticle.empty() == false)
				{
					auto& particle = m_queueParticle.top();

					const size_t destSize = sizeof(VertexPosTexCol) * (m_vertices.size() - index);
					memory::Copy(&m_vertices[index], destSize, &particle.vertex, copySize);

					index += 4;
					m_queueParticle.pop();
				}

				//RenderSubsetParticleEmitter renderSubset(m_attributes.pBlendState,
				//	m_attributes.position, &m_vertices.front(), nVertexCount, m_pTexture);
				//RendererManager::GetInstance()->AddRender(renderSubset);
			}

			if (math::IsZero(m_attributes.endTime) == false &&
				m_updateTime > m_attributes.endTime)
			{
				Stop();
				return;
			}

			if (m_updateTime - m_lastUpdateTim > m_attributes.releaseInterval)
			{
				m_lastUpdateTim = m_updateTime;

				for (uint32_t i = 0; i < m_attributes.numToRelease; ++i)
				{
					if (m_aliveCount >= m_attributes.maxParticles)
						break;

					ParticleInstance* pInstance = idleInstances.back();
					idleInstances.pop_back();

					++m_aliveCount;

					pInstance->position = m_attributes.position;
					pInstance->velocity = m_attributes.velocity;

					if (math::IsZero(m_attributes.velocityIntensity) == false)
					{
						math::float3 randomDirection(math::RandomReal<float>(-1.f, 1.f),
							math::RandomReal<float>(-1.f, 1.f),
							math::RandomReal<float>(-1.f, 1.f));

						randomDirection.Normalize();

						pInstance->velocity += randomDirection * m_attributes.velocityIntensity;
					}

					pInstance->creationTime = m_updateTime;

					if (m_attributes.isRandomColor == true)
					{
						pInstance->color.r = math::RandomReal<float>(0.f, 1.f);
						pInstance->color.g = math::RandomReal<float>(0.f, 1.f);
						pInstance->color.b = math::RandomReal<float>(0.f, 1.f);
						pInstance->color.a = 1.f;
					}
					else
					{
						pInstance->color = m_attributes.color;
					}
				}
			}
		}
	}
}