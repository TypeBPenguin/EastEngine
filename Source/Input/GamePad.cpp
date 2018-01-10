#include "stdafx.h"
#include "GamePad.h"

namespace EastEngine
{
	namespace Input
	{
		float ApplyLinearDeadZone(float fValue, float fMaxValue, float fDeadZoneSize)
		{
			if (fValue < -fDeadZoneSize)
			{
				// Increase negative values to remove the deadzone discontinuity.
				fValue += fDeadZoneSize;
			}
			else if (fValue > fDeadZoneSize)
			{
				// Decrease positive values to remove the deadzone discontinuity.
				fValue -= fDeadZoneSize;
			}
			else
			{
				// Values inside the deadzone come out zero.
				return 0;
			}

			// Scale into 0-1 range.
			float scaledValue = fValue / (fMaxValue - fDeadZoneSize);

			return Math::Max(-1.f, Math::Min(scaledValue, 1.f));
		}

		void ApplyStickDeadZone(float x, float y, GamePad::EmDeadZone emDeadZoneMode, float fMaxValue, float fDeadZoneSize,
			_Out_ float& fResultX_out, _Out_ float& fResultY_out)
		{
			switch (emDeadZoneMode)
			{
			case GamePad::eIndependentAxes:
				fResultX_out = ApplyLinearDeadZone(x, fMaxValue, fDeadZoneSize);
				fResultY_out = ApplyLinearDeadZone(y, fMaxValue, fDeadZoneSize);
				break;
			case GamePad::eCircular:
			{
				float dist = sqrtf(x*x + y*y);
				float wanted = ApplyLinearDeadZone(dist, fMaxValue, fDeadZoneSize);

				float scale = (wanted > 0.f) ? (wanted / dist) : 0.f;

				fResultX_out = Math::Max(-1.f, Math::Min(x * scale, 1.f));
				fResultY_out = Math::Max(-1.f, Math::Min(y * scale, 1.f));
			}
			break;
			default: // GamePad::DEAD_ZONE_NONE
				fResultX_out = ApplyLinearDeadZone(x, fMaxValue, 0);
				fResultY_out = ApplyLinearDeadZone(y, fMaxValue, 0);
				break;
			}
		}

#define UPDATE_BUTTON_STATE(field) field = static_cast<EmButtonState>( ( !!state.buttons.field ) | ( ( !!state.buttons.field ^ !!lastState.buttons.field ) << 1 ) );

		void GamePad::ButtonStateTracker::Update(const GamePad::State& state)
		{
			UPDATE_BUTTON_STATE(a);

			assert((!state.buttons.a && !lastState.buttons.a) == (a == eUp));
			assert((state.buttons.a && lastState.buttons.a) == (a == eHeld));
			assert((!state.buttons.a && lastState.buttons.a) == (a == eReleased));
			assert((state.buttons.a && !lastState.buttons.a) == (a == ePressed));

			UPDATE_BUTTON_STATE(b);
			UPDATE_BUTTON_STATE(x);
			UPDATE_BUTTON_STATE(y);

			UPDATE_BUTTON_STATE(leftStick);
			UPDATE_BUTTON_STATE(rightStick);

			UPDATE_BUTTON_STATE(leftShoulder);
			UPDATE_BUTTON_STATE(rightShoulder);

			UPDATE_BUTTON_STATE(back);
			UPDATE_BUTTON_STATE(start);

			dpadUp = static_cast<EmButtonState>((!!state.dpad.up) | ((!!state.dpad.up ^ !!lastState.dpad.up) << 1));
			dpadDown = static_cast<EmButtonState>((!!state.dpad.down) | ((!!state.dpad.down ^ !!lastState.dpad.down) << 1));
			dpadLeft = static_cast<EmButtonState>((!!state.dpad.left) | ((!!state.dpad.left ^ !!lastState.dpad.left) << 1));
			dpadRight = static_cast<EmButtonState>((!!state.dpad.right) | ((!!state.dpad.right ^ !!lastState.dpad.right) << 1));

			assert((!state.dpad.up && !lastState.dpad.up) == (dpadUp == eUp));
			assert((state.dpad.up && lastState.dpad.up) == (dpadUp == eHeld));
			assert((!state.dpad.up && lastState.dpad.up) == (dpadUp == eReleased));
			assert((state.dpad.up && !lastState.dpad.up) == (dpadUp == ePressed));

			lastState = state;
		}

#undef UPDATE_BUTTON_STATE

		void GamePad::ButtonStateTracker::Reset()
		{
			Memory::Clear(this, sizeof(ButtonStateTracker));
		}

		GamePad::Player::Player(EmPlayerID emPlayer)
			: m_emPlayerID(emPlayer)
			, m_isConnected(false)
			, m_fLastReadTime(0.f)
			, m_emDeadZoneMode(EmDeadZone::eIndependentAxes)
		{
		}

		GamePad::Player::~Player()
		{
		}

		void GamePad::Player::Update(float fElapsedTime)
		{
			if (throttleRetry(fElapsedTime) == true)
			{
				Memory::Clear(&m_state, sizeof(State));
				Memory::Clear(&m_capabilities, sizeof(Capabilities));
			}
			else
			{
				refreshState();
				refreshCapabilities();

				m_buttonStateTracker.Update(m_state);
			}
		}

