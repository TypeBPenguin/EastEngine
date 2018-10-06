#include "stdafx.h"
#include "BloomFilterDX11.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "RenderTargetDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct BloomFilterContents
				{
					//Needed for pixel offset
					math::Vector2 InverseResolution;

					//The threshold of pixels that are brighter than that.
					float Threshold{ 0.8f };

					//MODIFIED DURING RUNTIME, CHANGING HERE MAKES NO DIFFERENCE;
					float Radius{ 0.f };
					float Strength{ 0.f };

					//How far we stretch the pixels
					float StreakLength{ 1.f };

					math::Vector2 padding;
				};

				enum CBSlot
				{
					eCB_BloomFilterContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_LinearPoint = 0,
				};

				enum SRVSlot
				{
					eSRV_Source = 0,
				};

				enum PSType
				{
					eExtract = 0,
					eExtractLuminance,
					eDownsample,
					eUpsample,
					eUpsampleLuminance,
					eApply,

					ePS_Count,
				};

				const char* GetBloomFilterPSTypeToString(PSType emPSType)
				{
					switch (emPSType)
					{
					case eExtract:
						return "ExtractPS";
					case eExtractLuminance:
						return "ExtractLuminancePS";
					case eDownsample:
						return "DownsamplePS";
					case eUpsample:
						return "UpsamplePS";
					case eUpsampleLuminance:
						return "UpsampleLuminancePS";
					case eApply:
						return "ApplyPS";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetBloomFilterContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<BloomFilterContents>* pCB_BloomFilterContents, 
					const math::Vector2& InverseResolution, float Threshold, float Radius, float Strength, float StreakLength)
				{
					BloomFilterContents* pBloomFilterContents = pCB_BloomFilterContents->Map(pDeviceContext);

					pBloomFilterContents->InverseResolution = InverseResolution;
					pBloomFilterContents->Threshold = Threshold;
					pBloomFilterContents->Radius = Radius;
					pBloomFilterContents->Strength = Strength;
					pBloomFilterContents->StreakLength = StreakLength;
					
					pBloomFilterContents->padding = {};

					pCB_BloomFilterContents->Unmap(pDeviceContext);
				}
			}

			class BloomFilter::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(RenderTarget* pSource);

			private:
				RenderTarget* Sampling(Device* pDevice, ID3D11DeviceContext* pDeviceContext, const Options::BloomFilterConfig& bloomFilterConfig, shader::PSType emPSType, bool isResult, uint32_t nWidth, uint32_t nHeight, int nPass, const math::Vector2& f2InverseResolution, RenderTarget* pSource, RenderTarget* pResult = nullptr);
				void SetBloomPreset(Options::BloomFilterConfig::Presets emPreset);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				std::array<ID3D11PixelShader*, shader::ePS_Count> m_pPixelShaders{ nullptr };

				ConstantBuffer<shader::BloomFilterContents> m_bloomFilterContents;

				std::array<float, 5> m_fRadius{ 1.f };
				std::array<float, 5> m_fStrengths{ 1.f };
				float m_fRadiusMultiplier{ 1.f };
				float m_fStreakLength{ 0.f };
				int m_nDownsamplePasses{ 5 };
			};

			BloomFilter::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\BloomFilter\\BloomFilter.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(String::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : BloomFilter.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "VS", "vs_5_0", &m_pVertexShader, "BloomFilter_VS") == false)
				{
					throw_line("failed to create BloomFilter_VS");
				}

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					shader::PSType emPSType = static_cast<shader::PSType>(i);

					const char* strPSName = shader::GetBloomFilterPSTypeToString(emPSType);
					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), strPSName, "ps_5_0", &m_pPixelShaders[i], strPSName) == false)
					{
						std::string dump = String::Format("failed to create %s", strPSName);
						throw_line(dump.c_str());
					}
				}

				SafeRelease(pShaderBlob);

				m_bloomFilterContents.Create(pDevice, "BloomFilterContents");
			}

			BloomFilter::Impl::~Impl()
			{
				m_bloomFilterContents.Destroy();

				SafeRelease(m_pVertexShader);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					SafeRelease(m_pPixelShaders[i]);
				}
			}

			void BloomFilter::Impl::Apply(RenderTarget* pSource)
			{
				if (pSource == nullptr)
					return;

				DX_PROFILING(BloomFilter);

				const Options::BloomFilterConfig& bloomFilterConfig = GetOptions().bloomFilterConfig;

				SetBloomPreset(bloomFilterConfig.emPreset);

				D3D11_TEXTURE2D_DESC sourceDesc{};
				pSource->GetDesc2D(&sourceDesc);

				const math::UInt2 n2TargetSize(sourceDesc.Width / 2, sourceDesc.Height / 2);
				m_fRadiusMultiplier = static_cast<float>(sourceDesc.Width) / static_cast<float>(sourceDesc.Height);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				pDeviceContext->ClearState();

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(EmRasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSamplerLinearPoint = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagLinearMipPointClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_LinearPoint, 1, &pSamplerLinearPoint);

				pDeviceContext->PSSetConstantBuffers(shader::eCB_BloomFilterContents, 1, &m_bloomFilterContents.pBuffer);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eOpacity);
				pDeviceContext->OMSetBlendState(pBlendState, &math::Vector4::Zero.x, 0xffffffff);

				const math::Vector2 f2InverseResolution(1.f / n2TargetSize.x, 1.f / n2TargetSize.y);

				shader::PSType emPSType = bloomFilterConfig.isEnableLuminance == true ? shader::eExtractLuminance : shader::eExtract;
				RenderTarget* pMip0 = Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, emPSType, true, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pSource);

				RenderTarget* pMip1 = Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, shader::eDownsample, false, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pMip0, nullptr);
				RenderTarget* pMip2 = Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, shader::eDownsample, false, n2TargetSize.x, n2TargetSize.y, 1, f2InverseResolution, pMip1, nullptr);
				RenderTarget* pMip3 = Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, shader::eDownsample, false, n2TargetSize.x, n2TargetSize.y, 2, f2InverseResolution, pMip2, nullptr);
				RenderTarget* pMip4 = Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, shader::eDownsample, false, n2TargetSize.x, n2TargetSize.y, 3, f2InverseResolution, pMip3, nullptr);
				RenderTarget* pMip5 = Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, shader::eDownsample, false, n2TargetSize.x, n2TargetSize.y, 4, f2InverseResolution, pMip4, nullptr);

				pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eAlphaBlend);
				pDeviceContext->OMSetBlendState(pBlendState, &math::Vector4::Zero.x, 0xffffffff);

				emPSType = bloomFilterConfig.isEnableLuminance == true ? shader::eUpsampleLuminance : shader::eUpsample;
				pMip4 = Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, emPSType, false, n2TargetSize.x, n2TargetSize.y, 4, f2InverseResolution, pMip5, pMip4);
				pMip3 = Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, emPSType, false, n2TargetSize.x, n2TargetSize.y, 3, f2InverseResolution, pMip4, pMip3);
				pMip2 = Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, emPSType, false, n2TargetSize.x, n2TargetSize.y, 2, f2InverseResolution, pMip3, pMip2);
				pMip1 = Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, emPSType, false, n2TargetSize.x, n2TargetSize.y, 1, f2InverseResolution, pMip2, pMip1);
				pMip0 = Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, emPSType, false, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pMip1, pMip0);

				pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eAdditive);
				pDeviceContext->OMSetBlendState(pBlendState, &math::Vector4::Zero.x, 0xffffffff);

				Sampling(pDeviceInstance, pDeviceContext, bloomFilterConfig, shader::eApply, true, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pMip0, pSource);

				pDeviceInstance->ReleaseRenderTargets(&pMip0, 1, false);
				pDeviceInstance->ReleaseRenderTargets(&pMip1, 1, false);
				pDeviceInstance->ReleaseRenderTargets(&pMip2, 1, false);
				pDeviceInstance->ReleaseRenderTargets(&pMip3, 1, false);
				pDeviceInstance->ReleaseRenderTargets(&pMip4, 1, false);
				pDeviceInstance->ReleaseRenderTargets(&pMip5, 1, false);
			}

			RenderTarget* BloomFilter::Impl::Sampling(Device* pDevice, ID3D11DeviceContext* pDeviceContext, const Options::BloomFilterConfig& bloomFilterConfig, shader::PSType emPSType, bool isResult, uint32_t nWidth, uint32_t nHeight, int nPass, const math::Vector2& f2InverseResolution, RenderTarget* pSource, RenderTarget* pResult)
			{
				const std::wstring wstrDebugName = String::MultiToWide(shader::GetBloomFilterPSTypeToString(emPSType));
				eastengine::graphics::dx11::util::DXProfiler profiler(wstrDebugName.c_str());

				if (m_nDownsamplePasses > nPass)
				{
					const int nPassFactor = static_cast<int>(std::pow(2, nPass));

					if (pResult == nullptr)
					{
						D3D11_TEXTURE2D_DESC desc{};
						pSource->GetDesc2D(&desc);
						desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

						desc.Width = nWidth / (nPassFactor * (isResult ? 1 : 2));
						desc.Height = nHeight / (nPassFactor * (isResult ? 1 : 2));

						pResult = pDevice->GetRenderTarget(&desc, false);
					}

					shader::SetBloomFilterContents(pDeviceContext, &m_bloomFilterContents,
						f2InverseResolution * static_cast<float>(nPassFactor),
						bloomFilterConfig.fThreshold,
						m_fRadius[nPass],
						m_fStrengths[nPass] * bloomFilterConfig.fStrengthMultiplier,
						m_fStreakLength);

					D3D11_TEXTURE2D_DESC desc{};
					pResult->GetDesc2D(&desc);

					D3D11_VIEWPORT viewport{};
					viewport.Width = static_cast<float>(desc.Width);
					viewport.Height = static_cast<float>(desc.Height);
					viewport.MinDepth = 0.f;
					viewport.MaxDepth = 1.f;
					pDeviceContext->RSSetViewports(1, &viewport);

					ID3D11RenderTargetView* pRTV[] = { pResult->GetRenderTargetView() };
					pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

					pDeviceContext->PSSetShader(m_pPixelShaders[emPSType], nullptr, 0);

					ID3D11ShaderResourceView* pSRV = pSource->GetShaderResourceView();
					pDeviceContext->PSSetShaderResources(shader::eSRV_Source, 1, &pSRV);

					pDeviceContext->Draw(4, 0);

					pSRV = nullptr;
					pDeviceContext->PSSetShaderResources(shader::eSRV_Source, 1, &pSRV);

					return pResult;
				}

				return pSource;
			}

			void BloomFilter::Impl::SetBloomPreset(Options::BloomFilterConfig::Presets emPreset)
			{
				switch (emPreset)
				{
				case Options::BloomFilterConfig::Presets::eWide:
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
				case Options::BloomFilterConfig::Presets::eSuperWide:
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
				case Options::BloomFilterConfig::Presets::eFocussed:
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
				case Options::BloomFilterConfig::Presets::eSmall:
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
				case Options::BloomFilterConfig::Presets::eCheap:
				{
					m_fStrengths[0] = 0.8f;
					m_fStrengths[1] = 2.f;
					m_fRadius[1] = 2.f;
					m_fRadius[0] = 2.f;
					m_fStreakLength = 1.f;
					m_nDownsamplePasses = 2;
				}
				break;
				case Options::BloomFilterConfig::Presets::eOne:
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
				default:
					throw_line("unknown type");
					break;
				}
			}

			BloomFilter::BloomFilter()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			BloomFilter::~BloomFilter()
			{
			}

			void BloomFilter::Apply(RenderTarget* pSource)
			{
				m_pImpl->Apply(pSource);
			}
		}
	}
}