#include "stdafx.h"
#include "InputInterface.h"

#include "InputDevice.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "GamePad.h"

namespace EastEngine
{
	namespace Input
	{
		namespace Mouse
		{
			bool IsButtonEvent(Mouse::Button emMouseButton)
			{
				return Device::GetInstance()->GetMouse()->IsButtonEvent(emMouseButton);
			}

			bool IsButtonDown(Mouse::Button emMouseButton)
			{
				return Device::GetInstance()->GetMouse()->IsButtonDown(emMouseButton);
			}

			bool IsButtonPressed(Mouse::Button emMouseButton)
			{
				return Device::GetInstance()->GetMouse()->IsButtonPressed(emMouseButton);
			}

			bool IsButtonUp(Mouse::Button emMouseButton)
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

		namespace Keyboard
		{
			bool IsKeyEvent(Keyboard::KeyCode emKeyCode)
			{
				return Device::GetInstance()->GetKeyboard()->IsKeyEvent(emKeyCode);
			}

			bool IsKeyDown(Keyboard::KeyCode emKeyCode)
			{
				return Device::GetInstance()->GetKeyboard()->IsKeyDown(emKeyCode);
			}

			bool IsKeyPressed(Keyboard::KeyCode emKeyCode)
			{
				return Device::GetInstance()->GetKeyboard()->IsKeyPressed(emKeyCode);
			}

			bool IsKeyUp(Keyboard::KeyCode emKeyCode)
			{
				return Device::GetInstance()->GetKeyboard()->IsKeyUp(emKeyCode);
			}
		}

		namespace GamePad
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