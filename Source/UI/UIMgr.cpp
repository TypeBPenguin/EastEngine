#include "stdafx.h"
#include "UIMgr.h"

#include "../XmlParser/XmlParser.h"
#include "../CommonLib/FileUtil.h"

#include "UIPanel.h"

#include "../DirectX/ISpriteFont.h"

namespace EastEngine
{
	namespace UI
	{
		UIManager::UIManager()
			: m_hWnd(nullptr)
			, m_bInit(false)
		{
		}

		UIManager::~UIManager()
		{
			Release();
		}

		bool UIManager::Init(HWND hWnd)
		{
			if (m_bInit == true)
				return true;

			m_hWnd = hWnd;

			if (loadFont() == false)
			{
				Release();
				return false;
			}

			if (loadTexture() == false)
			{
				Release();
				return false;
			}

			CUIEditBox::IME_Initialize(hWnd);

			m_bInit = true;

			return true;
		}

		void UIManager::Release()
		{
			if (m_bInit == false)
				return;

			CUIEditBox::IME_Uninitialize();

			std::for_each(m_umapPanel.begin(), m_umapPanel.end(), DeleteSTLMapObject());
			m_umapPanel.clear();

			std::for_each(m_vecFontNode.begin(), m_vecFontNode.end(), DeleteSTLObject());
			m_vecFontNode.clear();

			std::for_each(m_vecTextureNode.begin(), m_vecTextureNode.end(), DeleteSTLObject());
			m_vecTextureNode.clear();

			m_hWnd = nullptr;

			m_bInit = false;
		}

		void UIManager::Update(float fElapsedTime)
		{
			for (auto iter = m_umapPanel.begin(); iter != m_umapPanel.end(); ++iter)
			{
				IUIPanel* pUIPanel = iter->second;

				pUIPanel->Update(fElapsedTime);
			}
		}

		bool UIManager::HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			for (auto iter = m_umapPanel.begin(); iter != m_umapPanel.end(); ++iter)
			{
				IUIPanel* pUIPanel = iter->second;

				pUIPanel->HandleMsg(hWnd, nMsg, wParam, lParam);
			}

			return true;
		}

		IUIPanel* UIManager::AddPanel(String::StringID strID, int x, int y, uint32_t nWidth, uint32_t nHeight)
		{
			auto iter = m_umapPanel.find(strID);
			if (iter != m_umapPanel.end())
				return iter->second;

			CUIPanel* pPanel = new CUIPanel(this, m_hWnd);
			pPanel->Init(strID, x, y, nWidth, nHeight);

			m_umapPanel.emplace(strID, pPanel);

			return pPanel;
		}

		bool UIManager::IsMouseOverOnUI()
		{
			for (auto iter = m_umapPanel.begin(); iter != m_umapPanel.end(); ++iter)
			{
				IUIPanel* pUIPanel = iter->second;

				auto pObject = pUIPanel->GetMouseOverUI();
				if (pObject != nullptr)
					return true;
			}

			return false;
		}

		bool UIManager::IsMouseClickOnUI()
		{
			for (auto iter = m_umapPanel.begin(); iter != m_umapPanel.end(); ++iter)
			{
				IUIPanel* pUIPanel = iter->second;

				auto pObject = pUIPanel->GetPressedUI();
				if (pObject != nullptr)
					return true;
			}

			return false;
		}

		/*IUIObject* UIManager::ShowUI(String::StringID strID)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter == m_umapUIObject.end())
				return nullptr;

			iter->second->SetVisible(true);

			return iter->second;
		}

		void UIManager::UnShowUI(String::StringID strID)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter == m_umapUIObject.end())
				return;

			iter->second->SetVisible(false);
		}

		void UIManager::UnShowUI(IUIObject* pUIObject)
		{
			pUIObject->SetVisible(false);
		}

		IUIObject* UIManager::GetUIObject(String::StringID strID)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter == m_umapUIObject.end())
				return nullptr;

			return iter->second;
		}*/

		/*IUIObject* UIManager::GetMouseOverUI()
		{
			for (auto iter = m_umapUIObject.begin(); iter != m_umapUIObject.end(); ++iter)
			{
				IUIObject* pUIObject = iter->second;

				if (pUIObject->IsMouseOverOnUI())
				{
					IUIObject* pChild = pUIObject->GetMouseOverUI();

					if (pChild != nullptr)
						return pChild;

					return pUIObject;
				}
			}

			return nullptr;
		}

		void UIManager::SetMouseOverUI(String::StringID strID)
		{
			auto iter = m_umapUIObject.find(strID);
			if (iter == m_umapUIObject.end())
				return;


		}

		void UIManager::SetMouseOverUI(IUIObject* pUIObject)
		{
		}*/

		IUIPanel* UIManager::GetPanelAtPoint(const POINT& pt)
		{
			for (auto iter = m_umapPanel.begin(); iter != m_umapPanel.end(); ++iter)
			{
				IUIPanel* pPanel = iter->second;

				if (pPanel->IsContainsPoint(pt))
					return pPanel;
			}

			return nullptr;
		}

