#include "stdafx.h"
#include "MotionLoader.h"

namespace est
{
	namespace graphics
	{
		MotionLoader::MotionLoader()
			: m_emLoadMotionType(EmMotionLoader::eFbx)
			, m_scaleFactor(1.f)
		{
		}

		MotionLoader::~MotionLoader()
		{
		}

		void MotionLoader::InitFBX(const string::StringID& strMotionName, const wchar_t* filePath, float fScaleFactor)
		{
			m_strMotionName = strMotionName;
			m_filePath = filePath;
			m_scaleFactor = fScaleFactor;

			m_emLoadMotionType = EmMotionLoader::eFbx;
		}

		void MotionLoader::InitXPS(const string::StringID& strMotionName, const wchar_t* filePath)
		{
			m_strMotionName = strMotionName;
			m_filePath = filePath;
			m_scaleFactor = 1.f;

			m_emLoadMotionType = EmMotionLoader::eXps;
		}

		void MotionLoader::InitEast(const string::StringID& strMotionName, const wchar_t* filePath)
		{
			m_strMotionName = strMotionName;
			m_filePath = filePath;
			m_scaleFactor = 1.f;

			m_emLoadMotionType = EmMotionLoader::eEast;
		}
	}
}