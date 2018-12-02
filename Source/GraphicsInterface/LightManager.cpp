#include "stdafx.h"
#include "LightManager.h"

namespace eastengine
{
	namespace graphics
	{
		class LightManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update(float elapsedTime);

		public:
			bool AddLight(ILight* pLight);
			void Remove(ILight* pLight);
			void Remove(ILight::Type emType, size_t nIndex);
			void RemoveAll();
			ILight* GetLight(ILight::Type emType, size_t nIndex);
			size_t GetLightCount(ILight::Type emType);

		public:
			uint32_t GetLightCountInView(ILight::Type emType);
			void GetDirectionalLightData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const;
			void GetPointLightData(const PointLightData** ppPointLightData, uint32_t* pSize) const;
			void GetSpotLightData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const;

		private:
			void UpdateLightBuffer();

		private:
			std::array<uint32_t, ILight::eCount> m_nLightCountInView;

			std::vector<IDirectionalLight*> m_vecDirectionalLights;
			std::vector<ISpotLight*> m_vecSpotLights;
			std::vector<IPointLight*> m_vecPointLights;

			std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> m_directionalLightData;
			std::array<PointLightData, ILight::eMaxPointLightCount> m_pointLightData;
			std::array<SpotLightData, ILight::eMaxSpotLightCount> m_spotLightData;
		};

		LightManager::Impl::Impl()
		{
			//m_pLightBuffers[ILight::eDirectional] = IStructuredBuffer::Create(m_directionalLightData.data(), m_directionalLightData.size(), sizeof(DirectionalLightData));
			//m_pLightBuffers[ILight::ePoint] = IStructuredBuffer::Create(m_pointLightData.data(), m_pointLightData.size(), sizeof(PointLightData));
			//m_pLightBuffers[ILight::eSpot] = IStructuredBuffer::Create(m_spotLightData.data(), m_spotLightData.size(), sizeof(SpotLightData));

			m_nLightCountInView.fill(0u);
		}

		LightManager::Impl::~Impl()
		{
			RemoveAll();

			//std::for_each(m_pLightBuffers.begin(), m_pLightBuffers.end(), DeleteSTLObject());
			//m_pLightBuffers.fill(nullptr);
		}

		void LightManager::Impl::Update(float elapsedTime)
		{
			TRACER_EVENT("LightManager::Update");

			TRACER_BEGINEVENT("Directional");
			std::for_each(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), [elapsedTime](IDirectionalLight* pLight)
			{
				pLight->Update(elapsedTime);
			});
			TRACER_ENDEVENT();

			TRACER_BEGINEVENT("Spot");
			std::for_each(m_vecSpotLights.begin(), m_vecSpotLights.end(), [elapsedTime](ISpotLight* pLight)
			{
				pLight->Update(elapsedTime);
			});
			TRACER_ENDEVENT();

			TRACER_BEGINEVENT("Point");
			std::for_each(m_vecPointLights.begin(), m_vecPointLights.end(), [elapsedTime](IPointLight* pLight)
			{
				pLight->Update(elapsedTime);
			});
			TRACER_ENDEVENT();

