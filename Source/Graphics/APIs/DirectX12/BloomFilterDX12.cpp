#include "stdafx.h"
#include "BloomFilterDX12.h"

#include "CommonLib/FileUtil.h"

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
				struct BloomFilterContents
				{
					//Needed for pixel offset
					math::float2 InverseResolution;

					//The threshold of pixels that are brighter than that.
					float Threshold{ 0.8f };

					//MODIFIED DURING RUNTIME, CHANGING HERE MAKES NO DIFFERENCE;
					float Radius{ 0.f };
					float Strength{ 0.f };

					//How far we stretch the pixels
					float StreakLength{ 1.f };

					uint32_t nTexSourceIndex{ 0 };
					float padding{ 0.f };
				};

				enum CBSlot
				{
					eCB_BloomFilterContents = 0,
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

				void SetBloomFilterContents(BloomFilterContents* pBloomFilterContents,
					const math::float2& InverseResolution, float Threshold, float Radius, float Strength, float StreakLength, uint32_t nTexSourceIndex)
				{
					pBloomFilterContents->InverseResolution = InverseResolution;
					pBloomFilterContents->Threshold = Threshold;
					pBloomFilterContents->Radius = Radius;
					pBloomFilterContents->Strength = Strength;
					pBloomFilterContents->StreakLength = StreakLength;
					pBloomFilterContents->nTexSourceIndex = nTexSourceIndex;

					pBloomFilterContents->padding = {};
				}
			}

			class BloomFilter::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);
				void Apply(RenderTarget* pSource);

			private:
				RenderTarget * Sampling(Device* pDevice, ID3D12GraphicsCommandList2* pCommandList, uint32_t frameIndex, const Options::BloomFilterConfig& bloomFilterConfig, shader::PSType emPSType, bool isResult, uint32_t nWidth, uint32_t nHeight, int nPass, const math::float2& f2InverseResolution, RenderTarget* pSource, RenderTarget* pResult = nullptr);
				void SetBloomPreset(Options::BloomFilterConfig::Presets emPreset);

				shader::BloomFilterContents* AllocateBloomFilterContents(uint32_t frameIndex)
				{
					assert(m_nBloomFilterBufferIndex < MaxBloomFilterContentsBufferCount);
					shader::BloomFilterContents* pBuffer = m_bloomFilterContents.Cast(frameIndex, m_nBloomFilterBufferIndex);
					m_bloomFilterContentsBufferGPUAddress = m_bloomFilterContents.GPUAddress(frameIndex, m_nBloomFilterBufferIndex);
					++m_nBloomFilterBufferIndex;

					return pBuffer;
				}

				void ResetBloomFilterContents(uint32_t frameIndex)
				{
					m_nBloomFilterBufferIndex = 0;
					m_bloomFilterContentsBufferGPUAddress = m_bloomFilterContents.GPUAddress(frameIndex, 0);
				}

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_BloomFilterContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType);
				void CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType);

			private:
				struct RenderPipeline
				{
					PSOCache psoCache;
					std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> pBundles{ nullptr };
				};
				std::array<RenderPipeline, shader::ePS_Count> m_pipelineStates;

				ConstantBuffer<shader::BloomFilterContents> m_bloomFilterContents;
				D3D12_GPU_VIRTUAL_ADDRESS m_bloomFilterContentsBufferGPUAddress{};
				uint32_t m_nBloomFilterBufferIndex{ 0 };
				const uint32_t MaxBloomFilterContentsBufferCount{ 12 };

				std::array<float, 5> m_fRadius{ 1.f };
				std::array<float, 5> m_fStrengths{ 1.f };
				float m_fRadiusMultiplier{ 1.f };
				float m_fStreakLength{ 0.f };
				int m_nDownsamplePasses{ 5 };
			};

			BloomFilter::Impl::Impl()
			{
				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\PostProcessing\\BloomFilter\\BloomFilter.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : BloomFilter.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str(), emPSType);
				}

				m_bloomFilterContents.Create(pDevice, MaxBloomFilterContentsBufferCount, "BloomFilterContents");

				SafeRelease(pShaderBlob);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreateBundles(pDevice, emPSType);
				}
			}

			BloomFilter::Impl::~Impl()
			{
				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					for (int j = 0; j < eFrameBufferCount; ++j)
					{
						util::ReleaseResource(m_pipelineStates[i].pBundles[j]);
						m_pipelineStates[i].pBundles[j] = nullptr;
					}

					m_pipelineStates[i].psoCache.Destroy();
				}

				m_bloomFilterContents.Destroy();
			}

			void BloomFilter::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(pDevice, nullptr, nullptr, emPSType);
					CreateBundles(pDevice, emPSType);
				}
			}

			void BloomFilter::Impl::Apply(RenderTarget* pSource)
			{
				if (pSource == nullptr)
					return;

				const Options::BloomFilterConfig& bloomFilterConfig = RenderOptions().bloomFilterConfig;

				const D3D12_RESOURCE_DESC sourceDesc = pSource->GetDesc();

				const math::uint2 n2TargetSize(static_cast<uint32_t>(sourceDesc.Width) / 2, static_cast<uint32_t>(sourceDesc.Height) / 2);
				m_fRadiusMultiplier = static_cast<float>(sourceDesc.Width) / static_cast<float>(sourceDesc.Height);

				Device* pDeviceInstance = Device::GetInstance();
				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				ResetBloomFilterContents(frameIndex);
				SetBloomPreset(bloomFilterConfig.emPreset);

				const math::float2 f2InverseResolution(1.f / n2TargetSize.x, 1.f / n2TargetSize.y);

				shader::PSType emPSType = bloomFilterConfig.isEnableLuminance == true ? shader::eExtractLuminance : shader::eExtract;
				RenderTarget* pMip0 = Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, emPSType, true, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pSource);

				RenderTarget* pMip1 = Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, shader::eDownsample, false, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pMip0);
				RenderTarget* pMip2 = Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, shader::eDownsample, false, n2TargetSize.x, n2TargetSize.y, 1, f2InverseResolution, pMip1);
				RenderTarget* pMip3 = Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, shader::eDownsample, false, n2TargetSize.x, n2TargetSize.y, 2, f2InverseResolution, pMip2);
				RenderTarget* pMip4 = Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, shader::eDownsample, false, n2TargetSize.x, n2TargetSize.y, 3, f2InverseResolution, pMip3);
				RenderTarget* pMip5 = Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, shader::eDownsample, false, n2TargetSize.x, n2TargetSize.y, 4, f2InverseResolution, pMip4);

				emPSType = bloomFilterConfig.isEnableLuminance == true ? shader::eUpsampleLuminance : shader::eUpsample;
				pMip4 = Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, emPSType, false, n2TargetSize.x, n2TargetSize.y, 4, f2InverseResolution, pMip5, pMip4);
				pMip3 = Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, emPSType, false, n2TargetSize.x, n2TargetSize.y, 3, f2InverseResolution, pMip4, pMip3);
				pMip2 = Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, emPSType, false, n2TargetSize.x, n2TargetSize.y, 2, f2InverseResolution, pMip3, pMip2);
				pMip1 = Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, emPSType, false, n2TargetSize.x, n2TargetSize.y, 1, f2InverseResolution, pMip2, pMip1);
				pMip0 = Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, emPSType, false, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pMip1, pMip0);

				Sampling(pDeviceInstance, pCommandList, frameIndex, bloomFilterConfig, shader::eApply, true, n2TargetSize.x, n2TargetSize.y, 0, f2InverseResolution, pMip0, pSource);

				pDeviceInstance->ReleaseRenderTargets(&pMip0, 1);
				pDeviceInstance->ReleaseRenderTargets(&pMip1, 1);
				pDeviceInstance->ReleaseRenderTargets(&pMip2, 1);
				pDeviceInstance->ReleaseRenderTargets(&pMip3, 1);
				pDeviceInstance->ReleaseRenderTargets(&pMip4, 1);
				pDeviceInstance->ReleaseRenderTargets(&pMip5, 1);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			RenderTarget* BloomFilter::Impl::Sampling(Device* pDevice, ID3D12GraphicsCommandList2* pCommandList, uint32_t frameIndex, const Options::BloomFilterConfig& bloomFilterConfig, shader::PSType emPSType, bool isResult, uint32_t nWidth, uint32_t nHeight, int nPass, const math::float2& f2InverseResolution, RenderTarget* pSource, RenderTarget* pResult)
			{
				if (m_nDownsamplePasses > nPass)
				{
					const int nPassFactor = static_cast<int>(std::pow(2, nPass));

					if (pResult == nullptr)
					{
						D3D12_RESOURCE_DESC desc = pSource->GetDesc();

						desc.Width = nWidth / (nPassFactor * (isResult ? 1 : 2));
						desc.Height = nHeight / (nPassFactor * (isResult ? 1 : 2));

						pResult = pDevice->GetRenderTarget(&desc, math::Color::Transparent);
					}

					if (pResult->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
					{
						const D3D12_RESOURCE_BARRIER transition[] =
						{
							pResult->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET),
						};
						pCommandList->ResourceBarrier(_countof(transition), transition);
					}

					if (pSource->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
					{
						const D3D12_RESOURCE_BARRIER transition[] =
						{
							pSource->Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
						};
						pCommandList->ResourceBarrier(_countof(transition), transition);
					}

					shader::BloomFilterContents* pBloomFilterContents = AllocateBloomFilterContents(frameIndex);
					shader::SetBloomFilterContents(pBloomFilterContents,
						f2InverseResolution * static_cast<float>(nPassFactor),
						bloomFilterConfig.threshold,
						m_fRadius[nPass],
						m_fStrengths[nPass] * bloomFilterConfig.strengthMultiplier,
						m_fStreakLength,
						pSource->GetTexture()->GetDescriptorIndex());

					const D3D12_RESOURCE_DESC desc = pResult->GetDesc();

					D3D12_VIEWPORT viewport{};
					viewport.Width = static_cast<float>(desc.Width);
					viewport.Height = static_cast<float>(desc.Height);
					viewport.MinDepth = 0.f;
					viewport.MaxDepth = 1.f;
					pCommandList->RSSetViewports(1, &viewport);

					const CD3DX12_RECT rect = CD3DX12_RECT(0, 0, static_cast<uint32_t>(desc.Width), static_cast<uint32_t>(desc.Height));
					pCommandList->RSSetScissorRects(1, &rect); // no scissor for this

					const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
					{
						pResult->GetCPUHandle(),
					};
					pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

					pCommandList->SetGraphicsRootSignature(m_pipelineStates[emPSType].psoCache.pRootSignature);

					DescriptorHeap* pDescriptorHeapSRV = pDevice->GetSRVDescriptorHeap();
					ID3D12DescriptorHeap* pDescriptorHeaps[] =
					{
						pDescriptorHeapSRV->GetHeap(),
					};
					pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					pCommandList->SetGraphicsRootConstantBufferView(eRP_BloomFilterContents, m_bloomFilterContentsBufferGPUAddress);

					pCommandList->ExecuteBundle(m_pipelineStates[emPSType].pBundles[frameIndex]);

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

			ID3D12RootSignature* BloomFilter::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& bloomFilterContentsParameter = vecRootParameters.emplace_back();
				bloomFilterContentsParameter.InitAsConstantBufferView(shader::eCB_BloomFilterContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(SamplerState::eMinMagLinearMipPointClamp, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void BloomFilter::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType)
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
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, shader::GetBloomFilterPSTypeToString(emPSType), shader::PS_CompileVersion, &psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				const std::wstring wstrDebugName = string::MultiToWide(shader::GetBloomFilterPSTypeToString(emPSType));

				if (psoCache.pRootSignature == nullptr)
				{
					psoCache.pRootSignature = CreateRootSignature(pDevice);
					psoCache.pRootSignature->SetName(wstrDebugName.c_str());
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
				psoDesc.RasterizerState = util::GetRasterizerDesc(RasterizerState::eSolidCullNone);

				switch (emPSType)
				{
				case shader::eExtract:
				case shader::eExtractLuminance:
				case shader::eDownsample:
					psoDesc.BlendState = util::GetBlendDesc(BlendState::eOpacity);
					break;
				case shader::eUpsample:
				case shader::eUpsampleLuminance:
					psoDesc.BlendState = util::GetBlendDesc(BlendState::eAlphaBlend);
					break;
				case shader::eApply:
					psoDesc.BlendState = util::GetBlendDesc(BlendState::eAdditive);
					break;
				}

				psoDesc.NumRenderTargets = 1;

				if (RenderOptions().OnHDR == true)
				{
					psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
				}
				else
				{
					D3D12_RESOURCE_DESC desc = Device::GetInstance()->GetSwapChainRenderTarget(0)->GetDesc();
					psoDesc.RTVFormats[0] = desc.Format;
				}

				psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(DepthStencilState::eRead_Write_Off);

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoCache.pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				psoCache.pPipelineState->SetName(wstrDebugName.c_str());
			}

			void BloomFilter::Impl::CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType)
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

			BloomFilter::BloomFilter()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			BloomFilter::~BloomFilter()
			{
			}

			void BloomFilter::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void BloomFilter::Apply(RenderTarget* pSource)
			{
				m_pImpl->Apply(pSource);
			}
		}
	}
}