#include "stdafx.h"
#include "MotionBlurDX12.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"

#include "DescriptorHeapDX12.h"
#include "RenderTargetDX12.h"
#include "DepthStencilDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				struct MotionBlur_PSConstants
				{
					float blurAmount{ 1.f };
					math::float3 padding;

					math::float2 SourceDimensions;
					math::float2 DestinationDimensions;

					uint32_t texColorIndex{ 0 };
					uint32_t texDepthIndex{ 0 };

					uint32_t texVelocityIndex{ 0 };
					uint32_t texPrevVelocityIndex{ 0 };
				};

				struct DepthMotionBlur_PSConstants
				{
					math::Matrix matInvView;
					math::Matrix matInvProj;
					math::Matrix matLastViewProj;
				};

				enum CBSlot
				{
					eCB_MotionBlur_PSConstants = 0,
					eCB_DepthMotionBlur_PSConstants = 1,
				};

				enum SamplerSlot
				{
					eSampler_Point = 0,
				};

				enum PSType
				{
					eDepthBuffer4SamplesPS = 0,
					eDepthBuffer8SamplesPS,
					eDepthBuffer12SamplesPS,

					eVelocityBuffer4SamplesPS,
					eVelocityBuffer8SamplesPS,
					eVelocityBuffer12SamplesPS,

					eDualVelocityBuffer4SamplesPS,
					eDualVelocityBuffer8SamplesPS,
					eDualVelocityBuffer12SamplesPS,

					ePS_Count,
				};
				static_assert(eDepthBuffer4SamplesPS == Options::MotionBlurConfig::eDepthBuffer_4Samples, "mismatch MotionBlur And Options");
				static_assert(eDepthBuffer8SamplesPS == Options::MotionBlurConfig::eDepthBuffer_8Samples, "mismatch MotionBlur And Options");
				static_assert(eDepthBuffer12SamplesPS == Options::MotionBlurConfig::eDepthBuffer_12Samples, "mismatch MotionBlur And Options");
				static_assert(eVelocityBuffer4SamplesPS == Options::MotionBlurConfig::eVelocityBuffer_4Samples, "mismatch MotionBlur And Options");
				static_assert(eVelocityBuffer8SamplesPS == Options::MotionBlurConfig::eVelocityBuffer_8Samples, "mismatch MotionBlur And Options");
				static_assert(eVelocityBuffer12SamplesPS == Options::MotionBlurConfig::eVelocityBuffer_12Samples, "mismatch MotionBlur And Options");
				static_assert(eDualVelocityBuffer4SamplesPS == Options::MotionBlurConfig::eDualVelocityBuffer_4Samples, "mismatch MotionBlur And Options");
				static_assert(eDualVelocityBuffer8SamplesPS == Options::MotionBlurConfig::eDualVelocityBuffer_8Samples, "mismatch MotionBlur And Options");
				static_assert(eDualVelocityBuffer12SamplesPS == Options::MotionBlurConfig::eDualVelocityBuffer_12Samples, "mismatch MotionBlur And Options");

				const char* GetMotionBlurPSTypeToString(PSType emPSType)
				{
					switch (emPSType)
					{
					case eDepthBuffer4SamplesPS:
						return "DepthBuffer4SamplesPS";
					case eDepthBuffer8SamplesPS:
						return "DepthBuffer8SamplesPS";
					case eDepthBuffer12SamplesPS:
						return "DepthBuffer12SamplesPS";
					case eVelocityBuffer4SamplesPS:
						return "VelocityBuffer4SamplesPS";
					case eVelocityBuffer8SamplesPS:
						return "VelocityBuffer8SamplesPS";
					case eVelocityBuffer12SamplesPS:
						return "VelocityBuffer12SamplesPS";
					case eDualVelocityBuffer4SamplesPS:
						return "DualVelocityBuffer4SamplesPS";
					case eDualVelocityBuffer8SamplesPS:
						return "DualVelocityBuffer8SamplesPS";
					case eDualVelocityBuffer12SamplesPS:
						return "DualVelocityBuffer12SamplesPS";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetMotionBlur_PSConstants(MotionBlur_PSConstants* pPSConstants,
					float blurAmount, const math::float2& sourceDimensions, const math::float2& destinationDimensions,
					uint32_t texColorIndex, uint32_t texDepthIndex, uint32_t texVelocityIndex, uint32_t texPrevVelocityIndex)
				{
					pPSConstants->blurAmount = blurAmount;

					pPSConstants->SourceDimensions = sourceDimensions;
					pPSConstants->DestinationDimensions = destinationDimensions;

					pPSConstants->texColorIndex = texColorIndex;
					pPSConstants->texDepthIndex = texDepthIndex;
					pPSConstants->texVelocityIndex = texVelocityIndex;
					pPSConstants->texPrevVelocityIndex = texPrevVelocityIndex;
				}

				void SetDepthMotionBlur_PSConstants(DepthMotionBlur_PSConstants* pPSConstants,
					const math::Matrix& matInvView, const math::Matrix& matInvProj, const math::Matrix& matLastViewProj)
				{
					pPSConstants->matInvView = matInvView.Transpose();
					pPSConstants->matInvProj = matInvProj.Transpose();
					pPSConstants->matLastViewProj = matLastViewProj.Transpose();
				}
			}

			class MotionBlur::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);
				void Apply(Camera* pCamera, const math::Matrix& matPrevViewProj, RenderTarget* pSource, DepthStencil* pDepth, RenderTarget* pResult);
				void Apply(Camera* pCamera, RenderTarget* pSource, RenderTarget* pVelocity, RenderTarget* pResult);
				void Apply(Camera* pCamera, RenderTarget* pSource, RenderTarget* pVelocity, RenderTarget* pPrevVelocity, RenderTarget* pResult);

			private:
				void CommonReady(ID3D12GraphicsCommandList2* pCommandList, uint32_t frameIndex, RenderTarget* pSource, DepthStencil* pDepth, RenderTarget* pVelocity, RenderTarget* pPrevVelocity, RenderTarget* pResult);
				void CommonDraw(ID3D12GraphicsCommandList2* pCommandList,DescriptorHeap* pSRVDescriptorHeap, uint32_t frameIndex, shader::PSType emPSType, RenderTarget* pResult);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_MotionBlurContents,
					eRP_DepthMotionBlurContents,

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

				ConstantBuffer<shader::MotionBlur_PSConstants> m_psContents;
				ConstantBuffer<shader::DepthMotionBlur_PSConstants> m_psContents_depth;
			};

			MotionBlur::Impl::Impl()
			{
				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\PostProcessing\\MotionBlur\\MotionBlur.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : MotionBlur.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str(), emPSType);
				}

				m_psContents.Create(pDevice, 1, "MotionBlur_PSConstants");
				m_psContents_depth.Create(pDevice, 1, "DepthMotionBlur_PSConstants");

				SafeRelease(pShaderBlob);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreateBundles(pDevice, emPSType);
				}
			}

			MotionBlur::Impl::~Impl()
			{
				m_psContents.Destroy();
				m_psContents_depth.Destroy();

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

			void MotionBlur::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(pDevice, nullptr, nullptr, emPSType);
				}

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreateBundles(pDevice, emPSType);
				}
			}

			void MotionBlur::Impl::Apply(Camera* pCamera, const math::Matrix& matPrevViewProj, RenderTarget* pSource, DepthStencil* pDepth, RenderTarget* pResult)
			{
				if (pSource == nullptr || pDepth == nullptr || pResult == nullptr)
					return;

				const Options::MotionBlurConfig& motionBlurConfig = GetOptions().motionBlurConfig;
				Device* pDeviceInstance = Device::GetInstance();

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				CommonReady(pCommandList, frameIndex, pSource, pDepth, nullptr, nullptr, pResult);

				shader::SetDepthMotionBlur_PSConstants(m_psContents_depth.Cast(frameIndex),
					pCamera->GetViewMatrix().Invert(), pCamera->GetProjectionMatrix().Invert(), matPrevViewProj);

				CommonDraw(pCommandList, pSRVDescriptorHeap, frameIndex, static_cast<shader::PSType>(motionBlurConfig.emMode), pResult);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}
				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			void MotionBlur::Impl::Apply(Camera* pCamera, RenderTarget* pSource, RenderTarget* pVelocity, RenderTarget* pResult)
			{
				if (pSource == nullptr || pVelocity == nullptr || pResult == nullptr)
					return;

				const Options::MotionBlurConfig& motionBlurConfig = GetOptions().motionBlurConfig;
				Device* pDeviceInstance = Device::GetInstance();

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				CommonReady(pCommandList, frameIndex, pSource, nullptr, pVelocity, nullptr, pResult);
				CommonDraw(pCommandList, pSRVDescriptorHeap, frameIndex, static_cast<shader::PSType>(motionBlurConfig.emMode), pResult);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}
				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			void MotionBlur::Impl::Apply(Camera* pCamera, RenderTarget* pSource, RenderTarget* pVelocity, RenderTarget* pPrevVelocity, RenderTarget* pResult)
			{
				if (pSource == nullptr || pVelocity == nullptr || pPrevVelocity == nullptr || pResult == nullptr)
					return;

				const Options::MotionBlurConfig& motionBlurConfig = GetOptions().motionBlurConfig;
				Device* pDeviceInstance = Device::GetInstance();

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				CommonReady(pCommandList, frameIndex, pSource, nullptr, pVelocity, pPrevVelocity, pResult);
				CommonDraw(pCommandList, pSRVDescriptorHeap, frameIndex, static_cast<shader::PSType>(motionBlurConfig.emMode), pResult);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}
				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			void MotionBlur::Impl::CommonReady(ID3D12GraphicsCommandList2* pCommandList, uint32_t frameIndex, RenderTarget* pSource, DepthStencil* pDepth, RenderTarget* pVelocity, RenderTarget* pPrevVelocity, RenderTarget* pResult)
			{
				const D3D12_RESOURCE_DESC sourceDesc = pSource->GetDesc();
				const math::float2 sourceDimensions{ static_cast<float>(sourceDesc.Width), static_cast<float>(sourceDesc.Height) };

				const D3D12_RESOURCE_DESC destinationDesc = pResult->GetDesc();
				const math::float2 destinationDimensions{ static_cast<float>(destinationDesc.Width), static_cast<float>(destinationDesc.Height) };

				const Options::MotionBlurConfig& motionBlurConfig = GetOptions().motionBlurConfig;

				const uint32_t depthDescriptorIndex = pDepth != nullptr ? pDepth->GetTexture()->GetDescriptorIndex() : eInvalidDescriptorIndex;
				const uint32_t velocityDescriptorIndex = pVelocity != nullptr ? pVelocity->GetTexture()->GetDescriptorIndex() : eInvalidDescriptorIndex;
				const uint32_t prevVelocityDescriptorIndex = pPrevVelocity != nullptr ? pPrevVelocity->GetTexture()->GetDescriptorIndex() : eInvalidDescriptorIndex;

				shader::SetMotionBlur_PSConstants(m_psContents.Cast(frameIndex),
					motionBlurConfig.blurAmount, sourceDimensions, destinationDimensions,
					pSource->GetTexture()->GetDescriptorIndex(),
					depthDescriptorIndex, velocityDescriptorIndex, prevVelocityDescriptorIndex);

				util::ChangeResourceState(pCommandList, pResult, D3D12_RESOURCE_STATE_RENDER_TARGET);
				util::ChangeResourceState(pCommandList, pSource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

				if (pVelocity != nullptr)
				{
					util::ChangeResourceState(pCommandList, pVelocity, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				if (pPrevVelocity != nullptr)
				{
					util::ChangeResourceState(pCommandList, pPrevVelocity, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}
			}

			void MotionBlur::Impl::CommonDraw(ID3D12GraphicsCommandList2* pCommandList, DescriptorHeap* pSRVDescriptorHeap, uint32_t frameIndex, shader::PSType emPSType, RenderTarget* pResult)
			{
				const D3D12_RESOURCE_DESC desc = pResult->GetDesc();

				D3D12_VIEWPORT viewport{};
				viewport.TopLeftX = 0.f;
				viewport.TopLeftY = 0.f;
				viewport.Width = static_cast<float>(desc.Width);
				viewport.Height = static_cast<float>(desc.Height);
				viewport.MinDepth = 0.f;
				viewport.MaxDepth = 1.f;
				pCommandList->RSSetViewports(1, &viewport);

				D3D12_RECT scissorRect{};
				scissorRect.left = 0;
				scissorRect.top = 0;
				scissorRect.right = static_cast<long>(desc.Width);
				scissorRect.bottom = static_cast<long>(desc.Height);
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
				pCommandList->SetGraphicsRootConstantBufferView(eRP_MotionBlurContents, m_psContents.GPUAddress(frameIndex));

				if (shader::eDepthBuffer4SamplesPS <= emPSType && emPSType <= shader::eDepthBuffer12SamplesPS)
				{
					pCommandList->SetGraphicsRootConstantBufferView(eRP_DepthMotionBlurContents, m_psContents_depth.GPUAddress(frameIndex));
				}

				pCommandList->ExecuteBundle(m_pipelineStates[emPSType].pBundles[frameIndex]);
			}

			ID3D12RootSignature* MotionBlur::Impl::CreateRootSignature(ID3D12Device* pDevice, shader::PSType emPSType)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& psContentParameter = vecRootParameters.emplace_back();
				psContentParameter.InitAsConstantBufferView(shader::eCB_MotionBlur_PSConstants, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				if (shader::eDepthBuffer4SamplesPS <= emPSType && emPSType <= shader::eDualVelocityBuffer12SamplesPS)
				{
					CD3DX12_ROOT_PARAMETER& depthContentsParameter = vecRootParameters.emplace_back();
					depthContentsParameter.InitAsConstantBufferView(shader::eCB_DepthMotionBlur_PSConstants, 0, D3D12_SHADER_VISIBILITY_PIXEL);
				}

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(SamplerState::eMinMagMipPointClamp, shader::eSampler_Point, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void MotionBlur::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath, shader::PSType emPSType)
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
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, shader::GetMotionBlurPSTypeToString(emPSType), shader::PS_CompileVersion, &psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				const std::wstring wstrDebugName = string::MultiToWide(shader::GetMotionBlurPSTypeToString(emPSType));

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
				psoDesc.RasterizerState = util::GetRasterizerDesc(RasterizerState::eSolidCullNone);
				psoDesc.BlendState = util::GetBlendDesc(BlendState::eOff);

				psoDesc.NumRenderTargets = 1;

				if (GetOptions().OnHDR == true)
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

			void MotionBlur::Impl::CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType)
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
					pBundles->SetGraphicsRootConstantBufferView(eRP_MotionBlurContents, m_psContents.GPUAddress(i));

					if (shader::eDepthBuffer4SamplesPS <= emPSType && emPSType <= shader::eDepthBuffer12SamplesPS)
					{
						pBundles->SetGraphicsRootConstantBufferView(eRP_DepthMotionBlurContents, m_psContents_depth.GPUAddress(i));
					}

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

			MotionBlur::MotionBlur()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			MotionBlur::~MotionBlur()
			{
			}

			void MotionBlur::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void MotionBlur::Apply(Camera* pCamera, const math::Matrix& matPrevViewProj, RenderTarget* pSource, DepthStencil* pDepth, RenderTarget* pResult)
			{
				m_pImpl->Apply(pCamera, matPrevViewProj, pSource, pDepth, pResult);
			}

			void MotionBlur::Apply(Camera* pCamera, RenderTarget* pSource, RenderTarget* pVelocity, RenderTarget* pResult)
			{
				m_pImpl->Apply(pCamera, pSource, pVelocity, pResult);
			}

			void MotionBlur::Apply(Camera* pCamera, RenderTarget* pSource, RenderTarget* pVelocity, RenderTarget* pPrevVelocity, RenderTarget* pResult)
			{
				m_pImpl->Apply(pCamera, pSource, pVelocity, pPrevVelocity, pResult);
			}
		}
	}
}