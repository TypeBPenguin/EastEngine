#include "stdafx.h"
#include "UtilDX11.h"

#include "DeviceDX11.h"
#include "TextureDX11.h"

#include "RenderManagerDX11.h"
#include "GaussianBlurDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			namespace postprocess
			{
				namespace gaussianblur
				{
					void Apply(const RenderTarget* pSource, RenderTarget* pResult, float sigma)
					{
						GaussianBlur* pGaussianBlur = static_cast<GaussianBlur*>(Device::GetInstance()->GetRenderManager()->GetRenderer(IRenderer::eGaussianBlur));
						if (pGaussianBlur != nullptr)
						{
							pGaussianBlur->Apply(pSource, pResult, sigma);
						}
					}

					void Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult, float sigma)
					{
						GaussianBlur* pGaussianBlur = static_cast<GaussianBlur*>(Device::GetInstance()->GetRenderManager()->GetRenderer(IRenderer::eGaussianBlur));
						if (pGaussianBlur != nullptr)
						{
							pGaussianBlur->Apply(pSource, pDepth, pResult, sigma);
						}
					}
				}
			}

			namespace shader
			{
				bool IsValidTexture(const IMaterial* pMaterial, IMaterial::Type emType)
				{
					if (pMaterial == nullptr)
						return false;

					TexturePtr pTexture = pMaterial->GetTexture(emType);
					if (pTexture == nullptr)
						return false;

					return pTexture->GetState() == IResource::eComplete;
				}

				uint32_t GetMaterialMask(const IMaterial* pMaterial)
				{
					uint32_t mask = 0;
					if (pMaterial != nullptr)
					{
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eAlbedo) ? shader::eUseTexAlbedo : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eMask) ? shader::eUseTexMask : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eNormal) ? shader::eUseTexNormal : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eRoughness) ? shader::eUseTexRoughness : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eMetallic) ? shader::eUseTexMetallic : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eEmissive) ? shader::eUseTexEmissive : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eEmissiveColor) ? shader::eUseTexEmissiveColor : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eSubsurface) ? shader::eUseTexSubsurface : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eSpecular) ? shader::eUseTexSpecular : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eSpecularTint) ? shader::eUseTexSpecularTint : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eAnisotropic) ? shader::eUseTexAnisotropic : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eSheen) ? shader::eUseTexSheen : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eSheenTint) ? shader::eUseTexSheenTint : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eClearcoat) ? shader::eUseTexClearcoat : 0;
						mask |= shader::IsValidTexture(pMaterial, IMaterial::eClearcoatGloss) ? shader::eUseTexClearcoatGloss : 0;
					}
					return mask;
				}

				void SetPSSRV(ID3D11DeviceContext* pDeviceContext, const IMaterial* pMaterial, IMaterial::Type emType, uint32_t slot)
				{
					if (IsValidTexture(pMaterial, emType) == true)
					{
						Texture* pTexture = static_cast<Texture*>(pMaterial->GetTexture(emType).get());
						ID3D11ShaderResourceView* pSRVs[] =
						{
							pTexture->GetShaderResourceView(),
						};
						pDeviceContext->PSSetShaderResources(slot, _countof(pSRVs), pSRVs);
					}
					else
					{
						pDeviceContext->PSSetShaderResources(slot, 0, nullptr);
					}
				}

				void SetMaterial(ID3D11DeviceContext* pDeviceContext, const IMaterial* pMaterial, uint32_t materialSamplerSlot, bool isAlphaPrePass)
				{
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eAlbedo, 0);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eMask, 1);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eNormal, 2);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eRoughness, 3);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eMetallic, 4);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eEmissive, 5);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eEmissiveColor, 6);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eSubsurface, 7);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eSpecular, 8);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eSpecularTint, 9);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eAnisotropic, 10);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eSheen, 11);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eSheenTint, 12);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eClearcoat, 13);
					SetPSSRV(pDeviceContext, pMaterial, IMaterial::eClearcoatGloss, 14);

					Device* pDeviceInstance = Device::GetInstance();

					if (pMaterial != nullptr)
					{
						ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(pMaterial->GetBlendState());
						pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

						ID3D11SamplerState* pSamplerStates[] =
						{
							pDeviceInstance->GetSamplerState(SamplerState::eMinMagLinearMipPointWrap),
						};
						pDeviceContext->PSSetSamplers(materialSamplerSlot, _countof(pSamplerStates), pSamplerStates);

						DepthStencilState::Type depthStencilState = pMaterial->GetDepthStencilState();
						RasterizerState::Type rasterizerState = pMaterial->GetRasterizerState();

						if (isAlphaPrePass == true)
						{
							if (depthStencilState == DepthStencilState::eRead_Write_On)
							{
								rasterizerState = RasterizerState::eSolidCullNone;
								depthStencilState = DepthStencilState::eRead_On_Write_Off;
							}
							else if (depthStencilState == DepthStencilState::eRead_Off_Write_On)
							{
								rasterizerState = RasterizerState::eSolidCullNone;
								depthStencilState = DepthStencilState::eRead_Write_Off;
							}
						}

						ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(rasterizerState);
						pDeviceContext->RSSetState(pRasterizerState);

						ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(depthStencilState);
						pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);
					}
					else
					{
						ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eSolidCCW);
						pDeviceContext->RSSetState(pRasterizerState);

						ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(BlendState::eOff);
						pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

						ID3D11SamplerState* pSamplerStates[] =
						{
							pDeviceInstance->GetSamplerState(SamplerState::eMinMagLinearMipPointWrap),
						};
						pDeviceContext->PSSetSamplers(materialSamplerSlot, _countof(pSamplerStates), pSamplerStates);

						ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(DepthStencilState::eRead_Write_On);
						pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);
					}
				}
			}

			namespace util
			{
				DXProfiler::DXProfiler(const wchar_t* str)
				{
					Device::GetInstance()->GetUserDefinedAnnotation()->BeginEvent(str);
				}

				DXProfiler::~DXProfiler()
				{
					Device::GetInstance()->GetUserDefinedAnnotation()->EndEvent();
				}

				void GetInputElementDesc(EmVertexFormat::Type emType, const D3D11_INPUT_ELEMENT_DESC** ppInputElements_out, size_t* pElementCount_out)
				{
					switch (emType)
					{
					case EmVertexFormat::ePos:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePos4:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosCol:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTex:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTexCol:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTexNor:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTexNorCol:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTexNorTanBin:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					case EmVertexFormat::ePosTexNorWeiIdx:
					{
						static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};

						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = inputElements;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = _countof(inputElements);
						}
					}
					break;
					default:
						if (ppInputElements_out != nullptr)
						{
							*ppInputElements_out = nullptr;
						}

						if (pElementCount_out != nullptr)
						{
							*pElementCount_out = 0;
						}
						break;
					}
				}

				bool CompileShader(ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pDefines, const wchar_t* sourceName, const char* functionName, const char* profile, ID3DBlob** ppCompliedShaderBlob_out)
				{
					uint32_t CompileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#if defined( DEBUG ) || defined( _DEBUG )
					CompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
					CompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

					const std::string source = string::WideToMulti(sourceName);

					ID3DBlob* pErrorBlob = nullptr;
					bool isSuccess = SUCCEEDED(D3DCompile(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), source.c_str(), pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, functionName, profile, CompileFlags, 0, ppCompliedShaderBlob_out, &pErrorBlob));
					if (isSuccess == false)
					{
						const std::wstring errorMessage = string::MultiToWide(static_cast<const char*>(pErrorBlob->GetBufferPointer()));
						LOG_ERROR(L"failed to compile shader : %s", errorMessage.c_str());
					}
					SafeRelease(pErrorBlob);

					return isSuccess;
				}

				bool CreateVertexShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11VertexShader** ppVertexShader, const char* debugName)
				{
					ID3DBlob* pVertexShaderBlob = nullptr;
					if (util::CompileShader(pShaderBlob, pMacros, shaderPath, functionName, profile, &pVertexShaderBlob) == false)
						return false;

					HRESULT hr = pDevice->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), nullptr, ppVertexShader);
					
					SafeRelease(pVertexShaderBlob);

					if (FAILED(hr))
						return false;

					util::SetDebugName(*ppVertexShader, debugName);

					return true;
				}

				bool CreateVertexShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11VertexShader** ppVertexShader, const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, size_t NumElements, ID3D11InputLayout** ppInputLayout, const char* debugName)
				{
					ID3DBlob* pVertexShaderBlob = nullptr;
					if (util::CompileShader(pShaderBlob, pMacros, shaderPath, functionName, profile, &pVertexShaderBlob) == false)
						return false;

					HRESULT hr = pDevice->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), nullptr, ppVertexShader);
					if (FAILED(hr))
					{
						SafeRelease(pVertexShaderBlob);
						return false;
					}

					hr = pDevice->CreateInputLayout(pInputElementDescs, static_cast<uint32_t>(NumElements), pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), ppInputLayout);
					if (FAILED(hr))
					{
						SafeRelease((*ppVertexShader));
						SafeRelease(pVertexShaderBlob);
						return false;
					}

					util::SetDebugName(*ppVertexShader, debugName);
					util::SetDebugName(*ppInputLayout, debugName);

					SafeRelease(pVertexShaderBlob);

					return true;
				}

				bool CreatePixelShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11PixelShader** ppPixelShader, const char* debugName)
				{
					ID3DBlob* pPixelShaderBlob = nullptr;
					if (util::CompileShader(pShaderBlob, pMacros, shaderPath, functionName, profile, &pPixelShaderBlob) == false)
						return false;

					HRESULT hr = pDevice->CreatePixelShader(pPixelShaderBlob->GetBufferPointer(), pPixelShaderBlob->GetBufferSize(), nullptr, ppPixelShader);

					SafeRelease(pPixelShaderBlob);

					if (FAILED(hr))
						return false;

					util::SetDebugName(*ppPixelShader, debugName);

					return true;
				}

				bool CreateHullShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11HullShader** ppHullShader, const char* debugName)
				{
					ID3DBlob* pHullShaderBlob = nullptr;
					if (util::CompileShader(pShaderBlob, pMacros, shaderPath, functionName, profile, &pHullShaderBlob) == false)
						return false;

					HRESULT hr = pDevice->CreateHullShader(pHullShaderBlob->GetBufferPointer(), pHullShaderBlob->GetBufferSize(), nullptr, ppHullShader);

					SafeRelease(pHullShaderBlob);

					if (FAILED(hr))
						return false;

					util::SetDebugName(*ppHullShader, debugName);

					return true;
				}

				bool CreateDomainShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11DomainShader** ppDomainShader, const char* debugName)
				{
					ID3DBlob* pDomainShaderBlob = nullptr;
					if (util::CompileShader(pShaderBlob, pMacros, shaderPath, functionName, profile, &pDomainShaderBlob) == false)
						return false;

					HRESULT hr = pDevice->CreateDomainShader(pDomainShaderBlob->GetBufferPointer(), pDomainShaderBlob->GetBufferSize(), nullptr, ppDomainShader);

					SafeRelease(pDomainShaderBlob);

					if (FAILED(hr))
						return false;

					util::SetDebugName(*ppDomainShader, debugName);

					return true;
				}

				bool CreateGeometryShader(ID3D11Device* pDevice, ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pMacros, const wchar_t* shaderPath, const char* functionName, const char* profile, ID3D11GeometryShader** ppGeometryShader, const char* debugName)
				{
					ID3DBlob* pGeometryShaderBlob = nullptr;
					if (util::CompileShader(pShaderBlob, pMacros, shaderPath, functionName, profile, &pGeometryShaderBlob) == false)
						return false;

					HRESULT hr = pDevice->CreateGeometryShader(pGeometryShaderBlob->GetBufferPointer(), pGeometryShaderBlob->GetBufferSize(), nullptr, ppGeometryShader);

					SafeRelease(pGeometryShaderBlob);

					if (FAILED(hr))
						return false;

					util::SetDebugName(*ppGeometryShader, debugName);

					return true;
				}

				bool CreateConstantBuffer(ID3D11Device* pDevice, size_t nSize, ID3D11Buffer** pBuffer_out, const char* debugName)
				{
					D3D11_BUFFER_DESC desc{};
					desc.ByteWidth = static_cast<uint32_t>(nSize);
					desc.Usage = D3D11_USAGE_DYNAMIC;
					desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
					desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
					desc.MiscFlags = 0;
					desc.StructureByteStride = 0;

					HRESULT hr = pDevice->CreateBuffer(&desc, nullptr, pBuffer_out);
					if (FAILED(hr))
						return false;

					SetDebugName(*pBuffer_out, debugName);

					return true;
				}

				const D3D11_VIEWPORT* Convert(const math::Viewport& viewport)
				{
					return reinterpret_cast<const D3D11_VIEWPORT*>(&viewport);
				}

				int GetPixelSizeInBytes(DXGI_FORMAT val)
				{
					switch (val)
					{
					case DXGI_FORMAT_UNKNOWN: return 0;
					case DXGI_FORMAT_R32G32B32A32_TYPELESS: return 4 * 4;
					case DXGI_FORMAT_R32G32B32A32_FLOAT: return 4 * 4;
					case DXGI_FORMAT_R32G32B32A32_UINT: return 4 * 4;
					case DXGI_FORMAT_R32G32B32A32_SINT: return 4 * 4;
					case DXGI_FORMAT_R32G32B32_TYPELESS: return 3 * 4;
					case DXGI_FORMAT_R32G32B32_FLOAT: return 3 * 4;
					case DXGI_FORMAT_R32G32B32_UINT: return 3 * 4;
					case DXGI_FORMAT_R32G32B32_SINT: return 3 * 4;
					case DXGI_FORMAT_R16G16B16A16_TYPELESS: return 4 * 2;
					case DXGI_FORMAT_R16G16B16A16_FLOAT: return 4 * 2;
					case DXGI_FORMAT_R16G16B16A16_UNORM: return 4 * 2;
					case DXGI_FORMAT_R16G16B16A16_UINT: return 4 * 2;
					case DXGI_FORMAT_R16G16B16A16_SNORM: return 4 * 2;
					case DXGI_FORMAT_R16G16B16A16_SINT: return 4 * 2;
					case DXGI_FORMAT_R32G32_TYPELESS: return 2 * 4;
					case DXGI_FORMAT_R32G32_FLOAT: return 2 * 4;
					case DXGI_FORMAT_R32G32_UINT: return 2 * 4;
					case DXGI_FORMAT_R32G32_SINT: return 2 * 4;
					case DXGI_FORMAT_R32G8X24_TYPELESS: return 4 + 1;
					case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return 4 + 1;
					case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: return 4 + 1;
					case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return 4 + 1;
					case DXGI_FORMAT_R10G10B10A2_TYPELESS: return 4;
					case DXGI_FORMAT_R10G10B10A2_UNORM: return 4;
					case DXGI_FORMAT_R10G10B10A2_UINT: return 4;
					case DXGI_FORMAT_R11G11B10_FLOAT: return 4;
					case DXGI_FORMAT_R8G8B8A8_TYPELESS: return 4;
					case DXGI_FORMAT_R8G8B8A8_UNORM: return 4;
					case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return 4;
					case DXGI_FORMAT_R8G8B8A8_UINT: return 4;
					case DXGI_FORMAT_R8G8B8A8_SNORM: return 4;
					case DXGI_FORMAT_R8G8B8A8_SINT: return 4;
					case DXGI_FORMAT_R16G16_TYPELESS: return 4;
					case DXGI_FORMAT_R16G16_FLOAT: return 4;
					case DXGI_FORMAT_R16G16_UNORM: return 4;
					case DXGI_FORMAT_R16G16_UINT: return 4;
					case DXGI_FORMAT_R16G16_SNORM: return 4;
					case DXGI_FORMAT_R16G16_SINT: return 4;
					case DXGI_FORMAT_R32_TYPELESS: return 4;
					case DXGI_FORMAT_D32_FLOAT: return 4;
					case DXGI_FORMAT_R32_FLOAT: return 4;
					case DXGI_FORMAT_R32_UINT: return 4;
					case DXGI_FORMAT_R32_SINT: return 4;
					case DXGI_FORMAT_R24G8_TYPELESS: return 4;
					case DXGI_FORMAT_D24_UNORM_S8_UINT: return 4;
					case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return 4;
					case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return 4;
					case DXGI_FORMAT_R8G8_TYPELESS: return 2;
					case DXGI_FORMAT_R8G8_UNORM: return 2;
					case DXGI_FORMAT_R8G8_UINT: return 2;
					case DXGI_FORMAT_R8G8_SNORM: return 2;
					case DXGI_FORMAT_R8G8_SINT: return 2;
					case DXGI_FORMAT_R16_TYPELESS: return 2;
					case DXGI_FORMAT_R16_FLOAT: return 2;
					case DXGI_FORMAT_D16_UNORM: return 2;
					case DXGI_FORMAT_R16_UNORM: return 2;
					case DXGI_FORMAT_R16_UINT: return 2;
					case DXGI_FORMAT_R16_SNORM: return 2;
					case DXGI_FORMAT_R16_SINT: return 2;
					case DXGI_FORMAT_R8_TYPELESS: return 1;
					case DXGI_FORMAT_R8_UNORM: return 1;
					case DXGI_FORMAT_R8_UINT: return 1;
					case DXGI_FORMAT_R8_SNORM: return 1;
					case DXGI_FORMAT_R8_SINT: return 1;
					case DXGI_FORMAT_A8_UNORM: return 1;
					case DXGI_FORMAT_R1_UNORM: return 1;
					case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return 4;
					case DXGI_FORMAT_R8G8_B8G8_UNORM: return 4;
					case DXGI_FORMAT_G8R8_G8B8_UNORM: return 4;
					case DXGI_FORMAT_BC1_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC1_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC1_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC2_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC2_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC2_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC3_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC3_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC3_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC4_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC4_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC4_SNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC5_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC5_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC5_SNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_B5G6R5_UNORM: return 2;
					case DXGI_FORMAT_B5G5R5A1_UNORM: return 2;
					case DXGI_FORMAT_B8G8R8A8_UNORM: return 4;
					case DXGI_FORMAT_B8G8R8X8_UNORM: return 4;
					case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return 4;
					case DXGI_FORMAT_B8G8R8A8_TYPELESS: return 4;
					case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return 4;
					case DXGI_FORMAT_B8G8R8X8_TYPELESS: return 4;
					case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return 4;
					case DXGI_FORMAT_BC6H_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC6H_UF16: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC6H_SF16: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC7_TYPELESS: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC7_UNORM: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_BC7_UNORM_SRGB: assert(false); return 0; // not supported for compressed formats
					case DXGI_FORMAT_AYUV: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_Y410: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_Y416: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_NV12: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_P010: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_P016: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_YUY2: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_Y210: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_Y216: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_NV11: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_AI44: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_IA44: assert(false); return 0; // not yet implemented
					case DXGI_FORMAT_P8: return 1;
					case DXGI_FORMAT_A8P8: return 2;
					case DXGI_FORMAT_B4G4R4A4_UNORM: return 2;
					default: break;
					}
					assert(false);
					return 0;
				}

				DXGI_FORMAT GetCascadedShadowFormat()
				{
					return DXGI_FORMAT_R32_TYPELESS;
				}

				void ReportLiveObjects(ID3D11Device* pDevice)
				{
					if (pDevice != nullptr)
					{
						ID3D11Debug* pDebug = nullptr;
						if (SUCCEEDED(pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pDebug))))
						{
							pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
						}
						SafeRelease(pDebug);

						IDXGIDebug1* pDxgiDebug = nullptr;
						if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDxgiDebug))))
						{
							pDxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
						}
						SafeRelease(pDxgiDebug);
					}
				}
			}
		}
	}
}