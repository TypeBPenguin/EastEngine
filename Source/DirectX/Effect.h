#pragma once

#include "EffectInterface.h"

namespace eastengine
{
	namespace graphics
	{
		struct EffectVariable;

		class EffectTech : public IEffectTech
		{
		public:
			EffectTech(ID3DX11EffectTechnique* pTech, EmVertexFormat::Type emLayoutFormat, const string::StringID& strTechName);
			virtual ~EffectTech();

			virtual void PassApply(uint32_t nIndex, IDeviceContext* pd3dDeviceContext, uint32_t applyFlags = 0) override;

			virtual void GetInputSignatureDesc(uint8_t** ppIAInputSignature_out, std::size_t& IAInputSignatureSize_out) override;
			virtual uint32_t GetPassCount() override;

		public:
			virtual const string::StringID& GetTechName() const override { return m_strTechName; }
			virtual EmVertexFormat::Type GetLayoutFormat() const override { return m_emVertexFormat; }

		private:
			ID3DX11EffectTechnique* m_pTech;
			string::StringID m_strTechName;
			EmVertexFormat::Type m_emVertexFormat;
		};

		class Effect : public IEffect
		{
		public:
			Effect(const string::StringID& strName);
			virtual ~Effect();

		public:
			void Init(ID3DX11Effect* pEffect) { m_pEffect = pEffect; }
			void Optimize();
			void SetValid(bool isValid) { m_isValid = isValid; }

		public:
			virtual bool IsValid() const override { return m_isValid; }
			virtual void ClearState(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech) override;

			virtual const string::StringID& GetName() override { return m_strName; }

		public:
			virtual IEffectTech* CreateTechnique(const string::StringID& strName, EmVertexFormat::Type emLayoutFormat) override;
			virtual IEffectTech* GetTechnique(const string::StringID& strName) override;

		public:
			virtual EffectRawHandle* GetRawHandle(const string::StringID& strName) override;
			virtual EffectScalarHandle* GetScalarHandle(const string::StringID& strName) override;
			virtual EffectVectorHandle* GetVectorHandle(const string::StringID& strName) override;
			virtual EffectMatrixHandle* GetMatrixHandle(const string::StringID& strName) override;
			virtual EffectStringHandle* GetStringHandle(const string::StringID& strName) override;
			virtual EffectShaderResourceHandle* GetShaderResourceHandle(const string::StringID& strName) override;
			virtual EffectUnorderedAccessViewHandle* GetUnorderedAccessViewHandle(const string::StringID& strName) override;
			virtual EffectRenderTargetViewHandle* GetRenderTargetViewHandle(const string::StringID& strName) override;
			virtual EffectDepthStencilViewHandle* GetDepthStencilViewHandle(const string::StringID& strName) override;
			virtual EffectConstanceBufferHandle* GetConstanceBufferHandle(const string::StringID& strName) override;
			virtual EffectBlendHandle* GetBlendHandle(const string::StringID& strName) override;
			virtual EffectDepthStencilHandle* GetDepthStencilHandle(const string::StringID& strName) override;
			virtual EffectRasterizerHandle* GetRasterizerHandle(const string::StringID& strName) override;
			virtual EffectSamplerHandle* GetSamplerHandle(const string::StringID& strName) override;

