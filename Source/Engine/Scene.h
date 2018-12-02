#pragma once

namespace eastengine
{
	class IScene
	{
	public:
		IScene(const string::StringID& strName);
		virtual ~IScene() = 0;

		virtual void Enter() = 0;
		virtual void Exit() = 0;

		virtual void Update(float elapsedTime) = 0;

	public:
		const string::StringID& GetName() const { return m_strName; }

	private:
		string::StringID m_strName;
	};
}