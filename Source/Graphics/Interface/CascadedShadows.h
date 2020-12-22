#pragma once

namespace est
{
	namespace graphics
	{
		class IDirectionalLight;

		struct CascadedShadowsConfig
		{
			enum
			{
				eMaxCascades = 5,
			};

			uint32_t numCascades{ 3 };
			uint32_t resolution{ 2048 };
			uint32_t pcfBlurSize{ 7 };
			float cascadeDistance{ 128.f };
			float depthBias{ 0.000001f };
		};

		struct CascadedShadowData
		{
			math::int2 pcfBlurSize;
			math::float2 texelOffset;

			uint32_t numCascades{ 0 };
			float depthBias{ 0.000001f };
			math::float2 padding;

			math::float4 viewPos[CascadedShadowsConfig::eMaxCascades];
			math::float4 splitDepths[CascadedShadowsConfig::eMaxCascades];
			math::Matrix cascadeViewProjectionMatrix[CascadedShadowsConfig::eMaxCascades];
			math::Matrix cascadeViewLinearProjectionMatrix[CascadedShadowsConfig::eMaxCascades];
		};

		class CascadedShadows
		{
		public:
			CascadedShadows();
			~CascadedShadows();

		public:
			void Update(const IDirectionalLight* pDirectionLight);

		public:
			const CascadedShadowsConfig& GetConfig() const { return m_config; }
			void SetConfig(const CascadedShadowsConfig& config) { m_config = config; }

			const CascadedShadowData& GetData() const { return m_data; }

			math::float2 GetTexelOffset() const { return { 1.f / static_cast<float>((m_config.resolution * m_config.numCascades)), 1.f / static_cast<float>(m_config.resolution) }; }

			const math::Matrix& GetViewMatrix(uint32_t index) const { return m_matViews[GetSafeIndex(index)]; }
			const math::Matrix& GetProjectionMatrix(uint32_t index) const { return m_matProjections[GetSafeIndex(index)]; }
			const math::Viewport& GetViewport(uint32_t index) const { return m_viewportCascade[GetSafeIndex(index)]; }
			const collision::Frustum& GetFrustum(uint32_t index) const { return m_frustums[GetSafeIndex(index)]; }

			const math::float4& GetSplitDepths(uint32_t index) const { return m_splitDepths[GetSafeIndex(index)]; }

		private:
			uint32_t GetSafeIndex(uint32_t index) const
			{
				if (index >= CascadedShadowsConfig::eMaxCascades)
					return CascadedShadowsConfig::eMaxCascades - 1;

				return index;
			}

		private:
			CascadedShadowsConfig m_config;
			CascadedShadowData m_data;

			std::array<math::Matrix, CascadedShadowsConfig::eMaxCascades> m_matViews;
			std::array<math::Matrix, CascadedShadowsConfig::eMaxCascades> m_matProjections;
			std::array<math::Viewport, CascadedShadowsConfig::eMaxCascades> m_viewportCascade;
			std::array<collision::Frustum, CascadedShadowsConfig::eMaxCascades> m_frustums;
			std::array<math::float4, CascadedShadowsConfig::eMaxCascades> m_splitDepths;
		};
	}
}