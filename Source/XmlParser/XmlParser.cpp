#include "stdafx.h"
#include "XmlParser.h"

using namespace tinyxml2;

namespace EastEngine
{
	namespace XML
	{
		CXmlNode::CXmlNode(XMLNode* pNode)
			: m_pNode(pNode)
		{
		}

		CXmlNode::CXmlNode(XMLElement* pElement)
			: m_pNode(pElement)
		{
		}

		CXmlNode::~CXmlNode()
		{
		}

		CXmlNode& CXmlNode::operator = (tinyxml2::XMLNode* pNode)
		{
			m_pNode = pNode; return *this;
		}

		CXmlNode& CXmlNode::operator = (tinyxml2::XMLElement* pElement)
		{
			m_pNode = pElement; return *this;
		}

		CXmlNode CXmlNode::GetFirstChild()
		{
			return m_pNode->FirstChild();
		}

		CXmlNode CXmlNode::GetLastChild()
		{
			return m_pNode->LastChild();
		}

		CXmlNode CXmlNode::GetNext()
		{
			return m_pNode->NextSibling();
		}

		CXmlNode CXmlNode::GetPrev()
		{
			return m_pNode->PreviousSibling();
		}

		tinyxml2::XMLElement* CXmlNode::GetFirstChildElement(const char* strName)
		{
			if (m_pNode == nullptr)
				return nullptr;

			return m_pNode->FirstChildElement(strName);
		}

		tinyxml2::XMLElement* CXmlNode::GetLastChildElement(const char* strName)
		{
			if (m_pNode == nullptr)
				return nullptr;

			return m_pNode->LastChildElement(strName);
		}

		tinyxml2::XMLElement* CXmlNode::ToElement()
		{
			if (m_pNode == nullptr)
				return nullptr;

			return m_pNode->ToElement();
		}

		CXmlElement::CXmlElement(XMLElement* pElement)
			: m_pElement(pElement)
		{
		}

		CXmlElement::~CXmlElement()
		{
		}

		const char* CXmlElement::GetName()
		{
			return m_pElement->Name();
		}

		const char* CXmlElement::Attribute(const char* strName)
		{
			return m_pElement->Attribute(strName);
		}

		bool CXmlElement::AttributeBool(const char* strName)
		{
			return m_pElement->BoolAttribute(strName);
		}

		int CXmlElement::AttributeInt(const char* strName)
		{
			return m_pElement->IntAttribute(strName);
		}

		float CXmlElement::AttributeFloat(const char* strName)
		{
			return m_pElement->FloatAttribute(strName);
		}

		double CXmlElement::AttributeDouble(const char* strName)
		{
			return m_pElement->DoubleAttribute(strName);
		}

		void CXmlElement::SetAttribute(const char* strName, const char* strValue)
		{
			return m_pElement->SetAttribute(strName, strValue);
		}

		void CXmlElement::SetAttribute(const char* strName, bool bValue)
		{
			return m_pElement->SetAttribute(strName, bValue);
		}

		void CXmlElement::SetAttribute(const char* strName, int nValue)
		{
			return m_pElement->SetAttribute(strName, nValue);
		}

		void CXmlElement::SetAttribute(const char* strName, float fValue)
		{
			return m_pElement->SetAttribute(strName, fValue);
		}

		void CXmlElement::SetAttribute(const char* strName, double dValue)
		{
			return m_pElement->SetAttribute(strName, dValue);
		}

		CXmlNode CXmlElement::FirstChild()
		{
			return m_pElement->FirstChild();
		}

		CXmlElement CXmlElement::FirstChildElement(const char* strName)
		{
			return m_pElement->FirstChildElement(strName);
		}

		CXmlNode CXmlElement::NextSibling()
		{
			return m_pElement->NextSibling();
		}

		CXmlElement CXmlElement::NextSiblingElement()
		{
			return m_pElement->NextSiblingElement();
		}

		CXmlDoc::CXmlDoc()
			: m_pDoc(new tinyxml2::XMLDocument)
		{
		}

		CXmlDoc::~CXmlDoc()
		{
			delete m_pDoc;
			m_pDoc = nullptr;
		}

		bool CXmlDoc::LoadFile(const char* strPath)
		{
			XMLError error = m_pDoc->LoadFile(strPath);

			if (error == XML_SUCCESS)
				return true;

			return false;
		}

		CXmlElement CXmlDoc::GetRootElement()
		{
			return m_pDoc->RootElement();
		}

		CXmlNode CXmlDoc::GetFirstChild()
		{
			return m_pDoc->FirstChild();
		}

		CXmlNode CXmlDoc::GetFirstChild(const char* strName)
		{
			return m_pDoc->FirstChildElement(strName);
		}

		CXmlNode CXmlDoc::GetLastChild()
		{
			return m_pDoc->LastChild();
		}

		CXmlNode CXmlDoc::GetLastChild(const char* strName)
		{
			return m_pDoc->FirstChildElement(strName);
		}

		CXmlElement CXmlDoc::GetFirstChildElement(const char* strName)
		{
			return m_pDoc->FirstChildElement(strName);
		}

		CXmlElement CXmlDoc::GetLastChildElement(const char* strName)
		{
			return m_pDoc->LastChildElement(strName);
		}
	}
}