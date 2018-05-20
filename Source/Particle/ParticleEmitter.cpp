#include "stdafx.h"
#include "ParticleEmitter.h"

#include "CommonLib/FileUtil.h"

#include "DirectX/D3DInterface.h"

#include "Renderer/RendererManager.h"

#include "ParticleMgr.h"

namespace eastengine
{
	namespace graphics
	{
		static boost::object_pool<ParticleInstanceInfo> m_poolParticleInstance;

		enum EmQuad
		{
			eBottomLeft = 0,
			eBottomRight,
			eTopRight,
			eTopLeft,
		};

		inline const math::Vector3& QuadVertex(EmQuad emQuad)
		{
			static math::Vector3 vertex[] =
			{
				math::Vector3(-1.f, -1.f, 0.f),	// Bottom Left
				math::Vector3(-1.f, 1.f, 0.f),	// Bottom Right
				math::Vector3(1.f, 1.f, 0.f),		// Top Right
				math::Vector3(1.f, -1.f, 0.f),	// Top Left
			};

			return vertex[emQuad];
		};

		inline EmParticleEmitter::ClassifyPoint ClassifyPoint(const math::Vector3& f3Point, const BouncePlane& plane)
		{
			math::Vector3 f3Direction = plane.f3Point - f3Point;
			float fResult = f3Direction.Dot(plane.f3Normal);

			if (fResult < -0.001f)
				return EmParticleEmitter::eFront;

			if (fResult > 0.001f)
				return EmParticleEmitter::eBack;

			return EmParticleEmitter::eOnPlane;
		}

		ParticleEmitter::ParticleEmitter()
			: m_fTime(0.f)
			, m_fLastUpdate(0.f)
			, m_pTexture(nullptr)
		{
		}

		ParticleEmitter::~ParticleEmitter()
		{
			std::for_each(m_listAliveParticle.begin(), m_listAliveParticle.end(), [&](ParticleInstanceInfo* pInstance)
			{
				m_poolParticleInstance.destroy(pInstance);
			});
			m_listAliveParticle.clear();

			m_listPlane.clear();

			m_vecVertices.clear();

			m_pTexture.reset();
		}

		bool ParticleEmitter::Init(const ParticleEmitterAttributes& attributes)
		{
			if (attributes.strTexFile.empty() == false)
				return false;

			std::string strPath = file::GetDataPath();
			strPath.append(attributes.strTexFile);

			std::shared_ptr<ITexture> pTexture = ITexture::Create(strPath);

			if (pTexture == nullptr)
				return false;

			m_pTexture = pTexture;
			m_attributes = attributes;

			m_vecVertices.resize(attributes.nMaxParticles * 4);

			SetAlive(true);
			Start();

			ParticleManager::GetInstance()->AddParticle(this);

			return true;
		}

