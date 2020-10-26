#include "stdafx.h"
#include "FxaaDX12.h"

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
				struct FxaaContents
				{
					math::float4 f4RcpFrame;

					uint32_t nTexInputIndex{ 0 };
					math::float3 padding;
				};

				enum CBSlot
				{
					eCB_FxaaContents = 0,
				};

				void SetFxaaContents(FxaaContents* pFxaaContents, const math::uint2& n2ScreenSize, uint32_t nTexInputIndex)
				{
					const math::float4 vRcpFrame(1.f / static_cast<float>(n2ScreenSize.x), 1.f / static_cast<float>(n2ScreenSize.y), 0.f, 0.f);
					pFxaaContents->f4RcpFrame = vRcpFrame;
					pFxaaContents->nTexInputIndex = nTexInputIndex;
				}
			}

			class Fxaa::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);
				void Apply(RenderTarget* pSource, RenderTarget* pResult);

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_FxaaContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath);
				void CreateBundles(ID3D12Device* pDevice);

			private:
				PSOCache m_psoCache;

				std::array<ID3D12GraphicsCommandList2*, eFrameBufferCount> m_pBundles{ nullptr };

				ConstantBuffer<shader::FxaaContents> m_fxaaContents;
			};

			Fxaa::Impl::Impl()
			{
				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\PostProcessing\\FXAA\\FXAA.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : FXAA.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str());

				m_fxaaContents.Create(pDevice, 1, "FxaaContents");

				SafeRelease(pShaderBlob);

				CreateBundles(pDevice);
			}

			Fxaa::Impl::~Impl()
			{
				m_fxaaContents.Destroy();

				for (auto& pBundle : m_pBundles)
				{
					util::ReleaseResource(pBundle);
					pBundle = nullptr;
				}

				m_psoCache.Destroy();
			}

			void Fxaa::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
				CreatePipelineState(pDevice, nullptr, nullptr);
				CreateBundles(pDevice);
			}

			void Fxaa::Impl::Apply(RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				Device* pDeviceInstance = Device::GetInstance();

				const math::uint2& n2ScreenSize = pDeviceInstance->GetScreenSize();
				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
				{
					pResult->GetCPUHandle(),
				};

				{
					shader::FxaaContents* pFxaaContents = m_fxaaContents.Cast(frameIndex);
					shader::SetFxaaContents(pFxaaContents, n2ScreenSize, pSource->GetTexture()->GetDescriptorIndex());
				}

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, nullptr);

				util::ChangeResourceState(pCommandList, pSource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				util::ChangeResourceState(pCommandList, pResult, D3D12_RESOURCE_STATE_RENDER_TARGET);

				pCommandList->RSSetViewports(1, pViewport);
				pCommandList->RSSetScissorRects(1, pScissorRect);
				pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

				pCommandList->SetGraphicsRootSignature(m_psoCache.pRootSignature);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->ExecuteBundle(m_pBundles[frameIndex]);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			ID3D12RootSignature* Fxaa::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& fxaaContentsParameter = vecRootParameters.emplace_back();
				fxaaContentsParameter.InitAsConstantBufferView(shader::eCB_FxaaContents, 0, D3D12_SHADER_VISIBILITY_PIXEL);

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(EmSamplerState::eAnisotropicWrap, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void Fxaa::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath)
			{
				if (pShaderBlob != nullptr)
				{
					const D3D_SHADER_MACRO macros[] =
					{
						{ "DX12", "1" },
						{ nullptr, nullptr },
					};

					if (m_psoCache.pVSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, "VS", shader::VS_CompileVersion, &m_psoCache.pVSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile vertex shader");
						}
					}

					if (m_psoCache.pPSBlob == nullptr)
					{
						const bool isSuccess = util::CompileShader(pShaderBlob, macros, shaderPath, "PS", shader::PS_CompileVersion, &m_psoCache.pPSBlob);
						if (isSuccess == false)
						{
							throw_line("failed to compile pixel shader");
						}
					}
				}

				if (m_psoCache.pRootSignature == nullptr)
				{
					m_psoCache.pRootSignature = CreateRootSignature(pDevice);
					m_psoCache.pRootSignature->SetName(L"FXAA");
				}

				if (m_psoCache.pPipelineState != nullptr)
				{
					util::ReleaseResource(m_psoCache.pPipelineState);
					m_psoCache.pPipelineState = nullptr;
				}

				D3D12_SHADER_BYTECODE vertexShaderBytecode{};
				vertexShaderBytecode.BytecodeLength = m_psoCache.pVSBlob->GetBufferSize();
				vertexShaderBytecode.pShaderBytecode = m_psoCache.pVSBlob->GetBufferPointer();

				D3D12_SHADER_BYTECODE pixelShaderBytecode{};
				pixelShaderBytecode.BytecodeLength = m_psoCache.pPSBlob->GetBufferSize();
				pixelShaderBytecode.pShaderBytecode = m_psoCache.pPSBlob->GetBufferPointer();

				DXGI_SAMPLE_DESC sampleDesc{};
				sampleDesc.Count = 1;

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.pRootSignature = m_psoCache.pRootSignature;
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

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psoCache.pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				m_psoCache.pPipelineState->SetName(L"FXAA");
			}

			void Fxaa::Impl::CreateBundles(ID3D12Device* pDevice)
			{
				DescriptorHeap* pSRVDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();

				for (int i = 0; i < eFrameBufferCount; ++i)
				{
					if (m_pBundles[i] != nullptr)
					{
						util::ReleaseResource(m_pBundles[i]);
						m_pBundles[i] = nullptr;
					}

					m_pBundles[i] = Device::GetInstance()->CreateBundle(m_psoCache.pPipelineState);

					m_pBundles[i]->SetGraphicsRootSignature(m_psoCache.pRootSignature);

					ID3D12DescriptorHeap* pDescriptorHeaps[] =
					{
						pSRVDescriptorHeap->GetHeap(),
					};
					m_pBundles[i]->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

					m_pBundles[i]->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pSRVDescriptorHeap->GetStartGPUHandle());
					m_pBundles[i]->SetGraphicsRootConstantBufferView(eRP_FxaaContents, m_fxaaContents.GPUAddress(i));

					m_pBundles[i]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
					m_pBundles[i]->IASetVertexBuffers(0, 0, nullptr);
					m_pBundles[i]->IASetIndexBuffer(nullptr);

					m_pBundles[i]->DrawInstanced(4, 1, 0, 0);

					HRESULT hr = m_pBundles[i]->Close();
					if (FAILED(hr))
					{
						throw_line("failed to close bundle");
					}
				}
			}

			Fxaa::Fxaa()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			Fxaa::~Fxaa()
			{
			}

			void Fxaa::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void Fxaa::Apply(RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Apply(pSource, pResult);
			}
		}
	}
}