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

#ifndef NLSOUND_AUDIO_DECODER_FFMPEG_H
#define NLSOUND_AUDIO_DECODER_FFMPEG_H
#include <nel/misc/types_nl.h>

#include <nel/sound/audio_decoder.h>

struct AVCodecContext;
struct AVFormatContext;
struct AVIOContext;
struct AVPacket;
struct SwrContext;

namespace NLSOUND {

/**
 * \brief CAudioDecoderFfmpeg
 * \date 2018-10-21 08:08GMT
 * \author Meelis Mägi (Nimetu)
 * CAudioDecoderFfmpeg
 * Create trough IAudioDecoder
 */
class CAudioDecoderFfmpeg : public IAudioDecoder
{
protected:
	NLMISC::IStream *_Stream;

	bool _IsSupported;
	bool _Loop;
	bool _IsMusicEnded;
	sint32 _StreamOffset;
	sint32 _StreamSize;

	AVIOContext *_AvioContext;
	AVFormatContext *_FormatContext;
	AVCodecContext *_AudioContext;
	SwrContext *_SwrContext;

	// selected stream
	sint32 _AudioStreamIndex;

	// output buffer for decoded frame
	SwrContext *_ConvertContext;

private:
	// called from constructor if ffmpeg fails to initialize
	// or from destructor to cleanup ffmpeg pointers
	void release();

public:
	CAudioDecoderFfmpeg(NLMISC::IStream *stream, bool loop);
	virtual ~CAudioDecoderFfmpeg();

	inline NLMISC::IStream *getStream() { return _Stream; }
	inline sint32 getStreamSize() { return _StreamSize; }
	inline sint32 getStreamOffset() { return _StreamOffset; }

	// Return true if ffmpeg is able to decode the stream
	bool isFormatSupported() const;

	/// Get information on a music file (only artist and title at the moment).
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
}; /* class CAudioDecoderFfmpeg */

} /* namespace NLSOUND */

#endif // NLSOUND_AUDIO_DECODER_FFMPEG_H

/* end of file */