		IUIObject* UIManager::GetControlAtPoint(const POINT& pt)
		{
			for (auto iter = m_umapPanel.begin(); iter != m_umapPanel.end(); ++iter)
			{
				IUIPanel* pPanel = iter->second;

				if (pPanel->IsContainsPoint(pt))
					return pPanel->GetControlAtPoint(pt);
			}

			return nullptr;
		}

		UIFontNode* UIManager::GetFontNode(String::StringID strNodeName)
		{
			uint32_t nSize = m_vecFontNode.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				if (m_vecFontNode[i]->strClassName == strNodeName)
					return m_vecFontNode[i];
			}

			return nullptr;
		}

		UITextureNode* UIManager::GetTextureNode(String::StringID strNodeName)
		{
			uint32_t nSize = m_vecTextureNode.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				if (m_vecTextureNode[i]->strClassName == strNodeName)
					return m_vecTextureNode[i];
			}

			return nullptr;
		}

		bool UIManager::loadFont()
		{
			std::string strPath = File::GetDataPath();
			std::string strFontListPath = strPath;
			strFontListPath.append("XML\\FontList.xml");

			XML::CXmlDoc doc;
			if (doc.LoadFile(strFontListPath.c_str()) == false)
				return false;

			XML::CXmlNode node = doc.GetFirstChild("Class");
			if (node.IsVaild() == false)
				return false;

			std::string strFilePath = strPath;
			strFilePath.append("Font\\");

			for (XML::CXmlElement element = node.GetFirstChildElement("Font");
				element.IsVaild() == true; element = element.NextSibling())
			{
				std::string strFileName = element.Attribute("File");
				if (strFileName.empty())
					continue;

				std::string strFile = strFilePath + strFileName;
				std::shared_ptr<Graphics::ISpriteFont> pSpriteFont(Graphics::ISpriteFont::Create(strFile.c_str()));

				if (pSpriteFont == nullptr)
					continue;

				Math::Vector2 vSize = pSpriteFont->MeasureString(L"M");

				UIFontNode* pFontNode = new UIFontNode;
				pFontNode->nClassID = element.AttributeInt("ClassID");
				pFontNode->strClassName = element.Attribute("ClassName");
				pFontNode->strFile = element.Attribute("File");
				pFontNode->strName = element.Attribute("Name");
				pFontNode->nSize = element.AttributeInt("Size");
				pFontNode->strNation = element.Attribute("Nation");
				pFontNode->pSpriteFont = pSpriteFont;
				pFontNode->fHeight = vSize.x;
				pFontNode->fWidth = vSize.y;

				m_vecFontNode.push_back(pFontNode);
			}

			return m_vecFontNode.empty() == false;
		}

		bool UIManager::loadTexture()
		{
			std::string strPath = File::GetDataPath();
			std::string strFontListPath = strPath;
			strFontListPath.append("XML\\UITextureList.xml");

			XML::CXmlDoc doc;
			if (doc.LoadFile(strFontListPath.c_str()) == false)
				return false;

			XML::CXmlNode node = doc.GetFirstChild("Class");
			if (node.IsVaild() == false)
				return false;

			std::string strFilePath = strPath;
			strFilePath.append("UI\\");

			for (XML::CXmlElement element = node.GetFirstChildElement("UI");
				element.IsVaild() == true; element = element.NextSibling())
			{
				String::StringID strFileName = element.Attribute("File");
				if (strFileName.empty())
					continue;

				std::string strFullPath = strFilePath;
				strFullPath.append(strFileName.c_str());

				std::shared_ptr<Graphics::ITexture> pTexture = Graphics::ITexture::Create(strFileName, strFullPath, false);
				if (pTexture == nullptr)
					continue;

				UITextureNode* pTextureNode = new UITextureNode;
				pTextureNode->nClassID = element.AttributeInt("ClassID");
				pTextureNode->strClassName = element.Attribute("ClassName");
				pTextureNode->strFile = element.Attribute("File");
				pTextureNode->strName = element.Attribute("Name");
				pTextureNode->pTexture = pTexture;

				for (XML::CXmlElement childElement = element.FirstChildElement("Rect");
					childElement.IsVaild() == true; childElement = childElement.NextSibling())
				{
					String::StringID strName = childElement.Attribute("Name");
					std::string s = strName.c_str();
					auto iter = pTextureNode->umapRect.find(strName);
					if (iter != pTextureNode->umapRect.end())
						continue;

					auto vecSlice = String::Tokenizer(childElement.Attribute("Rect"), " ");
					if (vecSlice.empty())
						continue;

					Math::Rect rect;
					long* pRect = &rect.left;

					uint32_t nSize = vecSlice.size();
					for (uint32_t i = 0; i < nSize; ++i)
					{
						if (vecSlice[i].empty())
							continue;

						pRect[i] = atoi(vecSlice[i].c_str());
					}

					pTextureNode->umapRect.emplace(strName, rect);
				}

				m_vecTextureNode.push_back(pTextureNode);
			}

			return m_vecTextureNode.empty() == false;
		}
	}
}