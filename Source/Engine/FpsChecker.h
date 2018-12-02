#pragma once

namespace eastengine
{
	class FpsChecker
	{
	public:
		FpsChecker();
		~FpsChecker();

		void Update(float elapsedTime);
		float GetFps() { return m_fFps; }

	private:
		uint32_t m_nCount{ 0 };
		float m_fFps{ 0.f };
		float m_fTime{ 0.f };
	};
}