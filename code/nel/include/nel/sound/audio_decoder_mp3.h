// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2018  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef NLSOUND_AUDIO_DECODER_MP3_H
#define NLSOUND_AUDIO_DECODER_MP3_H
#include <nel/misc/types_nl.h>

#if !defined(NL_OS_WINDOWS) || (NL_COMP_VC_VERSION > 90) /* VS2008 does not have stdint.h */

#include <nel/sound/audio_decoder.h>

// disable drmp3_init_file()
#define DR_MP3_NO_STDIO
#include <nel/sound/decoder/dr_mp3.h>

namespace NLSOUND {

/**
 * \brief CAudioDecoderMP3
 * \date 2019-01-13 12:39GMT
 * \author Meelis Mägi (Nimetu)
 * CAudioDecoderMP3
 * Create trough IAudioDecoder, type "mp3"
 */
class CAudioDecoderMP3 : public IAudioDecoder
{
protected:
	NLMISC::IStream *_Stream;

	bool _IsSupported;
	bool _Loop;
	bool _IsMusicEnded;
	sint32 _StreamOffset;
	sint32 _StreamSize;

	drmp3 _Decoder;

	// set to total pcm frames after getLength() is called
	uint64 _PCMFrameCount;

public:
	CAudioDecoderMP3(NLMISC::IStream *stream, bool loop);
	virtual ~CAudioDecoderMP3();

	inline NLMISC::IStream *getStream() { return _Stream; }
	inline sint32 getStreamSize() { return _StreamSize; }
	inline sint32 getStreamOffset() { return _StreamOffset; }

	// Return true if mp3 is valid
	bool isFormatSupported() const;

	/// Get information on a music file (only ID3v1 tag is read.
	static bool getInfo(NLMISC::IStream *stream, std::string &artist, std::string &title, float &length);

	/// Get how many bytes the music buffer requires for output minimum.
	virtual uint32 getRequiredBytes();

	/// Get an amount of bytes between minimum and maximum (can be lower than minimum if at end).
	virtual uint32 getNextBytes(uint8 *buffer, uint32 minimum, uint32 maximum);

	/// Get the amount of channels (2 is stereo) in output.
	virtual uint8 getChannels();

	/// Get the samples per second (often 44100) in output.
	virtual uint getSamplesPerSec();

	/// Get the bits per sample (often 16) in output.
	virtual uint8 getBitsPerSample();

	/// Get if the music has ended playing (never true if loop).
	virtual bool isMusicEnded();

	/// Get the total time in seconds.
	virtual float getLength();

	/// Set looping
	virtual void setLooping(bool loop);

}; /* class CAudioDecoderMP3 */

} /* namespace NLSOUND */

#endif /* (NL_COMP_VC_VERSION > 90) */

#endif // NLSOUND_AUDIO_DECODER_MP3_H

/* end of file */
