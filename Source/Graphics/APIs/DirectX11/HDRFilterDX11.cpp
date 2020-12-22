#include "stdafx.h"
#include "HDRFilterDX11.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Timer.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "RenderTargetDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct HDR_PSConstants
				{
					math::float2 InputSize0;
					math::float2 InputSize1;
					math::float2 InputSize2;
					math::float2 InputSize3;
					math::float2 OutputSize;
					math::float2 padding;
				};

				struct HDRConstants
				{
					float BloomThreshold{ 0.f };
					float BloomMagnitude{ 0.f };
					float BloomBlurSigma{ 0.f };
					float Tau{ 0.f };
					float Exposure{ 0.f };
					float KeyValue{ 0.f };
					float WhiteLevel{ 0.f };
					float ShoulderStrength{ 0.f };
					float LinearStrength{ 0.f };
					float LinearAngle{ 0.f };
					float ToeStrength{ 0.f };
					float ToeNumerator{ 0.f };
					float ToeDenominator{ 0.f };
					float LinearWhite{ 0.f };
					float LuminanceSaturation{ 0.f };
					float Bias{ 0.f };

					int LumMapMipLevel{ 0 };

					int ToneMapTechnique{ 0 };
					int AutoExposure{ 0 };

					float TimeDelta{ 0.f };
				};

				enum CBSlot
				{
					eCB_HDR_PSConstants = 0,
					eCB_HDR_Constants = 1,
				};

				enum SamplerSlot
				{
					eSampler_Point = 0,
					eSampler_Linear = 1,
				};

				enum SRVSlot
				{
					eSRV_Input0 = 0,
					eSRV_Input1 = 1,
					eSRV_Input2 = 2,
					eSRV_Input3 = 3,
				};

				enum PSType
				{
					eThreshold = 0,
					eBloomBlurH,
					eBloomBlurV,
					eLuminanceMap,
					eComposite,
					//eCompositeWithExposure,
					eScale,
					eAdaptLuminance,

					ePS_Count,
				};

				const char* GetHDRPSTypeToString(PSType emPSType)
				{
					switch (emPSType)
					{
					case eThreshold:
						return "ThresholdPS";
					case eBloomBlurH:
						return "BloomBlurHPS";
					case eBloomBlurV:
						return "BloomBlurVPS";
					case eLuminanceMap:
						return "LuminanceMapPS";
					case eComposite:
						return "CompositePS";
					//case eCompositeWithExposure:
					//	return "CompositeWithExposurePS";
					case eScale:
						return "ScalePS";
					case eAdaptLuminance:
						return "AdaptLuminancePS";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetHDR_PSConstants(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<HDR_PSConstants>* pCB_PSConstants,
					const math::float2 InputSize[4],
					const math::float2& OutputSize)
				{
					HDR_PSConstants* pPSConstants = pCB_PSConstants->Map(pDeviceContext);
					{
						pPSConstants->InputSize0 = InputSize[0];
						pPSConstants->InputSize1 = InputSize[1];
						pPSConstants->InputSize2 = InputSize[2];
						pPSConstants->InputSize3 = InputSize[3];
						pPSConstants->OutputSize = OutputSize;
						pPSConstants->padding = {};
					}
					pCB_PSConstants->Unmap(pDeviceContext);
				}

				void SetHDRContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<HDRConstants>* pCB_HDRConstants, const Options::HDRConfig& hdrConfig, float elapsedTime)
				{
					HDRConstants* pHDRConstants = pCB_HDRConstants->Map(pDeviceContext);
					{
						pHDRConstants->BloomThreshold = hdrConfig.BloomThreshold;
						pHDRConstants->BloomMagnitude = hdrConfig.BloomMagnitude;
						pHDRConstants->BloomBlurSigma = hdrConfig.BloomBlurSigma;
						pHDRConstants->Tau = hdrConfig.Tau;
						pHDRConstants->Exposure = hdrConfig.Exposure;
						pHDRConstants->KeyValue = hdrConfig.KeyValue;
						pHDRConstants->WhiteLevel = hdrConfig.WhiteLevel;
						pHDRConstants->ShoulderStrength = hdrConfig.ShoulderStrength;
						pHDRConstants->LinearStrength = hdrConfig.LinearStrength;
						pHDRConstants->LinearAngle = hdrConfig.LinearAngle;
						pHDRConstants->ToeStrength = hdrConfig.ToeStrength;
						pHDRConstants->ToeNumerator = hdrConfig.ToeNumerator;
						pHDRConstants->ToeDenominator = hdrConfig.ToeDenominator;
						pHDRConstants->LinearWhite = hdrConfig.LinearWhite;
						pHDRConstants->LuminanceSaturation = hdrConfig.LuminanceSaturation;
						pHDRConstants->Bias = hdrConfig.Bias;

						pHDRConstants->TimeDelta = elapsedTime;
						pHDRConstants->LumMapMipLevel = hdrConfig.LumMapMipLevel;

						pHDRConstants->ToneMapTechnique = static_cast<int>(hdrConfig.emToneMappingType);
						pHDRConstants->AutoExposure = static_cast<int>(hdrConfig.emAutoExposureType);
					}
					pCB_HDRConstants->Unmap(pDeviceContext);
				}
			}

			class HDRFilter::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				void Apply(ID3D11DeviceContext* pDeviceContext, shader::PSType emPSType, const RenderTarget* const* ppSource, size_t nSourceCount, RenderTarget* pResult);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				std::array<ID3D11PixelShader*, shader::ePS_Count> m_pPixelShaders{ nullptr };

				ConstantBuffer<shader::HDR_PSConstants> m_psContents;
				ConstantBuffer<shader::HDRConstants> m_hdrContents;

				uint32_t m_nCurLumTarget{ 0 };
				std::array<std::unique_ptr<RenderTarget>, 2> m_pAdaptedLuminances;
				std::unique_ptr<RenderTarget> m_pInitialLuminances;
			};

			HDRFilter::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\PostProcessing\\HDRFilter\\HDRFilter.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : HDRFilter.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), "VS", shader::VS_CompileVersion, &m_pVertexShader, "HDR_VS") == false)
				{
					throw_line("failed to create HDR_VS");
				}

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					shader::PSType emPSType = static_cast<shader::PSType>(i);

					const char* psName = shader::GetHDRPSTypeToString(emPSType);
					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), psName, shader::PS_CompileVersion, &m_pPixelShaders[i], psName) == false)
					{
						std::string dump = string::Format("failed to create %s", psName);
						throw_line(dump.c_str());
					}
				}

				SafeRelease(pShaderBlob);

				m_psContents.Create(pDevice, "HDR_PSContents");
				m_hdrContents.Create(pDevice, "HDRContents");

				{
					D3D11_TEXTURE2D_DESC desc{};
					desc.Format = DXGI_FORMAT_R32_FLOAT;
					desc.Width = 1024;
					desc.Height = 1024;
					desc.ArraySize = 1;
					desc.MipLevels = 1;
					desc.SampleDesc.Count = 1;
					desc.Usage = D3D11_USAGE_DEFAULT;
					desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
					desc.CPUAccessFlags = 0;
					//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
					desc.MiscFlags = 0;

					m_pAdaptedLuminances[0] = RenderTarget::Create(&desc);
					m_pAdaptedLuminances[1] = RenderTarget::Create(&desc);
				}

				{
					D3D11_TEXTURE2D_DESC desc{};
					desc.Format = DXGI_FORMAT_R32_FLOAT;
					desc.Width = 1024;
					desc.Height = 1024;
					desc.ArraySize = 1;
					desc.MipLevels = 1;
					desc.SampleDesc.Count = 1;
					desc.Usage = D3D11_USAGE_DEFAULT;
					desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
					desc.CPUAccessFlags = 0;
					desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

					m_pInitialLuminances = RenderTarget::Create(&desc);
				}
			}

			HDRFilter::Impl::~Impl()
			{
				m_psContents.Destroy();
				m_hdrContents.Destroy();

				SafeRelease(m_pVertexShader);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					SafeRelease(m_pPixelShaders[i]);
				}
			}

			void HDRFilter::Impl::Apply(const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(HDR);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				pDeviceContext->ClearState();

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(BlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(DepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipPointClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_Point, 1, &pSamplerPointClamp);

				ID3D11SamplerState* pSamplerLinearClamp = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipLinearClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_Linear, 1, &pSamplerLinearClamp);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);

				const Options::HDRConfig& hdrConfig = GetOptions().hdrConfig;
				shader::SetHDRContents(pDeviceContext, &m_hdrContents, hdrConfig, Timer::GetInstance()->GetElapsedTime());
				pDeviceContext->PSSetConstantBuffers(shader::eCB_HDR_Constants, 1, &m_hdrContents.pBuffer);

				RenderTarget* pAdaptedLuminance_downScale = nullptr;

				// CalcAvgLuminance
				{
					// Luminance mapping
					Apply(pDeviceContext, shader::eLuminanceMap, &pSource, 1, m_pInitialLuminances.get());

					// Adaptation
					const RenderTarget* ppAdaptation[] =
					{
						m_pAdaptedLuminances[!m_nCurLumTarget].get(),
						m_pInitialLuminances.get(),
					};
					Apply(pDeviceContext, shader::eAdaptLuminance, ppAdaptation, 2, m_pAdaptedLuminances[m_nCurLumTarget].get());

					if (hdrConfig.LumMapMipLevel == 0)
					{
						pAdaptedLuminance_downScale = m_pAdaptedLuminances[m_nCurLumTarget].get();
					}
					else
					{
						RenderTarget* pAdaptedLuminances_Source = m_pAdaptedLuminances[m_nCurLumTarget].get();

						D3D11_TEXTURE2D_DESC desc{};
						pAdaptedLuminances_Source->GetDesc2D(&desc);

						for (int nMipLevel = 0; nMipLevel < hdrConfig.LumMapMipLevel; ++nMipLevel)
						{
							desc.Width = std::max(desc.Width >> 1u, 1u);
							desc.Height = std::max(desc.Height >> 1u, 1u);

							pAdaptedLuminance_downScale = pDeviceInstance->GetRenderTarget(&desc);

							Apply(pDeviceContext, shader::eScale, &pAdaptedLuminances_Source, 1, pAdaptedLuminance_downScale);

							if (0 < nMipLevel && nMipLevel < hdrConfig.LumMapMipLevel)
							{
								pDeviceInstance->ReleaseRenderTargets(&pAdaptedLuminances_Source, 1);
							}

							pAdaptedLuminances_Source = pAdaptedLuminance_downScale;
						}
					}
				}

				// Bloom
				RenderTarget* pBloom = nullptr;
				{
					D3D11_TEXTURE2D_DESC resultDesc{};
					pResult->GetDesc2D(&resultDesc);

					D3D11_TEXTURE2D_DESC desc = resultDesc;
					desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					desc.Width = resultDesc.Width / 1;
					desc.Height = resultDesc.Height / 1;
					pBloom = pDeviceInstance->GetRenderTarget(&desc);

					const RenderTarget* ppBloomThreshold[] =
					{
						pSource,
						pAdaptedLuminance_downScale,
					};
					Apply(pDeviceContext, shader::eThreshold, ppBloomThreshold, 2, pBloom);

					desc.Width = resultDesc.Width / 2;
					desc.Height = resultDesc.Height / 2;
					RenderTarget* pDownScale1 = pDeviceInstance->GetRenderTarget(&desc);
					Apply(pDeviceContext, shader::eScale, &pBloom, 1, pDownScale1);
					pDeviceInstance->ReleaseRenderTargets(&pBloom);

					desc.Width = resultDesc.Width / 4;
					desc.Height = resultDesc.Height / 4;
					RenderTarget* pDownScale2 = pDeviceInstance->GetRenderTarget(&desc);
					Apply(pDeviceContext, shader::eScale, &pDownScale1, 1, pDownScale2);

					desc.Width = resultDesc.Width / 8;
					desc.Height = resultDesc.Height / 8;
					RenderTarget* pDownScale3 = pDeviceInstance->GetRenderTarget(&desc);
					Apply(pDeviceContext, shader::eScale, &pDownScale2, 1, pDownScale3);

					// Blur it
					for (size_t i = 0; i < 4; ++i)
					{
						RenderTarget* pBlur = pDeviceInstance->GetRenderTarget(&desc);
						Apply(pDeviceContext, shader::eBloomBlurH, &pDownScale3, 1, pBlur);
						Apply(pDeviceContext, shader::eBloomBlurV, &pBlur, 1, pDownScale3);
						pDeviceInstance->ReleaseRenderTargets(&pBlur);
					}

					Apply(pDeviceContext, shader::eScale, &pDownScale3, 1, pDownScale2);
					pDeviceInstance->ReleaseRenderTargets(&pDownScale3);

					Apply(pDeviceContext, shader::eScale, &pDownScale2, 1, pDownScale1);
					pDeviceInstance->ReleaseRenderTargets(&pDownScale2);

					pBloom = pDownScale1;
				}

				// Final composite
				{
					const RenderTarget* ppComposite[] =
					{
						pSource,
						pAdaptedLuminance_downScale,
						pBloom,
					};
					Apply(pDeviceContext, shader::eComposite, ppComposite, 3, pResult);
					pDeviceInstance->ReleaseRenderTargets(&pBloom);
				}

				if (hdrConfig.LumMapMipLevel != 0)
				{
					pDeviceInstance->ReleaseRenderTargets(&pAdaptedLuminance_downScale);
				}

				m_nCurLumTarget = !m_nCurLumTarget;
			}

			void HDRFilter::Impl::Apply(ID3D11DeviceContext* pDeviceContext, shader::PSType emPSType, const RenderTarget* const* ppSource, size_t nSourceCount, RenderTarget* pResult)
			{
				auto GetSize = [](const RenderTarget* pSource) -> math::float2
				{
					D3D11_TEXTURE2D_DESC desc{};
					pSource->GetDesc2D(&desc);

					return
					{
						static_cast<float>(desc.Width),
						static_cast<float>(desc.Height)
					};

					//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
					//pSource->GetShaderResourceView()->GetDesc(&srvDesc);
					//
					//const uint32_t nMipLevel = srvDesc.Texture2D.MostDetailedMip;
					//
					//const math::float2 f2Result
					//{
					//	static_cast<float>(std::max(desc.Width / (1u << nMipLevel), 1u)),
					//	static_cast<float>(std::max(desc.Height / (1u << nMipLevel), 1u))
					//};
					//return f2Result;
				};

				math::float2 InputSize[4];
				math::float2 OutputSize;

				for (size_t i = 0; i < nSourceCount; ++i)
				{
					InputSize[i] = GetSize(ppSource[i]);

					const shader::SRVSlot emSRVSlot = static_cast<shader::SRVSlot>(i);
					ID3D11ShaderResourceView* pSourceSRV = ppSource[i]->GetShaderResourceView();
					pDeviceContext->PSSetShaderResources(emSRVSlot, 1, &pSourceSRV);
				}

				OutputSize = GetSize(pResult);

				D3D11_VIEWPORT viewport{};
				viewport.Width = static_cast<float>(OutputSize.x);
				viewport.Height = static_cast<float>(OutputSize.y);
				viewport.MinDepth = 0.f;
				viewport.MaxDepth = 1.f;
				pDeviceContext->RSSetViewports(1, &viewport);

				ID3D11RenderTargetView* pRTV[] = { pResult->GetRenderTargetView() };
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

				shader::SetHDR_PSConstants(pDeviceContext, &m_psContents, InputSize, OutputSize);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_HDR_PSConstants, 1, &m_psContents.pBuffer);

				pDeviceContext->PSSetShader(m_pPixelShaders[emPSType], nullptr, 0);
				pDeviceContext->Draw(4, 0);

				pRTV[0] = nullptr;
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

				for (size_t i = 0; i < nSourceCount; ++i)
				{
					ID3D11ShaderResourceView* pSourceSRV = nullptr;
					pDeviceContext->PSSetShaderResources(static_cast<uint32_t>(i), 1, &pSourceSRV);
				}
			}

			HDRFilter::HDRFilter()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			HDRFilter::~HDRFilter()
			{
			}

			void HDRFilter::Apply(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Apply(pSource, pResult);
			}
		}
	}
}