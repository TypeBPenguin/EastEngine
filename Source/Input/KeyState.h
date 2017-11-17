#pragma once

namespace EastEngine
{
	namespace Input
	{
		namespace EmKeyboard
		{
			// dinput.h 와 대응하는 키 값
			enum Button
			{
				eEscape = 0x01,
				e1 = 0x02,
				e2 = 0x03,
				e3 = 0x04,
				e4 = 0x05,
				e5 = 0x06,
				e6 = 0x07,
				e7 = 0x08,
				e8 = 0x09,
				e9 = 0x0A,
				e0 = 0x0B,
				eMinus = 0x0C,						/* - on main keyboard */
				eEquals = 0x0D,
				eBack = 0x0E,						/* backspace */
				eTab = 0x0F,
				eQ = 0x10,
				eW = 0x11,
				eE = 0x12,
				eR = 0x13,
				eT = 0x14,
				eY = 0x15,
				eU = 0x16,
				eI = 0x17,
				eO = 0x18,
				eP = 0x19,
				eLeftBracket = 0x1A,				/* [ */
				eRightBracket = 0x1B,				/* ] */
				eEnter = 0x1C,						/* Enter on main keyboard */
				eLeftControl = 0x1D,
				eA = 0x1E,
				eS = 0x1F,
				eD = 0x20,
				eF = 0x21,
				eG = 0x22,
				eH = 0x23,
				eJ = 0x24,
				eK = 0x25,
				eL = 0x26,
				eSemiColon = 0x27,
				eApostrophe = 0x28,
				eGrave = 0x29,						/* accent grave */
				eLeftShift = 0x2A,
				eBackSlash = 0x2B,
				eZ = 0x2C,
				eX = 0x2D,
				eC = 0x2E,
				eV = 0x2F,
				eB = 0x30,
				eN = 0x31,
				eM = 0x32,
				eComma = 0x33,
				ePeriod = 0x34,						/* . on main keyboard */
				eSlash = 0x35,						/* / on main keyboard */
				eRightShift = 0x36,
				eMultiply = 0x37,					/* * on numeric keypad */
				eLeftAlt = 0x38,					/* left Alt */
				eSpace = 0x39,
				eCapital = 0x3A,					/* caps lock*/
				eF1 = 0x3B,
				eF2 = 0x3C,
				eF3 = 0x3D,
				eF4 = 0x3E,
				eF5 = 0x3F,
				eF6 = 0x40,
				eF7 = 0x41,
				eF8 = 0x42,
				eF9 = 0x43,
				eF10 = 0x44,
				eNumLock = 0x45,
				eScrollLock = 0x46,					/* Scroll Lock */
				eNumPad7 = 0x47,
				eNumPad8 = 0x48,
				eNumPad9 = 0x49,
				eSubTract = 0x4A,					/* - on numeric keypad */
				eNumPad4 = 0x4B,
				eNumPad5 = 0x4C,
				eNumPad6 = 0x4D,
				eAdd = 0x4E,						/* + on numeric keypad */
				eNumPad1 = 0x4F,
				eNumPad2 = 0x50,
				eNumPad3 = 0x51,
				eNumPad0 = 0x52,
				eDecimal = 0x53,					/* . on numeric keypad */
				eOEM_102 = 0x56,					/* <> or \| on RT 102-key keyboard (Non-U.S.) */
				eF11 = 0x57,
				eF12 = 0x58,
				eF13 = 0x64,						/*                     (NEC PC98) */
				eF14 = 0x65,						/*                     (NEC PC98) */
				eF15 = 0x66,						/*                     (NEC PC98) */
				eKaka = 0x70,						/* (Japanese keyboard)            */
				eABNT_C1 = 0x73,					/* /? on Brazilian keyboard */
				eConvert = 0x79,					/* (Japanese keyboard)            */
				eNoConvert = 0x7B,					/* (Japanese keyboard)            */
				eYen = 0x7D,						/* (Japanese keyboard)            */
				eABNT_C2 = 0x7E,					/* Numpad . on Brazilian keyboard */
				eNumpadEquals = 0x8D,				/* = on numeric keypad (NEC PC98) */
				ePrevTrack = 0x90,					/* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
				eAT = 0x91,							/*                     (NEC PC98) */
				eColon = 0x92,						/*                     (NEC PC98) */
				eUnderLine = 0x93,					/*                     (NEC PC98) */
				eKanji = 0x94,						/* (Japanese keyboard)            */
				eStop = 0x95,						/*                     (NEC PC98) */
				eAX = 0x96,							/*                     (Japan AX) */
				eUnLabeled = 0x97,					/*                        (J3100) */
				eNextTrack = 0x99,					/* Next Track */
				eNumPadEnter = 0x9C,				/* Enter on numeric keypad */
				eRightControl = 0x9D,
				eMute = 0xA0,						/* Mute */
				eCalculator = 0xA1,					/* Calculator */
				ePlayPause = 0xA2,					/* Play / Pause */
				eMediaStop = 0xA4,					/* Media Stop */
				eVolumeDown = 0xAE,					/* Volume - */
				eVolumeUp = 0xB0,					/* Volume + */
				eWebHome = 0xB2,					/* Web home */
				eNumPadComma = 0xB3,				/*  = DIK_, on numeric keypad (NEC PC98) */
				eDivide = 0xB5,						/* / on numeric keypad */
				eSysRQ = 0xB7,
				eRightAlt = 0xB8,					/* right Alt */
				ePause = 0xC5,						/* Pause */
				eHome = 0xC7,						/* Home on arrow keypad */
				eUp = 0xC8,							/* UpArrow on arrow keypad */
				ePageUp = 0xC9,						/* PgUp on arrow keypad */
				eLeft = 0xCB,						/* LeftArrow on arrow keypad */
				eRight = 0xCD,						/* RightArrow on arrow keypad */
				eEnd = 0xCF,						/* End on arrow keypad */
				eDown = 0xD0,						/* DownArrow on arrow keypad */
				ePageDown = 0xD1,					/* PgDn on arrow keypad */
				eInsert = 0xD2,						/* Insert on arrow keypad */
				eDelete = 0xD3,						/* Delete on arrow keypad */
				eLeftWin = 0xDB,					/* Left Windows key */
				eRightWin = 0xDC,					/* Right Windows key */
				eApps = 0xDD,						/* AppMenu key */
				eSysPower = 0xDE,					/* System Power */
				eSysSleep = 0xDF,					/* System Sleep */
				eSysWake = 0xE3,					/* System Wake */
				eWebSearch = 0xE5,					/* Web Search */
				eWebFavorites = 0xE6,				/* Web Favorites */
				eWebRefresh = 0xE7,					/* Web Refresh */
				eWebStop = 0xE8,					/* Web Stop */
				eWebForward = 0xE9,					/* Web Forward */
				eWebBack = 0xEA,					/* Web Back */
				eMyComputer = 0xEB,					/* My Computer */
				eMail = 0xEC,						/* Mail */
				eMediaSelect = 0xED,				/* Media Select */

				eInvalidKey = 0xff,
			};
		}
	}
}