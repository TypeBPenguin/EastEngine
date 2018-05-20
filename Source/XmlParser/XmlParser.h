#pragma once

namespace tinyxml2
{
	class XMLNode;
	class XMLElement;
	class XMLDocument;
}

namespace eastengine
{
	namespace XML
	{
		class CXmlNode
		{
		public:
			CXmlNode(tinyxml2::XMLNode* pNode);
			CXmlNode(tinyxml2::XMLElement* pElement);
			~CXmlNode();

			CXmlNode& operator = (tinyxml2::XMLNode* pNode);
			CXmlNode& operator = (tinyxml2::XMLElement* pElement);

			bool IsVaild() { return m_pNode != nullptr; }

			CXmlNode GetFirstChild();
			CXmlNode GetLastChild();

			CXmlNode GetNext();
			CXmlNode GetPrev();

			tinyxml2::XMLElement* GetFirstChildElement(const char* strName = nullptr);
			tinyxml2::XMLElement* GetLastChildElement(const char* strName = nullptr);

			tinyxml2::XMLElement* ToElement();

		private:
			tinyxml2::XMLNode* m_pNode;
		};

		class CXmlElement
		{
		public:
			CXmlElement(tinyxml2::XMLElement* pElement);
			~CXmlElement();

			CXmlElement& operator = (tinyxml2::XMLElement* pElement) { m_pElement = pElement; return *this; }
			CXmlElement& operator = (CXmlNode node) { m_pElement = node.ToElement(); return *this; }

			bool IsVaild() { return m_pElement != nullptr; }

			const char* GetName();

			const char* Attribute(const char* strName);
			bool AttributeBool(const char* strName);
			int AttributeInt(const char* strName);
			float AttributeFloat(const char* strName);
			double AttributeDouble(const char* strName);

			void SetAttribute(const char* strName, const char* strValue);
			void SetAttribute(const char* strName, bool bValue);
			void SetAttribute(const char* strName, int nValue);
			void SetAttribute(const char* strName, float fValue);
			void SetAttribute(const char* strName, double dValue);

			CXmlNode FirstChild();
			CXmlElement FirstChildElement(const char* strName = nullptr);

			CXmlNode NextSibling();
			CXmlElement NextSiblingElement();

		private:
			tinyxml2::XMLElement* m_pElement;
		};

		class CXmlDoc
		{
		public:
			CXmlDoc();
			~CXmlDoc();

			bool LoadFile(const char* strPath);

			CXmlElement GetRootElement();

			CXmlNode GetFirstChild();
			CXmlNode GetFirstChild(const char* strName);
			CXmlNode GetLastChild();
			CXmlNode GetLastChild(const char* strName);

			CXmlElement GetFirstChildElement(const char* strName = nullptr);
			CXmlElement GetLastChildElement(const char* strName = nullptr);

		private:
			tinyxml2::XMLDocument* m_pDoc;
		};
	};
}