#include "stdafx.h"
#include "EnvironmentRendererDX12.h"

#include "CommonLib/FileUtil.h"

#include "GraphicsInterface/Camera.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"

#include "DescriptorHeapDX12.h"
#include "RenderTargetDX12.h"

#include "TextureDX12.h"
#include "VertexBufferDX12.h"
#include "IndexBufferDX12.h"

namespace eastengine
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
					math::Vector2 padding;
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

				void SetEnvironmentContents(EnvironmentContents* pEnvironmentContents, const Camera* pCamera, float fTextureGamma, uint32_t nTexEnvironmentMapIndex)
				{
					pEnvironmentContents->matInvView = pCamera->GetViewMatrix().Invert().Transpose();
					pEnvironmentContents->matProjection = pCamera->GetProjMatrix().Transpose();
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
				void Render(Camera* pCamera);
				void Flush();

			private:
				enum RootParameters : uint32_t
				{
					eRP_StandardDescriptor = 0,

					eRP_EnvironmentContents,

					eRP_Count,

					eRP_InvalidIndex = std::numeric_limits<uint32_t>::max(),
				};

				ID3D12RootSignature* CreateRootSignature(ID3D12Device* pDevice);
				void CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath);

			private:
				ID3D12PipelineState* m_pPipelineState{ nullptr };
				ID3D12RootSignature* m_pRootSignature{ nullptr };

				ConstantBuffer<shader::EnvironmentContents> m_environmentContents;
			};

			EnvironmentRenderer::Impl::Impl()
			{
				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("Environment\\Environment.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(String::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Environment.hlsl");
				}

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CreatePipelineState(pDevice, pShaderBlob, strShaderPath.c_str());

				m_environmentContents.Create(pDevice, 1, "EnvironmentContents");

				SafeRelease(pShaderBlob);
			}

			EnvironmentRenderer::Impl::~Impl()
			{
				m_environmentContents.Destroy();

				SafeRelease(m_pPipelineState);
				SafeRelease(m_pRootSignature);
			}

			void EnvironmentRenderer::Impl::Render(Camera* pCamera)
			{
				Device* pDeviceInstance = Device::GetInstance();

				const IImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();
				Texture* pEnvironmentHDR = static_cast<Texture*>(pImageBasedLight->GetEnvironmentHDR());
				if (pEnvironmentHDR == nullptr)
					return;

				VertexBuffer* pVertexBuffer = static_cast<VertexBuffer*>(pImageBasedLight->GetEnvironmentSphereVB());
				IndexBuffer* pIndexBuffer = static_cast<IndexBuffer*>(pImageBasedLight->GetEnvironmentSphereIB());
				if (pVertexBuffer == nullptr || pIndexBuffer == nullptr)
					return;

				int nFrameIndex = pDeviceInstance->GetFrameIndex();
				DescriptorHeap* pSRVDescriptorHeap = pDeviceInstance->GetSRVDescriptorHeap();

				const D3D12_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				const D3D12_RECT* pScissorRect = pDeviceInstance->GetScissorRect();

				D3D12_RESOURCE_DESC desc = pDeviceInstance->GetSwapChainRenderTarget(nFrameIndex)->GetDesc();

				if (GetOptions().OnHDR == true)
				{
					desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				}

				RenderTarget* pRenderTarget = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent, true);
				if (pRenderTarget == nullptr)
				{
					throw_line("failed to get render target");
				}

				const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
				{
					pRenderTarget->GetCPUHandle(),
				};

				shader::EnvironmentContents* pEnvironmentContents = m_environmentContents.Cast(nFrameIndex);
				shader::SetEnvironmentContents(pEnvironmentContents, pCamera, 1.f, pEnvironmentHDR->GetDescriptorIndex());

				ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
				pDeviceInstance->ResetCommandList(0, m_pPipelineState);

				pRenderTarget->Clear(pCommandList);

				pCommandList->SetGraphicsRootSignature(m_pRootSignature);

				ID3D12DescriptorHeap* pDescriptorHeaps[] =
				{
					pSRVDescriptorHeap->GetHeap(),
				};
				pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

				pCommandList->RSSetViewports(1, pViewport);
				pCommandList->RSSetScissorRects(1, pScissorRect);
				pCommandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, nullptr);

				pCommandList->SetGraphicsRootDescriptorTable(eRP_StandardDescriptor, pSRVDescriptorHeap->GetStartGPUHandle(nFrameIndex));
				pCommandList->SetGraphicsRootConstantBufferView(eRP_EnvironmentContents, m_environmentContents.GPUAddress(nFrameIndex));

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
				pDeviceInstance->ReleaseRenderTargets(&pRenderTarget);
			}

			void EnvironmentRenderer::Impl::Flush()
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
					util::GetStaticSamplerDesc(EmSamplerState::eAnisotropicWrap, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL),
				};

				return util::CreateRootSignature(pDevice, static_cast<uint32_t>(vecRootParameters.size()), vecRootParameters.data(),
					_countof(staticSamplerDesc), staticSamplerDesc,
					D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);
			}

			void EnvironmentRenderer::Impl::CreatePipelineState(ID3D12Device* pDevice, ID3DBlob* pShaderBlob, const char* strShaderPath)
			{
				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX12", "1" },
					{ nullptr, nullptr },
				};

				ID3DBlob* pVertexShaderBlob = nullptr;
				bool isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, "VS", "vs_5_1", &pVertexShaderBlob);
				if (isSuccess == false)
				{
					throw_line("failed to compile vertex shader");
				}

				ID3DBlob* pPixelShaderBlob = nullptr;
				isSuccess = util::CompileShader(pShaderBlob, macros, strShaderPath, "PS", "ps_5_1", &pPixelShaderBlob);
				if (isSuccess == false)
				{
					throw_line("failed to compile pixel shader");
				}

				D3D12_SHADER_BYTECODE vertexShaderBytecode{};
				vertexShaderBytecode.BytecodeLength = pVertexShaderBlob->GetBufferSize();
				vertexShaderBytecode.pShaderBytecode = pVertexShaderBlob->GetBufferPointer();

				D3D12_SHADER_BYTECODE pixelShaderBytecode{};
				pixelShaderBytecode.BytecodeLength = pPixelShaderBlob->GetBufferSize();
				pixelShaderBytecode.pShaderBytecode = pPixelShaderBlob->GetBufferPointer();

				const D3D12_INPUT_ELEMENT_DESC* pInputElements = nullptr;
				size_t nElementCount = 0;
				util::GetInputElementDesc(VertexPosTexNor::Format(), &pInputElements, &nElementCount);

				D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
				inputLayoutDesc.NumElements = static_cast<uint32_t>(nElementCount);
				inputLayoutDesc.pInputElementDescs = pInputElements;

				DXGI_SAMPLE_DESC sampleDesc{};
				sampleDesc.Count = 1;

				m_pRootSignature = CreateRootSignature(pDevice);
				m_pRootSignature->SetName(L"EnvironmentRenderer");

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
				psoDesc.InputLayout = inputLayoutDesc;
				psoDesc.pRootSignature = m_pRootSignature;
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

				HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));
				if (FAILED(hr))
				{
					throw_line("failed to create graphics pipeline state");
				}
				m_pPipelineState->SetName(L"EnvironmentRenderer");

				SafeRelease(pVertexShaderBlob);
				SafeRelease(pPixelShaderBlob);
			}

			EnvironmentRenderer::EnvironmentRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			EnvironmentRenderer::~EnvironmentRenderer()
			{
			}

			void EnvironmentRenderer::Render(Camera* pCamera)
			{
				m_pImpl->Render(pCamera);
			}

			void EnvironmentRenderer::Flush()
			{
				m_pImpl->Flush();
			}
		}
	}
}