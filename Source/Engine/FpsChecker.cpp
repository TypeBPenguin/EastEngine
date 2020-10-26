#include "stdafx.h"
#include "FpsChecker.h"

namespace est
{
	FpsChecker::FpsChecker()
	{
	}

	FpsChecker::~FpsChecker()
	{
	}

	void FpsChecker::Update(float elapsedTime)
	{
		++m_nCount;
		m_fTime += elapsedTime;

		if (m_fTime >= 1.f)
		{
			m_fFps = static_cast<float>(m_nCount) / m_fTime;
			m_nCount = 0;

			m_fTime -= 1.f;
		}
	}
}