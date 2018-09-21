#include "stdafx.h"
#include "FpsChecker.h"

namespace eastengine
{
	FpsChecker::FpsChecker()
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
			m_fFps = static_cast<float>(m_nCount) / m_fTime;
			m_nCount = 0;

			m_fTime -= 1.f;
		}
	}
}