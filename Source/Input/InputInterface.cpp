#include "stdafx.h"
#include "InputInterface.h"

#include "InputDevice.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "GamePad.h"

namespace eastengine
{
	namespace input
	{
		namespace mouse
		{
			bool IsButtonEvent(mouse::Button emMouseButton)
			{
				return Device::GetInstance()->GetMouse()->IsButtonEvent(emMouseButton);
			}

			bool IsButtonDown(mouse::Button emMouseButton)
			{
				return Device::GetInstance()->GetMouse()->IsButtonDown(emMouseButton);
			}

			bool IsButtonPressed(mouse::Button emMouseButton)
			{
				return Device::GetInstance()->GetMouse()->IsButtonPressed(emMouseButton);
			}

			bool IsButtonUp(mouse::Button emMouseButton)
			{
				return Device::GetInstance()->GetMouse()->IsButtonUp(emMouseButton);
			}

			int	GetX()
			{
				return Device::GetInstance()->GetMouse()->GetX();
			}

			int GetY()
			{
				return Device::GetInstance()->GetMouse()->GetY();
			}

			long GetMoveX()
			{
				return Device::GetInstance()->GetMouse()->GetMoveX();
			}

			long GetMoveY()
			{
				return Device::GetInstance()->GetMouse()->GetMoveY();
			}

			long GetMoveWheel()
			{
				return Device::GetInstance()->GetMouse()->GetMoveWheel();
			}
		}

