#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class ImageBasedLight : public IImageBasedLight
		{
		public:
			ImageBasedLight();
			virtual ~ImageBasedLight();

		public:
			virtual ITexture* GetEnvironmentHDR() const override { return m_pEnvironmentHDR; }
			virtual void SetEnvironmentHDR(ITexture* pEnvironmentHDR) override { SetTexture(&m_pEnvironmentHDR, pEnvironmentHDR); }

			virtual ITexture* GetDiffuseHDR() const override { return m_pDiffuseHDR; }
			virtual void SetDiffuseHDR(ITexture* pDiffuseHDR) override { SetTexture(&m_pDiffuseHDR, pDiffuseHDR); }

			virtual ITexture* GetSpecularHDR() const override { return m_pSpecularHDR; }
			virtual void SetSpecularHDR(ITexture* pSpecularHDR) override { SetTexture(&m_pSpecularHDR, pSpecularHDR); }

			virtual ITexture* GetSpecularBRDF() const override { return m_pSpecularBRDF; }
			virtual void SetSpecularBRDF(ITexture* pSpecularBRDF) override { SetTexture(&m_pSpecularBRDF, pSpecularBRDF); }
			
			virtual IVertexBuffer* GetEnvironmentSphereVB() const override { return m_pEnvironmentSphereVB; }
			virtual IIndexBuffer* GetEnvironmentSphereIB() const override { return m_pEnvironmentSphereIB; }

			virtual void SetEnvironmentSphere(IVertexBuffer* pEnvironmentSphereVB, IIndexBuffer* pEnvironmentSphereIB) override
			{
				SetVertexBuffer(&m_pEnvironmentSphereVB, pEnvironmentSphereVB);
				SetIndexBuffer(&m_pEnvironmentSphereIB, pEnvironmentSphereIB);
			}

		private:
			void SetTexture(ITexture** ppDest, ITexture* pSource);
			void SetVertexBuffer(IVertexBuffer** ppDest, IVertexBuffer* pSource);
			void SetIndexBuffer(IIndexBuffer** ppDest, IIndexBuffer* pSource);

		public:
			ITexture* m_pEnvironmentHDR{ nullptr };
			ITexture* m_pDiffuseHDR{ nullptr };
			ITexture* m_pSpecularHDR{ nullptr };
			ITexture* m_pSpecularBRDF{ nullptr };

			IVertexBuffer* m_pEnvironmentSphereVB{ nullptr };
			IIndexBuffer* m_pEnvironmentSphereIB{ nullptr };
		};
	}
}