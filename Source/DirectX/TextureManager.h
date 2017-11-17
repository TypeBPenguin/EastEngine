#pragma once

#include "../CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;

		/*enum
		{
			eTextureAtlasWidth = (1 << 14),
			eTextureAtlasHeight = (1 << 14),
		};*/
		
		class TextureManager : public Singleton<TextureManager>
		{
			friend Singleton<TextureManager>;
		private:
			TextureManager();
			virtual ~TextureManager();

			/*class TextureAtlasNode
			{
				friend TextureManager;
			private:
				TextureAtlasNode(const Math::Rect& rect);
				~TextureAtlasNode();

			public:
				TextureAtlasNode* Insert(const Math::Rect& rectTexture);
				bool Delete(int nAtalasNodeID);

				TextureAtlasNode* GetNode(int nAtalasNodeID);

				const Math::Rect& GetRect() const { return m_rect; }
				int GetID() const { return m_nAtlasNodeID; }

			private:
				int m_nAtlasNodeID;
				TextureAtlasNode* m_pLeft;
				TextureAtlasNode* m_pRight;
				Math::Rect m_rect;
				bool m_isFilled;

			private:
				static int s_nTotalPixelCount;
				static int s_nFilledPixelCount;
			};*/

			struct RequestLoadTextureInfo
			{
				String::StringID strName;
				std::string strFilePath;
				std::shared_ptr<ITexture> pTexture_out = nullptr;

				RequestLoadTextureInfo()
				{
				}

				RequestLoadTextureInfo(String::StringID strName, const std::string& strFilePath, const std::shared_ptr<ITexture>& pTexture_out)
					: strName(strName)
					, strFilePath(strFilePath)
					, pTexture_out(pTexture_out)
				{
				}

				RequestLoadTextureInfo(const RequestLoadTextureInfo& source)
				{
					strName = source.strName;
					strFilePath = source.strFilePath;
					pTexture_out = source.pTexture_out;
				}
			};

			struct ResultLoadTextureInfo
			{
				std::shared_ptr<ITexture> pTexture_out = nullptr;
				bool isSuccess = false;
			};

		public:
			bool Init();
			void Release();

			void Update(float fElapsedTime);
			void Flush(bool isForceFlush = false);

			void ProcessRequestTexture(const RequestLoadTextureInfo& loader);

			void LoadTextureSync(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const std::string& strFilePath);

			bool AddTexture(const String::StringID& strFileName, const std::shared_ptr<ITexture>& pTexture);
			const std::shared_ptr<ITexture>& GetTexture(const String::StringID& strFileName);
			const std::shared_ptr<ITexture>& GetUVCheckerTexture() { return m_pUvCheckerTexture; }
			//const std::shared_ptr<ITexture>& GetTextureAtlas() { return m_pTextureAtlas; }

			//void WriteTextureAtlas(const std::shared_ptr<ITexture>& pTexture) { m_conQueueTextureAtlas.push(pTexture); }

			bool LoadFromFIle(const std::shared_ptr<ITexture>& pTexture, const String::StringID& strName, const char* strFilePath);

		//private:
		//	bool initAtlasPool();

		private:
			std::shared_ptr<ITexture> m_pUvCheckerTexture;
			std::shared_ptr<ITexture> m_pEmptyTextureRed;

			//std::shared_ptr<ITexture> m_pTextureAtlas;
			//TextureAtlasNode* m_pTextureAtlasNode;

			//Concurrency::concurrent_queue<std::shared_ptr<ITexture>> m_conQueueTextureAtlas;
			//Concurrency::concurrent_queue<int> m_conQueueTextureAtlasRemove;

			Concurrency::concurrent_queue<RequestLoadTextureInfo> m_conQueueRequestLoadTexture;
			Concurrency::concurrent_queue<ResultLoadTextureInfo> m_conQueueCompleteTexture;
			boost::unordered_map<String::StringID, std::shared_ptr<ITexture>> m_umapTexture;

			bool m_isInit;
			bool m_isLoading;
		};
	}
}