		namespace keyboard
		{
			static_assert(DIK_ESCAPE == eEscape, "DIK Mismatch");
			static_assert(DIK_1 == e1, "DIK Mismatch");
			static_assert(DIK_2 == e2, "DIK Mismatch");
			static_assert(DIK_3 == e3, "DIK Mismatch");
			static_assert(DIK_4 == e4, "DIK Mismatch");
			static_assert(DIK_5 == e5, "DIK Mismatch");
			static_assert(DIK_6 == e6, "DIK Mismatch");
			static_assert(DIK_7 == e7, "DIK Mismatch");
			static_assert(DIK_8 == e8, "DIK Mismatch");
			static_assert(DIK_9 == e9, "DIK Mismatch");
			static_assert(DIK_0 == e0, "DIK Mismatch");
			static_assert(DIK_MINUS == eMinus, "DIK Mismatch");
			static_assert(DIK_EQUALS == eEquals, "DIK Mismatch");
			static_assert(DIK_BACK == eBack, "DIK Mismatch");
			static_assert(DIK_TAB == eTab, "DIK Mismatch");
			static_assert(DIK_Q == eQ, "DIK Mismatch");
			static_assert(DIK_W == eW, "DIK Mismatch");
			static_assert(DIK_E == eE, "DIK Mismatch");
			static_assert(DIK_R == eR, "DIK Mismatch");
			static_assert(DIK_T == eT, "DIK Mismatch");
			static_assert(DIK_Y == eY, "DIK Mismatch");
			static_assert(DIK_U == eU, "DIK Mismatch");
			static_assert(DIK_I == eI, "DIK Mismatch");
			static_assert(DIK_O == eO, "DIK Mismatch");
			static_assert(DIK_P == eP, "DIK Mismatch");
			static_assert(DIK_LBRACKET == eLeftBracket, "DIK Mismatch");
			static_assert(DIK_RBRACKET == eRightBracket, "DIK Mismatch");
			static_assert(DIK_RETURN == eEnter, "DIK Mismatch");
			static_assert(DIK_LCONTROL == eLeftControl, "DIK Mismatch");
			static_assert(DIK_A == eA, "DIK Mismatch");
			static_assert(DIK_S == eS, "DIK Mismatch");
			static_assert(DIK_D == eD, "DIK Mismatch");
			static_assert(DIK_F == eF, "DIK Mismatch");
			static_assert(DIK_G == eG, "DIK Mismatch");
			static_assert(DIK_H == eH, "DIK Mismatch");
			static_assert(DIK_J == eJ, "DIK Mismatch");
			static_assert(DIK_K == eK, "DIK Mismatch");
			static_assert(DIK_L == eL, "DIK Mismatch");
			static_assert(DIK_SEMICOLON == eSemiColon, "DIK Mismatch");
			static_assert(DIK_APOSTROPHE == eApostrophe, "DIK Mismatch");
			static_assert(DIK_GRAVE == eGrave, "DIK Mismatch");
			static_assert(DIK_LSHIFT == eLeftShift, "DIK Mismatch");
			static_assert(DIK_BACKSLASH == eBackSlash, "DIK Mismatch");
			static_assert(DIK_Z == eZ, "DIK Mismatch");
			static_assert(DIK_X == eX, "DIK Mismatch");
			static_assert(DIK_C == eC, "DIK Mismatch");
			static_assert(DIK_V == eV, "DIK Mismatch");
			static_assert(DIK_B == eB, "DIK Mismatch");
			static_assert(DIK_N == eN, "DIK Mismatch");
			static_assert(DIK_M == eM, "DIK Mismatch");
			static_assert(DIK_COMMA == eComma, "DIK Mismatch");
			static_assert(DIK_PERIOD == ePeriod, "DIK Mismatch");
			static_assert(DIK_SLASH == eSlash, "DIK Mismatch");
			static_assert(DIK_RSHIFT == eRightShift, "DIK Mismatch");
			static_assert(DIK_MULTIPLY == eMultiply, "DIK Mismatch");
			static_assert(DIK_LMENU == eLeftAlt, "DIK Mismatch");
			static_assert(DIK_SPACE == eSpace, "DIK Mismatch");
			static_assert(DIK_CAPITAL == eCapital, "DIK Mismatch");
			static_assert(DIK_F1 == eF1, "DIK Mismatch");
			static_assert(DIK_F2 == eF2, "DIK Mismatch");
			static_assert(DIK_F3 == eF3, "DIK Mismatch");
			static_assert(DIK_F4 == eF4, "DIK Mismatch");
			static_assert(DIK_F5 == eF5, "DIK Mismatch");
			static_assert(DIK_F6 == eF6, "DIK Mismatch");
			static_assert(DIK_F7 == eF7, "DIK Mismatch");
			static_assert(DIK_F8 == eF8, "DIK Mismatch");
			static_assert(DIK_F9 == eF9, "DIK Mismatch");
			static_assert(DIK_F10 == eF10, "DIK Mismatch");
			static_assert(DIK_NUMLOCK == eNumLock, "DIK Mismatch");
			static_assert(DIK_SCROLL == eScrollLock, "DIK Mismatch");
			static_assert(DIK_NUMPAD7 == eNumPad7, "DIK Mismatch");
			static_assert(DIK_NUMPAD8 == eNumPad8, "DIK Mismatch");
			static_assert(DIK_NUMPAD9 == eNumPad9, "DIK Mismatch");
			static_assert(DIK_SUBTRACT == eSubTract, "DIK Mismatch");
			static_assert(DIK_NUMPAD4 == eNumPad4, "DIK Mismatch");
			static_assert(DIK_NUMPAD5 == eNumPad5, "DIK Mismatch");
			static_assert(DIK_NUMPAD6 == eNumPad6, "DIK Mismatch");
			static_assert(DIK_ADD == eAdd, "DIK Mismatch");
			static_assert(DIK_NUMPAD1 == eNumPad1, "DIK Mismatch");
			static_assert(DIK_NUMPAD2 == eNumPad2, "DIK Mismatch");
			static_assert(DIK_NUMPAD3 == eNumPad3, "DIK Mismatch");
			static_assert(DIK_NUMPAD0 == eNumPad0, "DIK Mismatch");
			static_assert(DIK_DECIMAL == eDecimal, "DIK Mismatch");
			static_assert(DIK_OEM_102 == eOEM_102, "DIK Mismatch");
			static_assert(DIK_F11 == eF11, "DIK Mismatch");
			static_assert(DIK_F12 == eF12, "DIK Mismatch");
			static_assert(DIK_F13 == eF13, "DIK Mismatch");
			static_assert(DIK_F14 == eF14, "DIK Mismatch");
			static_assert(DIK_F15 == eF15, "DIK Mismatch");
			static_assert(DIK_KANA == eKaka, "DIK Mismatch");
			static_assert(DIK_ABNT_C1 == eABNT_C1, "DIK Mismatch");
			static_assert(DIK_CONVERT == eConvert, "DIK Mismatch");
			static_assert(DIK_NOCONVERT == eNoConvert, "DIK Mismatch");
			static_assert(DIK_YEN == eYen, "DIK Mismatch");
			static_assert(DIK_ABNT_C2 == eABNT_C2, "DIK Mismatch");
			static_assert(DIK_NUMPADEQUALS == eNumpadEquals, "DIK Mismatch");
			static_assert(DIK_PREVTRACK == ePrevTrack, "DIK Mismatch");
			static_assert(DIK_AT == eAT, "DIK Mismatch");
			static_assert(DIK_COLON == eColon, "DIK Mismatch");
			static_assert(DIK_UNDERLINE == eUnderLine, "DIK Mismatch");
			static_assert(DIK_KANJI == eKanji, "DIK Mismatch");
			static_assert(DIK_STOP == eStop, "DIK Mismatch");
			static_assert(DIK_AX == eAX, "DIK Mismatch");
			static_assert(DIK_UNLABELED == eUnLabeled, "DIK Mismatch");
			static_assert(DIK_NEXTTRACK == eNextTrack, "DIK Mismatch");
			static_assert(DIK_NUMPADENTER == eNumPadEnter, "DIK Mismatch");
			static_assert(DIK_RCONTROL == eRightControl, "DIK Mismatch");
			static_assert(DIK_MUTE == eMute, "DIK Mismatch");
			static_assert(DIK_CALCULATOR == eCalculator, "DIK Mismatch");
			static_assert(DIK_PLAYPAUSE == ePlayPause, "DIK Mismatch");
			static_assert(DIK_MEDIASTOP == eMediaStop, "DIK Mismatch");
			static_assert(DIK_VOLUMEDOWN == eVolumeDown, "DIK Mismatch");
			static_assert(DIK_VOLUMEUP == eVolumeUp, "DIK Mismatch");
			static_assert(DIK_WEBHOME == eWebHome, "DIK Mismatch");
			static_assert(DIK_NUMPADCOMMA == eNumPadComma, "DIK Mismatch");
			static_assert(DIK_DIVIDE == eDivide, "DIK Mismatch");
			static_assert(DIK_SYSRQ == eSysRQ, "DIK Mismatch");
			static_assert(DIK_RMENU == eRightAlt, "DIK Mismatch");
			static_assert(DIK_PAUSE == ePause, "DIK Mismatch");
			static_assert(DIK_HOME == eHome, "DIK Mismatch");
			static_assert(DIK_UP == eUp, "DIK Mismatch");
			static_assert(DIK_PRIOR == ePageUp, "DIK Mismatch");
			static_assert(DIK_LEFT == eLeft, "DIK Mismatch");
			static_assert(DIK_RIGHT == eRight, "DIK Mismatch");
			static_assert(DIK_END == eEnd, "DIK Mismatch");
			static_assert(DIK_DOWN == eDown, "DIK Mismatch");
			static_assert(DIK_NEXT == ePageDown, "DIK Mismatch");
			static_assert(DIK_INSERT == eInsert, "DIK Mismatch");
			static_assert(DIK_DELETE == eDelete, "DIK Mismatch");
			static_assert(DIK_LWIN == eLeftWin, "DIK Mismatch");
			static_assert(DIK_RWIN == eRightWin, "DIK Mismatch");
			static_assert(DIK_APPS == eApps, "DIK Mismatch");
			static_assert(DIK_POWER == eSysPower, "DIK Mismatch");
			static_assert(DIK_SLEEP == eSysSleep, "DIK Mismatch");
			static_assert(DIK_WAKE == eSysWake, "DIK Mismatch");
			static_assert(DIK_WEBSEARCH == eWebSearch, "DIK Mismatch");
			static_assert(DIK_WEBFAVORITES == eWebFavorites, "DIK Mismatch");
			static_assert(DIK_WEBREFRESH == eWebRefresh, "DIK Mismatch");
			static_assert(DIK_WEBSTOP == eWebStop, "DIK Mismatch");
			static_assert(DIK_WEBFORWARD == eWebForward, "DIK Mismatch");
			static_assert(DIK_WEBBACK == eWebBack, "DIK Mismatch");
			static_assert(DIK_MYCOMPUTER == eMyComputer, "DIK Mismatch");
			static_assert(DIK_MAIL == eMail, "DIK Mismatch");
			static_assert(DIK_MEDIASELECT == eMediaSelect, "DIK Mismatch");

