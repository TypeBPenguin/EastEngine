#include "stdafx.h"
#include "GamePad.h"

namespace eastengine
{
	namespace input
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
			const float scaledValue = fValue / (fMaxValue - fDeadZoneSize);

			return std::clamp(scaledValue, -1.f, 1.f);
		}

		void ApplyStickDeadZone(float x, float y, gamepad::DeadZone emDeadZoneMode, float fMaxValue, float fDeadZoneSize,
			_Out_ float& fResultX_out, _Out_ float& fResultY_out)
		{
			switch (emDeadZoneMode)
			{
			case gamepad::eIndependentAxes:
				fResultX_out = ApplyLinearDeadZone(x, fMaxValue, fDeadZoneSize);
				fResultY_out = ApplyLinearDeadZone(y, fMaxValue, fDeadZoneSize);
				break;
			case gamepad::eCircular:
			{
				const float dist = sqrtf(x*x + y*y);
				const float wanted = ApplyLinearDeadZone(dist, fMaxValue, fDeadZoneSize);

				const float scale = (wanted > 0.f) ? (wanted / dist) : 0.f;

				fResultX_out = std::clamp(x * scale, -1.f, 1.f);
				fResultY_out = std::clamp(y * scale, -1.f, 1.f);
			}
			break;
			default: // gamepad::DEAD_ZONE_NONE
				fResultX_out = ApplyLinearDeadZone(x, fMaxValue, 0);
				fResultY_out = ApplyLinearDeadZone(y, fMaxValue, 0);
				break;
			}
		}

#define UPDATE_BUTTON_STATE(field) field = static_cast<gamepad::ButtonState>( ( !!state.buttons.field ) | ( ( !!state.buttons.field ^ !!lastState.buttons.field ) << 1 ) );

		void GamePadInstance::ButtonStateTracker::Update(const GamePadInstance::State& state)
		{
			UPDATE_BUTTON_STATE(a);

			assert((!state.buttons.a && !lastState.buttons.a) == (a == gamepad::eIdle));
			assert((state.buttons.a && lastState.buttons.a) == (a == gamepad::ePressed));
			assert((!state.buttons.a && lastState.buttons.a) == (a == gamepad::eUp));
			assert((state.buttons.a && !lastState.buttons.a) == (a == gamepad::eDown));

			UPDATE_BUTTON_STATE(b);
			UPDATE_BUTTON_STATE(x);
			UPDATE_BUTTON_STATE(y);

			UPDATE_BUTTON_STATE(leftStick);
			UPDATE_BUTTON_STATE(rightStick);

			UPDATE_BUTTON_STATE(leftShoulder);
			UPDATE_BUTTON_STATE(rightShoulder);

			UPDATE_BUTTON_STATE(back);
			UPDATE_BUTTON_STATE(start);

			dpadUp = static_cast<gamepad::ButtonState>((!!state.dpad.up) | ((!!state.dpad.up ^ !!lastState.dpad.up) << 1));
			dpadDown = static_cast<gamepad::ButtonState>((!!state.dpad.down) | ((!!state.dpad.down ^ !!lastState.dpad.down) << 1));
			dpadLeft = static_cast<gamepad::ButtonState>((!!state.dpad.left) | ((!!state.dpad.left ^ !!lastState.dpad.left) << 1));
			dpadRight = static_cast<gamepad::ButtonState>((!!state.dpad.right) | ((!!state.dpad.right ^ !!lastState.dpad.right) << 1));

			assert((!state.dpad.up && !lastState.dpad.up) == (dpadUp == gamepad::eIdle));
			assert((state.dpad.up && lastState.dpad.up) == (dpadUp == gamepad::ePressed));
			assert((!state.dpad.up && lastState.dpad.up) == (dpadUp == gamepad::eUp));
			assert((state.dpad.up && !lastState.dpad.up) == (dpadUp == gamepad::eDown));

			lastState = state;
		}

