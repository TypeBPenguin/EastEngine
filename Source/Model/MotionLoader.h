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
			void InitFBX(const string::StringID& strMotionName, const char* strFilePath, float fScaleFactor = 1.f);
			void InitXPS(const string::StringID& strMotionName, const char* strFilePath);
			void InitEast(const string::StringID& strMotionName, const char* strFilePath);

		public:
			EmMotionLoader::Type GetLoadMotionType() const { return m_emLoadMotionType; }

			const std::string& GetFilePath() const { return m_strFilePath; }
			const string::StringID& GetName() const { return m_strMotionName; }
			float GetScaleFactor() const { return m_fScaleFactor; }

		private:
			EmMotionLoader::Type m_emLoadMotionType;

			std::string m_strFilePath;
			string::StringID m_strMotionName;
			float m_fScaleFactor;
		};
	}
}