			bool IsKeyEvent(keyboard::KeyCode emKeyCode)
			{
				return Device::GetInstance()->GetKeyboard()->IsKeyEvent(emKeyCode);
			}

			bool IsKeyDown(keyboard::KeyCode emKeyCode)
			{
				return Device::GetInstance()->GetKeyboard()->IsKeyDown(emKeyCode);
			}

			bool IsKeyPressed(keyboard::KeyCode emKeyCode)
			{
				return Device::GetInstance()->GetKeyboard()->IsKeyPressed(emKeyCode);
			}

			bool IsKeyUp(keyboard::KeyCode emKeyCode)
			{
				return Device::GetInstance()->GetKeyboard()->IsKeyUp(emKeyCode);
			}
		}

		namespace gamepad
		{
			GamePadInstance::Player* GetPlayer(PlayerID playerID)
			{
				GamePadInstance::Player* pPlayer = Device::GetInstance()->GetGamePad()->GetPlayer(playerID);
				assert(pPlayer != nullptr);
				return pPlayer;
			}

			bool IsConnected(PlayerID playerID)
			{
				return GetPlayer(playerID)->IsConnected();
			}

			Type GetType(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetCapabilities().emGamepadType;
			}

			DeadZone GetDeadZone(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetDeadZone();
			}

