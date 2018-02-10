#include "stdafx.h"
#include "LightMgr.h"

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class LightManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update(float fElapsedTime);
			void Synchronize();

		public:
			bool AddLight(ILight* pLight);
			void Remove(ILight* pLight);
			void Remove(EmLight::Type emType, size_t nIndex);
			void RemoveAll();
			ILight* GetLight(EmLight::Type emType, size_t nIndex);
			size_t GetLightCount(EmLight::Type emType);

		public:
			IStructuredBuffer* GetLightBuffer(EmLight::Type emType);
			uint32_t GetLightCountInView(EmLight::Type emType);

		private:
			void UpdateLightBuffer();

		private:
			std::array<IStructuredBuffer*, EmLight::eCount> m_pLightBuffers;
			std::array<uint32_t, EmLight::eCount> m_nLightCountInView;

			std::vector<IDirectionalLight*> m_vecDirectionalLights;
			std::vector<ISpotLight*> m_vecSpotLights;
			std::vector<IPointLight*> m_vecPointLights;

			struct DirectionalLightData
			{
				Math::Vector3 f3Color;
				float fLightIntensity = 0.f;

				Math::Vector3 f3Dir;
				float fAmbientIntensity = 0.f;

				Math::Vector3 padding;
				float fReflectionIntensity = 0.f;

				void Set(const Math::Color& color, const Math::Vector3& direction, float lightIntensity, float ambientIntensity, float reflectionIntensity)
				{
					f3Color.x = color.r;
					f3Color.y = color.g;
					f3Color.z = color.b;

					f3Dir = direction;
					fLightIntensity = lightIntensity;
					fAmbientIntensity = ambientIntensity;
					fReflectionIntensity = reflectionIntensity;
				}
			};
			std::array<DirectionalLightData, EmLight::eMaxDirectionalLightCount> m_directionalLightData;

			struct PointLightData
			{
				Math::Vector3 f3Color;
				float fLightIntensity = 0.f;

				Math::Vector3 f3Pos;
				float fAmbientIntensity = 0.f;

				Math::Vector3 padding;
				float fReflectionIntensity = 0.f;

				void Set(const Math::Color& color, const Math::Vector3& pos, float lightIntensity, float ambientIntensity, float reflectionIntensity)
				{
					f3Color.x = color.r;
					f3Color.y = color.g;
					f3Color.z = color.b;

					f3Pos = pos;
					fLightIntensity = lightIntensity;
					fAmbientIntensity = ambientIntensity;
					fReflectionIntensity = reflectionIntensity;
				}
			};
			std::array<PointLightData, EmLight::eMaxPointLightCount> m_pointLightData;

			struct SpotLightData
			{
				Math::Vector3 f3Color;
				float fLightIntensity = 0.f;

				Math::Vector3 f3Pos;
				float fAmbientIntensity = 0.f;

				Math::Vector3 f3Dir;
				float fReflectionIntensity = 0.f;

				Math::Vector3 padding;
				float fAngle = 0.f;

				void Set(const Math::Color& color, const Math::Vector3& position, const Math::Vector3& direction, float lightIntensity, float ambientIntensity, float reflectionIntensity, float angle)
				{
					f3Color.x = color.r;
					f3Color.y = color.g;
					f3Color.z = color.b;

					f3Pos = position;
					f3Dir = direction;
					fLightIntensity = lightIntensity;
					fAmbientIntensity = ambientIntensity;
					fReflectionIntensity = reflectionIntensity;
					fAngle = angle;
				}
			};
			std::array<SpotLightData, EmLight::eMaxSpotLightCount> m_spotLightData;
		};

		LightManager::Impl::Impl()
		{
			m_pLightBuffers[EmLight::eDirectional] = IStructuredBuffer::Create(m_directionalLightData.data(), m_directionalLightData.size(), sizeof(DirectionalLightData));
			m_pLightBuffers[EmLight::ePoint] = IStructuredBuffer::Create(m_pointLightData.data(), m_pointLightData.size(), sizeof(PointLightData));
			m_pLightBuffers[EmLight::eSpot] = IStructuredBuffer::Create(m_spotLightData.data(), m_spotLightData.size(), sizeof(SpotLightData));

			m_nLightCountInView.fill(0u);
		}

		LightManager::Impl::~Impl()
		{
			RemoveAll();

			std::for_each(m_pLightBuffers.begin(), m_pLightBuffers.end(), DeleteSTLObject());
			m_pLightBuffers.fill(nullptr);
		}

		void LightManager::Impl::Update(float fElapsedTime)
		{
			PERF_TRACER_EVENT("LightManager::Update", "");

			PERF_TRACER_BEGINEVENT("LightManager::Update", "Directional");
			std::for_each(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), [fElapsedTime](IDirectionalLight* pLight)
			{
				pLight->Update(fElapsedTime);
			});
			PERF_TRACER_ENDEVENT();

			PERF_TRACER_BEGINEVENT("LightManager::Update", "Spot");
			std::for_each(m_vecSpotLights.begin(), m_vecSpotLights.end(), [fElapsedTime](ISpotLight* pLight)
			{
				pLight->Update(fElapsedTime);
			});
			PERF_TRACER_ENDEVENT();

			PERF_TRACER_BEGINEVENT("LightManager::Update", "Point");
			std::for_each(m_vecPointLights.begin(), m_vecPointLights.end(), [fElapsedTime](IPointLight* pLight)
			{
				pLight->Update(fElapsedTime);
			});
			PERF_TRACER_ENDEVENT();
		}

		void LightManager::Impl::Synchronize()
		{
			PERF_TRACER_EVENT("LightManager::Synchronize", "");
			UpdateLightBuffer();
		}

		bool LightManager::Impl::AddLight(ILight* pLight)
		{
			switch (pLight->GetType())
			{
			case EmLight::Type::eDirectional:
			{
				auto iter = std::find(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), pLight);
				if (iter != m_vecDirectionalLights.end())
					return false;

				m_vecDirectionalLights.emplace_back(static_cast<IDirectionalLight*>(pLight));
			}
			break;
			case EmLight::Type::ePoint:
			{
				auto iter = std::find(m_vecPointLights.begin(), m_vecPointLights.end(), pLight);
				if (iter != m_vecPointLights.end())
					return false;

				m_vecPointLights.emplace_back(static_cast<IPointLight*>(pLight));
			}
			break;
			case EmLight::Type::eSpot:
			{
				auto iter = std::find(m_vecSpotLights.begin(), m_vecSpotLights.end(), pLight);
				if (iter != m_vecSpotLights.end())
					return false;

				m_vecSpotLights.emplace_back(static_cast<ISpotLight*>(pLight));
			}
			break;
			default:
				return false;
			}

			return true;
		}

		void LightManager::Impl::Remove(ILight* pLight)
		{
			switch (pLight->GetType())
			{
			case EmLight::Type::eDirectional:
			{
				auto iter = std::find(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), pLight);
				if (iter != m_vecDirectionalLights.end())
				{
					m_vecDirectionalLights.erase(iter);
				}
			}
			break;
			case EmLight::Type::ePoint:
			{
				auto iter = std::find(m_vecPointLights.begin(), m_vecPointLights.end(), pLight);
				if (iter != m_vecPointLights.end())
				{
					m_vecPointLights.erase(iter);
				}
			}
			break;
			case EmLight::Type::eSpot:
			{
				auto iter = std::find(m_vecSpotLights.begin(), m_vecSpotLights.end(), pLight);
				if (iter != m_vecSpotLights.end())
				{
					m_vecSpotLights.erase(iter);
				}
			}
			break;
			default:
				break;
			}
		}

		void LightManager::Impl::Remove(EmLight::Type emType, size_t nIndex)
		{
			switch (emType)
			{
			case EmLight::Type::eDirectional:
			{
				if (nIndex >= m_vecDirectionalLights.size())
					return;

				auto iter = m_vecDirectionalLights.begin();
				std::advance(iter, nIndex);

				if (iter != m_vecDirectionalLights.end())
				{
					m_vecDirectionalLights.erase(iter);
				}
			}
			break;
			case EmLight::Type::ePoint:
			{
				if (nIndex >= m_vecPointLights.size())
					return;

				auto iter = m_vecPointLights.begin();
				std::advance(iter, nIndex);

				if (iter != m_vecPointLights.end())
				{
					m_vecPointLights.erase(iter);
				}
			}
			break;
			case EmLight::Type::eSpot:
			{
				if (nIndex >= m_vecSpotLights.size())
					return;

				auto iter = m_vecSpotLights.begin();
				std::advance(iter, nIndex);

				if (iter != m_vecSpotLights.end())
				{
					m_vecSpotLights.erase(iter);
				}
			}
			break;
			default:
				break;
			}
		}

		void LightManager::Impl::RemoveAll()
		{
			std::for_each(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), DeleteSTLObject());
			m_vecDirectionalLights.clear();

			std::for_each(m_vecSpotLights.begin(), m_vecSpotLights.end(), DeleteSTLObject());
			m_vecDirectionalLights.clear();

			std::for_each(m_vecPointLights.begin(), m_vecPointLights.end(), DeleteSTLObject());
			m_vecDirectionalLights.clear();
		}

		ILight* LightManager::Impl::GetLight(EmLight::Type emType, size_t nIndex)
		{
			switch (emType)
			{
			case EmLight::Type::eDirectional:
			{
				if (nIndex >= m_vecDirectionalLights.size())
					return nullptr;

				return m_vecDirectionalLights[nIndex];
			}
			break;
			case EmLight::Type::ePoint:
			{
				if (nIndex >= m_vecPointLights.size())
					return nullptr;

				return m_vecPointLights[nIndex];
			}
			break;
			case EmLight::Type::eSpot:
			{
				if (nIndex >= m_vecSpotLights.size())
					return nullptr;

				return m_vecSpotLights[nIndex];
			}
			break;
			default:
				return nullptr;
			}
		}

		size_t LightManager::Impl::GetLightCount(EmLight::Type emType)
		{
			switch (emType)
			{
			case EmLight::Type::eDirectional:
				return m_vecDirectionalLights.size();
			case EmLight::Type::ePoint:
				return m_vecPointLights.size();
			case EmLight::Type::eSpot:
				return m_vecSpotLights.size();
			default:
				return 0;
			}
		}

		IStructuredBuffer* LightManager::Impl::GetLightBuffer(EmLight::Type emType)
		{
			return m_pLightBuffers[emType];
		}

		uint32_t LightManager::Impl::GetLightCountInView(EmLight::Type emType)
		{
			return m_nLightCountInView[emType];
		}

		void LightManager::Impl::UpdateLightBuffer()
		{
			m_nLightCountInView.fill(0);

			PERF_TRACER_BEGINEVENT("LightManager::UpdateLightBuffer", "Directional");
			std::for_each(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), [&](IDirectionalLight* pLight)
			{
				uint32_t& nLightIndex = m_nLightCountInView[EmLight::eDirectional];
				if (nLightIndex < EmLight::eMaxDirectionalLightCount)
				{
					m_directionalLightData[nLightIndex].Set(pLight->GetColor(), pLight->GetDirection(), pLight->GetIntensity(), pLight->GetAmbientIntensity(), pLight->GetReflectionIntensity());
					++nLightIndex;
				}
			});
			PERF_TRACER_ENDEVENT();

			PERF_TRACER_BEGINEVENT("LightManager::UpdateLightBuffer", "Point");
			std::for_each(m_vecPointLights.begin(), m_vecPointLights.end(), [&](IPointLight* pLight)
			{
				Collision::Sphere sphere(pLight->GetPosition(), Math::PI * pLight->GetIntensity());

				// 이걸 보거든, 방향 및 거리 계산을 계산해서 프러스텀 안에 들어갈때 버퍼에 추가하도록 바꾸시오.
				//if (frustum.Contains(sphere) == Collision::EmContainment::eContains)
				{
					uint32_t& nLightIndex = m_nLightCountInView[EmLight::ePoint];
					if (nLightIndex < EmLight::eMaxPointLightCount)
					{
						m_pointLightData[nLightIndex].Set(pLight->GetColor(), pLight->GetPosition(), pLight->GetIntensity(), pLight->GetAmbientIntensity(), pLight->GetReflectionIntensity());
						++nLightIndex;
					}
				}
			});
			PERF_TRACER_ENDEVENT();

			PERF_TRACER_BEGINEVENT("LightManager::UpdateLightBuffer", "Spot");
			std::for_each(m_vecSpotLights.begin(), m_vecSpotLights.end(), [&](ISpotLight* pLight)
			{
				// 이걸 보거든, 방향 및 거리 계산을 계산해서 프러스텀 안에 들어갈때 버퍼에 추가하도록 바꾸시오.
				if (true)
				{
					uint32_t& nLightIndex = m_nLightCountInView[EmLight::eSpot];
					if (nLightIndex < EmLight::eMaxSpotLightCount)
					{
						m_spotLightData[nLightIndex].Set(pLight->GetColor(), pLight->GetPosition(), pLight->GetDirection(), pLight->GetIntensity(), pLight->GetAmbientIntensity(), pLight->GetReflectionIntensity(), pLight->GetAngle());
						++nLightIndex;
					}
				}
			});
			PERF_TRACER_ENDEVENT();

			PERF_TRACER_BEGINEVENT("LightManager::UpdateLightBuffer", "UpdateSubresource");
			m_pLightBuffers[EmLight::eDirectional]->UpdateSubresource(ThreadType::eImmediate, 0, m_directionalLightData.data(), m_nLightCountInView[EmLight::eDirectional]);
			m_pLightBuffers[EmLight::ePoint]->UpdateSubresource(ThreadType::eImmediate, 0, m_pointLightData.data(), m_nLightCountInView[EmLight::ePoint]);
			m_pLightBuffers[EmLight::eSpot]->UpdateSubresource(ThreadType::eImmediate, 0, m_spotLightData.data(), m_nLightCountInView[EmLight::eSpot]);
			PERF_TRACER_ENDEVENT();
		}

		LightManager::LightManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		LightManager::~LightManager()
		{
		}

		void LightManager::Update(float fElapsedTime)
		{
			m_pImpl->Update(fElapsedTime);
		}

		void LightManager::Synchronize()
		{
			m_pImpl->Synchronize();
		}

		bool LightManager::AddLight(ILight* pLight)
		{
			return m_pImpl->AddLight(pLight);
		}

		void LightManager::Remove(ILight* pLight)
		{
			m_pImpl->Remove(pLight);
		}

		void LightManager::Remove(EmLight::Type emType, size_t nIndex)
		{
			m_pImpl->Remove(emType, nIndex);
		}

		void LightManager::RemoveAll()
		{
			m_pImpl->RemoveAll();
		}

		ILight* LightManager::GetLight(EmLight::Type emType, size_t nIndex)
		{
			return m_pImpl->GetLight(emType, nIndex);
		}

		size_t LightManager::GetLightCount(EmLight::Type emType)
		{
			return m_pImpl->GetLightCount(emType);
		}

		IStructuredBuffer* LightManager::GetLightBuffer(EmLight::Type emType)
		{
			return m_pImpl->GetLightBuffer(emType);
		}

		uint32_t LightManager::GetLightCountInView(EmLight::Type emType)
		{
			return m_pImpl->GetLightCountInView(emType);
		}
	}
}