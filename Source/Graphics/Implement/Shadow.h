#pragma once

#include "Light.h"

namespace est
{
	namespace graphics
	{
		class IDepthStencil;
		class ITexture;

		class ShadowMap : public IShadowMap
		{
		public:
			ShadowMap();
			virtual ~ShadowMap();

			bool Init(ISpotLight* pLight, const ShadowConfig* pShadowConfig);
			void Release();

		public:
			virtual void Update() override;

		public:
			virtual int GetBufferSize() const override { return m_copyConfig.nBufferSize; }

			virtual float GetDepthBias() const override { return m_fCalcDepthBias; }
			virtual void SetDepthBias(float fDepthBias) override { m_fDepthBias = fDepthBias; }

			virtual IDepthStencil* GetDepthStencil() const override { return m_pShadowMap; }
			virtual const std::shared_ptr<ITexture>& GetShadowMap() const override;

		public:
			virtual math::int2 GetPCFBlurSize() const override { return math::int2(m_nPCFBlurSize / -2, m_nPCFBlurSize / 2 + 1); }
			virtual void SetPCFBlurSize(int nPCFBlurSize) override { m_nPCFBlurSize = nPCFBlurSize; }

			virtual math::float2 GetTexelOffset() override { return { 1.f / static_cast<float>(m_copyConfig.nBufferSize), 1.f / static_cast<float>(m_copyConfig.nBufferSize) }; }

			virtual const math::Matrix& GetViewMatrix() const override { return m_viewMatrix; }
			virtual const math::Matrix& GetProjectionMatrix() const override { return m_projectionMatrix; }
			virtual const math::Viewport& GetViewport() const override { return m_viewport; }
			virtual const collision::Frustum& GetFrustum() const override { return m_frustum; }

			virtual ISamplerState* GetSamplerPCF() const override { return m_pSamplerShadowPCF; }
			virtual ISamplerState* GetSamplerPoint() const override { return m_pSamplerShadowPoint; }
			virtual IRasterizerState* GetRasterizerShadow() const override { return m_pRasterizerShadow; }

		private:
			void RefreshShadowResource();

		private:
			math::Matrix m_viewMatrix;
			math::Matrix m_projectionMatrix;

			math::Viewport m_viewport;

			collision::Frustum m_frustum;

			int m_nPCFBlurSize;
			float m_fDepthBias;
			float m_fCalcDepthBias;

			ShadowConfig m_copyConfig;
			const ShadowConfig* m_pShadowConfig;

			ISpotLight* m_pLight;

			IDepthStencil* m_pShadowMap;

			IRasterizerState* m_pRasterizerShadow;

			ISamplerState* m_pSamplerShadowPCF;
			ISamplerState* m_pSamplerShadowPoint;
		};
	}
}