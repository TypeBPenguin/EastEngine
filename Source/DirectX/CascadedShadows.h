#pragma once

#include "Light.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IDepthStencil;
		class ITexture;
		class IRasterizerState;
		class ISamplerState;

		class CascadedShadows : public ICascadedShadows
		{
		public:
			CascadedShadows();
			virtual ~CascadedShadows();

			bool Init(IDirectionalLight* pLight, const CascadedShadowsConfig* pConfig);
			void Release();

		public:
			virtual void Update() override;

		public:
			virtual uint32_t GetCascadeLevel() const override { return m_copyConfig.nLevel; }
			virtual int GetBufferSize() const { return m_copyConfig.nBufferSize; };

			virtual float GetDepthBias() const override { return m_fDepthBias; }
			virtual void SetDepthBias(float fDepthBias) override { m_fDepthBias = fDepthBias; }

			virtual Math::Int2 GetPCFBlurSize() const override { return Math::Int2(m_nPCFBlurSize / -2, m_nPCFBlurSize / 2 + 1); }
			virtual void SetPCFBlurSize(int nPCFBlurSize) override { m_nPCFBlurSize = nPCFBlurSize; }

			virtual Math::Vector2 GetTexelOffset() override { return { 1.f / static_cast<float>((m_copyConfig.nBufferSize * m_copyConfig.nLevel)), 1.f / static_cast<float>(m_copyConfig.nBufferSize) }; }

			virtual const Math::Matrix& GetViewMatrix(uint32_t nIndex) override { return m_matViews[GetSafeIndex(nIndex)]; }
			virtual const Math::Matrix& GetProjectionMatrix(uint32_t nIndex) override { return m_matProjections[GetSafeIndex(nIndex)]; }
			virtual const Math::Viewport& GetViewport(uint32_t nIndex) override { return m_viewportCascade[GetSafeIndex(nIndex)]; }
			virtual const Collision::Frustum& GetFrustum(uint32_t nIndex) override { return m_frustums[GetSafeIndex(nIndex)]; }

			virtual const Math::Vector2& GetSplitDepths(uint32_t nIndex) override { return m_f2SplitDepths[GetSafeIndex(nIndex)]; }

			virtual IDepthStencil* GetDepthStencil() const override { return m_pCascadedShadowMap; }
			virtual const std::shared_ptr<ITexture>& GetShadowMap() const override;

			virtual ISamplerState* GetSamplerPCF() const override { return m_pSamplerShadowPCF; }
			virtual ISamplerState* GetSamplerPoint() const override { return m_pSamplerShadowPoint; }
			virtual IRasterizerState* GetRasterizerShadow() const override { return m_pRasterizerShadow; }

		private:
			void RefreshShadowResource();
			uint32_t GetSafeIndex(uint32_t nIndex)
			{
				if (nIndex >= CascadedShadowsConfig::eMaxLevel)
					return CascadedShadowsConfig::eMaxLevel - 1;

				return nIndex;
			}

		private:
			std::array<Math::Matrix, CascadedShadowsConfig::eMaxLevel> m_matViews;
			std::array<Math::Matrix, CascadedShadowsConfig::eMaxLevel> m_matProjections;
			
			std::array<Math::Viewport, CascadedShadowsConfig::eMaxLevel> m_viewportCascade;

			std::array<Collision::Frustum, CascadedShadowsConfig::eMaxLevel> m_frustums;

			std::array<Math::Vector2, CascadedShadowsConfig::eMaxLevel> m_f2SplitDepths;

			int m_nPCFBlurSize;
			float m_fDepthBias;

			CascadedShadowsConfig m_copyConfig;
			const CascadedShadowsConfig* m_pConfig;

			IDirectionalLight* m_pLight;

			IDepthStencil* m_pCascadedShadowMap;

			IRasterizerState* m_pRasterizerShadow;

			ISamplerState* m_pSamplerShadowPCF;
			ISamplerState* m_pSamplerShadowPoint;
		};
	}
}