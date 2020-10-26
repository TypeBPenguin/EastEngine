#include "stdafx.h"
#include "SoundDefine.h"

namespace est
{
	namespace sound
	{
		static_assert(Mode::eDefault == FMOD_DEFAULT, "FMOD mode mismatch");
		static_assert(Mode::eLoopOff == FMOD_LOOP_OFF, "FMOD mode mismatch");
		static_assert(Mode::eLoopNormal == FMOD_LOOP_NORMAL, "FMOD mode mismatch");
		static_assert(Mode::eLoopBIDI == FMOD_LOOP_BIDI, "FMOD mode mismatch");
		static_assert(Mode::e2D == FMOD_2D, "FMOD mode mismatch");
		static_assert(Mode::e3D == FMOD_3D, "FMOD mode mismatch");
		static_assert(Mode::eCreateStream == FMOD_CREATESTREAM, "FMOD mode mismatch");
		static_assert(Mode::eCreateSample == FMOD_CREATESAMPLE, "FMOD mode mismatch");
		static_assert(Mode::eCreateCompressedSample == FMOD_CREATECOMPRESSEDSAMPLE, "FMOD mode mismatch");
		static_assert(Mode::eOpenUser == FMOD_OPENUSER, "FMOD mode mismatch");
		static_assert(Mode::eOpenMemory == FMOD_OPENMEMORY, "FMOD mode mismatch");
		static_assert(Mode::eOpenMemoryPoint == FMOD_OPENMEMORY_POINT, "FMOD mode mismatch");
		static_assert(Mode::eOpenRaw == FMOD_OPENRAW, "FMOD mode mismatch");
		static_assert(Mode::eOpenOnly == FMOD_OPENONLY, "FMOD mode mismatch");
		static_assert(Mode::eAccurateTime == FMOD_ACCURATETIME, "FMOD mode mismatch");
		static_assert(Mode::eMpegSearch == FMOD_MPEGSEARCH, "FMOD mode mismatch");
		static_assert(Mode::eNonBlocking == FMOD_NONBLOCKING, "FMOD mode mismatch");
		static_assert(Mode::eUnique == FMOD_UNIQUE, "FMOD mode mismatch");
		static_assert(Mode::e3D_HeadRelative == FMOD_3D_HEADRELATIVE, "FMOD mode mismatch");
		static_assert(Mode::e3D_WorldRelative == FMOD_3D_WORLDRELATIVE, "FMOD mode mismatch");
		static_assert(Mode::e3D_InverseRolloff == FMOD_3D_INVERSEROLLOFF, "FMOD mode mismatch");
		static_assert(Mode::e3D_LinearRolloff == FMOD_3D_LINEARROLLOFF, "FMOD mode mismatch");
		static_assert(Mode::e3D_LinearSquareRolloff == FMOD_3D_LINEARSQUAREROLLOFF, "FMOD mode mismatch");
		static_assert(Mode::e3D_CustomRolloff == FMOD_3D_CUSTOMROLLOFF, "FMOD mode mismatch");
		static_assert(Mode::e3D_IgnoreGeometry == FMOD_3D_IGNOREGEOMETRY, "FMOD mode mismatch");
		static_assert(Mode::eIgnoreTags == FMOD_IGNORETAGS, "FMOD mode mismatch");
		static_assert(Mode::eLowMem == FMOD_LOWMEM, "FMOD mode mismatch");
		static_assert(Mode::eLoadSecondaryRam == FMOD_LOADSECONDARYRAM, "FMOD mode mismatch");
		static_assert(Mode::eVirtualPlayFromStart == FMOD_VIRTUAL_PLAYFROMSTART, "FMOD mode mismatch");
	}
}