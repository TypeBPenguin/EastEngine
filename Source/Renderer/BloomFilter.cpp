#include "stdafx.h"
#include "BloomFilter.h"

#include "CommonLib/FileUtil.h"

namespace StrID
{
	RegisterStringID(EffectBloomFilter);

	RegisterStringID(Extract);
	RegisterStringID(ExtractLuminance);
	RegisterStringID(Downsample);
	RegisterStringID(Upsample);
	RegisterStringID(UpsampleLuminance);
	RegisterStringID(Apply);

	RegisterStringID(InverseResolution);
	RegisterStringID(Threshold);
	RegisterStringID(Radius);
	RegisterStringID(Strength);
	RegisterStringID(StreakLength);

	RegisterStringID(ScreenTexture);
}

namespace EastEngine
{
	namespace Graphics
	{
		BloomFilter::BloomFilter()
			: m_isInit(false)
			, m_pEffect(nullptr)
			, m_fRadiusMultiplier(1.f)
			, m_fStreakLength(0.f)
			, m_nDownsamplePasses(5)
		{
			m_fRadius.fill(1.f);
			m_fStrengths.fill(1.f);
		}

		BloomFilter::~BloomFilter()
		{
			Release();
		}

		bool BloomFilter::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("PostProcessing\\BloomFilter\\BloomFilter_D.cso");
#else
			strPath.append("PostProcessing\\BloomFilter\\BloomFilter.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectBloomFilter, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::Extract, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::ExtractLuminance, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::Downsample, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::Upsample, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::UpsampleLuminance, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::Apply, EmVertexFormat::eUnknown);

			return true;
		}

		void BloomFilter::Release()
		{
			if (m_isInit == false)
				return;

			IEffect::Destroy(&m_pEffect);

			m_isInit = false;
		}

		void ApplyEffect(IDeviceContext* pDeviceContext, IEffect* pEffect, IEffectTech* pTech, IRenderTarget* pResult)
		{
			Math::Viewport viewport;
			viewport.width = static_cast<float>(pResult->GetSize().x);
			viewport.height = static_cast<float>(pResult->GetSize().y);
			pDeviceContext->SetViewport(viewport);
			pDeviceContext->SetRenderTargets(&pResult, 1);

			uint32_t nPassCount = pTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pTech->PassApply(p, pDeviceContext);

				pDeviceContext->Draw(4, 0);
			}
		}

