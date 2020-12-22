#include "stdafx.h"
#include "EnvironmentRendererDX12.h"

#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/Camera.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"

#include "DescriptorHeapDX12.h"
#include "RenderTargetDX12.h"

#include "TextureDX12.h"
#include "VertexBufferDX12.h"
#include "IndexBufferDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			namespace shader
			{
				struct EnvironmentContents
				{
					math::Matrix matInvView;
					math::Matrix matProjection;
					float fTextureGamma{ 1.f };
					uint32_t nTexEnvironmentMapIndex{ 0 };
					math::float2 padding;
				};

				enum CBSlot
				{
					eCB_EnvironmentContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_Anisotropic = 0,
				};

				enum SRVSlot
				{
					eSRV_EnvironmentMap = 0,
				};

				void SetEnvironmentContents(EnvironmentContents* pEnvironmentContents, Camera* pCamera, float fTextureGamma, uint32_t nTexEnvironmentMapIndex)
				{
					pEnvironmentContents->matInvView = pCamera->GetViewMatrix().Invert().Transpose();
					pEnvironmentContents->matProjection = pCamera->GetProjectionMatrix().Transpose();
					pEnvironmentContents->fTextureGamma = fTextureGamma;
					pEnvironmentContents->nTexEnvironmentMapIndex = nTexEnvironmentMapIndex;
					pEnvironmentContents->padding = {};
				}
			}

			class EnvironmentRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void RefreshPSO(ID3D12Device* pDevice);
				void Render(const RenderElement& renderElement);
				void Cleanup();

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_EnvironmentContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath);

			private:
				PSOCache m_psoCache;

				ConstantBuffer<shader::EnvironmentContents> m_environmentContents;
			};

			EnvironmentRenderer::Impl::Impl()
			{
				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\Environment\\Environment.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Environment.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CreatePipelineState(pDevice, pShaderBlob, shaderPath.c_str());

				m_environmentContents.Create(pDevice, 1, "EnvironmentContents");

				SafeRelease(pShaderBlob);
			}

			EnvironmentRenderer::Impl::~Impl()
			{
				m_environmentContents.Destroy();

				m_psoCache.Destroy();
			}

			void EnvironmentRenderer::Impl::RefreshPSO(ID3D12Device* pDevice)
			{
				CreatePipelineState(pDevice, nullptr, nullptr);
			}

			void EnvironmentRenderer::Impl::Render(const RenderElement& renderElement)
			{
				TRACER_EVENT(__FUNCTIONW__);
				Device* pDeviceInstance = Device::GetInstance();

				const IImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();
				Texture* pEnvironmentHDR = static_cast<Texture*>(pImageBasedLight->GetEnvironmentHDR().get());
				if (pEnvironmentHDR == nullptr)
					return;

				VertexBuffer* pVertexBuffer = static_cast<VertexBuffer*>(pImageBasedLight->GetEnvironmentSphereVB().get());
				IndexBuffer* pIndexBuffer = static_cast<IndexBuffer*>(pImageBasedLight->GetEnvironmentSphereIB().get());
				if (pVertexBuffer == nullptr || pIndexBuffer == nullptr)
					return;

				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				const math::Viewport& viewport= pDeviceInstance->GetViewport();
				const math::Rect& scissorRect = pDeviceInstance->GetScissorRect();

				shader::EnvironmentContents* pEnvironmentContents = m_environmentContents.Cast(frameIndex);
				shader::SetEnvironmentContents(pEnvironmentContents, renderElement.pCamera, 1.f, pEnvironmentHDR->GetDescriptorIndex());

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, m_psoCache.pPipelineState);

				pCommandList->SetGraphicsRootSignature(m_psoCache.pRootSignature);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->RSSetViewports(1, util::Convert(viewport));
				pCommandList->RSSetScissorRects(1, &scissorRect);

				pCommandList->OMSetRenderTargets(renderElement.rtvCount, renderElement.rtvHandles, FALSE, renderElement.GetDSVHandle());

				pCommandList->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pSRVDescriptorHeap->GetStartGPUHandle());
				pCommandList->SetGraphicsRootConstantBufferView(eRP_EnvironmentContents, m_environmentContents.GPUAddress(frameIndex));

				pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				pCommandList->IASetVertexBuffers(0, 1, pVertexBuffer->GetView());
				pCommandList->IASetIndexBuffer(pIndexBuffer->GetView());

				pCommandList->DrawIndexedInstanced(pIndexBuffer->GetIndexCount(), 1, 0, 0, 0);

				HRESULT hr = pCommandList->Close();
				if (FAILED(hr))
				{
					throw_line("failed to close command list");
				}

				pDeviceInstance->ExecuteCommandList(pCommandList);
			}

			void EnvironmentRenderer::Impl::Cleanup()
			{
			}

			ID3D12RootSignature* EnvironmentRenderer::Impl::CreateRootSignature(ID3D12Device* pDevice)
			{
				std::vector<CD3DX12_ROOT_PARAMETER> vecRootParameters;
				CD3DX12_ROOT_PARAMETER& standardDescriptorTable = vecRootParameters.emplace_back();
				standardDescriptorTable.InitAsDescriptorTable(eStandardDescriptorRangesCount_SRV, Device::GetInstance()->GetStandardDescriptorRanges(), D3D12_SHADER_VISIBILITY_PIXEL);

				CD3DX12_ROOT_PARAMETER& enviromentContentsParameter = vecRootParameters.emplace_back();
				enviromentContentsParameter.InitAsConstantBufferView(shader::eCB_EnvironmentContents);

				const D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[]
				{
					util::GetStaticSamplerDesc(SamplerState::eAnisotropicWrap, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void EnvironmentRenderer::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const wchar_t* shaderPath)
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
					m_psoCache.pRootSignature->SetName(L"EnvironmentRenderer");
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

				const D3D12_INPUT_ELEMENT_DESC* pInputElements = nullptr;
				size_t nElementCount = 0;
				util::GetInputElementDesc(VertexPosTexNor::Format(), &pInputElements, &nElementCount);

				D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
				inputLayoutDesc.NumElements = static_cast<uint32_t>(nElementCount);
				inputLayoutDesc.pInputElementDescs = pInputElements;

				DXGI_SAMPLE_DESC sampleDesc{};
				sampleDesc.Count = 1;

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.InputLayout = inputLayoutDesc;
				psoDesc.pRootSignature = m_psoCache.pRootSignature;
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

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psoCache.pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				m_psoCache.pPipelineState->SetName(L"EnvironmentRenderer");
			}

			EnvironmentRenderer::EnvironmentRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			EnvironmentRenderer::~EnvironmentRenderer()
			{
			}

			void EnvironmentRenderer::RefreshPSO(ID3D12Device* pDevice)
			{
				m_pImpl->RefreshPSO(pDevice);
			}

			void EnvironmentRenderer::Render(const RenderElement& renderElement)
			{
				m_pImpl->Render(renderElement);
			}

			void EnvironmentRenderer::Cleanup()
			{
				m_pImpl->Cleanup();
			}
		}
	}
}