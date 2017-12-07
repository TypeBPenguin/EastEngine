#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;
		class ISpriteFont;
	}

	namespace UI
	{
		const float UI_NEAR_DEPTH = 0.6f;
		const float UI_FAR_DEPTH = 0.8f;

		namespace EmUI
		{
			enum Type
			{
				eStatic = 0,
				eButton,
				eCheckBox,
				eRadioButton,
				eComboBox,
				eSlider,
				eEditBox,
				eListBox,
				eScrollBar,

				eTypeCount,
			};

			enum State
			{
				eNormal = 0,
				eDisabled,
				eHidden,
				eFocus,
				eMouseOver,
				ePressed,

				eStateCount,
			};

			enum AlignHorizontal
			{
				eLeft = 0,
				eCenter,
				eRight,
			};

			enum AlignVertical
			{
				eTop = 0,
				eMiddle,
				eBottom,
			};
		}

		struct UIBlendColor
		{
			Math::Color states[EmUI::eStateCount];
			Math::Color current;

			void Init(const Math::Color& defaultColor,
				const Math::Color& disableColor = Math::Color::Gray,
				const Math::Color& hiddenColor = Math::Color::Transparent,
				const Math::Color* pFocusColor = nullptr,
				const Math::Color* pMouseOverColor = nullptr,
				const Math::Color* pPressedColor = nullptr)
			{
				states[EmUI::eNormal] = defaultColor;
				states[EmUI::eDisabled] = disableColor;
				states[EmUI::eHidden] = hiddenColor;
				states[EmUI::eFocus] = pFocusColor != nullptr ? *pFocusColor : defaultColor;
				states[EmUI::eMouseOver] = pMouseOverColor != nullptr ? *pMouseOverColor : defaultColor;
				states[EmUI::ePressed] = pPressedColor != nullptr ? *pPressedColor : defaultColor;

				current = hiddenColor;
			}

			void SetColor(EmUI::State emState, const Math::Color& color)
			{
				states[emState] = color;
			}

			bool Blend(EmUI::State emState, float fElapsedTime, float fRate = 0.7f)
			{
				current = Math::Color::Lerp(current, states[(uint32_t)(emState)], 1.f - powf(fRate, 30.f * fElapsedTime));

				float* pC = &current.r;
				float* pS = &states[(uint32_t)(emState)].r;

				for (int i = 0; i < 4; ++i)
				{
					if (Math::IsEqual(pC[i], pS[i]) == false)
						return false;

					if (pC[i] >= 0.99f)
					{
						pC[i] = 1.f;
					}

					if (pC[i] <= 0.01f)
					{
						pC[i] = 0.f;
					}
				}

				return true;
			}
		};

		struct UITextureNode
		{
			uint32_t nClassID = UINT32_MAX;
			String::StringID strClassName;
			String::StringID strName;
			String::StringID strFile;

			std::shared_ptr<Graphics::ITexture> pTexture = nullptr;
			std::unordered_map<String::StringID, Math::Rect> umapRect;

			Math::Rect* GetRect(String::StringID strID)
			{
				auto iter = umapRect.find(strID);
				if (iter != umapRect.end())
					return &iter->second;

				return nullptr;
			}
		};

		struct UIFontNode
		{
			uint32_t nClassID;
			String::StringID strClassName;
			String::StringID strName;
			String::StringID strFile;
			uint32_t nSize;
			String::StringID strNation;

			float fWidth = 0;
			float fHeight = 0;

			std::shared_ptr<Graphics::ISpriteFont> pSpriteFont = nullptr;
		};
	}
}
