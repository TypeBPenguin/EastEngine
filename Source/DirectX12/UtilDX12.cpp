#include "stdafx.h"
#include "UtilDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			namespace util
			{
				void WaitForFence(ID3D12Fence* pFence, uint64_t nCompletionValue, HANDLE hWaitEvent)
				{
					if (pFence->GetCompletedValue() < nCompletionValue)
					{
						if (FAILED(pFence->SetEventOnCompletion(nCompletionValue, hWaitEvent)))
						{
							throw_line("failed to wait for fence");
						}

						WaitForSingleObject(hWaitEvent, INFINITE);
					}
				}

				void GetInputElementDesc(EmVertexFormat::Type emType, const D3D12_INPUT_ELEMENT_DESC** ppInputElements_out, size_t* pElementCount_out)
				{
					switch (emType)
					{
					case EmVertexFormat::ePos:
					{
						static const D3D12_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
						static const D3D12_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
						static const D3D12_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
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
						static const D3D12_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
						static const D3D12_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
						static const D3D12_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
						static const D3D12_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
						static const D3D12_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
						static const D3D12_INPUT_ELEMENT_DESC inputElements[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
							{ "BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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

				bool CompileShader(ID3DBlob* pShaderBlob, const D3D_SHADER_MACRO* pDefines, const char* strSourceName, const char* strFunctionName, const char* strProfile, ID3DBlob** ppCompliedShaderBlob_out)
				{
					uint32_t CompileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ALL_RESOURCES_BOUND | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
#if defined( DEBUG ) || defined( _DEBUG )
					CompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
					CompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

					ID3DBlob* pErrorBlob = nullptr;
					bool isSuccess = SUCCEEDED(D3DCompile(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), strSourceName, pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, strFunctionName, strProfile, CompileFlags, 0, ppCompliedShaderBlob_out, &pErrorBlob));
					if (isSuccess == false)
					{
						LOG_ERROR("failed to compile shader : %s", static_cast<const char*>(pErrorBlob->GetBufferPointer()));
					}
					SafeRelease(pErrorBlob);

					return isSuccess;
				}

				bool CreateConstantBuffer(ID3D12Device* pDevice, size_t nSize, ID3D12Resource** ppBuffer_out, const wchar_t* wstrDebugName)
				{
					size_t nAlignedSize = Align(nSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

					CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
					CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(nAlignedSize);
					HRESULT hr = pDevice->CreateCommittedResource(&heapProperties,
						D3D12_HEAP_FLAG_NONE,
						&resourceDesc,
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(ppBuffer_out));
					if (SUCCEEDED(hr))
					{
						(*ppBuffer_out)->SetName(wstrDebugName);
						return true;
					}

					return false;
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

				D3D12_STATIC_SAMPLER_DESC GetStaticSamplerDesc(EmSamplerState::Type emSamplerState, uint32_t nShaderRegister, uint32_t nRegisterSpace, D3D12_SHADER_VISIBILITY shaderVisibility)
				{
					D3D12_SAMPLER_DESC samplerDesc = GetSamplerDesc(emSamplerState);

					D3D12_STATIC_SAMPLER_DESC staticDesc{};
					staticDesc.Filter = samplerDesc.Filter;
					staticDesc.AddressU = samplerDesc.AddressU;
					staticDesc.AddressV = samplerDesc.AddressV;
					staticDesc.AddressW = samplerDesc.AddressW;
					staticDesc.MipLODBias = samplerDesc.MipLODBias;
					staticDesc.MaxAnisotropy = samplerDesc.MaxAnisotropy;
					staticDesc.ComparisonFunc = samplerDesc.ComparisonFunc;
					staticDesc.MinLOD = samplerDesc.MinLOD;
					staticDesc.MaxLOD = samplerDesc.MaxLOD;
					staticDesc.ShaderRegister = nShaderRegister;
					staticDesc.RegisterSpace = nRegisterSpace;
					staticDesc.ShaderVisibility = shaderVisibility;

					math::Color colorBorder = { samplerDesc.BorderColor[0], samplerDesc.BorderColor[1], samplerDesc.BorderColor[2], samplerDesc.BorderColor[3] };
					if (colorBorder == math::Color::White)
					{
						staticDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
					}
					else if (colorBorder == math::Color::Black)
					{
						staticDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
					}
					else
					{
						staticDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
					}

					return staticDesc;
				}

				D3D12_SAMPLER_DESC GetSamplerDesc(EmSamplerState::Type emSamplerState)
				{
					D3D12_SAMPLER_DESC samplerDesc{};
					switch (emSamplerState)
					{
					case EmSamplerState::eMinMagMipLinearWrap:
					case EmSamplerState::eMinMagMipLinearClamp:
					case EmSamplerState::eMinMagMipLinearBorder:
					case EmSamplerState::eMinMagMipLinearMirror:
					case EmSamplerState::eMinMagMipLinearMirrorOnce:
						samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
						samplerDesc.MipLODBias = 0.f;
						samplerDesc.MaxAnisotropy = 1;
						samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
						samplerDesc.BorderColor[0] = 0;
						samplerDesc.BorderColor[1] = 0;
						samplerDesc.BorderColor[2] = 0;
						samplerDesc.BorderColor[3] = 0;
						samplerDesc.MinLOD = 0;
						samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
						switch (emSamplerState)
						{
						case EmSamplerState::eMinMagMipLinearWrap:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							break;
						case EmSamplerState::eMinMagMipLinearClamp:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							break;
						case EmSamplerState::eMinMagMipLinearBorder:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							break;
						case EmSamplerState::eMinMagMipLinearMirror:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							break;
						case EmSamplerState::eMinMagMipLinearMirrorOnce:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							break;
						}
						break;
					case EmSamplerState::eMinMagLinearMipPointWrap:
					case EmSamplerState::eMinMagLinearMipPointClamp:
					case EmSamplerState::eMinMagLinearMipPointBorder:
					case EmSamplerState::eMinMagLinearMipPointMirror:
					case EmSamplerState::eMinMagLinearMipPointMirrorOnce:
						samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
						samplerDesc.MipLODBias = 0.f;
						samplerDesc.MaxAnisotropy = 1;
						samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
						samplerDesc.BorderColor[0] = 0.f;
						samplerDesc.BorderColor[1] = 0.f;
						samplerDesc.BorderColor[2] = 0.f;
						samplerDesc.BorderColor[3] = 0.f;
						samplerDesc.MinLOD = 0.f;
						samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
						switch (emSamplerState)
						{
						case EmSamplerState::eMinMagLinearMipPointWrap:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							break;
						case EmSamplerState::eMinMagLinearMipPointClamp:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							break;
						case EmSamplerState::eMinMagLinearMipPointBorder:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							break;
						case EmSamplerState::eMinMagLinearMipPointMirror:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							break;
						case EmSamplerState::eMinMagLinearMipPointMirrorOnce:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							break;
						}
						break;
					case EmSamplerState::eAnisotropicWrap:
					case EmSamplerState::eAnisotropicClamp:
					case EmSamplerState::eAnisotropicBorder:
					case EmSamplerState::eAnisotropicMirror:
					case EmSamplerState::eAnisotropicMirrorOnce:
						samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
						samplerDesc.MipLODBias = 0.f;
						samplerDesc.MaxAnisotropy = 16;
						samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
						samplerDesc.BorderColor[0] = 0.f;
						samplerDesc.BorderColor[1] = 0.f;
						samplerDesc.BorderColor[2] = 0.f;
						samplerDesc.BorderColor[3] = 0.f;
						samplerDesc.MinLOD = 0.f;
						samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
						switch (emSamplerState)
						{
						case EmSamplerState::eAnisotropicWrap:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							break;
						case EmSamplerState::eAnisotropicClamp:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							break;
						case EmSamplerState::eAnisotropicBorder:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							break;
						case EmSamplerState::eAnisotropicMirror:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							break;
						case EmSamplerState::eAnisotropicMirrorOnce:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							break;
						}
						break;
					case EmSamplerState::eMinMagMipPointWrap:
					case EmSamplerState::eMinMagMipPointClamp:
					case EmSamplerState::eMinMagMipPointBorder:
					case EmSamplerState::eMinMagMipPointMirror:
					case EmSamplerState::eMinMagMipPointMirrorOnce:
						samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
						samplerDesc.MipLODBias = 0.f;
						samplerDesc.MaxAnisotropy = 1;
						samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
						samplerDesc.BorderColor[0] = 0;
						samplerDesc.BorderColor[1] = 0;
						samplerDesc.BorderColor[2] = 0;
						samplerDesc.BorderColor[3] = 0;
						samplerDesc.MinLOD = 0;
						samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
						switch (emSamplerState)
						{
						case EmSamplerState::eMinMagMipPointWrap:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							break;
						case EmSamplerState::eMinMagMipPointClamp:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							break;
						case EmSamplerState::eMinMagMipPointBorder:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
							break;
						case EmSamplerState::eMinMagMipPointMirror:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							break;
						case EmSamplerState::eMinMagMipPointMirrorOnce:
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
							break;
						}
						break;
					default:
						assert(false);
						break;
					}

					return samplerDesc;
				}

				D3D12_RASTERIZER_DESC GetRasterizerDesc(EmRasterizerState::Type emRasterizerState)
				{
					switch (emRasterizerState)
					{
					case EmRasterizerState::eSolidCCW:
					{
						return
						{
							D3D12_FILL_MODE_SOLID,	// FillMode
							D3D12_CULL_MODE_BACK,	// CullMode
							false,	// FrontCounterClockwise
							0,	// DepthBias
							0.f,	// DepthBiasClamp
							0.f,	// SlopeScaledDepthBias
							true,	// DepthClipEnable
							false,	// MultisampleEnable
							false,	// AntialiasedLineEnable
							0,	// ForcedSampleCount
							D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	// ConservativeRaster
						};
					}
					case EmRasterizerState::eSolidCW:
					{
						return
						{
							D3D12_FILL_MODE_SOLID,	// FillMode
							D3D12_CULL_MODE_FRONT,	// CullMode
							false,	// FrontCounterClockwise
							0,	// DepthBias
							0.f,	// DepthBiasClamp
							0.f,	// SlopeScaledDepthBias
							true,	// DepthClipEnable
							false,	// MultisampleEnable
							false,	// AntialiasedLineEnable
							0,	// ForcedSampleCount
							D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	// ConservativeRaster
						};
					}
					case EmRasterizerState::eSolidCullNone:
					{
						return
						{
							D3D12_FILL_MODE_SOLID,	// FillMode
							D3D12_CULL_MODE_NONE,	// CullMode
							false,	// FrontCounterClockwise
							0,	// DepthBias
							0.f,	// DepthBiasClamp
							0.f,	// SlopeScaledDepthBias
							true,	// DepthClipEnable
							false,	// MultisampleEnable
							false,	// AntialiasedLineEnable
							0,	// ForcedSampleCount
							D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	// ConservativeRaster
						};
					}
					case EmRasterizerState::eWireframeCCW:
					{
						return
						{
							D3D12_FILL_MODE_WIREFRAME,	// FillMode
							D3D12_CULL_MODE_BACK,	// CullMode
							false,	// FrontCounterClockwise
							0,	// DepthBias
							0.f,	// DepthBiasClamp
							0.f,	// SlopeScaledDepthBias
							true,	// DepthClipEnable
							false,	// MultisampleEnable
							false,	// AntialiasedLineEnable
							0,	// ForcedSampleCount
							D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	// ConservativeRaster
						};
					}
					case EmRasterizerState::eWireframeCW:
					{
						return
						{
							D3D12_FILL_MODE_WIREFRAME,	// FillMode
							D3D12_CULL_MODE_FRONT,	// CullMode
							false,	// FrontCounterClockwise
							0,	// DepthBias
							0.f,	// DepthBiasClamp
							0.f,	// SlopeScaledDepthBias
							true,	// DepthClipEnable
							false,	// MultisampleEnable
							false,	// AntialiasedLineEnable
							0,	// ForcedSampleCount
							D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	// ConservativeRaster
						};
					}
					case EmRasterizerState::eWireframeCullNone:
					{
						return
						{
							D3D12_FILL_MODE_WIREFRAME,	// FillMode
							D3D12_CULL_MODE_NONE,	// CullMode
							false,	// FrontCounterClockwise
							0,	// DepthBias
							0.f,	// DepthBiasClamp
							0.f,	// SlopeScaledDepthBias
							true,	// DepthClipEnable
							false,	// MultisampleEnable
							false,	// AntialiasedLineEnable
							0,	// ForcedSampleCount
							D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	// ConservativeRaster
						};
					}
					default:
						assert(false);
						return CD3DX12_RASTERIZER_DESC{ D3D12_DEFAULT };
					}
				}

				D3D12_BLEND_DESC GetBlendDesc(EmBlendState::Type emBlendState)
				{
					CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
					switch (emBlendState)
					{
					case EmBlendState::eOff:
					{
						blendDesc.AlphaToCoverageEnable = false;
						blendDesc.IndependentBlendEnable = false;
						for (int i = 0; i < 8; ++i)
						{
							blendDesc.RenderTarget[i].BlendEnable = false;
							blendDesc.RenderTarget[i].LogicOpEnable = false;
							blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
							blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
							blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
							blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
							blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
						}
					}
					break;
					case EmBlendState::eLinear:
					{
						blendDesc.AlphaToCoverageEnable = false;
						blendDesc.IndependentBlendEnable = false;
						for (int i = 0; i < 8; ++i)
						{
							blendDesc.RenderTarget[i].BlendEnable = true;
							blendDesc.RenderTarget[i].LogicOpEnable = false;
							blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
							blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
							blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
							blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
							blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
						}
					}
					break;
					case EmBlendState::eAdditive:
					{
						blendDesc.AlphaToCoverageEnable = false;
						blendDesc.IndependentBlendEnable = false;
						for (int i = 0; i < 8; ++i)
						{
							blendDesc.RenderTarget[i].BlendEnable = true;
							blendDesc.RenderTarget[i].LogicOpEnable = false;
							blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
							blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
							blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
							blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
						}
					}
					break;
					case EmBlendState::eSubTractive:
					{
						blendDesc.AlphaToCoverageEnable = false;
						blendDesc.IndependentBlendEnable = false;
						for (int i = 0; i < 8; ++i)
						{
							blendDesc.RenderTarget[i].BlendEnable = true;
							blendDesc.RenderTarget[i].LogicOpEnable = false;
							blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
							blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_SUBTRACT;
							blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
							blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
						}
					}
					break;
					case EmBlendState::eMultiplicative:
					{
						blendDesc.AlphaToCoverageEnable = false;
						blendDesc.IndependentBlendEnable = false;
						for (int i = 0; i < 8; ++i)
						{
							blendDesc.RenderTarget[i].BlendEnable = true;
							blendDesc.RenderTarget[i].LogicOpEnable = false;
							blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_ZERO;
							blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_SRC_COLOR;
							blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
							blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
						}
					}
					break;
					case EmBlendState::eSquared:
					{
						blendDesc.AlphaToCoverageEnable = false;
						blendDesc.IndependentBlendEnable = false;
						for (int i = 0; i < 8; ++i)
						{
							blendDesc.RenderTarget[i].BlendEnable = true;
							blendDesc.RenderTarget[i].LogicOpEnable = false;
							blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_ZERO;
							blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_DEST_COLOR;
							blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
							blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
						}
					}
					break;
					case EmBlendState::eNegative:
					{
						blendDesc.AlphaToCoverageEnable = false;
						blendDesc.IndependentBlendEnable = false;
						for (int i = 0; i < 8; ++i)
						{
							blendDesc.RenderTarget[i].BlendEnable = true;
							blendDesc.RenderTarget[i].LogicOpEnable = false;
							blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
							blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
							blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
							blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
						}
					}
					break;
					case EmBlendState::eOpacity:
					{
						blendDesc.AlphaToCoverageEnable = false;
						blendDesc.IndependentBlendEnable = false;
						for (int i = 0; i < 8; ++i)
						{
							blendDesc.RenderTarget[i].BlendEnable = true;
							blendDesc.RenderTarget[i].LogicOpEnable = false;
							blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
							blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
							blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
							blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
						}
					}
					break;
					case EmBlendState::eAlphaBlend:
					{
						blendDesc.AlphaToCoverageEnable = false;
						blendDesc.IndependentBlendEnable = false;
						for (int i = 0; i < 8; ++i)
						{
							blendDesc.RenderTarget[i].BlendEnable = true;
							blendDesc.RenderTarget[i].LogicOpEnable = false;
							blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
							blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
							blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
							blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
							blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
							blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
						}
					}
					break;
					default:
						assert(false);
						break;
					}

					return blendDesc;
				}

				D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc(EmDepthStencilState::Type emDepthStencilState)
				{
					switch (emDepthStencilState)
					{
					case EmDepthStencilState::eRead_Write_On:
					{
						return
						{
							true,	// DepthEnable
							D3D12_DEPTH_WRITE_MASK_ALL,	// DepthWriteMask
							D3D12_COMPARISON_FUNC_LESS,	// DepthFunc
							true,	// StencilEnable
							0xff,	// StencilReadMask
							0xff,	// StencilWriteMask
							{
								D3D12_STENCIL_OP_KEEP,	// StencilFailOp
								D3D12_STENCIL_OP_INCR,	// StencilDepthFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilPassOp
								D3D12_COMPARISON_FUNC_ALWAYS,	// StencilFunc
							},
							{
								D3D12_STENCIL_OP_KEEP,	// StencilFailOp
								D3D12_STENCIL_OP_DECR,	// StencilDepthFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilPassOp
								D3D12_COMPARISON_FUNC_ALWAYS,	// StencilFunc
							},
						};
					}
					case EmDepthStencilState::eRead_Write_Off:
					{
						return
						{
							false,	// DepthEnable
							D3D12_DEPTH_WRITE_MASK_ZERO,	// DepthWriteMask
							D3D12_COMPARISON_FUNC_NEVER,	// DepthFunc
							false,	// StencilEnable
							0xff,	// StencilReadMask
							0xff,	// StencilWriteMask
							{
								D3D12_STENCIL_OP_KEEP,	// StencilFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilDepthFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilPassOp
								D3D12_COMPARISON_FUNC_ALWAYS,	// StencilFunc
							},
							{
								D3D12_STENCIL_OP_KEEP,	// StencilFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilDepthFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilPassOp
								D3D12_COMPARISON_FUNC_ALWAYS,	// StencilFunc
							},
						};
					}
					case EmDepthStencilState::eRead_On_Write_Off:
					{
						return
						{
							true,	// DepthEnable
							D3D12_DEPTH_WRITE_MASK_ZERO,	// DepthWriteMask
							D3D12_COMPARISON_FUNC_LESS,	// DepthFunc
							true,	// StencilEnable
							0xff,	// StencilReadMask
							0xff,	// StencilWriteMask
							{
								D3D12_STENCIL_OP_KEEP,	// StencilFailOp
								D3D12_STENCIL_OP_INCR,	// StencilDepthFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilPassOp
								D3D12_COMPARISON_FUNC_ALWAYS,	// StencilFunc
							},
							{
								D3D12_STENCIL_OP_KEEP,	// StencilFailOp
								D3D12_STENCIL_OP_DECR,	// StencilDepthFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilPassOp
								D3D12_COMPARISON_FUNC_ALWAYS,	// StencilFunc
							},
						};
					}
					case EmDepthStencilState::eRead_Off_Write_On:
					{
						return
						{
							true,	// DepthEnable
							D3D12_DEPTH_WRITE_MASK_ALL,	// DepthWriteMask
							D3D12_COMPARISON_FUNC_ALWAYS,	// DepthFunc
							true,	// StencilEnable
							0xff,	// StencilReadMask
							0xff,	// StencilWriteMask
							{
								D3D12_STENCIL_OP_KEEP,	// StencilFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilDepthFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilPassOp
								D3D12_COMPARISON_FUNC_ALWAYS,	// StencilFunc
							},
							{
								D3D12_STENCIL_OP_KEEP,	// StencilFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilDepthFailOp
								D3D12_STENCIL_OP_KEEP,	// StencilPassOp
								D3D12_COMPARISON_FUNC_ALWAYS,	// StencilFunc
							},
						};
					}
					break;
					default:
						assert(false);
						return CD3DX12_DEPTH_STENCIL_DESC{ D3D12_DEFAULT };
					}
				}
			}
		}
	}
}