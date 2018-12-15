#pragma once

#include "Light.h"

namespace eastengine
{
	namespace graphics
	{
		class IDepthStencil;
		class ITexture;

		class ShadowCubeMap : public IShadowCubeMap
		{
		public:
			ShadowCubeMap();
			virtual ~ShadowCubeMap();

			bool Init(IPointLight* pLight, const ShadowConfig* pShadowConfig);
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

			virtual ISamplerState* GetSamplerPCF() const override { return m_pSamplerShadowPCF; }
			virtual ISamplerState* GetSamplerPoint() const override { return m_pSamplerShadowPoint; }
			virtual IRasterizerState* GetRasterizerShadow() const override { return m_pRasterizerShadow; }

		public:
			virtual const math::Matrix& GetViewMatrix(EmDirection emDirection) const override { return m_matViews[emDirection]; }
			virtual const math::Matrix& GetProjectionMatrix() const override { return m_matProjection; }
			virtual const math::Viewport& GetViewport() const override { return m_viewport; }
			virtual const collision::Frustum& GetFrustum(EmDirection emDirection) const override { return m_frustums[emDirection]; }

			virtual float GetFarPlane() const override { return m_fFarPlane; }

		private:
			void RefreshShadowResource();

		private:
			std::array<math::Matrix, DirectionCount> m_matViews;
			math::Matrix m_matProjection;

			math::Viewport m_viewport;

			std::array<collision::Frustum, DirectionCount> m_frustums;

			int m_nPCFBlurSize;
			float m_fDepthBias;
			float m_fCalcDepthBias;
			float m_fFarPlane;

			ShadowConfig m_copyConfig;
			const ShadowConfig* m_pShadowConfig;

			IPointLight* m_pLight;

			IDepthStencil* m_pShadowMap;

			IRasterizerState* m_pRasterizerShadow;

			ISamplerState* m_pSamplerShadowPCF;
			ISamplerState* m_pSamplerShadowPoint;
		};
	}
}