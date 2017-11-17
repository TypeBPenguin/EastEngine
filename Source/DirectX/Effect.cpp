#include "stdafx.h"
#include "Effect.h"

#include "D3DInterface.h"
#include "Vertex.h"

#include "Light.h"

#include <D3DCompiler.h>

namespace StrID
{
	RegisterStringID(OcclusionCulling);
	RegisterStringID(OcclusionCullingCS);
	RegisterStringID(ScreenQuad);
	
	RegisterStringID(Color);
	RegisterStringID(Vertex);
	RegisterStringID(ModelStatic);
	RegisterStringID(ModelSkinned);

	RegisterStringID(Sky);
	RegisterStringID(SkyEffect);
	RegisterStringID(SkyCloud);

	RegisterStringID(Water);
	RegisterStringID(WaterWireframe);
	RegisterStringID(WaterSimulator_SpectrumCS);
	RegisterStringID(WaterSimulator_Displacement);
	RegisterStringID(WaterSimulator_Gradient);
	RegisterStringID(WaterFFT_CS);
	RegisterStringID(WaterFFT_CS2);

	RegisterStringID(Light);
	RegisterStringID(Material);
	RegisterStringID(Shadow);

	RegisterStringID(AO);
	RegisterStringID(FXAA);
	RegisterStringID(ColorGrading);

	RegisterStringID(ToneMapWithBloom);
	RegisterStringID(Add);
	RegisterStringID(BlurVertical);
	RegisterStringID(BlurHorizontal);
	RegisterStringID(Bright);
	RegisterStringID(CalcLuminance);
	RegisterStringID(AvgLuminance);

	RegisterStringID(MotionBlur);

	RegisterStringID(Deferred);

	RegisterStringID(BlurPass);
	RegisterStringID(BlurPassWithDiffuse);
	RegisterStringID(BlurPassSupersampling);
	RegisterStringID(BlurPassWithDiffuseSupersampling);
	RegisterStringID(BlurPassthrough);
	RegisterStringID(BlurEdgeDetection);

	RegisterStringID(UI);
}

namespace EastEngine
{
	namespace Graphics
	{
		EffectTech::EffectTech(ID3DX11EffectTechnique* pTech, EmVertexFormat::Type emLayoutFormat, const String::StringID& strTechName)
			: m_pTech(pTech)
			, m_emVertexFormat(emLayoutFormat)
			, m_strTechName(strTechName)
		{
		}

		EffectTech::~EffectTech()
		{
		}

		void EffectTech::PassApply(uint32_t nIdx, IDeviceContext* pd3dDeviceContext, uint32_t applyFlags)
		{
			m_pTech->GetPassByIndex(nIdx)->Apply(applyFlags, pd3dDeviceContext->GetInterface());
		}

		void EffectTech::GetInputSignatureDesc(uint8_t** ppIAInputSignature_out, std::size_t& IAInputSignatureSize_out)
		{
			D3DX11_PASS_DESC desc;
			if (FAILED(m_pTech->GetPassByIndex(0)->GetDesc(&desc)))
			{
				*ppIAInputSignature_out = nullptr;
				IAInputSignatureSize_out = 0;
				return;
			}

			*ppIAInputSignature_out = desc.pIAInputSignature;
			IAInputSignatureSize_out = desc.IAInputSignatureSize;
		}

		uint32_t EffectTech::GetPassCount()
		{
			D3DX11_TECHNIQUE_DESC desc;
			m_pTech->GetDesc(&desc);

			return desc.Passes;
		}

		Effect::Effect(ID3DX11Effect* pEffect, const String::StringID& strName)
			: m_pEffect(pEffect)
			, m_strName(strName)
			, m_isOptimized(false)
		{
		}

		Effect::~Effect()
		{
			std::for_each(m_umapEffectTech.begin(), m_umapEffectTech.end(), DeleteSTLMapObject());
			m_umapEffectTech.clear();

			std::for_each(m_umapEffectHandle.begin(), m_umapEffectHandle.end(), DeleteSTLMapObject());
			m_umapEffectHandle.clear();

			SafeRelease(m_pEffect);
		}

		void Effect::Optimize()
		{
			if (m_isOptimized == true)
				return;

			m_pEffect->Optimize();

			m_isOptimized = true;
		}

