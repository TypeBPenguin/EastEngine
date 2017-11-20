#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Input
	{
		class GamePad : public Singleton<GamePad>
		{
			friend Singleton<GamePad>;
		private:
			GamePad();
			virtual ~GamePad();

		public:
			enum EmPlayerID
			{
				e1P = 0,
				e2P,
				e3P,
				e4P,

				MaxCount,
			};

			enum EmDeadZone
			{
				eIndependentAxes = 0,
				eCircular,
				eNone,
			};

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
				bool IsViewPressed() const { return buttons.back; }
				bool IsStartPressed() const { return buttons.start; }
				bool IsMenuPressed() const { return buttons.start; }

				bool IsDPadDownPressed() const { return dpad.down; };
				bool IsDPadUpPressed() const { return dpad.up; };
				bool IsDPadLeftPressed() const { return dpad.left; };
				bool IsDPadRightPressed() const { return dpad.right; };

				bool IsLeftThumbStickUp() const { return (thumbSticks.leftY > 0.5f) != 0; }
				bool IsLeftThumbStickDown() const { return (thumbSticks.leftY < -0.5f) != 0; }
				bool IsLeftThumbStickLeft() const { return (thumbSticks.leftX < -0.5f) != 0; }
				bool IsLeftThumbStickRight() const { return (thumbSticks.leftX > 0.5f) != 0; }

				bool IsRightThumbStickUp() const { return (thumbSticks.rightY > 0.5f) != 0; }
				bool IsRightThumbStickDown() const { return (thumbSticks.rightY < -0.5f) != 0; }
				bool IsRightThumbStickLeft() const { return (thumbSticks.rightX < -0.5f) != 0; }
				bool IsRightThumbStickRight() const { return (thumbSticks.rightX > 0.5f) != 0; }

				bool IsLeftTriggerPressed() const { return (triggers.left > 0.5f) != 0; }
				bool IsRightTriggerPressed() const { return (triggers.right > 0.5f) != 0; }
			};

			struct Capabilities
			{
				enum EmType
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

				bool isConnected = false;
				EmType emGamepadType = EmType::eUnknown;
				EmPlayerID emPlayerID = EmPlayerID::e1P;

				bool IsConnected() const { return isConnected; }
			};

			class ButtonStateTracker
			{
			public:
				enum EmButtonState
				{
					eUp = 0,		// Button is up
					eHeld,			// Button is held down
					eReleased,		// Button was just released
					ePressed,		// Button was just pressed
				};

				EmButtonState a;
				EmButtonState b;
				EmButtonState x;
				EmButtonState y;

				EmButtonState leftStick;
				EmButtonState rightStick;

				EmButtonState leftShoulder;
				EmButtonState rightShoulder;

				EmButtonState back;
				EmButtonState start;

				EmButtonState dpadUp;
				EmButtonState dpadDown;
				EmButtonState dpadLeft;
				EmButtonState dpadRight;

				ButtonStateTracker() { Reset(); }

				void Update(const State& state);

				void Reset();

			private:
				State lastState;
			};

			class Player
			{
			public:
				Player(EmPlayerID emPlayer)
					: m_emPlayerID(emPlayer)
					, m_isConnected(false)
					, m_fLastReadTime(0.f)
					, m_emDeadZoneMode(EmDeadZone::eIndependentAxes)
				{
				}

				~Player()
				{
				}

				void Update(float fElapsedTime);

				bool SetVibration(float fLeftMotor, float fRightMotor, float fLeftTrigger = 0.f, float fRightTrigger = 0.f);

			public:
				EmPlayerID GetPlayerID() const { return m_emPlayerID; }

				bool IsConnected() const { return m_isConnected; }

				EmDeadZone GetDeadZone() const { return m_emDeadZoneMode; }
				void SetDeadZone(EmDeadZone emDeadZone) { m_emDeadZoneMode = emDeadZone; }

				const State& GetCurState() const { return m_state; }
				const ButtonStateTracker& GetButtonStateTracker() const { return m_buttonStateTracker; }
				const Capabilities& GetCapabilities() const { return m_capabilities; }

			private:
				void refreshState();
				void refreshCapabilities();

				bool throttleRetry(float fElapsedTime);
				void clearSlot(float fTime)
				{
					m_isConnected = false;
					m_fLastReadTime = fTime;
				}

			private:
				EmPlayerID m_emPlayerID;

				bool m_isConnected;
				float m_fLastReadTime;

				EmDeadZone m_emDeadZoneMode;

				State m_state;
				ButtonStateTracker m_buttonStateTracker;

				Capabilities m_capabilities;
			};

			Player* GetPlayer(EmPlayerID emPlayerID) { return &m_players[emPlayerID]; }

			// Set the vibration motor speeds of the gamepad
			bool SetVibration(EmPlayerID emPlayerID, float fLeftMotor, float fRightMotor, float fLeftTrigger = 0.f, float fRightTrigger = 0.f)
			{
				return m_players[emPlayerID].SetVibration(fLeftMotor, fRightMotor, fLeftTrigger, fRightTrigger);
			}

			// Handle suspending/resuming
			/*void Suspend();
			void Resume();*/

			void Update(float fElapsedTime);

		private:
			std::array<Player, EmPlayerID::MaxCount> m_players;
		};
	}
}