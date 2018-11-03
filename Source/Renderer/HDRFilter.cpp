#include "stdafx.h"
#include "HDRFilter.h"

#include "CommonLib/Timer.h"
#include "CommonLib/FileUtil.h"

namespace StrID
{
	RegisterStringID(EffectHDRFilter);

	RegisterStringID(Threshold);
	RegisterStringID(BloomBlurH);
	RegisterStringID(BloomBlurV);
	RegisterStringID(LuminanceMap);
	RegisterStringID(Composite);
	RegisterStringID(CompositeWithExposure);
	RegisterStringID(Scale);
	RegisterStringID(AdaptLuminance);

	RegisterStringID(BloomThreshold);
	RegisterStringID(BloomMagnitude);
	RegisterStringID(BloomBlurSigma);
	RegisterStringID(Tau);
	RegisterStringID(TimeDelta);
	RegisterStringID(ToneMapTechnique);
	RegisterStringID(Exposure);
	RegisterStringID(KeyValue);
	RegisterStringID(AutoExposure);
	RegisterStringID(WhiteLevel);
	RegisterStringID(ShoulderStrength);
	RegisterStringID(LinearStrength);
	RegisterStringID(LinearAngle);
	RegisterStringID(ToeStrength);
	RegisterStringID(ToeNumerator);
	RegisterStringID(ToeDenominator);
	RegisterStringID(LinearWhite);
	RegisterStringID(LuminanceSaturation);
	RegisterStringID(LumMapMipLevel);
	RegisterStringID(Bias);

	RegisterStringID(InputSize0);
	RegisterStringID(InputSize1);
	RegisterStringID(InputSize2);
	RegisterStringID(InputSize3);
	RegisterStringID(OutputSize);

	RegisterStringID(PointSampler);
	RegisterStringID(LinearSampler);

	RegisterStringID(InputTexture0);
	RegisterStringID(InputTexture1);
	RegisterStringID(InputTexture2);
	RegisterStringID(InputTexture3);
}

namespace eastengine
{
	namespace graphics
	{
		class HDRFilter::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			bool Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pResult, IRenderTarget* pSource);

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		public:
			Settings& GetSettings() { return m_settings; }
			void SetSettings(const Settings& settings) { m_settings = settings; }

			ToneMappingType GetToneMappingType() const { return m_emToneMappingType; }
			void SetToneMappingType(ToneMappingType emToneMappingType) { m_emToneMappingType = emToneMappingType; }

			AutoExposureType GetAutoExposureType() const { return m_emAutoExposureType; }
			void SetAutoExposureType(AutoExposureType emAutoExposureType) { m_emAutoExposureType = emAutoExposureType; }

		private:
			ToneMappingType m_emToneMappingType{ ToneMappingType::eNone };
			AutoExposureType m_emAutoExposureType{ AutoExposureType::eManual };
			Settings m_settings;

			IEffect* m_pEffect{ nullptr };