		void Effect::ClearState(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech)
		{
			if (pEffectTech == nullptr)
				return;

			pEffectTech->PassApply(0, pd3dDeviceContext);
		}

		IEffectTech* Effect::CreateTechnique(const String::StringID& strName, EmVertexFormat::Type emLayoutFormat)
		{
			auto iter = m_umapEffectTech.find(strName);
			if (iter != m_umapEffectTech.end())
				return iter->second;

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectTechnique* pEffectTech = m_pEffect->GetTechniqueByName(strName.c_str());
			if (pEffectTech->IsValid() == false)
			{
				SafeRelease(pEffectTech);
				return nullptr;
			}

			EffectTech* pTech = new EffectTech(pEffectTech, emLayoutFormat, strName);

			uint8_t* pIAInputSignature = nullptr;
			std::size_t IAInputSignatureSize = 0;
			pTech->GetInputSignatureDesc(&pIAInputSignature, IAInputSignatureSize);

			if (FAILED(GetDevice()->CreateInputLayout(emLayoutFormat, pIAInputSignature, IAInputSignatureSize)))
			{
				PRINT_LOG("Failed create technique, Problem InputLayout : %s", strName.c_str());
				SafeDelete(pTech);
				return nullptr;
			}

			m_umapEffectTech.emplace(strName, pTech);

			return pTech;
		}

		IEffectTech* Effect::GetTechnique(const String::StringID& strName)
		{
			auto iter = m_umapEffectTech.find(strName);
			if (iter != m_umapEffectTech.end())
				return iter->second;

			return nullptr;
		}

