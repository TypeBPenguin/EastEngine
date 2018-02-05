#include "stdafx.h"
#include "Performance.h"

#include "Log.h"
#include "Timer.h"

namespace EastEngine
{
	namespace Performance
	{
		Counter::Counter()
		{
		}

		Counter::~Counter()
		{
		}

		void SimpleProfiler::Update()
		{
			double dGameTime = Timer::GetInstance()->GetGameTime();
			float fElapsedTime = static_cast<float>(dGameTime - dPrevTime);
			fTime += fElapsedTime;
			if (fTime >= 1.f)
			{
				LOG_MESSAGE("%s : %f", strName.c_str(), fTotalTime / nFrame);
				nFrame = 0;
				fTotalTime = 0.f;
				fTime -= 1.f;
			}

			Performance::Counter counter;
			counter.Start();

			func(fElapsedTime);

			counter.End();
			fTotalTime += static_cast<float>(counter.Count());
			++nFrame;

			dPrevTime = dGameTime;
		}
	}
}