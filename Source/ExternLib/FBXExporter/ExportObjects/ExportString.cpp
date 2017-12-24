#include "stdafx.h"
#include "ExportString.h"

namespace ATG
{
	typedef std::list<std::unique_ptr<const CHAR[]>> StringList;
	static StringList s_StringLists[EXPORTSTRING_HASHSIZE];

	void ExportString::ReleaseStringList()
	{
		for (int i = 0; i < EXPORTSTRING_HASHSIZE; ++i)
		{
			s_StringLists[i].clear();
		}
	}

	const CHAR* ExportString::AddString(const CHAR* strString)
	{
		if (!strString)
			return nullptr;

		BYTE uBucketIndex = HashString(strString) & (EXPORTSTRING_HASHSIZE - 1);
		StringList& CurrentList = s_StringLists[uBucketIndex];

		StringList::iterator iter = CurrentList.begin();
		StringList::iterator end = CurrentList.end();

		while (iter != end)
		{
			const CHAR* strTest = iter->get();
			if (EXPORTSTRING_COMPARE(strTest, strString) == 0)
				return strTest;
			++iter;
		}

		size_t dwSize = strlen(strString) + 1;
		char* strCopy = new char[dwSize];
		strcpy_s(strCopy, dwSize, strString);
		CurrentList.push_back(std::move(std::unique_ptr<const char[]>(strCopy)));
		return strCopy;
	}
}