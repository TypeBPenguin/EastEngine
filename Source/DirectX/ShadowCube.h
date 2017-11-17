#pragma once

#include "Light.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IDepthStencil;
		class ITexture;

		class ShadowCubeMap : public IShadowCubeMap
		{
		public:
			ShadowCubeMap();
			virtual ~ShadowCubeMap();

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
			virtual Math::Int2 GetPCFBlurSize() const override { return Math::Int2(m_nPCFBlurSize / -2, m_nPCFBlurSize / 2 + 1); }
			virtual void SetPCFBlurSize(int nPCFBlurSize) override { m_nPCFBlurSize = nPCFBlurSize; }

			virtual Math::Vector2 GetTexelOffset() override { return { 1.f / static_cast<float>(m_copyConfig.nBufferSize), 1.f / static_cast<float>(m_copyConfig.nBufferSize) }; }

			virtual ISamplerState* GetSamplerPCF() const override { return m_pSamplerShadowPCF; }
			virtual ISamplerState* GetSamplerPoint() const override { return m_pSamplerShadowPoint; }
			virtual IRasterizerState* GetRasterizerShadow() const override { return m_pRasterizerShadow; }

		public:
			virtual const Math::Matrix& GetViewMatrix(EmDirection emDirection) const override;
			virtual const Math::Matrix& GetProjectionMatrix(EmDirection emDirection) const override;
			virtual const Math::Viewport& GetViewport() const override;
			virtual const Collision::Frustum& GetFrustum(EmDirection emDirection) const override;

		private:
			void RefreshShadowResource();

		private:
			std::array<Math::Matrix, DirectionCount> m_matViews;
			std::array<Math::Matrix, DirectionCount> m_matProjections;

			Math::Viewport m_viewport;

			std::array<Collision::Frustum, DirectionCount> m_frustum;

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