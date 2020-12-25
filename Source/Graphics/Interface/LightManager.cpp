#include "stdafx.h"
#include "LightManager.h"

#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

#include "ParallelUpdateRender.h"

namespace est
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
			DirectionalLightPtr CreateDirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData);
			PointLightPtr CreatePointLight(const string::StringID& name, bool isEnableShadow, const PointLightData& lightData);
			SpotLightPtr CreateSpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData);

			void Remove(const LightPtr& pLight);
			void Remove(ILight::Type emType, size_t index);
			void RemoveAll();
			LightPtr GetLight(ILight::Type emType, size_t index) const;
			size_t GetLightCount(ILight::Type emType) const;

		public:
			uint32_t GetLightCountInView(ILight::Type emType) const;
			void GetDirectionalLightRenderData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const;
			void GetPointLightRenderData(const PointLightData** ppPointLightData, uint32_t* pSize) const;
			void GetSpotLightRenderData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const;

			uint32_t GetShadowCount(ILight::Type type) const { return m_shadowCount[type]; }
			void IncreaseShadowCount(ILight::Type type) { ++m_shadowCount[type]; }
			void DecreaseShadowCount(ILight::Type type) { --m_shadowCount[type]; }

		private:
			void UpdateLightBuffer();

		private:
			std::array<uint32_t, ILight::eCount> m_lightCountInView{ 0 };
			std::array<uint32_t, ILight::eCount> m_shadowCount{ 0 };

			std::vector<std::shared_ptr<DirectionalLight>> m_directionalLights;
			std::vector<std::shared_ptr<SpotLight>> m_spotLights;
			std::vector<std::shared_ptr<PointLight>> m_pointLights;

			std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> m_directionalLightData[2];
			std::array<PointLightData, ILight::eMaxPointLightCount> m_pointLightData[2];
			std::array<SpotLightData, ILight::eMaxSpotLightCount> m_spotLightData[2];
		};

		LightManager::Impl::Impl()
		{
		}

		LightManager::Impl::~Impl()
		{
		}

		void LightManager::Impl::Update(float elapsedTime)
		{
			TRACER_EVENT(__FUNCTIONW__);

			if (m_directionalLights.empty() == false)
			{
				TRACER_EVENT(L"Directional");
				m_directionalLights.erase(std::remove_if(m_directionalLights.begin(), m_directionalLights.end(), [elapsedTime](const std::shared_ptr<DirectionalLight>& pDirectionalLight)
					{
						if (pDirectionalLight.use_count() <= 1)
							return true;

						pDirectionalLight->Update(elapsedTime);
						return false;
					}), m_directionalLights.end());
			}

			if (m_spotLights.empty() == false)
			{
				TRACER_EVENT(L"Spot");
				m_spotLights.erase(std::remove_if(m_spotLights.begin(), m_spotLights.end(), [elapsedTime](const std::shared_ptr<SpotLight>& pSpotLight)
					{
						if (pSpotLight.use_count() <= 1)
							return true;

						pSpotLight->Update(elapsedTime);
						return false;
					}), m_spotLights.end());
			}

			if (m_pointLights.empty() == false)
			{
				TRACER_EVENT(L"Point");
				m_pointLights.erase(std::remove_if(m_pointLights.begin(), m_pointLights.end(), [elapsedTime](const std::shared_ptr<PointLight>& pPointLight)
					{
						if (pPointLight.use_count() <= 1)
							return true;

						pPointLight->Update(elapsedTime);
						return false;
					}), m_pointLights.end());
			}

			UpdateLightBuffer();
		}

		DirectionalLightPtr LightManager::Impl::CreateDirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData)
		{
			if (m_directionalLights.size() >= ILight::eMaxDirectionalLightCount)
			{
				LOG_WARNING(L"failed to create directional light : Too many directional lights[%d]", ILight::eMaxDirectionalLightCount);
				return nullptr;
			}

			return m_directionalLights.emplace_back(std::make_shared<DirectionalLight>(name, isEnableShadow, lightData));
		}

		PointLightPtr LightManager::Impl::CreatePointLight(const string::StringID& name, bool isEnableShadow, const PointLightData& lightData)
		{
			if (m_pointLights.size() >= ILight::eMaxPointLightCount)
			{
				LOG_WARNING(L"failed to create point light : Too many point lights[%d]", ILight::eMaxPointLightCount);
				return nullptr;
			}

			return m_pointLights.emplace_back(std::make_shared<PointLight>(name, isEnableShadow, lightData));
		}

		SpotLightPtr LightManager::Impl::CreateSpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData)
		{
			if (m_spotLights.size() >= ILight::eMaxSpotLightCount)
			{
				LOG_WARNING(L"failed to create spot light : Too many spot lights[%d]", ILight::eMaxSpotLightCount);
				return nullptr;
			}

			return m_spotLights.emplace_back(std::make_shared<SpotLight>(name, isEnableShadow, lightData));
		}

		void LightManager::Impl::Remove(const LightPtr& pLight)
		{
			switch (pLight->GetType())
			{
			case ILight::Type::eDirectional:
			{
				auto iter = std::find_if(m_directionalLights.begin(), m_directionalLights.end(), [pLight](const std::shared_ptr<DirectionalLight>& pDirectionalLight)
					{
						return pDirectionalLight == pLight;
					});

				if (iter != m_directionalLights.end())
				{
					m_directionalLights.erase(iter);
				}
			}
			break;
			case ILight::Type::ePoint:
			{
				auto iter = std::find_if(m_pointLights.begin(), m_pointLights.end(), [pLight](const std::shared_ptr<PointLight>& pPointLight)
					{
						return pPointLight == pLight;
					});

				if (iter != m_pointLights.end())
				{
					m_pointLights.erase(iter);
				}
			}
			break;
			case ILight::Type::eSpot:
			{
				auto iter = std::find_if(m_spotLights.begin(), m_spotLights.end(), [pLight](const std::shared_ptr<SpotLight>& pSpotLight)
					{
						return pSpotLight == pLight;
					});

				if (iter != m_spotLights.end())
				{
					m_spotLights.erase(iter);
				}
			}
			break;
			default:
				break;
			}
		}

		void LightManager::Impl::Remove(ILight::Type emType, size_t index)
		{
			switch (emType)
			{
			case ILight::Type::eDirectional:
			{
				if (index >= m_directionalLights.size())
					return;

				auto iter = m_directionalLights.begin();
				std::advance(iter, index);

				if (iter != m_directionalLights.end())
				{
					m_directionalLights.erase(iter);
				}
			}
			break;
			case ILight::Type::ePoint:
			{
				if (index >= m_pointLights.size())
					return;

				auto iter = m_pointLights.begin();
				std::advance(iter, index);

				if (iter != m_pointLights.end())
				{
					m_pointLights.erase(iter);
				}
			}
			break;
			case ILight::Type::eSpot:
			{
				if (index >= m_spotLights.size())
					return;

				auto iter = m_spotLights.begin();
				std::advance(iter, index);

				if (iter != m_spotLights.end())
				{
					m_spotLights.erase(iter);
				}
			}
			break;
			default:
				break;
			}
		}

		void LightManager::Impl::RemoveAll()
		{
			m_directionalLights.clear();
			m_spotLights.clear();
			m_pointLights.clear();
		}

		LightPtr LightManager::Impl::GetLight(ILight::Type emType, size_t index) const
		{
			switch (emType)
			{
			case ILight::Type::eDirectional:
			{
				if (index >= m_directionalLights.size())
					return nullptr;

				return m_directionalLights[index];
			}
			break;
			case ILight::Type::ePoint:
			{
				if (index >= m_pointLights.size())
					return nullptr;

				return m_pointLights[index];
			}
			break;
			case ILight::Type::eSpot:
			{
				if (index >= m_spotLights.size())
					return nullptr;

				return m_spotLights[index];
			}
			break;
			default:
				return nullptr;
			}
		}

		size_t LightManager::Impl::GetLightCount(ILight::Type emType) const
		{
			switch (emType)
			{
			case ILight::Type::eDirectional:
				return m_directionalLights.size();
			case ILight::Type::ePoint:
				return m_pointLights.size();
			case ILight::Type::eSpot:
				return m_spotLights.size();
			default:
				return 0;
			}
		}

		uint32_t LightManager::Impl::GetLightCountInView(ILight::Type emType) const
		{
			return m_lightCountInView[emType];
		}

		void LightManager::Impl::GetDirectionalLightRenderData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const
		{
			if (ppDirectionalLightData != nullptr)
			{
				*ppDirectionalLightData = m_directionalLightData[RenderThread()].data();
			}

			if (pSize != nullptr)
			{
				*pSize = static_cast<uint32_t>(m_directionalLights.size());
			}
		}

		void LightManager::Impl::GetPointLightRenderData(const PointLightData** ppPointLightData, uint32_t* pSize) const
		{
			if (ppPointLightData != nullptr)
			{
				*ppPointLightData = m_pointLightData[RenderThread()].data();
			}

			if (pSize != nullptr)
			{
				*pSize = static_cast<uint32_t>(m_pointLights.size());
			}
		}

		void LightManager::Impl::GetSpotLightRenderData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const
		{
			if (ppSpotLightData != nullptr)
			{
				*ppSpotLightData = m_spotLightData[RenderThread()].data();
			}

			if (pSize != nullptr)
			{
				*pSize = static_cast<uint32_t>(m_spotLights.size());
			}
		}

		void LightManager::Impl::UpdateLightBuffer()
		{
			TRACER_EVENT(__FUNCTIONW__);

			m_lightCountInView.fill(0);

			{
				TRACER_EVENT(L"Directional");
				std::for_each(m_directionalLights.begin(), m_directionalLights.end(), [&](std::shared_ptr<DirectionalLight>& pLight)
					{
						uint32_t& lightIndex = m_lightCountInView[ILight::eDirectional];
						if (lightIndex < ILight::eMaxDirectionalLightCount)
						{
							m_directionalLightData[UpdateThread()][lightIndex] = pLight->GetData();
							++lightIndex;
						}
					});
			}

			{
				TRACER_EVENT(L"Point");
				std::for_each(m_pointLights.begin(), m_pointLights.end(), [&](std::shared_ptr<PointLight>& pLight)
					{
						collision::Sphere sphere(pLight->GetPosition(), math::PI * pLight->GetIntensity());

						// 이걸 보거든, 방향 및 거리 계산을 계산해서 프러스텀 안에 들어갈때 버퍼에 추가하도록 바꾸시오.
						//if (frustum.Contains(sphere) == collision::EmContainment::eContains)
						{
							uint32_t& lightIndex = m_lightCountInView[ILight::ePoint];
							if (lightIndex < ILight::eMaxPointLightCount)
							{
								m_pointLightData[UpdateThread()][lightIndex] = pLight->GetData();
								++lightIndex;
							}
						}
					});
			}

			{
				TRACER_EVENT(L"Spot");
				std::for_each(m_spotLights.begin(), m_spotLights.end(), [&](std::shared_ptr<SpotLight>& pLight)
					{
						// 이걸 보거든, 방향 및 거리 계산을 계산해서 프러스텀 안에 들어갈때 버퍼에 추가하도록 바꾸시오.
						if (true)
						{
							uint32_t& lightIndex = m_lightCountInView[ILight::eSpot];
							if (lightIndex < ILight::eMaxSpotLightCount)
							{
								m_spotLightData[UpdateThread()][lightIndex] = pLight->GetData();
								++lightIndex;
							}
						}
					});
			}
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

		DirectionalLightPtr LightManager::CreateDirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData)
		{
			return m_pImpl->CreateDirectionalLight(name, isEnableShadow, lightData);
		}

		PointLightPtr LightManager::CreatePointLight(const string::StringID& name, bool isEnableShadow, const PointLightData& lightData)
		{
			return m_pImpl->CreatePointLight(name, isEnableShadow, lightData);
		}

		SpotLightPtr LightManager::CreateSpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData)
		{
			return m_pImpl->CreateSpotLight(name, isEnableShadow, lightData);
		}

		void LightManager::Remove(const LightPtr& pLight)
		{
			m_pImpl->Remove(pLight);
		}

		void LightManager::Remove(ILight::Type emType, size_t index)
		{
			m_pImpl->Remove(emType, index);
		}

		void LightManager::RemoveAll()
		{
			m_pImpl->RemoveAll();
		}

		LightPtr LightManager::GetLight(ILight::Type emType, size_t index) const
		{
			return m_pImpl->GetLight(emType, index);
		}

		size_t LightManager::GetLightCount(ILight::Type emType) const
		{
			return m_pImpl->GetLightCount(emType);
		}

		uint32_t LightManager::GetLightCountInView(ILight::Type emType) const
		{
			return m_pImpl->GetLightCountInView(emType);
		}

		void LightManager::GetDirectionalLightRenderData(const DirectionalLightData** ppDirectionalLightData, uint32_t* pSize) const
		{
			m_pImpl->GetDirectionalLightRenderData(ppDirectionalLightData, pSize);
		}

		void LightManager::GetPointLightRenderData(const PointLightData** ppPointLightData, uint32_t* pSize) const
		{
			m_pImpl->GetPointLightRenderData(ppPointLightData, pSize);
		}

		void LightManager::GetSpotLightRenderData(const SpotLightData** ppSpotLightData, uint32_t* pSize) const
		{
			m_pImpl->GetSpotLightRenderData(ppSpotLightData, pSize);
		}

		uint32_t LightManager::GetShadowCount(ILight::Type type) const
		{
			return m_pImpl->GetShadowCount(type);
		}

		void LightManager::IncreaseShadowCount(ILight::Type type)
		{
			m_pImpl->IncreaseShadowCount(type);
		}

		void LightManager::DecreaseShadowCount(ILight::Type type)
		{
			m_pImpl->DecreaseShadowCount(type);
		}
	}
}