			uint32_t m_nCurLumTarget{ 0 };
			std::array<IRenderTarget*, 2> m_pAdaptedLuminances{ nullptr };
			IRenderTarget* m_pInitialLuminances{ nullptr };
		};

		HDRFilter::Impl::Impl()
		{
			std::string strPath(file::GetPath(file::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("PostProcessing\\HDRFilter\\HDRFilter_D.cso");
#else
			strPath.append("PostProcessing\\HDRFilter\\HDRFilter.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectHDRFilter, strPath.c_str());
			if (m_pEffect == nullptr)
			{
				assert(false);
				return;
			}

			m_pEffect->CreateTechnique(StrID::Threshold, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::BloomBlurH, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::BloomBlurV, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::LuminanceMap, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::Composite, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::CompositeWithExposure, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::Scale, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::AdaptLuminance, EmVertexFormat::eUnknown);

			{
				RenderTargetDesc2D rtDesc(DXGI_FORMAT_R32_FLOAT, 1024, 1024, 1, 11);
				rtDesc.Build();
				m_pAdaptedLuminances[0] = IRenderTarget::Create(rtDesc);
				m_pAdaptedLuminances[1] = IRenderTarget::Create(rtDesc);
			}

			{
				RenderTargetDesc2D rtDesc(DXGI_FORMAT_R32_FLOAT, 1024, 1024);
				rtDesc.Build();
				m_pInitialLuminances = IRenderTarget::Create(rtDesc);
			}
		}

		HDRFilter::Impl::~Impl()
		{
			IEffect::Destroy(&m_pEffect);

			std::for_each(m_pAdaptedLuminances.begin(), m_pAdaptedLuminances.end(), [](IRenderTarget* pRenderTarget)
			{
				SafeDelete(pRenderTarget);
			});
			SafeDelete(m_pInitialLuminances);
		}

		bool HDRFilter::Impl::Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pResult, IRenderTarget* pSource)
		{
			TRACER_EVENT("HDRFilter::Apply");
			D3D_PROFILING(pDeviceContext, HDRFilter);

			pDeviceContext->ClearState();

			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			m_settings.TimeDelta = Timer::GetInstance()->GetElapsedTime();
			m_settings.ToneMapTechnique = static_cast<float>(m_emToneMappingType);
			m_settings.AutoExposure = static_cast<float>(m_emAutoExposureType);

			m_pEffect->SetFloat(StrID::BloomThreshold, m_settings.BloomThreshold);
			m_pEffect->SetFloat(StrID::BloomMagnitude, m_settings.BloomMagnitude);
			m_pEffect->SetFloat(StrID::BloomBlurSigma, m_settings.BloomBlurSigma);
			m_pEffect->SetFloat(StrID::Tau, m_settings.Tau);
			m_pEffect->SetFloat(StrID::TimeDelta, m_settings.TimeDelta);
			m_pEffect->SetFloat(StrID::ToneMapTechnique, m_settings.ToneMapTechnique);
			m_pEffect->SetFloat(StrID::Exposure, m_settings.Exposure);
			m_pEffect->SetFloat(StrID::KeyValue, m_settings.KeyValue);
			m_pEffect->SetFloat(StrID::AutoExposure, m_settings.AutoExposure);
			m_pEffect->SetFloat(StrID::WhiteLevel, m_settings.WhiteLevel);
			m_pEffect->SetFloat(StrID::ShoulderStrength, m_settings.ShoulderStrength);
			m_pEffect->SetFloat(StrID::LinearStrength, m_settings.LinearStrength);
			m_pEffect->SetFloat(StrID::LinearAngle, m_settings.LinearAngle);
			m_pEffect->SetFloat(StrID::ToeStrength, m_settings.ToeStrength);
			m_pEffect->SetFloat(StrID::ToeNumerator, m_settings.ToeNumerator);
			m_pEffect->SetFloat(StrID::ToeDenominator, m_settings.ToeDenominator);
			m_pEffect->SetFloat(StrID::LinearWhite, m_settings.LinearWhite);
			m_pEffect->SetFloat(StrID::LuminanceSaturation, m_settings.LuminanceSaturation);
			m_pEffect->SetFloat(StrID::LumMapMipLevel, m_settings.LumMapMipLevel);
			m_pEffect->SetFloat(StrID::Bias, m_settings.Bias);

			auto PostProcess = [&](const string::StringID& strTechName, IRenderTarget* pResult, IRenderTarget** ppSource, size_t nSourceCount = 1)
			{
				IEffectTech* pTech = m_pEffect->GetTechnique(strTechName);
				if (pTech == nullptr)
				{
					assert(false);
					return;
				}

				auto GetSize = [](IRenderTarget* pSource)
				{
					TextureDesc2D desc;
					pSource->GetTexture()->GetTexture2D()->GetDesc(&desc);

					CD3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
					pSource->GetTexture()->GetShaderResourceView()->GetDesc(&srDesc);

					uint32_t nMipLevel = srDesc.Texture2D.MostDetailedMip;

					math::Vector2 f2Result;
					f2Result.x = static_cast<float>(std::max(desc.Width / (1 << nMipLevel), 1u));
					f2Result.y = static_cast<float>(std::max(desc.Height / (1 << nMipLevel), 1u));

					return f2Result;
				};

				math::Vector2 OutputSize;

				for (size_t i = 0; i < nSourceCount; ++i)
				{
					string::StringID strSizeName;
					strSizeName.Format("InputSize%d", i);
					math::Vector2 InputSize = GetSize(ppSource[i]);
					m_pEffect->SetVector(strSizeName, InputSize);

					string::StringID strTextureName;
					strTextureName.Format("InputTexture%d", i);
					m_pEffect->SetTexture(strTextureName, ppSource[i]->GetTexture());
				}

				OutputSize = GetSize(pResult);
				m_pEffect->SetVector("OutputSize", OutputSize);

				m_pEffect->SetSamplerState(StrID::PointSampler, pDevice->GetSamplerState(EmSamplerState::eMinMagMipPointClamp), 0);
				m_pEffect->SetSamplerState(StrID::LinearSampler, pDevice->GetSamplerState(EmSamplerState::eMinMagMipLinearClamp), 0);

				math::Viewport viewport;
				viewport.width = OutputSize.x;
				viewport.height = OutputSize.y;

				pDeviceContext->SetViewport(viewport);
				pDeviceContext->SetRenderTargets(&pResult, 1);

				uint32_t nPassCount = pTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pTech->PassApply(p, pDeviceContext);

					pDeviceContext->Draw(4, 0);
				}

				ClearEffect(pDeviceContext, pTech);
			};

			// CalcAvgLuminance
			{
				// Luminance mapping
				PostProcess(StrID::LuminanceMap, m_pInitialLuminances, &pSource);

				// Adaptation
				IRenderTarget* ppAdaptation[] =
				{
					m_pAdaptedLuminances[!m_nCurLumTarget],
					m_pInitialLuminances,
				};
				PostProcess(StrID::AdaptLuminance, m_pAdaptedLuminances[m_nCurLumTarget], ppAdaptation, 2);

				pDeviceContext->GenerateMips(m_pAdaptedLuminances[m_nCurLumTarget]->GetTexture()->GetShaderResourceView());
			}

			// Bloom
			IRenderTarget* pBloom = nullptr;
			{
				const math::UInt2& n2Size = pResult->GetSize();
				RenderTargetDesc2D desc = pResult->GetDesc2D();
				desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				desc.Width = n2Size.x / 1;
				desc.Height = n2Size.y / 1;
				desc.Build();
				pBloom = pDevice->GetRenderTarget(desc, false);

				IRenderTarget* ppBloomThreshold[] =
				{
					pSource,
					m_pAdaptedLuminances[m_nCurLumTarget],
				};
				PostProcess(StrID::Threshold, pBloom, ppBloomThreshold, 2);

				desc.Width = n2Size.x / 2;
				desc.Height = n2Size.y / 2;
				desc.Build();
				IRenderTarget* pDownScale1 = pDevice->GetRenderTarget(desc, false);
				PostProcess(StrID::Scale, pDownScale1, &pBloom);
				pDevice->ReleaseRenderTargets(&pBloom);

				desc.Width = n2Size.x / 4;
				desc.Height = n2Size.y / 4;
				desc.Build();
				IRenderTarget* pDownScale2 = pDevice->GetRenderTarget(desc, false);
				PostProcess(StrID::Scale, pDownScale2, &pDownScale1);

				desc.Width = n2Size.x / 8;
				desc.Height = n2Size.y / 8;
				desc.Build();
				IRenderTarget* pDownScale3 = pDevice->GetRenderTarget(desc, false);
				PostProcess(StrID::Scale, pDownScale3, &pDownScale2);

				// Blur it
				for (size_t i = 0; i < 4; ++i)
				{
					IRenderTarget* pBlur = pDevice->GetRenderTarget(desc, true);
					PostProcess(StrID::BloomBlurH, pBlur, &pDownScale3);
					PostProcess(StrID::BloomBlurV, pDownScale3, &pBlur);
					pDevice->ReleaseRenderTargets(&pBlur);
				}

				PostProcess(StrID::Scale, pDownScale2, &pDownScale3);
				pDevice->ReleaseRenderTargets(&pDownScale3);

				PostProcess(StrID::Scale, pDownScale1, &pDownScale2);
				pDevice->ReleaseRenderTargets(&pDownScale2);

				pBloom = pDownScale1;
			}

			// Final composite
			{
				IRenderTarget* ppComposite[] =
				{
					pSource,
					m_pAdaptedLuminances[m_nCurLumTarget],
					pBloom,
				};
				PostProcess(StrID::Composite, pResult, ppComposite, 3);
				pDevice->ReleaseRenderTargets(&pBloom);
			}

			m_nCurLumTarget = !m_nCurLumTarget;

			return true;
		}

		void HDRFilter::Impl::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->UndoSamplerState(StrID::PointSampler, 0);
			m_pEffect->UndoSamplerState(StrID::LinearSampler, 0);

			m_pEffect->SetTexture(StrID::InputTexture0, nullptr);
			m_pEffect->SetTexture(StrID::InputTexture1, nullptr);
			m_pEffect->SetTexture(StrID::InputTexture2, nullptr);
			m_pEffect->SetTexture(StrID::InputTexture3, nullptr);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}

		HDRFilter::HDRFilter()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		HDRFilter::~HDRFilter()
		{
		}

		bool HDRFilter::Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pResult, IRenderTarget* pSource)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			return m_pImpl->Apply(pDevice, pDeviceContext, pResult, pSource);
		}

		HDRFilter::Settings& HDRFilter::GetSettings()
		{
			return m_pImpl->GetSettings();
		}

		void HDRFilter::SetSettings(const Settings& settings)
		{
			return m_pImpl->SetSettings(settings);
		}

		HDRFilter::ToneMappingType HDRFilter::GetToneMappingType() const
		{
			return m_pImpl->GetToneMappingType();
		}

		void HDRFilter::SetToneMappingType(HDRFilter::ToneMappingType emToneMappingType)
		{
			m_pImpl->SetToneMappingType(emToneMappingType);
		}

		HDRFilter::AutoExposureType HDRFilter::GetAutoExposureType() const
		{
			return m_pImpl->GetAutoExposureType();
		}

		void HDRFilter::SetAutoExposureType(HDRFilter::AutoExposureType emAutoExposureType)
		{
			m_pImpl->SetAutoExposureType(emAutoExposureType);
		}
	}
}