		void ParticleEmitter::Update(float fElapsedTime, const math::Matrix& matView, const math::Matrix& matViewProjection, const Collision::Frustum& frustum)
		{
			if (IsStart() == false)
				return;

			m_fTime += fElapsedTime;

			math::Matrix matBilboardView = matView;
			matBilboardView = matBilboardView.Invert();
			matBilboardView.m[3][0] = m_attributes.f3Pos.x;
			matBilboardView.m[3][1] = m_attributes.f3Pos.y;
			matBilboardView.m[3][2] = m_attributes.f3Pos.z;
			matBilboardView.m[3][3] = 1.f;

			math::Matrix matVP = matBilboardView * matViewProjection;

			math::Vector3 f3OldPos;

			math::Vector3 f3Vertexs[] = { QuadVertex(eBottomLeft) * m_attributes.fSize,
				QuadVertex(eBottomRight) * m_attributes.fSize,
				QuadVertex(eTopRight) * m_attributes.fSize,
				QuadVertex(eTopLeft) * m_attributes.fSize };

			static math::Vector2 f2UVs[] =
			{
				{ 0.f, 1.f },
				{ 0.f, 0.f },
				{ 1.f, 0.f },
				{ 1.f, 1.f }
			};

			uint32_t nVertexCount = 0;

			auto iter = m_listAliveParticle.begin();
			while (iter != m_listAliveParticle.end())
			{
				ParticleInstanceInfo* pInstance = *iter;

				float fTimePassed = m_fTime - pInstance->fCreationTime;

				if (fTimePassed >= m_attributes.fLifeCycle)
				{
					m_poolParticleInstance.destroy(pInstance);
					iter = m_listAliveParticle.erase(iter);
				}
				else
				{
					pInstance->f3Velocity += m_attributes.f3Gravity * fElapsedTime;

					if (m_attributes.isAirResistance == true)
					{
						pInstance->f3Velocity += (m_attributes.f3Wind - pInstance->f3Velocity) * fElapsedTime;
					}

					f3OldPos = pInstance->f3Pos;
					pInstance->f3Pos += pInstance->f3Velocity * fElapsedTime;

					for (auto iter_plane = m_listPlane.begin();
						iter_plane != m_listPlane.end();
						++iter_plane)
					{
						BouncePlane& plane = *iter_plane;

						EmParticleEmitter::ClassifyPoint emResult = ClassifyPoint(pInstance->f3Pos, plane);

						if (emResult == EmParticleEmitter::eBack)
						{
							switch (plane.emResult)
							{
							case EmParticleEmitter::eBounce:
							{
								pInstance->f3Pos = f3OldPos;

								float factor = plane.fBounceFactor;

								math::Vector3 n = plane.f3Normal.Dot(pInstance->f3Velocity) * plane.f3Normal;
								math::Vector3 t = pInstance->f3Velocity - n;

								pInstance->f3Velocity = t - factor * n;
							}
							break;
							case EmParticleEmitter::eRecycle:
							{
								pInstance->fCreationTime -= m_attributes.fLifeCycle;
							}
							break;
							case EmParticleEmitter::eStick:
							{
								pInstance->f3Pos = f3OldPos;
								pInstance->f3Velocity = math::Vector3::Zero;
							}
							break;
							}
						}
					}

					math::Vector3 f3Pos[] =
					{
						math::Vector3::Transform(f3Vertexs[eBottomLeft] + pInstance->f3Pos, matVP),
						math::Vector3::Transform(f3Vertexs[eBottomRight] + pInstance->f3Pos, matVP),
						math::Vector3::Transform(f3Vertexs[eTopRight] + pInstance->f3Pos, matVP),
						math::Vector3::Transform(f3Vertexs[eTopLeft] + pInstance->f3Pos, matVP)
					};

					for (uint32_t i = 0; i < 4; ++i)
					{
						if (frustum.Contains(f3Pos[i]) != Collision::EmContainment::eDisjoint)
						{
							EmitterVertex particle;

							particle.vertex[0].pos = f3Pos[eBottomLeft];
							particle.vertex[0].uv = f2UVs[eBottomLeft];

							particle.vertex[1].pos = f3Pos[eBottomRight];
							particle.vertex[1].uv = f2UVs[eBottomRight];

							particle.vertex[2].pos = f3Pos[eTopRight];
							particle.vertex[2].uv = f2UVs[eTopRight];

							particle.vertex[3].pos = f3Pos[eTopLeft];
							particle.vertex[3].uv = f2UVs[eTopLeft];

							if (m_attributes.isRandomColor == true)
							{
								math::Color(math::Random<float>(0.f, 1.f), math::Random<float>(0.f, 1.f), math::Random<float>(0.f, 1.f), 1.f);
							}

							particle.vertex[0].color = particle.vertex[1].color =
								particle.vertex[2].color = particle.vertex[3].color = m_attributes.color;

							particle.f3Pos = pInstance->f3Pos;

							m_queueParticle.push(particle);

							nVertexCount += 4;

							break;
						}
					}

					++iter;
				}
			}

			if (nVertexCount > 0)
			{
				size_t nCopySize = sizeof(VertexPosTexCol) * 4;
				size_t nIndex = 0;
				while (m_queueParticle.empty() == false)
				{
					auto& particle = m_queueParticle.top();

					size_t nDestSize = sizeof(VertexPosTexCol) * (m_vecVertices.size() - nIndex);
					Memory::Copy(&m_vecVertices[nIndex], nDestSize, &particle.vertex, nCopySize);

					nIndex += 4;

					m_queueParticle.pop();
				}

				RenderSubsetParticleEmitter renderSubset(m_attributes.pBlendState,
					m_attributes.f3Pos, &m_vecVertices.front(), nVertexCount, m_pTexture);
				RendererManager::GetInstance()->AddRender(renderSubset);
			}

			if (math::IsZero(m_attributes.fEndTime) == false &&
				m_fTime > m_attributes.fEndTime)
			{
				SetAlive(false);
				return;
			}

			if (m_fTime - m_fLastUpdate > m_attributes.fReleaseInterval)
			{
				m_fLastUpdate = m_fTime;

				for (uint32_t i = 0; i < m_attributes.nNumToRelease; ++i)
				{
					if (m_listAliveParticle.size() < m_attributes.nMaxParticles)
					{
						ParticleInstanceInfo* pInstance = m_poolParticleInstance.construct();

						pInstance->f3Velocity = m_attributes.f3Velocity;

						if (math::IsZero(m_attributes.fVelocityVar) == false)
						{
							math::Vector3 f3RandomDirection(math::Random<float>(-1.f, 1.f),
								math::Random<float>(-1.f, 1.f),
								math::Random<float>(-1.f, 1.f));

							f3RandomDirection.Normalize();

							pInstance->f3Velocity += f3RandomDirection * m_attributes.fVelocityVar;
						}

						pInstance->fCreationTime = m_fTime;
						pInstance->f3Pos = m_attributes.f3Pos;

						m_listAliveParticle.push_back(pInstance);
					}
				}
			}
		}
	}
}