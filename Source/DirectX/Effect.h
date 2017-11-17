#pragma once

#include "EffectInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		struct EffectVariable;

		class EffectTech : public IEffectTech
		{
		public:
			EffectTech(ID3DX11EffectTechnique* pTech, EmVertexFormat::Type emLayoutFormat, const String::StringID& strTechName);
			virtual ~EffectTech();

			virtual void PassApply(uint32_t nIndex, IDeviceContext* pd3dDeviceContext, uint32_t applyFlags = 0) override;

			virtual void GetInputSignatureDesc(uint8_t** ppIAInputSignature_out, std::size_t& IAInputSignatureSize_out) override;
			virtual uint32_t GetPassCount() override;

		public:
			virtual const String::StringID& GetTechName() const override { return m_strTechName; }
			virtual EmVertexFormat::Type GetLayoutFormat() const override { return m_emVertexFormat; }

		private:
			ID3DX11EffectTechnique* m_pTech;
			String::StringID m_strTechName;
			EmVertexFormat::Type m_emVertexFormat;
		};

		class Effect : public IEffect
		{
		public:
			Effect(ID3DX11Effect* pEffect, const String::StringID& strName);
			virtual ~Effect();

			void Optimize();

		public:
			virtual void ClearState(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech) override;

			virtual const String::StringID& GetName() override { return m_strName; }

		public:
			virtual IEffectTech* CreateTechnique(const String::StringID& strName, EmVertexFormat::Type emLayoutFormat) override;
			virtual IEffectTech* GetTechnique(const String::StringID& strName) override;

		public:
			virtual EffectRawHandle* GetRawHandle(const String::StringID& strName) override;
			virtual EffectScalarHandle* GetScalarHandle(const String::StringID& strName) override;
			virtual EffectVectorHandle* GetVectorHandle(const String::StringID& strName) override;
			virtual EffectMatrixHandle* GetMatrixHandle(const String::StringID& strName) override;
			virtual EffectStringHandle* GetStringHandle(const String::StringID& strName) override;
			virtual EffectShaderResourceHandle* GetShaderResourceHandle(const String::StringID& strName) override;
			virtual EffectUnorderedAccessViewHandle* GetUnorderedAccessViewHandle(const String::StringID& strName) override;
			virtual EffectRenderTargetViewHandle* GetRenderTargetViewHandle(const String::StringID& strName) override;
			virtual EffectDepthStencilViewHandle* GetDepthStencilViewHandle(const String::StringID& strName) override;
			virtual EffectConstanceBufferHandle* GetConstanceBufferHandle(const String::StringID& strName) override;
			virtual EffectBlendHandle* GetBlendHandle(const String::StringID& strName) override;
			virtual EffectDepthStencilHandle* GetDepthStencilHandle(const String::StringID& strName) override;
			virtual EffectRasterizerHandle* GetRasterizerHandle(const String::StringID& strName) override;
			virtual EffectSamplerHandle* GetSamplerHandle(const String::StringID& strName) override;

		public:
			virtual void SetRawValue(const String::StringID& strName, const void* pData, uint32_t nByteOffset, uint32_t nByteCount) override;
			virtual void SetBool(const String::StringID& strName, bool value) override;
			virtual void SetBoolArray(const String::StringID& strName, bool* pValue, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetInt(const String::StringID& strName, int nValue) override;
			virtual void SetIntArray(const String::StringID& strName, const int* pValue, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetFloat(const String::StringID& strName, float fValue) override;
			virtual void SetFloatArray(const String::StringID& strName, const float* pValue, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVector(const String::StringID& strName, const Math::Int2& n2Value) override;
			virtual void SetVector(const String::StringID& strName, const Math::Int3& n3Value) override;
			virtual void SetVector(const String::StringID& strName, const Math::Int4& n4Value) override;
			virtual void SetVector(const String::StringID& strName, const Math::Vector2& f2Value) override;
			virtual void SetVector(const String::StringID& strName, const Math::Vector3& f3Value) override;
			virtual void SetVector(const String::StringID& strName, const Math::Vector4& f4Value) override;
			virtual void SetVectorArray(const String::StringID& strName, const Math::Int2* pn2Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const String::StringID& strName, const Math::Int3* pn3Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const String::StringID& strName, const Math::Int4* pn4Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const String::StringID& strName, const Math::Vector2* pf2Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const String::StringID& strName, const Math::Vector3* pf3Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const String::StringID& strName, const Math::Vector4* pf4Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetMatrix(const String::StringID& strName, const Math::Matrix& matrix) override;
			virtual void SetMatrixArray(const String::StringID& strName, const Math::Matrix* pMatrix, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetString(const String::StringID& strName, const char* str, uint32_t nCount) override;
			virtual void SetTexture(const String::StringID& strName, const std::shared_ptr<ITexture>& pTexture) override;
			virtual void SetTextureArray(const String::StringID& strName, const std::shared_ptr<ITexture>* pTextures, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetStructuredBuffer(const String::StringID& strName, IStructuredBuffer* pDataBuffer) override;
			virtual void SetUnorderedAccessView(const String::StringID& strName, IStructuredBuffer* pDataBuffer) override;
			virtual void SetUnorderedAccessViewArray(const String::StringID& strName, IStructuredBuffer** ppDataBuffer, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetBlendState(const String::StringID& strName, IBlendState* pBlendState, uint32_t nIndex) override;
			virtual void UndoBlendState(const String::StringID& strName, uint32_t nIndex) override;
			virtual void SetDepthStencilState(const String::StringID& strName, IDepthStencilState* pDepthStencilState, uint32_t nIndex) override;
			virtual void UndoDepthStencilState(const String::StringID& strName, uint32_t nIndex) override;
			virtual void SetRasterizerState(const String::StringID& strName, IRasterizerState* pRasterizerState, uint32_t nIndex) override;
			virtual void UndoRasterizerState(const String::StringID& strName, uint32_t nIndex) override;
			virtual void SetSamplerState(const String::StringID& strName, ISamplerState* pSamplerState, uint32_t nIndex) override;
			virtual void UndoSamplerState(const String::StringID& strName, uint32_t nIndex) override;

		protected:
			EffectHandle* GetHandle(const String::StringID& strName);

		private:
			ID3DX11Effect* m_pEffect;

			String::StringID m_strName;

			bool m_isOptimized;

			boost::unordered_map<String::StringID, IEffectTech*> m_umapEffectTech;
			boost::unordered_map<String::StringID, EffectHandle*> m_umapEffectHandle;
		};

		struct EffectVariable
		{
		private:
			ID3DX11EffectVariable* pVariable = nullptr;

		public:
			EffectVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectVariable() = 0;

		protected:
			bool IsValid();
			ID3DX11EffectVariable* GetInterface() { return pVariable; }
		};

		struct EffectRawVariable : public EffectVariable, public EffectRawHandle
		{
		public:
			EffectRawVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectRawVariable();

			virtual void SetRawValue(const void* pData, uint32_t nByteOffset, uint32_t nByteCount) override;
		};

		struct EffectScalarVariable : public EffectVariable, public EffectScalarHandle
		{
			ID3DX11EffectScalarVariable* pScalar = nullptr;

			EffectScalarVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectScalarVariable();

			virtual void SetBool(bool value) override;
			virtual void SetBoolArray(bool* pValue, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetInt(int nValue) override;
			virtual void SetIntArray(const int* pData, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetFloat(float fValue) override;
			virtual void SetFloatArray(const float* pData, uint32_t nOffset, uint32_t nCount) override;
		};

		struct EffectVectorVariable : public EffectVariable, public EffectVectorHandle
		{
			ID3DX11EffectVectorVariable* pVector = nullptr;

			EffectVectorVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectVectorVariable();

			virtual void SetVector(const Math::Int2& n2Value) override;
			virtual void SetVector(const Math::Int3& n3Value) override;
			virtual void SetVector(const Math::Int4& n4Value) override;
			virtual void SetVector(const Math::Vector2& f2Value) override;
			virtual void SetVector(const Math::Vector3& f3Value) override;
			virtual void SetVector(const Math::Vector4& f4Value) override;
			virtual void SetVectorArray(const Math::Int2* pData, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const Math::Int3* pData, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const Math::Int4* pData, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const Math::Vector2* pData, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const Math::Vector3* pData, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const Math::Vector4* pData, uint32_t nOffset, uint32_t nCount) override;
		};

		struct EffectMatrixVariable : public EffectVariable, public EffectMatrixHandle
		{
			ID3DX11EffectMatrixVariable* pMatrix = nullptr;

			EffectMatrixVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectMatrixVariable();

			virtual void SetMatrix(const Math::Matrix& matrix) override;
			virtual void SetMatrixArray(const Math::Matrix* pData, uint32_t nOffset, uint32_t nCount) override;
		};

		struct EffectStringVariable : public EffectVariable, public EffectStringHandle
		{
			ID3DX11EffectStringVariable* pString = nullptr;

			EffectStringVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectStringVariable();

			virtual void SetString(const char* str, uint32_t nCount) override;
		};

		struct EffectShaderResourceVariable : public EffectVariable, public EffectShaderResourceHandle
		{
			ID3DX11EffectShaderResourceVariable* pShaderResource = nullptr;

			EffectShaderResourceVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectShaderResourceVariable();

			virtual void SetTexture(const std::shared_ptr<ITexture>& pTexture) override;
			virtual void SetTextureArray(const std::shared_ptr<ITexture>* pTextures, uint32_t nOffset, uint32_t nCount) override;

			virtual void SetStructuredBuffer(IStructuredBuffer* pDataBuffer) override;
		};

		struct EffectUnorderedAccessViewVariable : public EffectVariable, public EffectUnorderedAccessViewHandle
		{
			ID3DX11EffectUnorderedAccessViewVariable* pUnorderedAccessView = nullptr;

			EffectUnorderedAccessViewVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectUnorderedAccessViewVariable();

			virtual void SetUnorderedAccessView(IStructuredBuffer* pDataBuffer) override;
			virtual void SetUnorderedAccessViewArray(IStructuredBuffer** ppDataBuffer, uint32_t nOffset, uint32_t nCount) override;
		};

		struct EffectRenderTargetViewVariable : public EffectVariable, public EffectRenderTargetViewHandle
		{
			EffectRenderTargetViewVariable(ID3DX11EffectVariable* pVariable);
		};

		struct EffectDepthStencilViewVariable : public EffectVariable, public EffectDepthStencilViewHandle
		{
			EffectDepthStencilViewVariable(ID3DX11EffectVariable* pVariable);
		};

		struct EffectConstanceBufferVariable : public EffectVariable, public EffectConstanceBufferHandle
		{
			EffectConstanceBufferVariable(ID3DX11EffectVariable* pVariable);
		};

		struct EffectBlendVariable : public EffectVariable, public EffectBlendHandle
		{
			ID3DX11EffectBlendVariable* pBlend = nullptr;

			EffectBlendVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectBlendVariable();

			virtual void SetBlendState(IBlendState* pBlendState, uint32_t nIndex) override;
			virtual void UndoBlendState(uint32_t nIndex) override;
		};

		struct EffectDepthStencilVariable : public EffectVariable, public EffectDepthStencilHandle
		{
			ID3DX11EffectDepthStencilVariable* pDepthStencil = nullptr;

			EffectDepthStencilVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectDepthStencilVariable();

			virtual void SetDepthStencilState(IDepthStencilState* pDepthStencilState, uint32_t nIndex) override;
			virtual void UndoDepthStencilState(uint32_t nIndex) override;
		};

		struct EffectRasterizerVariable : public EffectVariable, public EffectRasterizerHandle
		{
			ID3DX11EffectRasterizerVariable* pRasterizer = nullptr;

			EffectRasterizerVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectRasterizerVariable();

			virtual void SetRasterizerState(IRasterizerState* pRasterizerState, uint32_t nIndex) override;
			virtual void UndoRasterizerState(uint32_t nIndex) override;
		};

		struct EffectSamplerVariable : public EffectVariable, public EffectSamplerHandle
		{
			ID3DX11EffectSamplerVariable* pSampler = nullptr;

			EffectSamplerVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectSamplerVariable();

			virtual void SetSamplerState(ISamplerState* pSamplerState, uint32_t nIndex) override;
			virtual void UndoSamplerState(uint32_t nIndex) override;
		};
	}
}