#undef UPDATE_BUTTON_STATE

		void GamePadInstance::ButtonStateTracker::Reset()
		{
			memory::Clear(this, sizeof(ButtonStateTracker));
		}

		GamePadInstance::Player::Player(gamepad::PlayerID emPlayer)
			: m_emPlayerID(emPlayer)
			, m_isConnected(false)
			, m_fLastReadTime(0.f)
			, m_fVibrationTime(0.f)
			, m_fMaxVibrationTime(0.f)
			, m_emDeadZoneMode(gamepad::DeadZone::eIndependentAxes)
		{
		}

		GamePadInstance::Player::~Player()
		{
		}

		void GamePadInstance::Player::Update(float elapsedTime)
		{
			if (ThrottleRetry(elapsedTime) == true)
			{
				memory::Clear(&m_state, sizeof(State));
				memory::Clear(&m_capabilities, sizeof(Capabilities));
			}
			else
			{
				RefreshState();
				RefreshCapabilities();

				m_buttonStateTracker.Update(m_state);

				if (math::IsZero(m_fMaxVibrationTime) == false)
				{
					if (m_fVibrationTime >= m_fMaxVibrationTime)
					{
						SetVibration(0.f, 0.f, 0.f);
					}
					else
					{
						m_fVibrationTime += elapsedTime;
					}
				}
			}
		}

		bool GamePadInstance::Player::SetVibration(float fLeftMotor, float fRightMotor, float fVibrationTime)
		{
			if (ThrottleRetry(0.f) == true)
				return false;

			XINPUT_VIBRATION xVibration;
			xVibration.wLeftMotorSpeed = WORD(fLeftMotor * 0xFFFF);
			xVibration.wRightMotorSpeed = WORD(fRightMotor * 0xFFFF);
			DWORD result = XInputSetState(m_emPlayerID, &xVibration);
			if (result == ERROR_DEVICE_NOT_CONNECTED)
			{
				ClearSlot(0.f);
				return false;
			}
			else
			{
				m_fVibrationTime = 0.f;
				m_fMaxVibrationTime = fVibrationTime;;

				m_isConnected = true;
				return result == ERROR_SUCCESS;
			}
		}

		void GamePadInstance::Player::RefreshState()
		{
			XINPUT_STATE xState;
			DWORD result = XInputGetState(m_emPlayerID, &xState);
			if (result == ERROR_DEVICE_NOT_CONNECTED)
			{
				ClearSlot(0.f);
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

				if (m_emDeadZoneMode == gamepad::DeadZone::eNone)
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

		void GamePadInstance::Player::RefreshCapabilities()
		{
			XINPUT_CAPABILITIES xCaps;
			DWORD result = XInputGetCapabilities(m_emPlayerID, 0, &xCaps);
			if (result == ERROR_DEVICE_NOT_CONNECTED)
			{
				ClearSlot(0.f);
			}
			else
			{
				m_isConnected = true;

				m_capabilities.isConnected = true;
				m_capabilities.emPlayerID = m_emPlayerID;
				if (xCaps.Type == XINPUT_DEVTYPE_GAMEPAD)
				{
					static_assert(XINPUT_DEVSUBTYPE_GAMEPAD == gamepad::eGamePad, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_WHEEL == gamepad::eWheel, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_ARCADE_STICK == gamepad::eArcadeStick, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_FLIGHT_STICK == gamepad::eFlightStick, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_DANCE_PAD == gamepad::eDancePad, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_GUITAR == gamepad::eGuitar, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE == gamepad::eGuitarAlternate, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_DRUM_KIT == gamepad::eDrumKit, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_GUITAR_BASS == gamepad::eGuitarBass, "xinput.h mismatch");
					static_assert(XINPUT_DEVSUBTYPE_ARCADE_PAD == gamepad::eArcadePad, "xinput.h mismatch");

					m_capabilities.emGamepadType = gamepad::Type(xCaps.SubType);
				}
			}
		}

		bool GamePadInstance::Player::ThrottleRetry(float elapsedTime)
		{
			// This function minimizes a potential performance issue with XInput on Windows when
			// checking a disconnected controller slot which requires device enumeration.
			// This throttling keeps checks for newly connected gamepads to about once a second
			if (m_isConnected == true)
				return false;

			m_fLastReadTime += elapsedTime;

			if (m_fLastReadTime < 1.f)
				return true;

			return false;
		}

		GamePadInstance::GamePadInstance()
			: m_players({ gamepad::e1P, gamepad::e2P, gamepad::e3P, gamepad::e4P })
		{
		}

		GamePadInstance::~GamePadInstance()
		{
		}

		void GamePadInstance::Update(float elapsedTime)
		{
			TRACER_EVENT("GamePadInstance::Update");
			for (auto& player : m_players)
			{
				player.Update(elapsedTime);
			}
		}
	}
}