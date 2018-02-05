#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;

		class TextureManager : public Singleton<TextureManager>
		{
			friend Singleton<TextureManager>;
		private:
			TextureManager();
			virtual ~TextureManager();

		public:
			void Flush(bool isEnableGarbageCollector);

			void AsyncLoadTexture(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const std::string& strFilePath);

			bool LoadFromFile(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const char* strFilePath);

		public:
			std::shared_ptr<ITexture> AllocateTexture(const String::StringID& strFileName);
			std::shared_ptr<ITexture> GetTexture(const String::StringID& strFileName);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}