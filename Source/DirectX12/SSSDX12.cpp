#include "stdafx.h"
#include "SSSDX12.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "DescriptorHeapDX12.h"

#include "RenderTargetDX12.h"
#include "DepthStencilDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				struct SSSContents
				{
					float sssWidth{ 1.f };

					uint32_t nTexColorIndex{ 0 };
					uint32_t nTexDepthIndex{ 0 };
					float padding{ 0 };
				};

				enum CBSlot
				{
					eCB_SSSContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_LinearPointClamp = 0,
					eSampler_PointClamp = 1,
				};

				enum PSType
				{
					eHorizontal = 0,
					eVertical,

					ePS_Count,
				};

				const char* GetSSSPSTypeToString(PSType emPSType)
				{
					switch (emPSType)
					{
					case eHorizontal:
						return "PS_Horizontal";
					case eVertical:
						return "PS_Vertical";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetSSSContents(SSSContents* pSSSContents, float sssWidth, uint32_t nTexColorIndex, uint32_t nTexDepthIndex)
				{
					pSSSContents->sssWidth = sssWidth;
					pSSSContents->nTexColorIndex = nTexColorIndex;
					pSSSContents->nTexDepthIndex = nTexDepthIndex;

					pSSSContents->padding = {};
				}
			}

			class SSS::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);
				void Apply(RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_SSSContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath, shader::PSType emPSType);
				void CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType);

			private:
				struct RenderPipeline
				{
					PSOCache psoCache;

					std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> pBundles{ nullptr };
				};
				std::array<RenderPipeline, shader::ePS_Count> m_pipelineStates;

				ConstantBuffer<shader::SSSContents> m_sssContents;
			};

			SSS::Impl::Impl()
			{
				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\SSS\\SSS.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(string::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : SSS.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(pDevice, pShaderBlob, strShaderPath.c_str(), emPSType);
				}

				m_sssContents.Create(pDevice, 1, "SSSContents");

				SafeRelease(pShaderBlob);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreateBundles(pDevice, emPSType);
				}
			}

			SSS::Impl::~Impl()
			{
				m_sssContents.Destroy();

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

			void SSS::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					const shader::PSType emPSType = static_cast<shader::PSType>(i);
					CreatePipelineState(pDevice, nullptr, nullptr, emPSType);
					CreateBundles(pDevice, emPSType);
				}
			}

			void SSS::Impl::Apply(RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				const uint32_t nFrameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
				{
					pResult->GetCPUHandle(),
				};

				{
					const Options& options = GetOptions();
					const Options::SSSConfig& config = options.sssConfig;

					shader::SSSContents* pSSSContents = m_sssContents.Cast(nFrameIndex);
					shader::SetSSSContents(pSSSContents, config.fWidth, pSource->GetTexture()->GetDescriptorIndex(), pDepth->GetTexture()->GetDescriptorIndex());
				}

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				util::ChangeResourceState(pCommandList, pResult, D3D12_RESOURCE_STATE_RENDER_TARGET);
				util::ChangeResourceState(pCommandList, pSource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				pResult->Clear(pCommandList);

				pCommandList->RSSetViewports(1, pViewport);
				pCommandList->RSSetScissorRects(1, pScissorRect);
				pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->SetGraphicsRootSignature(m_pipelineStates[shader::eHorizontal].psoCache.pRootSignature);
				pCommandList->ExecuteBundle(m_pipelineStates[shader::eHorizontal].pBundles[nFrameIndex]);

				pCommandList->SetGraphicsRootSignature(m_pipelineStates[shader::eVertical].psoCache.pRootSignature);
				pCommandList->ExecuteBundle(m_pipelineStates[shader::eVertical].pBundles[nFrameIndex]);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}
				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			ID3D12RootSignature* SSS::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& sssContentsParameter = vecRootParameters.emplace_back();
				sssContentsParameter.InitAsConstantBufferView(shader::eCB_SSSContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagLinearMipPointClamp, shader::eSampler_LinearPointClamp, 100, D3D12_SHADER_VISIBILITY_PIXEL),
					util::GetStaticSamplerDesc(EmSamplerState::eMinMagMipPointClamp, shader::eSampler_PointClamp, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void SSS::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath, shader::PSType emPSType)
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
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, "SSSSBlurVS", shader::VS_CompileVersion, &psoCache.pVSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile vertex shader");
						}
					}

					if (psoCache.pPSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, shader::GetSSSPSTypeToString(emPSType), shader::PS_CompileVersion, &psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				const std::wstring wstrDebugName = string::MultiToWide(shader::GetSSSPSTypeToString(emPSType));

				if (psoCache.pRootSignature == nullptr)
				{
					psoCache.pRootSignature = CreateRootSignature(pDevice);
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
				psoDesc.DepthStencilState = util::GetDepthStencilDesc(EmDepthStencilState::eRead_Write_Off);

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoCache.pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				psoCache.pPipelineState->SetName(wstrDebugName.c_str());
			}

			void SSS::Impl::CreateBundles(ID3D12Device* pDevice, shader::PSType emPSType)
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
						pDescriptorHeapSRV->GetHeap(i),
					};
					pBundles->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					pBundles->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pDescriptorHeapSRV->GetStartGPUHandle(i));

					pBundles->SetGraphicsRootConstantBufferView(eRP_SSSContents, m_sssContents.GPUAddress(i));

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

			SSS::SSS()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			SSS::~SSS()
			{
			}

			void SSS::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void SSS::Apply(RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				m_pImpl->Apply(pSource, pDepth, pResult);
			}
		}
	}
}
