#pragma once

namespace eastengine
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
			void InitFBX(const String::StringID& strMotionName, const char* strFilePath, float fScaleFactor = 1.f);
			void InitXPS(const String::StringID& strMotionName, const char* strFilePath);
			void InitEast(const String::StringID& strMotionName, const char* strFilePath);

		public:
			EmMotionLoader::Type GetLoadMotionType() const { return m_emLoadMotionType; }

			const std::string& GetFilePath() const { return m_strFilePath; }
			const String::StringID& GetName() const { return m_strMotionName; }
			float GetScaleFactor() const { return m_fScaleFactor; }

		private:
			EmMotionLoader::Type m_emLoadMotionType;

			std::string m_strFilePath;
			String::StringID m_strMotionName;
			float m_fScaleFactor;
		};
	}
}