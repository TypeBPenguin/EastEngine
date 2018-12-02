#pragma once

namespace eastengine
{
	namespace input
	{
		namespace mouse
		{
			enum Button
			{
				eLeft = 0,
				eRight,
				eMiddle,
				eUndefine1,
				eUndefine2,
				eUndefine3,
				eUndefine4,
				eUndefine5,

				ButtonCount,
			};

			enum ButtonState
			{
				eIdle = 0,
				eDown,
				eUp,
				ePressed,
			};

			bool IsButtonEvent(mouse::Button emMouseButton);
			bool IsButtonDown(mouse::Button emMouseButton);
			bool IsButtonPressed(mouse::Button emMouseButton);
			bool IsButtonUp(mouse::Button emMouseButton);

			int	GetX();
			int GetY();

			long GetMoveX();		// 마우스 X 이동거리
			long GetMoveY();		// 마우스 Y 이동거리
			long GetMoveWheel();	// 휠 이동거리
		}

		namespace keyboard
		{
#include "KeyCode.inl"

			bool IsKeyEvent(keyboard::KeyCode emKeyCode);
			bool IsKeyDown(keyboard::KeyCode emKeyCode);
			bool IsKeyPressed(keyboard::KeyCode emKeyCode);
			bool IsKeyUp(keyboard::KeyCode emKeyCode);
		}

		namespace gamepad
		{
			enum Type
			{
				eUnknown = 0,
				eGamePad,
				eWheel,
				eArcadeStick,
				eFlightStick,
				eDancePad,
				eGuitar,
				eGuitarAlternate,
				eDrumKit,
				eGuitarBass = 11,
				eArcadePad = 19,
			};

			enum PlayerID
			{
				e1P = 0,
				e2P,
				e3P,
				e4P,

				PlayerCount,
			};

			enum DeadZone
			{
				eIndependentAxes = 0,
				eCircular,
				eNone,
			};

			enum ButtonState
			{
				eIdle = 0,
				ePressed,
				eUp,
				eDown,
			};

			bool IsConnected(PlayerID playerID = PlayerID::e1P);

			Type GetType(PlayerID playerID = PlayerID::e1P);

			DeadZone GetDeadZone(PlayerID playerID = PlayerID::e1P);
			void SetDeadZone(DeadZone emDeadZone, PlayerID playerID = PlayerID::e1P);

			ButtonState A(PlayerID playerID = PlayerID::e1P);
			ButtonState B(PlayerID playerID = PlayerID::e1P);
			ButtonState X(PlayerID playerID = PlayerID::e1P);
			ButtonState Y(PlayerID playerID = PlayerID::e1P);

			ButtonState LeftStick(PlayerID playerID = PlayerID::e1P);
			ButtonState RightStick(PlayerID playerID = PlayerID::e1P);

			ButtonState LeftShoulder(PlayerID playerID = PlayerID::e1P);
			ButtonState RightShoulder(PlayerID playerID = PlayerID::e1P);

			ButtonState Back(PlayerID playerID = PlayerID::e1P);
			ButtonState Start(PlayerID playerID = PlayerID::e1P);

			ButtonState DPadUp(PlayerID playerID = PlayerID::e1P);
			ButtonState DPadDown(PlayerID playerID = PlayerID::e1P);
			ButtonState DPadLeft(PlayerID playerID = PlayerID::e1P);
			ButtonState DPadRight(PlayerID playerID = PlayerID::e1P);

			float LeftThumbStickX(PlayerID playerID = PlayerID::e1P);
			float LeftThumbStickY(PlayerID playerID = PlayerID::e1P);

			float RightThumbStickX(PlayerID playerID = PlayerID::e1P);
			float RightThumbStickY(PlayerID playerID = PlayerID::e1P);

			float LeftTrigger(PlayerID playerID = PlayerID::e1P);
			float RightTrigger(PlayerID playerID = PlayerID::e1P);

			bool SetVibration(float fLeftMotor, float fRightMotor, float fVibrationTime = 0.f, PlayerID playerID = PlayerID::e1P);
		}
	}
}