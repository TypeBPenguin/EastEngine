#pragma once

namespace est
{
	namespace graphics
	{
		namespace EmMotionLoader
		{
			enum Type
			{
				eFbx = 0,
				eXps,
				eEast,
			};
		}

		class MotionLoader
		{
		public:
			MotionLoader();
			~MotionLoader();

		public:
			void InitFBX(const string::StringID& strMotionName, const wchar_t* filePath, float fScaleFactor = 1.f);
			void InitXPS(const string::StringID& strMotionName, const wchar_t* filePath);
			void InitEast(const string::StringID& strMotionName, const wchar_t* filePath);

		public:
			EmMotionLoader::Type GetLoadMotionType() const { return m_emLoadMotionType; }

			const std::wstring& GetFilePath() const { return m_filePath; }
			const string::StringID& GetName() const { return m_strMotionName; }
			float GetScaleFactor() const { return m_scaleFactor; }

		private:
			EmMotionLoader::Type m_emLoadMotionType;

			std::wstring m_filePath;
			string::StringID m_strMotionName;
			float m_scaleFactor{ 1.f };
		};
	}
}