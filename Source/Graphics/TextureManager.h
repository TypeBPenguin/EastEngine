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
			void Flush(float fElapsedTime);

			void AsyncLoadTexture(ITexture* pTexture, const char* strFilePath, std::function<bool(const std::string&)> funcLoad);

		public:
			void PushTexture(ITexture* pTexture);
			ITexture* GetTexture(const ITexture::Key& key);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}