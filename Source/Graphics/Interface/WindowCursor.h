#pragma once

#include "GraphicsInterface.h"

namespace est
{
	namespace graphics
	{
		class WindowCursor : public ICursor
		{
		public:
			WindowCursor();
			virtual ~WindowCursor();

		public:
			virtual void Update(float elapsedTime) override;

		public:
			virtual bool LoadCursor(const string::StringID& key, const std::vector<ImageData>& imageDatas, uint32_t hotSpotX, uint32_t hotSpotY, float playTime, bool isLoop) override;
			virtual bool IsHasCursor(const string::StringID& key) const override;
			virtual void ChangeCursor(const string::StringID& key) override;
			virtual void SetVisible(bool isVisible) override;
			virtual bool IsVisible() override { return m_isVisible; }

		public:
			virtual void SetDefaultCursor(const string::StringID& key) override { m_defaultCursorName = key; }

		public:
			void SetFrame(const string::StringID& key, size_t frame);
			void ResetFrame();

		private:
			struct Cursor
			{
				std::vector<HCURSOR> hCursors;
				uint32_t hotSpotX{ 0 };
				uint32_t hotSpotY{ 0 };
				float playTime{ 0.f };
				bool isLoop{ false };
			};
			tsl::robin_map<string::StringID, Cursor> m_cursors;

			string::StringID m_defaultCursorName;
			string::StringID m_cursorName;

			float m_accumTime{ 0.f };
			bool m_isVisible{ true };
		};
	}
}