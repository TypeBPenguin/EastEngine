#pragma once

#include "InputInterface.h"

namespace est
{
	namespace input
	{
		class GamePadInstance
		{
		public:
			GamePadInstance();
			~GamePadInstance();

		public:
			struct Buttons
			{
				bool a = false;
				bool b = false;
				bool x = false;
				bool y = false;
				bool leftStick = false;
				bool rightStick = false;
				bool leftShoulder = false;
				bool rightShoulder = false;
				bool back = false;
				bool start = false;
			};

			struct DPad
			{
				bool up = false;
				bool down = false;
				bool right = false;
				bool left = false;
			};

			struct ThumbSticks
			{
				float leftX = 0.f;
				float leftY = 0.f;
				float rightX = 0.f;
				float rightY = 0.f;
			};

			struct Triggers
			{
				float left = 0.f;
				float right = 0.f;
			};

			struct State
			{
				bool isConnected = false;
				uint64_t nPacket = 0;
				Buttons buttons;
				DPad dpad;
				ThumbSticks thumbSticks;
				Triggers triggers;

				bool IsConnected() const { return isConnected; }

				// Is the button pressed currently?
				bool IsAPressed() const { return buttons.a; }
				bool IsBPressed() const { return buttons.b; }
				bool IsXPressed() const { return buttons.x; }
				bool IsYPressed() const { return buttons.y; }

				bool IsLeftStickPressed() const { return buttons.leftStick; }
				bool IsRightStickPressed() const { return buttons.rightStick; }

				bool IsLeftShoulderPressed() const { return buttons.leftShoulder; }
				bool IsRightShoulderPressed() const { return buttons.rightShoulder; }

				bool IsBackPressed() const { return buttons.back; }
				bool IsStartPressed() const { return buttons.start; }

				bool IsDPadDownPressed() const { return dpad.down; };
				bool IsDPadUpPressed() const { return dpad.up; };
				bool IsDPadLeftPressed() const { return dpad.left; };
				bool IsDPadRightPressed() const { return dpad.right; };

				bool IsLeftThumbStickUp() const { return (thumbSticks.leftY > 0.5f) != 0; }
				bool IsLeftThumbStickDown() const { return (thumbSticks.leftY < -0.5f) != 0; }
				bool IsLeftThumbStickLeft() const { return (thumbSticks.leftX < -0.5f) != 0; }
				bool IsLeftThumbStickRight() const { return (thumbSticks.leftX > 0.5f) != 0; }

				float GetLeftThumbStickX() const { return thumbSticks.leftX; }
				float GetLeftThumbStickY() const { return thumbSticks.leftY; }

				bool IsRightThumbStickUp() const { return (thumbSticks.rightY > 0.5f) != 0; }
				bool IsRightThumbStickDown() const { return (thumbSticks.rightY < -0.5f) != 0; }
				bool IsRightThumbStickLeft() const { return (thumbSticks.rightX < -0.5f) != 0; }
				bool IsRightThumbStickRight() const { return (thumbSticks.rightX > 0.5f) != 0; }

				float GetRightThumbStickX() const { return thumbSticks.rightX; }
				float GetRightThumbStickY() const { return thumbSticks.rightY; }

				bool IsLeftTriggerPressed() const { return (triggers.left > 0.5f) != 0; }
				bool IsRightTriggerPressed() const { return (triggers.right > 0.5f) != 0; }

				float GetLeftTrigger() const { return triggers.left; }
				float GetRightTrigger() const { return triggers.right; }
			};

			struct Capabilities
			{
				bool isConnected = false;
				gamepad::Type emGamepadType = gamepad::eUnknown;
				gamepad::PlayerID emPlayerID = gamepad::e1P;

				bool IsConnected() const { return isConnected; }
			};

			class ButtonStateTracker
			{
			public:
				ButtonStateTracker() { Reset(); }
				~ButtonStateTracker() = default;

			public:
				void Update(const State& state);

				void Reset();

			public:
				const gamepad::ButtonState& A() const { return a; }
				const gamepad::ButtonState& B() const { return b; }
				const gamepad::ButtonState& X() const { return x; }
				const gamepad::ButtonState& Y() const { return y; }

				const gamepad::ButtonState& LeftStick() const { return leftStick; }
				const gamepad::ButtonState& RightStick() const { return rightStick; }

				const gamepad::ButtonState& LeftShoulder() const { return leftShoulder; }
				const gamepad::ButtonState& RightShoulder() const { return rightShoulder; }

				const gamepad::ButtonState& Back() const { return back; }
				const gamepad::ButtonState& Start() const { return start; }

				const gamepad::ButtonState& DPadUp() const { return dpadUp; }
				const gamepad::ButtonState& DPadDown() const { return dpadDown; }
				const gamepad::ButtonState& DPadLeft() const { return dpadLeft; }
				const gamepad::ButtonState& DPadRight() const { return dpadRight; }

			private:
				gamepad::ButtonState a;
				gamepad::ButtonState b;
				gamepad::ButtonState x;
				gamepad::ButtonState y;

				gamepad::ButtonState leftStick;
				gamepad::ButtonState rightStick;

				gamepad::ButtonState leftShoulder;
				gamepad::ButtonState rightShoulder;

				gamepad::ButtonState back;
				gamepad::ButtonState start;

				gamepad::ButtonState dpadUp;
				gamepad::ButtonState dpadDown;
				gamepad::ButtonState dpadLeft;
				gamepad::ButtonState dpadRight;

				State lastState;
			};

			class Player
			{
			public:
				Player(gamepad::PlayerID emPlayer);
				~Player();

			public:
				void Update(float elapsedTime);

				bool SetVibration(float fLeftMotor, float fRightMotor, float fVibrationTime = 0.f);

			public:
				gamepad::PlayerID GetPlayerID() const { return m_emPlayerID; }

				bool IsConnected() const { return m_isConnected; }

				gamepad::DeadZone GetDeadZone() const { return m_emDeadZoneMode; }
				void SetDeadZone(gamepad::DeadZone emDeadZone) { m_emDeadZoneMode = emDeadZone; }

				const State& GetCurState() const { return m_state; }
				const ButtonStateTracker& GetButtonStateTracker() const { return m_buttonStateTracker; }
				const Capabilities& GetCapabilities() const { return m_capabilities; }

			private:
				void RefreshState();
				void RefreshCapabilities();

				bool ThrottleRetry(float elapsedTime);
				void ClearSlot(float time)
				{
					m_isConnected = false;
					m_fLastReadTime = time;
					m_fVibrationTime = 0.f;
					m_fMaxVibrationTime = 0.f;
				}

			private:
				gamepad::PlayerID m_emPlayerID;

				bool m_isConnected;
				float m_fLastReadTime;

				float m_fVibrationTime;
				float m_fMaxVibrationTime;

				gamepad::DeadZone m_emDeadZoneMode;

				State m_state;
				ButtonStateTracker m_buttonStateTracker;

				Capabilities m_capabilities;
			};

			Player* GetPlayer(gamepad::PlayerID emPlayerID) { return &m_players[emPlayerID]; }

			// Set the vibration motor speeds of the gamepad
			bool SetVibration(gamepad::PlayerID emPlayerID, float fLeftMotor, float fRightMotor, float fVibrationTime = 0.f)
			{
				return m_players[emPlayerID].SetVibration(fLeftMotor, fRightMotor, fVibrationTime);
			}

			void Update(float elapsedTime);

		private:
			std::array<Player, gamepad::PlayerCount> m_players;
		};
	}
}