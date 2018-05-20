#pragma once

namespace eastengine
{
	class IScene
	{
	public:
		IScene(const String::StringID& strName);
		virtual ~IScene() = 0;

		virtual void Enter() = 0;
		virtual void Exit() = 0;

		virtual void Update(float fElapsedTime) = 0;

	public:
		const String::StringID& GetName() const { return m_strName; }

	private:
		String::StringID m_strName;
	};
}