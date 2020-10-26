#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

namespace est
{
	namespace graphics
	{
		class TextureManager
		{
		public:
			TextureManager();
			~TextureManager();

		public:
			void Cleanup(float elapsedTime);

			TexturePtr AsyncLoadTexture(const TexturePtr& pResource, const wchar_t* filePath, std::function<bool(TexturePtr pTexture, const std::wstring&)> funcLoad);

		public:
			TexturePtr PushTexture(const TexturePtr& pResource);
			TexturePtr GetTexture(const ITexture::Key& key);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}