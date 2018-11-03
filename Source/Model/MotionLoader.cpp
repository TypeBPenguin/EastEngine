#include "stdafx.h"
#include "MotionLoader.h"

namespace eastengine
{
	namespace graphics
	{
		MotionLoader::MotionLoader()
			: m_emLoadMotionType(EmMotionLoader::eFbx)
			, m_fScaleFactor(1.f)
		{
		}

		MotionLoader::~MotionLoader()
		{
		}

		void MotionLoader::InitFBX(const string::StringID& strMotionName, const char* strFilePath, float fScaleFactor)
		{
			m_strMotionName = strMotionName;
			m_strFilePath = strFilePath;
			m_fScaleFactor = fScaleFactor;

			m_emLoadMotionType = EmMotionLoader::eFbx;
		}

		void MotionLoader::InitXPS(const string::StringID& strMotionName, const char* strFilePath)
		{
			m_strMotionName = strMotionName;
			m_strFilePath = strFilePath;
			m_fScaleFactor = 1.f;

			m_emLoadMotionType = EmMotionLoader::eXps;
		}

		void MotionLoader::InitEast(const string::StringID& strMotionName, const char* strFilePath)
		{
			m_strMotionName = strMotionName;
			m_strFilePath = strFilePath;
			m_fScaleFactor = 1.f;

			m_emLoadMotionType = EmMotionLoader::eEast;
		}
	}
}