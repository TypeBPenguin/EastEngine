#pragma once

#include "CommonLib/PhantomType.h"

namespace eastengine
{
	namespace sound
	{
		enum Mode
		{
			eDefault = 0x00000000,					/* Default for all modes listed below. FMOD_LOOP_OFF, FMOD_2D, FMOD_HARDWARE */
			eLoopOff = 0x00000001,					/* For non looping sounds. (DEFAULT).  Overrides FMOD_LOOP_NORMAL / FMOD_LOOP_BIDI. */
			eLoopNormal = 0x00000002,				/* For forward looping sounds. */
			eLoopBIDI = 0x00000004,					/* For bidirectional looping sounds. (only works on software mixed static sounds). */
			e2D = 0x00000008,						/* Ignores any 3d processing. (DEFAULT). */
			e3D = 0x00000010,						/* Makes the sound positionable in 3D.  Overrides FMOD_2D. */
			eHardware = 0x00000020,					/* Attempts to make sounds use hardware acceleration. (DEFAULT).  Note on platforms that don't support FMOD_HARDWARE (only 3DS, PS Vita, PSP, Wii and Wii U support FMOD_HARDWARE), this will be internally treated as FMOD_SOFTWARE. */
			eSoftware = 0x00000040,					/* Makes the sound be mixed by the FMOD CPU based software mixer.  Overrides FMOD_HARDWARE.  Use this for FFT, DSP, compressed sample support, 2D multi-speaker support and other software related features. */
			eCreateStream = 0x00000080,				/* Decompress at runtime, streaming from the source provided (ie from disk).  Overrides FMOD_CREATESAMPLE and FMOD_CREATECOMPRESSEDSAMPLE.  Note a stream can only be played once at a time due to a stream only having 1 stream buffer and file handle.  Open multiple streams to have them play concurrently. */
			eCreateSample = 0x00000100,				/* Decompress at loadtime, decompressing or decoding whole file into memory as the target sample format (ie PCM).  Fastest for FMOD_SOFTWARE based playback and most flexible.  */
			eCreateCompressedSample = 0x00000200,	/* Load MP2/MP3/IMAADPCM/CELT/Vorbis/AT9 or XMA into memory and leave it compressed.  CELT/Vorbis/AT9 encoding only supported in the FSB file format.  During playback the FMOD software mixer will decode it in realtime as a 'compressed sample'.  Can only be used in combination with FMOD_SOFTWARE.  Overrides FMOD_CREATESAMPLE.  If the sound data is not one of the supported formats, it will behave as if it was created with FMOD_CREATESAMPLE and decode the sound into PCM. */
			eOpenUser = 0x00000400,					/* Opens a user created static sample or stream. Use FMOD_CREATESOUNDEXINFO to specify format and/or read callbacks.  If a user created 'sample' is created with no read callback, the sample will be empty.  Use Sound::lock and Sound::unlock to place sound data into the sound if this is the case. */
			eOpenMemory = 0x00000800,				/* "name_or_data" will be interpreted as a pointer to memory instead of filename for creating sounds.  Use FMOD_CREATESOUNDEXINFO to specify length.  If used with FMOD_CREATESAMPLE or FMOD_CREATECOMPRESSEDSAMPLE, FMOD duplicates the memory into its own buffers.  Your own buffer can be freed after open.  If used with FMOD_CREATESTREAM, FMOD will stream out of the buffer whose pointer you passed in.  In this case, your own buffer should not be freed until you have finished with and released the stream.*/
			eOpenMemoryPoint = 0x10000000,			/* "name_or_data" will be interpreted as a pointer to memory instead of filename for creating sounds.  Use FMOD_CREATESOUNDEXINFO to specify length.  This differs to FMOD_OPENMEMORY in that it uses the memory as is, without duplicating the memory into its own buffers.  For Wii/PSP FMOD_HARDWARE supports this flag for the GCADPCM/VAG formats.  On other platforms FMOD_SOFTWARE must be used, as sound hardware on the other platforms (ie PC) cannot access main ram.  Cannot be freed after open, only after Sound::release.   Will not work if the data is compressed and FMOD_CREATECOMPRESSEDSAMPLE is not used. */
			eOpenRaw = 0x00001000,					/* Will ignore file format and treat as raw pcm.  Use FMOD_CREATESOUNDEXINFO to specify format.  Requires at least defaultfrequency, numchannels and format to be specified before it will open.  Must be little endian data. */
			eOpenOnly = 0x00002000,					/* Just open the file, dont prebuffer or read.  Good for fast opens for info, or when sound::readData is to be used. */
			eAccurateTime = 0x00004000,				/* For System::createSound - for accurate Sound::getLength/Channel::setPosition on VBR MP3, and MOD/S3M/XM/IT/MIDI files.  Scans file first, so takes longer to open. FMOD_OPENONLY does not affect this. */
			eMpegSearch = 0x00008000,				/* For corrupted / bad MP3 files.  This will search all the way through the file until it hits a valid MPEG header.  Normally only searches for 4k. */
			eNonBlocking = 0x00010000,				/* For opening sounds and getting streamed subsounds (seeking) asyncronously.  Use Sound::getOpenState to poll the state of the sound as it opens or retrieves the subsound in the background. */
			eUnique = 0x00020000,					/* Unique sound, can only be played one at a time */
			e3D_HeadRelative = 0x00040000,			/* Make the sound's position, velocity and orientation relative to the listener. */
			e3D_WorldRelative = 0x00080000,			/* Make the sound's position, velocity and orientation absolute (relative to the world). (DEFAULT) */
			e3D_InverseRolloff = 0x00100000,		/* This sound will follow the inverse rolloff model where mindistance = full volume, maxdistance = where sound stops attenuating, and rolloff is fixed according to the global rolloff factor.  (DEFAULT) */
			e3D_LinearRolloff = 0x00200000,			/* This sound will follow a linear rolloff model where mindistance = full volume, maxdistance = silence.  Rolloffscale is ignored. */
			e3D_LinearSquareRolloff = 0x00400000,	/* This sound will follow a linear-square rolloff model where mindistance = full volume, maxdistance = silence.  Rolloffscale is ignored. */
			e3D_CustomRolloff = 0x04000000,			/* This sound will follow a rolloff model defined by Sound::set3DCustomRolloff / Channel::set3DCustomRolloff.  */
			e3D_IgnoreGeometry = 0x40000000,		/* Is not affect by geometry occlusion.  If not specified in Sound::setMode, or Channel::setMode, the flag is cleared and it is affected by geometry again. */
			eUnicode = 0x01000000,					/* Filename is double-byte unicode. */
			eIgnoreTags = 0x02000000,				/* Skips id3v2/asf/etc tag checks when opening a sound, to reduce seek/read overhead when opening files (helps with CD performance). */
			eLowMem = 0x08000000,					/* Removes some features from samples to give a lower memory overhead, like Sound::getName.  See remarks. */
			eLoadSecondaryRam = 0x20000000,			/* Load sound into the secondary RAM of supported platform. On PS3, sounds will be loaded into RSX/VRAM. */
			eVirtualPlayFromStart = 0x80000000,		/* For sounds that start virtual (due to being quiet or low importance), instead of swapping back to audible, and playing at the correct offset according to time, this flag makes the sound play from the start. */
		};

		enum : uint64_t
		{
			eMaxChannel = 32,
			eInvalidChannelID = std::numeric_limits<uint64_t>::max(),
		};

		struct ListenerAttributes
		{
			math::Vector3 f3Position{ math::Vector3::Zero };
			math::Vector3 f3Velocity{ math::Vector3::Zero };
			math::Vector3 f3Forward{ math::Vector3::Forward };
			math::Vector3 f3Up{ math::Vector3::Up };
		};

		struct tChannelID {};
		using ChannelID = PhantomType<tChannelID, uint64_t>;
	}
}