			void SetDeadZone(DeadZone emDeadZone, PlayerID playerID)
			{
				return GetPlayer(playerID)->SetDeadZone(emDeadZone);
			}

			ButtonState A(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().A();
			}

			ButtonState B(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().B();
			}

			ButtonState X(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().X();
			}

			ButtonState Y(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().Y();
			}

			ButtonState LeftStick(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().LeftStick();
			}

			ButtonState RightStick(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().RightStick();
			}

			ButtonState LeftShoulder(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().LeftShoulder();
			}

			ButtonState RightShoulder(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().RightShoulder();
			}

			ButtonState Back(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().Back();
			}

			ButtonState Start(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().Start();
			}

			ButtonState DPadUp(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().DPadUp();
			}

			ButtonState DPadDown(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().DPadDown();
			}

			ButtonState DPadLeft(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().DPadLeft();
			}

			ButtonState DPadRight(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetButtonStateTracker().DPadRight();
			}

			float LeftThumbStickX(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetCurState().GetLeftThumbStickX();
			}

			float LeftThumbStickY(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetCurState().GetLeftThumbStickY();
			}

			float RightThumbStickX(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetCurState().GetRightThumbStickX();
			}

			float RightThumbStickY(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetCurState().GetRightThumbStickY();
			}

			float LeftTrigger(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetCurState().GetLeftTrigger();
			}

			float RightTrigger(PlayerID playerID)
			{
				return GetPlayer(playerID)->GetCurState().GetRightTrigger();
			}

			bool SetVibration(float fLeftMotor, float fRightMotor, float fVibrationTime, PlayerID playerID)
			{
				return GetPlayer(playerID)->SetVibration(fLeftMotor, fRightMotor, fVibrationTime);
			}
		}
	}
}