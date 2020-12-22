#pragma once

namespace est
{
	namespace graphics
	{
		class ISpotLight;

		struct ShadowConfig
		{
			uint32_t resolution{ 1024 };
			float depthBias{ 0.001f };
			uint32_t pcfBlurSize{ 5 };
		};

		class Shadow
		{
		public:
			Shadow();
			virtual ~Shadow();

		public:
			void Update(const ISpotLight* pSpotLight);

		public:
			const ShadowConfig& GetConfig() const { return m_config; }
			void SetConfig(const ShadowConfig& config) { m_config = config; }

			math::float2 GetTexelOffset() const { return { 1.f / static_cast<float>(m_config.resolution), 1.f / static_cast<float>(m_config.resolution) }; }

			const math::Matrix& GetViewMatrix() const { return m_viewMatrix; }
			const math::Matrix& GetProjectionMatrix() const { return m_projectionMatrix; }
			const math::Viewport& GetViewport() const { return m_viewport; }
			const collision::Frustum& GetFrustum() const { return m_frustum; }

		private:
			void Update(const math::float3& position, const math::float3& direction, float intensity, float angle);

		private:
			ShadowConfig m_config;

			float m_calcDepthBias{ 0.001f };
			math::Matrix m_viewMatrix;
			math::Matrix m_projectionMatrix;
			math::Viewport m_viewport;
			collision::Frustum m_frustum;
		};
	}
}