		bool GamePad::Player::SetVibration(float fLeftMotor, float fRightMotor)
		{
			if (throttleRetry(0.f) == true)
				return false;

			XINPUT_VIBRATION xVibration;
			xVibration.wLeftMotorSpeed = WORD(fLeftMotor * 0xFFFF);
			xVibration.wRightMotorSpeed = WORD(fRightMotor * 0xFFFF);
			DWORD result = XInputSetState(m_emPlayerID, &xVibration);
			if (result == ERROR_DEVICE_NOT_CONNECTED)
			{
				clearSlot(0.f);
				return false;
			}
			else
			{
				m_isConnected = true;
				return result == ERROR_SUCCESS;
			}
		}

		void GamePad::Player::refreshState()
		{
			XINPUT_STATE xState;
			DWORD result = XInputGetState(m_emPlayerID, &xState);
			if (result == ERROR_DEVICE_NOT_CONNECTED)
			{
				clearSlot(0.f);
			}
			else
			{
				m_isConnected = true;

				m_state.isConnected = true;
				m_state.nPacket = xState.dwPacketNumber;

				WORD xbuttons = xState.Gamepad.wButtons;
				m_state.buttons.a = (xbuttons & XINPUT_GAMEPAD_A) != 0;
				m_state.buttons.b = (xbuttons & XINPUT_GAMEPAD_B) != 0;
				m_state.buttons.x = (xbuttons & XINPUT_GAMEPAD_X) != 0;
				m_state.buttons.y = (xbuttons & XINPUT_GAMEPAD_Y) != 0;
				m_state.buttons.leftStick = (xbuttons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
				m_state.buttons.rightStick = (xbuttons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
				m_state.buttons.leftShoulder = (xbuttons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
				m_state.buttons.rightShoulder = (xbuttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
				m_state.buttons.back = (xbuttons & XINPUT_GAMEPAD_BACK) != 0;
				m_state.buttons.start = (xbuttons & XINPUT_GAMEPAD_START) != 0;

				m_state.dpad.up = (xbuttons & XINPUT_GAMEPAD_DPAD_UP) != 0;
				m_state.dpad.down = (xbuttons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
				m_state.dpad.right = (xbuttons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
				m_state.dpad.left = (xbuttons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;

				if (m_emDeadZoneMode == EmDeadZone::eNone)
				{
					m_state.triggers.left = ApplyLinearDeadZone(float(xState.Gamepad.bLeftTrigger), 255.f, 0.f);
					m_state.triggers.right = ApplyLinearDeadZone(float(xState.Gamepad.bRightTrigger), 255.f, 0.f);
				}
				else
				{
					m_state.triggers.left = ApplyLinearDeadZone(float(xState.Gamepad.bLeftTrigger), 255.f, float(XINPUT_GAMEPAD_TRIGGER_THRESHOLD));
					m_state.triggers.right = ApplyLinearDeadZone(float(xState.Gamepad.bRightTrigger), 255.f, float(XINPUT_GAMEPAD_TRIGGER_THRESHOLD));
				}

				ApplyStickDeadZone(float(xState.Gamepad.sThumbLX), float(xState.Gamepad.sThumbLY),
					m_emDeadZoneMode, 32767.f, float(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE),
					m_state.thumbSticks.leftX, m_state.thumbSticks.leftY);

				ApplyStickDeadZone(float(xState.Gamepad.sThumbRX), float(xState.Gamepad.sThumbRY),
					m_emDeadZoneMode, 32767.f, float(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE),
					m_state.thumbSticks.rightX, m_state.thumbSticks.rightY);
			}
		}

		void GamePad::Player::refreshCapabilities()
		{
			XINPUT_CAPABILITIES xCaps;
			DWORD result = XInputGetCapabilities(m_emPlayerID, 0, &xCaps);
			if (result == ERROR_DEVICE_NOT_CONNECTED)
			{
				clearSlot(0.f);
			}
			else
			{
				m_isConnected = true;

				m_capabilities.isConnected = true;
				m_capabilities.emPlayerID = m_emPlayerID;
				if (xCaps.Type == XINPUT_DEVTYPE_GAMEPAD)
				{
					static_assert(Capabilities::eGamePad == XINPUT_DEVSUBTYPE_GAMEPAD, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_WHEEL == Capabilities::eWheel, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_ARCADE_STICK == Capabilities::eArcadeStick, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_FLIGHT_STICK == Capabilities::eFlightStick, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_DANCE_PAD == Capabilities::eDancePad, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_GUITAR == Capabilities::eGuitar, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE == Capabilities::eGuitarAlternate, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_DRUM_KIT == Capabilities::eDrumKit, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_GUITAR_BASS == Capabilities::eGuitarBass, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_ARCADE_PAD == Capabilities::eArcadePad, "xinput.h mismatch");

					m_capabilities.emGamepadType = Capabilities::EmType(xCaps.SubType);
				}
			}
		}

		bool GamePad::Player::throttleRetry(float fElapsedTime)
		{
			// This function minimizes a potential performance issue with XInput on Windows when
			// checking a disconnected controller slot which requires device enumeration.
			// This throttling keeps checks for newly connected gamepads to about once a second
			if (m_isConnected == true)
				return false;

			m_fLastReadTime += fElapsedTime;

			if (m_fLastReadTime < 1.f)
				return true;

			return false;
		}

		GamePad::GamePad()
			: m_players({ EmPlayerID::e1P, EmPlayerID::e2P, EmPlayerID::e3P, EmPlayerID::e4P })
		{
		}

		GamePad::~GamePad()
		{
		}

		/*void GamePad::Suspend()
		{
			XInputEnable(FALSE);
		}

		void GamePad::Resume()
		{
			XInputEnable(TRUE);
		}*/

		void GamePad::Update(float fElapsedTime)
		{
			for (auto& player : m_players)
			{
				player.Update(fElapsedTime);
			}
		}
	}
}