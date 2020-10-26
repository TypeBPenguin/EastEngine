#include "stdafx.h"
#include "HDRFilterDX12.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Timer.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "DescriptorHeapDX12.h"

#include "RenderTargetDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
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

					uint32_t nTexInputIndex0{ 0 };
					uint32_t nTexInputIndex1{ 0 };
					uint32_t nTexInputIndex2{ 0 };
					uint32_t nTexInputIndex3{ 0 };
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

				enum PSType
				{
					eThreshold = 0,
					eBloomBlurH,
					eBloomBlurV,
					eLuminanceMap,
					eComposite,
					//eCompositeWithExposure,
					eScale_AdaptLuminance,
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
					case eScale_AdaptLuminance:
					case eScale:
						return "ScalePS";
					case eAdaptLuminance:
						return "AdaptLuminancePS";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetHDR_PSConstants(HDR_PSConstants* pPSConstants,
					const math::float2 InputSize[4],
					const math::float2& OutputSize,
					const uint32_t nTexInputIndex[4])
				{
					pPSConstants->InputSize0 = InputSize[0];
					pPSConstants->InputSize1 = InputSize[1];
					pPSConstants->InputSize2 = InputSize[2];
					pPSConstants->InputSize3 = InputSize[3];
					pPSConstants->OutputSize = OutputSize;
					pPSConstants->padding = {};

					pPSConstants->nTexInputIndex0 = nTexInputIndex[0];
					pPSConstants->nTexInputIndex1 = nTexInputIndex[1];
					pPSConstants->nTexInputIndex2 = nTexInputIndex[2];
					pPSConstants->nTexInputIndex3 = nTexInputIndex[3];
				}

				void SetHDRContents(HDRConstants* pHDRConstants, const Options::HDRConfig& hdrConfig, float elapsedTime)
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
			}

			class HDRFilter::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);
				void Apply(RenderTarget* pSource, RenderTarget* pResult);

			private:
				void Apply(ID3D12GraphicsCommandList2* pCommandList, DescriptorHeap* pSRVDescriptorHeap, uint32_t frameIndex, shader::PSType emPSType, RenderTarget* const* ppSource, size_t nSourceCount, RenderTarget* pResult);

			private:
				shader::HDR_PSConstants* AllocateHDRPSContents(uint32_t frameIndex)
				{
					assert(m_nPsContentsBufferIndex < eMaxPSContentsBuffercount);
					shader::HDR_PSConstants* pPSContents = m_psContents.Cast(frameIndex, m_nPsContentsBufferIndex);
					m_psContentsBufferGPUAddress = m_psContents.GPUAddress(frameIndex, m_nPsContentsBufferIndex);
					m_nPsContentsBufferIndex++;

					return pPSContents;
				}

				void ResetHDRPSContents(uint32_t frameIndex)
				{
					m_nPsContentsBufferIndex = 0;
					m_psContentsBufferGPUAddress = m_psContents.GPUAddress(frameIndex, 0);
				}

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_PsContents,
					eRP_HDRContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice, shader::PSType emPSType);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType);
				void CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType);

			private:
				struct RenderPipeline
				{
					PSOCache psoCache;

					std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> pBundles{ nullptr };
				};
				std::array<RenderPipeline, shader::ePS_Count> m_pipelineStates;

				enum
				{
					eMaxPSContentsBuffercount = 27,
					eMaxHDRContentsBufferCount = 1,
				};

				ConstantBuffer<shader::HDR_PSConstants> m_psContents;
				D3D12_GPU_VIRTUAL_ADDRESS m_psContentsBufferGPUAddress{};
				uint32_t m_nPsContentsBufferIndex{ 0 };

				ConstantBuffer<shader::HDRConstants> m_hdrContents;

				uint32_t m_nCurLumTarget{ 0 };
				std::array<std::unique_ptr<RenderTarget>, 2> m_pAdaptedLuminances;
				std::unique_ptr<RenderTarget> m_pInitialLuminances;
			};

			HDRFilter::Impl::Impl()
			{
				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\PostProcessing\\HDRFilter\\HDRFilter.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : HDRFilter.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);

					CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str(), emPSType);
				}

				m_psContents.Create(pDevice, eMaxPSContentsBuffercount, "HDR_psContents");
				m_hdrContents.Create(pDevice, eMaxHDRContentsBufferCount, "HDRContents");

				SafeRelease(pShaderBlob);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreateBundles(pDevice, emPSType);
				}

				{
					D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_FLOAT, 1024, 1024, 1, 11, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
					m_pAdaptedLuminances[0] = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_RENDER_TARGET);
					m_pAdaptedLuminances[1] = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_RENDER_TARGET);
				}

				{

					D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_FLOAT, 1024, 1024, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
					m_pInitialLuminances = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_RENDER_TARGET);
				}
			}

			HDRFilter::Impl::~Impl()
			{
				m_psContents.Destroy();
				m_hdrContents.Destroy();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					for (int j = 0; j < eFrameBufferCount; ++j)
					{
						util::ReleaseResource(m_pipelineStates[i].pBundles[j]);
						m_pipelineStates[i].pBundles[j] = nullptr;
					}

					m_pipelineStates[i].psoCache.Destroy();
				}
			}

			void HDRFilter::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
			}

			void HDRFilter::Impl::Apply(RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				ResetHDRPSContents(frameIndex);

				const Options& options = GetOptions();
				const Options::HDRConfig& hdrConfig = options.hdrConfig;
				shader::HDRConstants* pHDRConstants = m_hdrContents.Cast(frameIndex);
				shader::SetHDRContents(pHDRConstants, hdrConfig, Timer::GetInstance()->GetElapsedTime());

				RenderTarget* pAdaptedLuminance_downScale = nullptr;

				// CalcAvgLuminance
				{
					// Luminance mapping
					Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eLuminanceMap, &pSource, 1, m_pInitialLuminances.get());

					// Adaptation
					RenderTarget* ppAdaptation[] =
					{
						m_pAdaptedLuminances[!m_nCurLumTarget].get(),
						m_pInitialLuminances.get(),
					};
					Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eAdaptLuminance, ppAdaptation, 2, m_pAdaptedLuminances[m_nCurLumTarget].get());

					if (hdrConfig.LumMapMipLevel == 0)
					{
						pAdaptedLuminance_downScale = m_pAdaptedLuminances[m_nCurLumTarget].get();
					}
					else
					{
						RenderTarget* pAdaptedLuminances_Source = m_pAdaptedLuminances[m_nCurLumTarget].get();

						D3D12_RESOURCE_DESC desc = pAdaptedLuminances_Source->GetDesc();

						for (int nMipLevel = 0; nMipLevel < hdrConfig.LumMapMipLevel; ++nMipLevel)
						{
							desc.Width = std::max(desc.Width >> 1llu, 1llu);
							desc.Height = std::max(desc.Height >> 1u, 1u);

							pAdaptedLuminance_downScale = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent);

							Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eScale_AdaptLuminance, &pAdaptedLuminances_Source, 1, pAdaptedLuminance_downScale);

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
					const D3D12_RESOURCE_DESC resultDesc = pResult->GetDesc();

					D3D12_RESOURCE_DESC desc = resultDesc;
					desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					desc.Width = resultDesc.Width / 1;
					desc.Height = resultDesc.Height / 1;
					pBloom = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent);

					RenderTarget* ppBloomThreshold[] =
					{
						pSource,
						pAdaptedLuminance_downScale,
					};
					Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eThreshold, ppBloomThreshold, 2, pBloom);

					// DownScale 1 / 2
					desc.Width = resultDesc.Width / 2;
					desc.Height = resultDesc.Height / 2;
					RenderTarget* pDownScale1 = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent);
					Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eScale, &pBloom, 1, pDownScale1);
					pDeviceInstance->ReleaseRenderTargets(&pBloom);

					// DownScale 1 / 4
					desc.Width = resultDesc.Width / 4;
					desc.Height = resultDesc.Height / 4;
					RenderTarget* pDownScale2 = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent);
					Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eScale, &pDownScale1, 1, pDownScale2);

					// DownScale 1 / 8
					desc.Width = resultDesc.Width / 8;
					desc.Height = resultDesc.Height / 8;
					RenderTarget* pDownScale3 = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent);
					Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eScale, &pDownScale2, 1, pDownScale3);

					// Blur it
					for (size_t i = 0; i < 4; ++i)
					{
						RenderTarget* pBlur = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent);

						Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eBloomBlurH, &pDownScale3, 1, pBlur);
						Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eBloomBlurV, &pBlur, 1, pDownScale3);

						pDeviceInstance->ReleaseRenderTargets(&pBlur);
					}

					// UpScale 1 / 4
					Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eScale, &pDownScale3, 1, pDownScale2);

					pDeviceInstance->ReleaseRenderTargets(&pDownScale3);

					// UpScale 1 / 2
					Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eScale, &pDownScale2, 1, pDownScale1);

					pDeviceInstance->ReleaseRenderTargets(&pDownScale2);

					pBloom = pDownScale1;
				}

				// Final composite
				{
					RenderTarget* ppComposite[] =
					{
						pSource,
						pAdaptedLuminance_downScale,
						pBloom,
					};
					Apply(pCommandList, pSRVDescriptorHeap, frameIndex, shader::eComposite, ppComposite, 3, pResult);

					pDeviceInstance->ReleaseRenderTargets(&pBloom);
				}

				if (hdrConfig.LumMapMipLevel != 0)
				{
					pDeviceInstance->ReleaseRenderTargets(&pAdaptedLuminance_downScale);
				}

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);

				m_nCurLumTarget = !m_nCurLumTarget;
			}

			void HDRFilter::Impl::Apply(ID3D12GraphicsCommandList2* pCommandList, DescriptorHeap* pSRVDescriptorHeap, uint32_t frameIndex, shader::PSType emPSType, RenderTarget* const* ppSource, size_t nSourceCount, RenderTarget* pResult)
			{
				auto GetSize = [](RenderTarget* pSource) -> math::float2
				{
					const D3D12_RESOURCE_DESC desc = pSource->GetDesc();
					return
					{
						static_cast<float>(desc.Width),
						static_cast<float>(desc.Height)
					};

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

				uint32_t TexInputIndex[4]{};

				for (size_t i = 0; i < nSourceCount; ++i)
				{
					InputSize[i] = GetSize(ppSource[i]);

					util::ChangeResourceState(pCommandList, ppSource[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					TexInputIndex[i] = ppSource[i]->GetTexture()->GetDescriptorIndex();
				}

				util::ChangeResourceState(pCommandList, pResult, D3D12_RESOURCE_STATE_RENDER_TARGET);
				OutputSize = GetSize(pResult);

				shader::HDR_PSConstants* pPSConstants = AllocateHDRPSContents(frameIndex);
				shader::SetHDR_PSConstants(pPSConstants, InputSize, OutputSize, TexInputIndex);

				D3D12_VIEWPORT viewport{};
				viewport.Width = static_cast<float>(OutputSize.x);
				viewport.Height = static_cast<float>(OutputSize.y);
				viewport.MinDepth = 0.f;
				viewport.MaxDepth = 1.f;
				pCommandList->RSSetViewports(1, &viewport);

				D3D12_RECT scissorRect{};
				scissorRect.left = 0;
				scissorRect.top = 0;
				scissorRect.right = static_cast<long>(OutputSize.x);
				scissorRect.bottom = static_cast<long>(OutputSize.y);
				pCommandList->RSSetScissorRects(1, &scissorRect);

				const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
				{
					pResult->GetCPUHandle(),
				};
				pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->SetGraphicsRootSignature(m_pipelineStates[emPSType].psoCache.pRootSignature);
				pCommandList->SetGraphicsRootConstantBufferView(eRP_PsContents, m_psContentsBufferGPUAddress);
				pCommandList->SetGraphicsRootConstantBufferView(eRP_HDRContents, m_hdrContents.GPUAddress(frameIndex));

				pCommandList->ExecuteBundle(m_pipelineStates[emPSType].pBundles[frameIndex]);
			}

			ID3D12RootSignature* HDRFilter::Impl::CreateRootSignature(ID3D12Device* pDevice, shader::PSType emPSType)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& psContentParameter = vecRootParameters.emplace_back();
				psContentParameter.InitAsConstantBufferView(shader::eCB_HDR_PSConstants, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& hdrContentsParameter = vecRootParameters.emplace_back();
				hdrContentsParameter.InitAsConstantBufferView(shader::eCB_HDR_Constants, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipPointClamp, shader::eSampler_Point, 100, D3D12_SHADER_VISIBILITY_PIXEL),
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipLinearClamp, shader::eSampler_Linear, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void HDRFilter::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType)
			{
				PSOCache& psoCache = m_pipelineStates[emPSType].psoCache;

				if (pShaderBlob != nullptr)
				{
					const D3D_SHADER_MACRO macros[] =
					{
						{ "DX12", "1" },
						{ nullptr, nullptr },
					};

					if (psoCache.pVSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, "VS", shader::VS_CompileVersion, &psoCache.pVSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile vertex shader");
						}
					}

					if (psoCache.pPSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, shader::GetHDRPSTypeToString(emPSType), shader::PS_CompileVersion, &psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				const std::wstring wstrDebugName = string::MultiToWide(shader::GetHDRPSTypeToString(emPSType));

				if (psoCache.pRootSignature == nullptr)
				{
					psoCache.pRootSignature = CreateRootSignature(pDevice, emPSType);
					psoCache.pRootSignature->SetName(wstrDebugName.c_str());
				}

				if (psoCache.pPipelineState != nullptr)
				{
					util::ReleaseResource(psoCache.pPipelineState);
					psoCache.pPipelineState = nullptr;
				}

				D3D12_SHADER_BYTECODE vertexShaderBytecode{};
				vertexShaderBytecode.BytecodeLength = psoCache.pVSBlob->GetBufferSize();
				vertexShaderBytecode.pShaderBytecode = psoCache.pVSBlob->GetBufferPointer();

				D3D12_SHADER_BYTECODE pixelShaderBytecode{};
				pixelShaderBytecode.BytecodeLength = psoCache.pPSBlob->GetBufferSize();
				pixelShaderBytecode.pShaderBytecode = psoCache.pPSBlob->GetBufferPointer();

				DXGI_SAMPLE_DESC sampleDesc{};
				sampleDesc.Count = 1;

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.pRootSignature = psoCache.pRootSignature;
				psoDesc.VS = vertexShaderBytecode;
				psoDesc.PS = pixelShaderBytecode;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				psoDesc.SampleDesc = sampleDesc;
				psoDesc.SampleMask = 0xffffffff;
				psoDesc.RasterizerState = util::GetRasterizerDesc(EmRasterizerState::eSolidCullNone);
				psoDesc.BlendState = util::GetBlendDesc(EmBlendState::eOff);

				psoDesc.NumRenderTargets = 1;

				switch (emPSType)
				{
				case shader::eThreshold:
				case shader::eBloomBlurH:
				case shader::eBloomBlurV:
				case shader::eScale:
				{
					psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
				}
				break;
				case shader::eLuminanceMap:
				case shader::eScale_AdaptLuminance:
				case shader::eAdaptLuminance:
				{
					psoDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
				}
				break;
				case shader::eComposite:
				{
					D3D12_RESOURCE_DESC desc = Device::GetInstance()->GetSwapChainRenderTarget(0)->GetDesc();
					psoDesc.RTVFormats[0] = desc.Format;
				}
				break;
				default:
					throw_line("unknown ps type");
					break;
				}

				psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(EmDepthStencilState::eRead_Write_Off);

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoCache.pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				psoCache.pPipelineState->SetName(wstrDebugName.c_str());
			}

			void HDRFilter::Impl::CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType)
			{
				DescriptorHeap* pDescriptorHeapSRV = Device::GetInstance()->GetSRVDescriptorHeap();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					if (m_pipelineStates[emPSType].pBundles[i] != nullptr)
					{
						util::ReleaseResource(m_pipelineStates[emPSType].pBundles[i]);
						m_pipelineStates[emPSType].pBundles[i] = nullptr;
					}

					m_pipelineStates[emPSType].pBundles[i] = Device::GetInstance()->CreateBundle(m_pipelineStates[emPSType].psoCache.pPipelineState);
					ID3D12GraphicsCommandList2* pBundles = m_pipelineStates[emPSType].pBundles[i];

					pBundles->SetGraphicsRootSignature(m_pipelineStates[emPSType].psoCache.pRootSignature);

					ID3D12DescriptorHeap* pDescriptorHeaps[] =
					{
						pDescriptorHeapSRV->GetHeap(),
					};
					pBundles->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					pBundles->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pDescriptorHeapSRV->GetStartGPUHandle());

					pBundles->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
					pBundles->IASetVertexBuffers(0, 0, nullptr);
					pBundles->IASetIndexBuffer(nullptr);

					pBundles->DrawInstanced(4, 1, 0, 0);

					HRESULT hr = pBundles->Close();
					if (FAILED(hr))
					{
						throw_line("failed to close bundle");
					}
				}
			}

			HDRFilter::HDRFilter()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			HDRFilter::~HDRFilter()
			{
			}

			void HDRFilter::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void HDRFilter::Apply(RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Apply(pSource, pResult);
			}
		}
	}
}