#include "stdafx.h"
#include "FpsChecker.h"

namespace EastEngine
{
	FpsChecker::FpsChecker()
		: m_fFps(0.f)
		, m_nCount(0)
		, m_fTime(0)
	{
	}

	FpsChecker::~FpsChecker()
	{
	}

	void FpsChecker::Update(float fElapsedTime)
	{
		++m_nCount;
		m_fTime += fElapsedTime;

		if (m_fTime >= 1.f)
		{
			m_fFps = m_nCount / m_fTime;
			m_nCount = 0;

			m_fTime -= 1.f;
		}
	}
}