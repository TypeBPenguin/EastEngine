#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

namespace est
{
	namespace graphics
	{
		class ImageBasedLight : public IImageBasedLight
		{
		public:
			ImageBasedLight();
			virtual ~ImageBasedLight();

		public:
			virtual TexturePtr GetEnvironmentHDR() const override { return m_pEnvironmentHDR; }
			virtual void SetEnvironmentHDR(const TexturePtr& pEnvironmentHDR) override { SetTexture(&m_pEnvironmentHDR, pEnvironmentHDR); }

			virtual TexturePtr GetDiffuseHDR() const override { return m_pDiffuseHDR; }
			virtual void SetDiffuseHDR(const TexturePtr& pDiffuseHDR) override { SetTexture(&m_pDiffuseHDR, pDiffuseHDR); }

			virtual TexturePtr GetSpecularHDR() const override { return m_pSpecularHDR; }
			virtual void SetSpecularHDR(const TexturePtr& pSpecularHDR) override { SetTexture(&m_pSpecularHDR, pSpecularHDR); }

			virtual TexturePtr GetSpecularBRDF() const override { return m_pSpecularBRDF; }
			virtual void SetSpecularBRDF(const TexturePtr& pSpecularBRDF) override { SetTexture(&m_pSpecularBRDF, pSpecularBRDF); }
			
			virtual VertexBufferPtr GetEnvironmentSphereVB() const override { return m_pEnvironmentSphereVB; }
			virtual IndexBufferPtr GetEnvironmentSphereIB() const override { return m_pEnvironmentSphereIB; }

			virtual void SetEnvironmentSphere(const VertexBufferPtr& pEnvironmentSphereVB, const IndexBufferPtr& pEnvironmentSphereIB) override
			{
				SetVertexBuffer(&m_pEnvironmentSphereVB, pEnvironmentSphereVB);
				SetIndexBuffer(&m_pEnvironmentSphereIB, pEnvironmentSphereIB);
			}

		private:
			void SetTexture(TexturePtr* ppDest, const TexturePtr& pSource);
			void SetVertexBuffer(VertexBufferPtr* ppDest, const VertexBufferPtr& pSource);
			void SetIndexBuffer(IndexBufferPtr* ppDest, const IndexBufferPtr& pSource);

		public:
			TexturePtr m_pEnvironmentHDR{ nullptr };
			TexturePtr m_pDiffuseHDR{ nullptr };
			TexturePtr m_pSpecularHDR{ nullptr };
			TexturePtr m_pSpecularBRDF{ nullptr };

			VertexBufferPtr m_pEnvironmentSphereVB{ nullptr };
			IndexBufferPtr m_pEnvironmentSphereIB{ nullptr };
		};
	}
}