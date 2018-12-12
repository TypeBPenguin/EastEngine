#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class ITexture;

		class TextureManager
		{
		public:
			TextureManager();
			~TextureManager();

		public:
			void Cleanup(float elapsedTime);

			ITexture* AsyncLoadTexture(std::unique_ptr<IResource> pResource, const char* strFilePath, std::function<bool(ITexture* pTexture, const std::string&)> funcLoad);

		public:
			ITexture* PushTexture(std::unique_ptr<IResource> pResource);
			ITexture* GetTexture(const ITexture::Key& key);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}