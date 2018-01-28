#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;

		class ImageBasedLight : public IImageBasedLight
		{
		public:
			ImageBasedLight();
			virtual ~ImageBasedLight();

		public:
			virtual const std::shared_ptr<ITexture>& GetEnvHDR() const override { return m_pEnvHDR; }
			virtual void SetEnvHDR(const std::shared_ptr<ITexture>& pEnvHDR) override { m_pEnvHDR = pEnvHDR; }

			virtual const std::shared_ptr<ITexture>& GetDiffuseHDR() const override { return m_pDiffuseHDR; }
			virtual void SetDiffuseHDR(const std::shared_ptr<ITexture>& pDiffuseHDR) override { m_pDiffuseHDR = pDiffuseHDR; }

			virtual const std::shared_ptr<ITexture>& GetSpecularHDR() const override { return m_pSpecularHDR; }
			virtual void SetSpecularHDR(const std::shared_ptr<ITexture>& pSpecularHDR) override { m_pSpecularHDR = pSpecularHDR; }

			virtual const std::shared_ptr<ITexture>& GetSpecularBRDF() const override { return m_pSpecularBRDF; }
			virtual void SetSpecularBRDF(const std::shared_ptr<ITexture>& pSpecularBRDF) override { m_pSpecularBRDF = pSpecularBRDF; }

		private:
			std::shared_ptr<ITexture> m_pEnvHDR;
			std::shared_ptr<ITexture> m_pDiffuseHDR;
			std::shared_ptr<ITexture> m_pSpecularHDR;
			std::shared_ptr<ITexture> m_pSpecularBRDF;
		};
	}
}