			UpdateLightBuffer();
		}

		bool LightManager::Impl::AddLight(ILight* pLight)
		{
			switch (pLight->GetType())
			{
			case ILight::Type::eDirectional:
			{
				auto iter = std::find(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), pLight);
				if (iter != m_vecDirectionalLights.end())
					return false;

				m_vecDirectionalLights.emplace_back(static_cast<IDirectionalLight*>(pLight));
			}
			break;
			case ILight::Type::ePoint:
			{
				auto iter = std::find(m_vecPointLights.begin(), m_vecPointLights.end(), pLight);
				if (iter != m_vecPointLights.end())
					return false;

				m_vecPointLights.emplace_back(static_cast<IPointLight*>(pLight));
			}
			break;
			case ILight::Type::eSpot:
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
			case ILight::Type::eDirectional:
			{
				auto iter = std::find(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), pLight);
				if (iter != m_vecDirectionalLights.end())
				{
					m_vecDirectionalLights.erase(iter);
				}
			}
			break;
			case ILight::Type::ePoint:
			{
				auto iter = std::find(m_vecPointLights.begin(), m_vecPointLights.end(), pLight);
				if (iter != m_vecPointLights.end())
				{
					m_vecPointLights.erase(iter);
				}
			}
			break;
			case ILight::Type::eSpot:
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

		void LightManager::Impl::Remove(ILight::Type emType, size_t nIndex)
		{
			switch (emType)
			{
			case ILight::Type::eDirectional:
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
			case ILight::Type::ePoint:
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
			case ILight::Type::eSpot:
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

		ILight* LightManager::Impl::GetLight(ILight::Type emType, size_t nIndex)
		{
			switch (emType)
			{
			case ILight::Type::eDirectional:
			{
				if (nIndex >= m_vecDirectionalLights.size())
					return nullptr;

				return m_vecDirectionalLights[nIndex];
			}
			break;
			case ILight::Type::ePoint:
			{
				if (nIndex >= m_vecPointLights.size())
					return nullptr;

				return m_vecPointLights[nIndex];
			}
			break;
			case ILight::Type::eSpot:
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

		size_t LightManager::Impl::GetLightCount(ILight::Type emType)
		{
			switch (emType)
			{
			case ILight::Type::eDirectional:
				return m_vecDirectionalLights.size();
			case ILight::Type::ePoint:
				return m_vecPointLights.size();
			case ILight::Type::eSpot:
				return m_vecSpotLights.size();
			default:
				return 0;
			}
		}

		uint32_t LightManager::Impl::GetLightCountInView(ILight::Type emType)
		{
			return m_nLightCountInView[emType];
		}

		void LightManager::Impl::GetDirectionalLightData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const
		{
			if (ppDirectionalLightData != nullptr)
			{
				*ppDirectionalLightData = m_directionalLightData.data();
			}

			if (pSize != nullptr)
			{
				*pSize = static_cast<uint32_t>(m_vecDirectionalLights.size());
			}
		}

		void LightManager::Impl::GetPointLightData(const PointLightData** ppPointLightData, uint32_t* pSize) const
		{
			if (ppPointLightData != nullptr)
			{
				*ppPointLightData = m_pointLightData.data();
			}

			if (pSize != nullptr)
			{
				*pSize = static_cast<uint32_t>(m_vecPointLights.size());
			}
		}

		void LightManager::Impl::GetSpotLightData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const
		{
			if (ppSpotLightData != nullptr)
			{
				*ppSpotLightData = m_spotLightData.data();
			}

			if (pSize != nullptr)
			{
				*pSize = static_cast<uint32_t>(m_vecSpotLights.size());
			}
		}

		void LightManager::Impl::UpdateLightBuffer()
		{
			m_nLightCountInView.fill(0);

			TRACER_BEGINEVENT("UpdateLightBuffer_Directional");
			std::for_each(m_vecDirectionalLights.begin(), m_vecDirectionalLights.end(), [&](IDirectionalLight* pLight)
			{
				uint32_t& nLightIndex = m_nLightCountInView[ILight::eDirectional];
				if (nLightIndex < ILight::eMaxDirectionalLightCount)
				{
					m_directionalLightData[nLightIndex].Set(pLight->GetColor(), pLight->GetDirection(), pLight->GetIntensity(), pLight->GetAmbientIntensity(), pLight->GetReflectionIntensity());
					++nLightIndex;
				}
			});
			TRACER_ENDEVENT();

			TRACER_BEGINEVENT("UpdateLightBuffer_Point");
			std::for_each(m_vecPointLights.begin(), m_vecPointLights.end(), [&](IPointLight* pLight)
			{
				Collision::Sphere sphere(pLight->GetPosition(), math::PI * pLight->GetIntensity());

				// 이걸 보거든, 방향 및 거리 계산을 계산해서 프러스텀 안에 들어갈때 버퍼에 추가하도록 바꾸시오.
				//if (frustum.Contains(sphere) == Collision::EmContainment::eContains)
				{
					uint32_t& nLightIndex = m_nLightCountInView[ILight::ePoint];
					if (nLightIndex < ILight::eMaxPointLightCount)
					{
						m_pointLightData[nLightIndex].Set(pLight->GetColor(), pLight->GetPosition(), pLight->GetIntensity(), pLight->GetAmbientIntensity(), pLight->GetReflectionIntensity());
						++nLightIndex;
					}
				}
			});
			TRACER_ENDEVENT();

			TRACER_BEGINEVENT("UpdateLightBuffer_Spot");
			std::for_each(m_vecSpotLights.begin(), m_vecSpotLights.end(), [&](ISpotLight* pLight)
			{
				// 이걸 보거든, 방향 및 거리 계산을 계산해서 프러스텀 안에 들어갈때 버퍼에 추가하도록 바꾸시오.
				if (true)
				{
					uint32_t& nLightIndex = m_nLightCountInView[ILight::eSpot];
					if (nLightIndex < ILight::eMaxSpotLightCount)
					{
						m_spotLightData[nLightIndex].Set(pLight->GetColor(), pLight->GetPosition(), pLight->GetDirection(), pLight->GetIntensity(), pLight->GetAmbientIntensity(), pLight->GetReflectionIntensity(), pLight->GetAngle());
						++nLightIndex;
					}
				}
			});
			TRACER_ENDEVENT();

			TRACER_BEGINEVENT("UpdateLightBuffer_UpdateSubresource");
			//m_pLightBuffers[ILight::eDirectional]->UpdateSubresource(ThreadType::eImmediate, 0, m_directionalLightData.data(), m_nLightCountInView[ILight::eDirectional]);
			//m_pLightBuffers[ILight::ePoint]->UpdateSubresource(ThreadType::eImmediate, 0, m_pointLightData.data(), m_nLightCountInView[ILight::ePoint]);
			//m_pLightBuffers[ILight::eSpot]->UpdateSubresource(ThreadType::eImmediate, 0, m_spotLightData.data(), m_nLightCountInView[ILight::eSpot]);
			TRACER_ENDEVENT();
		}

		LightManager::LightManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		LightManager::~LightManager()
		{
		}

		void LightManager::Update(float elapsedTime)
		{
			m_pImpl->Update(elapsedTime);
		}

		bool LightManager::AddLight(ILight* pLight)
		{
			return m_pImpl->AddLight(pLight);
		}

		void LightManager::Remove(ILight* pLight)
		{
			m_pImpl->Remove(pLight);
		}

		void LightManager::Remove(ILight::Type emType, size_t nIndex)
		{
			m_pImpl->Remove(emType, nIndex);
		}

		void LightManager::RemoveAll()
		{
			m_pImpl->RemoveAll();
		}

		ILight* LightManager::GetLight(ILight::Type emType, size_t nIndex)
		{
			return m_pImpl->GetLight(emType, nIndex);
		}

		size_t LightManager::GetLightCount(ILight::Type emType)
		{
			return m_pImpl->GetLightCount(emType);
		}

		uint32_t LightManager::GetLightCountInView(ILight::Type emType)
		{
			return m_pImpl->GetLightCountInView(emType);
		}

		void LightManager::GetDirectionalLightData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const
		{
			m_pImpl->GetDirectionalLightData(ppDirectionalLightData, pSize);
		}

		void LightManager::GetPointLightData(const PointLightData** ppPointLightData, uint32_t* pSize) const
		{
			m_pImpl->GetPointLightData(ppPointLightData, pSize);
		}

		void LightManager::GetSpotLightData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const
		{
			m_pImpl->GetSpotLightData(ppSpotLightData, pSize);
		}
	}
}