		public:
			virtual void SetRawValue(const string::StringID& strName, const void* pData, uint32_t nByteOffset, uint32_t nByteCount) override;
			virtual void SetBool(const string::StringID& strName, bool value) override;
			virtual void SetBoolArray(const string::StringID& strName, bool* pValue, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetInt(const string::StringID& strName, int nValue) override;
			virtual void SetIntArray(const string::StringID& strName, const int* pValue, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetFloat(const string::StringID& strName, float fValue) override;
			virtual void SetFloatArray(const string::StringID& strName, const float* pValue, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVector(const string::StringID& strName, const math::Int2& n2Value) override;
			virtual void SetVector(const string::StringID& strName, const math::Int3& n3Value) override;
			virtual void SetVector(const string::StringID& strName, const math::Int4& n4Value) override;
			virtual void SetVector(const string::StringID& strName, const math::Vector2& f2Value) override;
			virtual void SetVector(const string::StringID& strName, const math::Vector3& f3Value) override;
			virtual void SetVector(const string::StringID& strName, const math::Vector4& f4Value) override;
			virtual void SetVectorArray(const string::StringID& strName, const math::Int2* pn2Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const string::StringID& strName, const math::Int3* pn3Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const string::StringID& strName, const math::Int4* pn4Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const string::StringID& strName, const math::Vector2* pf2Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const string::StringID& strName, const math::Vector3* pf3Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const string::StringID& strName, const math::Vector4* pf4Value, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetMatrix(const string::StringID& strName, const math::Matrix& matrix) override;
			virtual void SetMatrixArray(const string::StringID& strName, const math::Matrix* pMatrix, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetString(const string::StringID& strName, const char* str, uint32_t nCount) override;
			virtual void SetTexture(const string::StringID& strName, const std::shared_ptr<ITexture>& pTexture) override;
			virtual void SetTextureArray(const string::StringID& strName, const std::shared_ptr<ITexture>* pTextures, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetStructuredBuffer(const string::StringID& strName, IStructuredBuffer* pDataBuffer) override;
			virtual void SetUnorderedAccessView(const string::StringID& strName, IStructuredBuffer* pDataBuffer) override;
			virtual void SetUnorderedAccessViewArray(const string::StringID& strName, IStructuredBuffer** ppDataBuffer, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetBlendState(const string::StringID& strName, IBlendState* pBlendState, uint32_t nIndex) override;
			virtual void UndoBlendState(const string::StringID& strName, uint32_t nIndex) override;
			virtual void SetDepthStencilState(const string::StringID& strName, IDepthStencilState* pDepthStencilState, uint32_t nIndex) override;
			virtual void UndoDepthStencilState(const string::StringID& strName, uint32_t nIndex) override;
			virtual void SetRasterizerState(const string::StringID& strName, IRasterizerState* pRasterizerState, uint32_t nIndex) override;
			virtual void UndoRasterizerState(const string::StringID& strName, uint32_t nIndex) override;
			virtual void SetSamplerState(const string::StringID& strName, ISamplerState* pSamplerState, uint32_t nIndex) override;
			virtual void UndoSamplerState(const string::StringID& strName, uint32_t nIndex) override;

		protected:
			EffectHandle* GetHandle(const string::StringID& strName);

		private:
			ID3DX11Effect* m_pEffect;

			string::StringID m_strName;

			bool m_isOptimized;
			bool m_isValid;

			std::unordered_map<string::StringID, IEffectTech*> m_umapEffectTech;
			std::unordered_map<string::StringID, EffectHandle*> m_umapEffectHandle;
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

			virtual void SetVector(const math::Int2& n2Value) override;
			virtual void SetVector(const math::Int3& n3Value) override;
			virtual void SetVector(const math::Int4& n4Value) override;
			virtual void SetVector(const math::Vector2& f2Value) override;
			virtual void SetVector(const math::Vector3& f3Value) override;
			virtual void SetVector(const math::Vector4& f4Value) override;
			virtual void SetVectorArray(const math::Int2* pData, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const math::Int3* pData, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const math::Int4* pData, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const math::Vector2* pData, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const math::Vector3* pData, uint32_t nOffset, uint32_t nCount) override;
			virtual void SetVectorArray(const math::Vector4* pData, uint32_t nOffset, uint32_t nCount) override;
		};

		struct EffectMatrixVariable : public EffectVariable, public EffectMatrixHandle
		{
			ID3DX11EffectMatrixVariable* pMatrix = nullptr;

			EffectMatrixVariable(ID3DX11EffectVariable* pVariable);
			virtual ~EffectMatrixVariable();

			virtual void SetMatrix(const math::Matrix& matrix) override;
			virtual void SetMatrixArray(const math::Matrix* pData, uint32_t nOffset, uint32_t nCount) override;
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