		bool BloomFilter::Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pSource)
		{
			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			D3D_PROFILING(pDeviceContext, BloomFilter);

			SetBloomPreset(m_settings.emPreset);

			pDeviceContext->ClearState();

			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			const Math::UInt2& n2Size = pSource->GetSize();
			const Math::UInt2 n2TargetSize(n2Size.x / 2, n2Size.y / 2);
			m_fRadiusMultiplier = static_cast<float>(n2Size.x) / n2TargetSize.x;

			auto Sampling = [&](bool isResult, uint32_t nWidth, uint32_t nHeight, int nPass, const Math::Vector2& f2InverseResolution, IRenderTarget* pSource, const String::StringID& strTechName, IRenderTarget* pResult = nullptr) -> IRenderTarget*
			{
				if (m_nDownsamplePasses > nPass)
				{
					int nPassFactor = static_cast<int>(std::pow(2, nPass));
					if (pResult == nullptr)
					{
						RenderTargetDesc2D desc = pSource->GetDesc2D();
						desc.Width = nWidth / (nPassFactor * (isResult ? 1 : 2));
						desc.Height = nHeight / (nPassFactor * (isResult ? 1 : 2));
						desc.Build();
						pResult = pDevice->GetRenderTarget(desc, false);
					}

					{
						IEffectTech* pTech = m_pEffect->GetTechnique(strTechName);
						if (pTech == nullptr)
						{
							assert(false);
							return false;
						}

						m_pEffect->SetTexture(StrID::ScreenTexture, pSource->GetTexture());
						m_pEffect->SetVector(StrID::InverseResolution, f2InverseResolution * static_cast<float>(nPassFactor));

						m_pEffect->SetFloat(StrID::Strength, m_fStrengths[nPass] * m_settings.fStrengthMultiplier);
						m_pEffect->SetFloat(StrID::Radius, m_fRadius[nPass]);
						m_pEffect->SetFloat(StrID::Threshold, m_settings.fThreshold);
						m_pEffect->SetFloat(StrID::StreakLength, m_fStreakLength);

						ApplyEffect(pDeviceContext, m_pEffect, pTech, pResult);
						ClearEffect(pDeviceContext, pTech);
					}

					return pResult;
				}

				return pSource;
			};

			pDeviceContext->SetBlendState(EmBlendState::eOpacity);

			Math::Vector2 f2InverseResolution(1.f / n2TargetSize.x, 1.f / n2TargetSize.y);
			IRenderTarget* pMip0 = Sampling(true, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pSource, m_settings.isEnableLuminance ? StrID::ExtractLuminance : StrID::Extract);

			IRenderTarget* pMip1 = Sampling(false, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pMip0, StrID::Downsample);
			IRenderTarget* pMip2 = Sampling(false, n2TargetSize.x, n2TargetSize.y, 1, f2InverseResolution, pMip1, StrID::Downsample);
			IRenderTarget* pMip3 = Sampling(false, n2TargetSize.x, n2TargetSize.y, 2, f2InverseResolution, pMip2, StrID::Downsample);
			IRenderTarget* pMip4 = Sampling(false, n2TargetSize.x, n2TargetSize.y, 3, f2InverseResolution, pMip3, StrID::Downsample);
			IRenderTarget* pMip5 = Sampling(false, n2TargetSize.x, n2TargetSize.y, 4, f2InverseResolution, pMip4, StrID::Downsample);

			pDeviceContext->SetBlendState(EmBlendState::eAlphaBlend);

			pMip4 = Sampling(false, n2TargetSize.x, n2TargetSize.y, 4, f2InverseResolution, pMip5, m_settings.isEnableLuminance ? StrID::UpsampleLuminance : StrID::Upsample, pMip4);
			pMip3 = Sampling(false, n2TargetSize.x, n2TargetSize.y, 3, f2InverseResolution, pMip4, m_settings.isEnableLuminance ? StrID::UpsampleLuminance : StrID::Upsample, pMip3);
			pMip2 = Sampling(false, n2TargetSize.x, n2TargetSize.y, 2, f2InverseResolution, pMip3, m_settings.isEnableLuminance ? StrID::UpsampleLuminance : StrID::Upsample, pMip2);
			pMip1 = Sampling(false, n2TargetSize.x, n2TargetSize.y, 1, f2InverseResolution, pMip2, m_settings.isEnableLuminance ? StrID::UpsampleLuminance : StrID::Upsample, pMip1);
			pMip0 = Sampling(false, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pMip1, m_settings.isEnableLuminance ? StrID::UpsampleLuminance : StrID::Upsample, pMip0);

			pDeviceContext->SetBlendState(EmBlendState::eAdditive);

			Sampling(true, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pMip0, StrID::Apply, pSource);

			pDevice->ReleaseRenderTargets(&pMip0, 1, false);
			pDevice->ReleaseRenderTargets(&pMip1, 1, false);
			pDevice->ReleaseRenderTargets(&pMip2, 1, false);
			pDevice->ReleaseRenderTargets(&pMip3, 1, false);
			pDevice->ReleaseRenderTargets(&pMip4, 1, false);
			pDevice->ReleaseRenderTargets(&pMip5, 1, false);

			return true;
		}

		void BloomFilter::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->SetTexture(StrID::ScreenTexture, nullptr);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}

		void BloomFilter::SetBloomPreset(Presets emPreset)
		{
			switch (emPreset)
			{
			case Presets::eWide:
			{
				m_fStrengths[0] = 0.5f;
				m_fStrengths[1] = 1.f;
				m_fStrengths[2] = 2.f;
				m_fStrengths[3] = 1.f;
				m_fStrengths[4] = 2.f;
				m_fRadius[4] = 4.f;
				m_fRadius[3] = 4.f;
				m_fRadius[2] = 2.f;
				m_fRadius[1] = 2.f;
				m_fRadius[0] = 1.f;
				m_fStreakLength = 1.f;
				m_nDownsamplePasses = 5;
			}
			break;
			case Presets::eSuperWide:
			{
				m_fStrengths[0] = 0.9f;
				m_fStrengths[1] = 1.f;
				m_fStrengths[2] = 1.f;
				m_fStrengths[3] = 2.f;
				m_fStrengths[4] = 6.f;
				m_fRadius[4] = 4.f;
				m_fRadius[3] = 2.f;
				m_fRadius[2] = 2.f;
				m_fRadius[1] = 2.f;
				m_fRadius[0] = 2.f;
				m_fStreakLength = 1.f;
				m_nDownsamplePasses = 5;
			}
			break;
			case Presets::eFocussed:
			{
				m_fStrengths[0] = 0.8f;
				m_fStrengths[1] = 1.f;
				m_fStrengths[2] = 1.f;
				m_fStrengths[3] = 1.f;
				m_fStrengths[4] = 2.f;
				m_fRadius[4] = 4.f;
				m_fRadius[3] = 2.f;
				m_fRadius[2] = 2.f;
				m_fRadius[1] = 2.f;
				m_fRadius[0] = 2.f;
				m_fStreakLength = 1.f;
				m_nDownsamplePasses = 5;
			}
			break;
			case Presets::eSmall:
			{
				m_fStrengths[0] = 0.8f;
				m_fStrengths[1] = 1.f;
				m_fStrengths[2] = 1.f;
				m_fStrengths[3] = 1.f;
				m_fStrengths[4] = 1.f;
				m_fRadius[4] = 1.f;
				m_fRadius[3] = 1.f;
				m_fRadius[2] = 1.f;
				m_fRadius[1] = 1.f;
				m_fRadius[0] = 1.f;
				m_fStreakLength = 1.f;
				m_nDownsamplePasses = 5;
			}
			break;
			case Presets::eCheap:
			{
				m_fStrengths[0] = 0.8f;
				m_fStrengths[1] = 2.f;
				m_fRadius[1] = 2.f;
				m_fRadius[0] = 2.f;
				m_fStreakLength = 1.f;
				m_nDownsamplePasses = 2;
			}
			break;
			case Presets::eOne:
			{
				m_fStrengths[0] = 4.f;
				m_fStrengths[1] = 1.f;
				m_fStrengths[2] = 1.f;
				m_fStrengths[3] = 1.f;
				m_fStrengths[4] = 2.f;
				m_fRadius[4] = 1.f;
				m_fRadius[3] = 1.f;
				m_fRadius[2] = 1.f;
				m_fRadius[1] = 1.f;
				m_fRadius[0] = 1.f;
				m_fStreakLength = 1.f;
				m_nDownsamplePasses = 5;
			}
			break;
			}
		}
	}
}