		EffectRawHandle* Effect::GetRawHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eRaw)
					return static_cast<EffectRawHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectRawVariable* pNewVariable = new EffectRawVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectScalarHandle* Effect::GetScalarHandle(const String::StringID& strName)
		{
			EffectHandle* pVariable = GetHandle(strName);
			if (pVariable != nullptr)
			{
				if (pVariable->GetType() == EmEffectHandle::Type::eScalar)
					return static_cast<EffectScalarVariable*>(pVariable);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectScalarVariable* pNewVariable = new EffectScalarVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectVectorHandle* Effect::GetVectorHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eVector)
					return static_cast<EffectVectorHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectVectorVariable* pNewVariable = new EffectVectorVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectMatrixHandle* Effect::GetMatrixHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eMatrix)
					return static_cast<EffectMatrixHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectMatrixVariable* pNewVariable = new EffectMatrixVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectStringHandle* Effect::GetStringHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eString)
					return static_cast<EffectStringHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectStringVariable* pNewVariable = new EffectStringVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectShaderResourceHandle* Effect::GetShaderResourceHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eShaderResource)
					return static_cast<EffectShaderResourceHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectShaderResourceVariable* pNewVariable = new EffectShaderResourceVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectUnorderedAccessViewHandle* Effect::GetUnorderedAccessViewHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eUnorderedAccessView)
					return static_cast<EffectUnorderedAccessViewHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectUnorderedAccessViewVariable* pNewVariable = new EffectUnorderedAccessViewVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectRenderTargetViewHandle* Effect::GetRenderTargetViewHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eRenderTargetView)
					return static_cast<EffectRenderTargetViewHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectRenderTargetViewVariable* pNewVariable = new EffectRenderTargetViewVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectDepthStencilViewHandle* Effect::GetDepthStencilViewHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eDepthStencilView)
					return static_cast<EffectDepthStencilViewHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectDepthStencilViewVariable* pNewVariable = new EffectDepthStencilViewVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectConstanceBufferHandle* Effect::GetConstanceBufferHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eConstanceBuffer)
					return static_cast<EffectConstanceBufferHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectConstanceBufferVariable* pNewVariable = new EffectConstanceBufferVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectBlendHandle* Effect::GetBlendHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eBlend)
					return static_cast<EffectBlendHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectBlendVariable* pNewVariable = new EffectBlendVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectDepthStencilHandle* Effect::GetDepthStencilHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eDepthStencil)
					return static_cast<EffectDepthStencilHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectDepthStencilVariable* pNewVariable = new EffectDepthStencilVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectRasterizerHandle* Effect::GetRasterizerHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eRasterizer)
					return static_cast<EffectRasterizerHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectRasterizerVariable* pNewVariable = new EffectRasterizerVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		EffectSamplerHandle* Effect::GetSamplerHandle(const String::StringID& strName)
		{
			EffectHandle* pHandle = GetHandle(strName);
			if (pHandle != nullptr)
			{
				if (pHandle->GetType() == EmEffectHandle::eSampler)
					return static_cast<EffectSamplerHandle*>(pHandle);

				return nullptr;
			}

			if (m_isOptimized == true)
				return nullptr;

			ID3DX11EffectVariable* pEffectVariable = m_pEffect->GetVariableByName(strName.c_str());
			EffectSamplerVariable* pNewVariable = new EffectSamplerVariable(pEffectVariable);

			m_umapEffectHandle.emplace(strName, pNewVariable);

			return pNewVariable;
		}

		void Effect::SetRawValue(const String::StringID& strName, const void* pData, uint32_t nByteOffset, uint32_t nByteCount)
		{
			EffectRawHandle* pHandle = GetRawHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetRawValue(pData, nByteOffset, nByteCount);
			}
		}

		void Effect::SetBool(const String::StringID& strName, bool value)
		{
			EffectScalarHandle* pHandle = GetScalarHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetBool(value);
			}
		}

		void Effect::SetBoolArray(const String::StringID& strName, bool* pValue, uint32_t nOffset, uint32_t nCount)
		{
			EffectScalarHandle* pHandle = GetScalarHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetBoolArray(pValue, nOffset, nCount);
			}
		}

		void Effect::SetInt(const String::StringID& strName, int nValue)
		{
			EffectScalarHandle* pHandle = GetScalarHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetInt(nValue);
			}
		}

		void Effect::SetIntArray(const String::StringID& strName, const int* pValue, uint32_t nOffset, uint32_t nCount)
		{
			EffectScalarHandle* pHandle = GetScalarHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetIntArray(pValue, nOffset, nCount);
			}
		}

		void Effect::SetFloat(const String::StringID& strName, float fValue)
		{
			EffectScalarHandle* pHandle = GetScalarHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetFloat(fValue);
			}
		}

		void Effect::SetFloatArray(const String::StringID& strName, const float* pValue, uint32_t nOffset, uint32_t nCount)
		{
			EffectScalarHandle* pHandle = GetScalarHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetFloatArray(pValue, nOffset, nCount);
			}
		}

		void Effect::SetVector(const String::StringID& strName, const Math::Int2& n2Value)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVector(n2Value);
			}
		}

		void Effect::SetVector(const String::StringID& strName, const Math::Int3& n3Value)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVector(n3Value);
			}
		}

		void Effect::SetVector(const String::StringID& strName, const Math::Int4& n4Value)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVector(n4Value);
			}
		}

		void Effect::SetVector(const String::StringID& strName, const Math::Vector2& f2Value)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVector(f2Value);
			}
		}

		void Effect::SetVector(const String::StringID& strName, const Math::Vector3& f3Value)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVector(f3Value);
			}
		}

		void Effect::SetVector(const String::StringID& strName, const Math::Vector4& f4Value)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVector(f4Value);
			}
		}

		void Effect::SetVectorArray(const String::StringID& strName, const Math::Int2* pn2Value, uint32_t nOffset, uint32_t nCount)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVectorArray(pn2Value, nOffset, nCount);
			}
		}

		void Effect::SetVectorArray(const String::StringID& strName, const Math::Int3* pn3Value, uint32_t nOffset, uint32_t nCount)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVectorArray(pn3Value, nOffset, nCount);
			}
		}

		void Effect::SetVectorArray(const String::StringID& strName, const Math::Int4* pn4Value, uint32_t nOffset, uint32_t nCount)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVectorArray(pn4Value, nOffset, nCount);
			}
		}

		void Effect::SetVectorArray(const String::StringID& strName, const Math::Vector2* pf2Value, uint32_t nOffset, uint32_t nCount)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVectorArray(pf2Value, nOffset, nCount);
			}
		}

		void Effect::SetVectorArray(const String::StringID& strName, const Math::Vector3* pf3Value, uint32_t nOffset, uint32_t nCount)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVectorArray(pf3Value, nOffset, nCount);
			}
		}

		void Effect::SetVectorArray(const String::StringID& strName, const Math::Vector4* pf4Value, uint32_t nOffset, uint32_t nCount)
		{
			EffectVectorHandle* pHandle = GetVectorHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetVectorArray(pf4Value, nOffset, nCount);
			}
		}

		void Effect::SetMatrix(const String::StringID& strName, const Math::Matrix& matrix)
		{
			EffectMatrixHandle* pHandle = GetMatrixHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetMatrix(matrix);
			}
		}

		void Effect::SetMatrixArray(const String::StringID& strName, const Math::Matrix* pMatrix, uint32_t nOffset, uint32_t nCount)
		{
			EffectMatrixHandle* pHandle = GetMatrixHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetMatrixArray(pMatrix, nOffset, nCount);
			}
		}

		void Effect::SetString(const String::StringID& strName, const char* str, uint32_t nCount)
		{
			EffectStringHandle* pHandle = GetStringHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetString(str, nCount);
			}
		}

		void Effect::SetTexture(const String::StringID& strName, const std::shared_ptr<ITexture>& pTexture)
		{
			EffectShaderResourceHandle* pHandle = GetShaderResourceHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetTexture(pTexture);
			}
		}

		void Effect::SetTextureArray(const String::StringID& strName, const std::shared_ptr<ITexture>* pTextures, uint32_t nOffset, uint32_t nCount)
		{
			EffectShaderResourceHandle* pHandle = GetShaderResourceHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetTextureArray(pTextures, nOffset, nCount);
			}
		}

		void Effect::SetStructuredBuffer(const String::StringID& strName, IStructuredBuffer* pDataBuffer)
		{
			EffectShaderResourceHandle* pHandle = GetShaderResourceHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetStructuredBuffer(pDataBuffer);
			}
		}

		void Effect::SetUnorderedAccessView(const String::StringID& strName, IStructuredBuffer* pDataBuffer)
		{
			EffectUnorderedAccessViewHandle* pHandle = GetUnorderedAccessViewHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetUnorderedAccessView(pDataBuffer);
			}
		}

		void Effect::SetUnorderedAccessViewArray(const String::StringID& strName, IStructuredBuffer** ppDataBuffer, uint32_t nOffset, uint32_t nCount)
		{
			EffectUnorderedAccessViewHandle* pHandle = GetUnorderedAccessViewHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetUnorderedAccessViewArray(ppDataBuffer, nOffset, nCount);
			}
		}

		void Effect::SetBlendState(const String::StringID& strName, IBlendState* pBlendState, uint32_t nIndex)
		{
			EffectBlendHandle* pHandle = GetBlendHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetBlendState(pBlendState, nIndex);
			}
		}

		void Effect::UndoBlendState(const String::StringID& strName, uint32_t nIndex)
		{
			EffectBlendHandle* pHandle = GetBlendHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->UndoBlendState(nIndex);
			}
		}

		void Effect::SetDepthStencilState(const String::StringID& strName, IDepthStencilState* pDepthStencilState, uint32_t nIndex)
		{
			EffectDepthStencilHandle* pHandle = GetDepthStencilHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetDepthStencilState(pDepthStencilState, nIndex);
			}
		}

		void Effect::UndoDepthStencilState(const String::StringID& strName, uint32_t nIndex)
		{
			EffectDepthStencilHandle* pHandle = GetDepthStencilHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->UndoDepthStencilState(nIndex);
			}
		}

		void Effect::SetRasterizerState(const String::StringID& strName, IRasterizerState* pRasterizerState, uint32_t nIndex)
		{
			EffectRasterizerHandle* pHandle = GetRasterizerHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetRasterizerState(pRasterizerState, nIndex);
			}
		}

		void Effect::UndoRasterizerState(const String::StringID& strName, uint32_t nIndex)
		{
			EffectRasterizerHandle* pHandle = GetRasterizerHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->UndoRasterizerState(nIndex);
			}
		}

		void Effect::SetSamplerState(const String::StringID& strName, ISamplerState* pSamplerState, uint32_t nIndex)
		{
			EffectSamplerHandle* pHandle = GetSamplerHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->SetSamplerState(pSamplerState, nIndex);
			}
		}

		void Effect::UndoSamplerState(const String::StringID& strName, uint32_t nIndex)
		{
			EffectSamplerHandle* pHandle = GetSamplerHandle(strName);
			if (pHandle != nullptr)
			{
				pHandle->UndoSamplerState(nIndex);
			}
		}

		EffectHandle* Effect::GetHandle(const String::StringID& strName)
		{
			auto iter = m_umapEffectHandle.find(strName);
			if (iter != m_umapEffectHandle.end())
				return iter->second;

			return nullptr;
		}

		EffectVariable::EffectVariable(ID3DX11EffectVariable* pVariable)
			: pVariable(pVariable)
		{
		}

		EffectVariable::~EffectVariable()
		{
			SafeRelease(pVariable);
		}

		bool EffectVariable::IsValid()
		{
			if (pVariable == nullptr || pVariable->IsValid() == false)
				return false;

			return true;
		}

		EffectRawVariable::EffectRawVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
		{
		}

		EffectRawVariable::~EffectRawVariable()
		{
		}

		void EffectRawVariable::SetRawValue(const void* pData, uint32_t nByteOffset, uint32_t nByteCount)
		{
			if (IsValid() == false)
				return;

			GetInterface()->SetRawValue(pData, nByteOffset, nByteCount);
		}

		EffectScalarVariable::EffectScalarVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
			, pScalar(pVariable->AsScalar())
		{
		}

		EffectScalarVariable::~EffectScalarVariable()
		{
			SafeRelease(pScalar);
		}

		void EffectScalarVariable::SetBool(bool value)
		{
			if (IsValid() == false)
				return;

			pScalar->SetBool(value);
		}

		void EffectScalarVariable::SetBoolArray(bool* pValue, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			pScalar->SetBoolArray(pValue, nOffset, nCount);
		}

		void EffectScalarVariable::SetInt(int nValue)
		{
			if (IsValid() == false)
				return;

			pScalar->SetInt(nValue);
		}

		void EffectScalarVariable::SetIntArray(const int* pData, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			pScalar->SetIntArray(pData, nOffset, nCount);
		}

		void EffectScalarVariable::SetFloat(float fValue)
		{
			if (IsValid() == false)
				return;

			pScalar->SetFloat(fValue);
		}

		void EffectScalarVariable::SetFloatArray(const float* pData, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			pScalar->SetFloatArray(pData, nOffset, nCount);
		}

		EffectVectorVariable::EffectVectorVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
			, pVector(pVariable->AsVector())
		{
		}

		EffectVectorVariable::~EffectVectorVariable()
		{
			SafeRelease(pVector);
		}

		void EffectVectorVariable::SetVector(const Math::Int2& n2Value)
		{
			if (IsValid() == false)
				return;

			pVector->SetIntVector(&n2Value.x);
		}

		void EffectVectorVariable::SetVector(const Math::Int3& n3Value)
		{
			if (IsValid() == false)
				return;

			pVector->SetIntVector(&n3Value.x);
		}

		void EffectVectorVariable::SetVector(const Math::Int4& n4Value)
		{
			if (IsValid() == false)
				return;

			pVector->SetIntVector(&n4Value.x);
		}

		void EffectVectorVariable::SetVector(const Math::Vector2& f2Value)
		{
			if (IsValid() == false)
				return;

			pVector->SetFloatVector(&f2Value.x);
		}

		void EffectVectorVariable::SetVector(const Math::Vector3& f3Value)
		{
			if (IsValid() == false)
				return;

			pVector->SetFloatVector(&f3Value.x);
		}

		void EffectVectorVariable::SetVector(const Math::Vector4& f4Value)
		{
			if (IsValid() == false)
				return;

			pVector->SetFloatVector(&f4Value.x);
		}

		void EffectVectorVariable::SetVectorArray(const Math::Int2* pData, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			pVector->SetIntVectorArray(&pData->x, nOffset, nCount);
		}

		void EffectVectorVariable::SetVectorArray(const Math::Int3* pData, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			pVector->SetIntVectorArray(&pData->x, nOffset, nCount);
		}

		void EffectVectorVariable::SetVectorArray(const Math::Int4* pData, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			pVector->SetIntVectorArray(&pData->x, nOffset, nCount);
		}

		void EffectVectorVariable::SetVectorArray(const Math::Vector2* pData, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			pVector->SetFloatVectorArray(&pData->x, nOffset, nCount);
		}

		void EffectVectorVariable::SetVectorArray(const Math::Vector3* pData, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			pVector->SetFloatVectorArray(&pData->x, nOffset, nCount);
		}

		void EffectVectorVariable::SetVectorArray(const Math::Vector4* pData, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			pVector->SetFloatVectorArray(&pData->x, nOffset, nCount);
		}

		EffectMatrixVariable::EffectMatrixVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
			, pMatrix(pVariable->AsMatrix())
		{
		}

		EffectMatrixVariable::~EffectMatrixVariable()
		{
			SafeRelease(pMatrix);
		}

		void EffectMatrixVariable::SetMatrix(const Math::Matrix& matrix)
		{
			if (IsValid() == false)
				return;

			pMatrix->SetMatrix(&matrix._11);
		}

		void EffectMatrixVariable::SetMatrixArray(const Math::Matrix* pData, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			pMatrix->SetMatrixArray(&pData->_11, nOffset, nCount);
		}
		
		EffectStringVariable::EffectStringVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
			, pString(pVariable->AsString())
		{
		}

		EffectStringVariable::~EffectStringVariable()
		{
			SafeRelease(pString);
		}

		void EffectStringVariable::SetString(const char* str, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			pString->SetRawValue(str, 0, nCount);
		}

		EffectShaderResourceVariable::EffectShaderResourceVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
			, pShaderResource(pVariable->AsShaderResource())
		{
		}

		EffectShaderResourceVariable::~EffectShaderResourceVariable()
		{
			SafeRelease(pShaderResource);
		}

		void EffectShaderResourceVariable::SetTexture(const std::shared_ptr<ITexture>& pTexture)
		{
			if (IsValid() == false)
				return;

			if (pTexture != nullptr)
			{
				pShaderResource->SetResource(pTexture->GetShaderResourceView());
			}
			else
			{
				pShaderResource->SetResource(nullptr);
			}
		}

		void EffectShaderResourceVariable::SetTextureArray(const std::shared_ptr<ITexture>* pTextures, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			if (*pTextures != nullptr)
			{
				std::vector<ID3D11ShaderResourceView*> vecResources;
				vecResources.resize(nCount);

				for (uint32_t i = 0; i < nCount; ++i)
				{
					vecResources[i] = pTextures[i]->GetShaderResourceView();
				}

				pShaderResource->SetResourceArray(&vecResources.front(), nOffset, nCount);
			}
			else
			{
				ID3D11ShaderResourceView* pNullptr = nullptr;
				pShaderResource->SetResourceArray(&pNullptr, nOffset, nCount);
			}
		}

		void EffectShaderResourceVariable::SetStructuredBuffer(IStructuredBuffer* pDataBuffer)
		{
			if (IsValid() == false)
				return;

			if (pDataBuffer != nullptr)
			{
				pShaderResource->SetResource(pDataBuffer->GetShaderResourceView());
			}
			else
			{
				pShaderResource->SetResource(nullptr);
			}
		}

		EffectUnorderedAccessViewVariable::EffectUnorderedAccessViewVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
			, pUnorderedAccessView(pVariable->AsUnorderedAccessView())
		{
		}

		EffectUnorderedAccessViewVariable::~EffectUnorderedAccessViewVariable()
		{
			SafeRelease(pUnorderedAccessView);
		}

		void EffectUnorderedAccessViewVariable::SetUnorderedAccessView(IStructuredBuffer* pDataBuffer)
		{
			if (IsValid() == false)
				return;

			if (pDataBuffer != nullptr)
			{
				pUnorderedAccessView->SetUnorderedAccessView(pDataBuffer->GetUnorderedAccessView());
			}
			else
			{
				pUnorderedAccessView->SetUnorderedAccessView(nullptr);
			}
		}

		void EffectUnorderedAccessViewVariable::SetUnorderedAccessViewArray(IStructuredBuffer** ppDataBuffer, uint32_t nOffset, uint32_t nCount)
		{
			if (IsValid() == false)
				return;

			if (*ppDataBuffer != nullptr)
			{
				std::vector<ID3D11UnorderedAccessView*> vecUnorderedAccessVIews;
				vecUnorderedAccessVIews.resize(nCount);

				for (uint32_t i = 0; i < nCount; ++i)
				{
					vecUnorderedAccessVIews[i] = ppDataBuffer[i]->GetUnorderedAccessView();
				}

				pUnorderedAccessView->SetUnorderedAccessViewArray(&vecUnorderedAccessVIews.front(), nOffset, nCount);
			}
			else
			{
				ID3D11UnorderedAccessView* pNullptr = nullptr;
				pUnorderedAccessView->SetUnorderedAccessViewArray(&pNullptr, nOffset, nCount);
			}
		}

		EffectRenderTargetViewVariable::EffectRenderTargetViewVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
		{
		}

		EffectDepthStencilViewVariable::EffectDepthStencilViewVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
		{
		}

		EffectConstanceBufferVariable::EffectConstanceBufferVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
		{
		}

		EffectBlendVariable::EffectBlendVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
			, pBlend(pVariable->AsBlend())
		{
		}

		EffectBlendVariable::~EffectBlendVariable()
		{
			SafeRelease(pBlend);
		}

		void EffectBlendVariable::SetBlendState(IBlendState* pBlendState, uint32_t nIndex)
		{
			if (IsValid() == false || pBlendState == nullptr)
				return;

			pBlend->SetBlendState(nIndex, pBlendState->GetInterface());
		}

		void EffectBlendVariable::UndoBlendState(uint32_t nIndex)
		{
			if (IsValid() == false)
				return;

			pBlend->UndoSetBlendState(nIndex);
		}

		EffectDepthStencilVariable::EffectDepthStencilVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
			, pDepthStencil(pVariable->AsDepthStencil())
		{
		}

		EffectDepthStencilVariable::~EffectDepthStencilVariable()
		{
			SafeRelease(pDepthStencil);
		}

		void EffectDepthStencilVariable::SetDepthStencilState(IDepthStencilState* pDepthStencilState, uint32_t nIndex)
		{
			if (IsValid() == false || pDepthStencilState == nullptr)
				return;

			pDepthStencil->SetDepthStencilState(nIndex, pDepthStencilState->GetInterface());
		}

		void EffectDepthStencilVariable::UndoDepthStencilState(uint32_t nIndex)
		{
			if (IsValid() == false)
				return;

			pDepthStencil->UndoSetDepthStencilState(nIndex);
		}

		EffectRasterizerVariable::EffectRasterizerVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
			, pRasterizer(pVariable->AsRasterizer())
		{
		}

		EffectRasterizerVariable::~EffectRasterizerVariable()
		{
			SafeRelease(pRasterizer);
		}

		void EffectRasterizerVariable::SetRasterizerState(IRasterizerState* pRasterizerState, uint32_t nIndex)
		{
			if (IsValid() == false || pRasterizerState == nullptr)
				return;

			pRasterizer->SetRasterizerState(nIndex, pRasterizerState->GetInterface());
		}

		void EffectRasterizerVariable::UndoRasterizerState(uint32_t nIndex)
		{
			if (IsValid() == false)
				return;

			pRasterizer->UndoSetRasterizerState(nIndex);
		}

		EffectSamplerVariable::EffectSamplerVariable(ID3DX11EffectVariable* pVariable)
			: EffectVariable(pVariable)
			, pSampler(pVariable->AsSampler())
		{
		}

		EffectSamplerVariable::~EffectSamplerVariable()
		{
			SafeRelease(pSampler);
		}

		void EffectSamplerVariable::SetSamplerState(ISamplerState* pSamplerState, uint32_t nIndex)
		{
			if (IsValid() == false || pSamplerState == nullptr)
				return;

			pSampler->SetSampler(nIndex, pSamplerState->GetInterface());
		}

		void EffectSamplerVariable::UndoSamplerState(uint32_t nIndex)
		{
			if (IsValid() == false)
				return;

			pSampler->UndoSetSampler(nIndex);
		}
	}
}