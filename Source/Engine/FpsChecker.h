#pragma once

namespace eastengine
{
	class FpsChecker
	{
	public:
		FpsChecker();
		~FpsChecker();

		void Update(float fElapsedTime);
		float GetFps() { return m_fFps; }

	private:
		float m_fFps;
		uint32_t m_nCount;
		float m_fTime;
	};
}