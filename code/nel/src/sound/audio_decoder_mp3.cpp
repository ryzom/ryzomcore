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


#include "stdsound.h"

#if !defined(NL_OS_WINDOWS) || (NL_COMP_VC_VERSION > 90) /* VS2008 does not have stdint.h */

#include <nel/sound/audio_decoder_mp3.h>

#define DR_MP3_IMPLEMENTATION
#include <nel/sound/decoder/dr_mp3.h>

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

namespace NLSOUND {

// callback for drmp3
static size_t drmp3_read(void* pUserData, void* pBufferOut, size_t bytesToRead)
{
	NLSOUND::CAudioDecoderMP3 *decoder = static_cast<NLSOUND::CAudioDecoderMP3 *>(pUserData);
	NLMISC::IStream *stream = decoder->getStream();
	nlassert(stream->isReading());

	uint32 available = decoder->getStreamSize() - stream->getPos();
	if (available == 0)
		return 0;

	if (bytesToRead > available)
		bytesToRead = available;

	stream->serialBuffer((uint8 *)pBufferOut, bytesToRead);
	return bytesToRead;
}

// callback for drmp3
static drmp3_bool32 drmp3_seek(void* pUserData, int offset, drmp3_seek_origin origin)
{
	NLSOUND::CAudioDecoderMP3 *decoder = static_cast<NLSOUND::CAudioDecoderMP3 *>(pUserData);
	NLMISC::IStream *stream = decoder->getStream();
	nlassert(stream->isReading());

	NLMISC::IStream::TSeekOrigin seekOrigin;
	if (origin == drmp3_seek_origin_start)
		seekOrigin = NLMISC::IStream::begin;
	else if (origin == drmp3_seek_origin_current)
		seekOrigin = NLMISC::IStream::current;
	else
		return false;

	stream->seek((sint32) offset, seekOrigin);
	return true;
}

// these should always be 44100Hz/16bit/2ch
#define MP3_SAMPLE_RATE 44100
#define MP3_BITS_PER_SAMPLE 16
#define MP3_CHANNELS 2

CAudioDecoderMP3::CAudioDecoderMP3(NLMISC::IStream *stream, bool loop)
: IAudioDecoder(),
	_Stream(stream), _Loop(loop), _IsMusicEnded(false), _StreamSize(0), _IsSupported(false), _PCMFrameCount(0)
{
	_StreamOffset = stream->getPos();
	stream->seek(0, NLMISC::IStream::end);
	_StreamSize = stream->getPos();
	stream->seek(_StreamOffset, NLMISC::IStream::begin);

	drmp3_config config;
	config.outputChannels = MP3_CHANNELS;
	config.outputSampleRate = MP3_SAMPLE_RATE;

	_IsSupported = drmp3_init(&_Decoder, &drmp3_read, &drmp3_seek, this, &config);
	if (!_IsSupported)
	{
		nlwarning("MP3: Decoder failed to read stream");
	}
}

CAudioDecoderMP3::~CAudioDecoderMP3()
{
	if (_IsSupported)
	{
		drmp3_uninit(&_Decoder);
	}
}

bool CAudioDecoderMP3::isFormatSupported() const
{
	return _IsSupported;
}

/// Get information on a music file.
bool CAudioDecoderMP3::getInfo(NLMISC::IStream *stream, std::string &artist, std::string &title, float &length)
{
	CAudioDecoderMP3 mp3(stream, false);
	if (!mp3.isFormatSupported())
	{
		title.clear();
		artist.clear();
		length = 0.f;

		return false;
	}
	length = mp3.getLength();

	// ID3v1
	stream->seek(-128, NLMISC::IStream::end);
	{
		uint8 buf[128];
		stream->serialBuffer(buf, 128);

		if(buf[0] == 'T' && buf[1] == 'A' && buf[2] == 'G')
		{
			uint i;
			for(i = 0; i < 30; ++i) if (buf[3+i] == '\0') break;
			artist.assign((char *)&buf[3], i);

			for(i = 0; i < 30; ++i) if (buf[33+i] == '\0') break;
			title.assign((char *)&buf[33], i);
		}
	}

	return true;
}

uint32 CAudioDecoderMP3::getRequiredBytes()
{
	return 0; // no minimum requirement of bytes to buffer out
}

uint32 CAudioDecoderMP3::getNextBytes(uint8 *buffer, uint32 minimum, uint32 maximum)
{
	if (_IsMusicEnded) return 0;
	nlassert(minimum <= maximum); // can't have this..

	// TODO: CStreamFileSource::play() will stall when there is no frames on warmup
	// supported can be set false if there is an issue creating converter
	if (!_IsSupported)
	{
		_IsMusicEnded = true;
		return 1;
	}

	sint16 *pFrameBufferOut = (sint16 *)buffer;
	uint32 bytesPerFrame = MP3_BITS_PER_SAMPLE / 8 * _Decoder.channels;

	uint32 totalFramesRead = 0;
	uint32 framesToRead = minimum / bytesPerFrame;
	while(framesToRead > 0)
	{
		float tempBuffer[4096];
		uint64 tempFrames = drmp3_countof(tempBuffer) / _Decoder.channels;

		if (tempFrames > framesToRead)
			tempFrames = framesToRead;

		tempFrames = drmp3_read_pcm_frames_f32(&_Decoder, tempFrames, tempBuffer);
		if (tempFrames == 0)
			break;

		drmp3dec_f32_to_s16(tempBuffer, pFrameBufferOut, tempFrames * _Decoder.channels);
		pFrameBufferOut += tempFrames * _Decoder.channels;

		framesToRead -= tempFrames;
		totalFramesRead += tempFrames;
	}

	_IsMusicEnded = (framesToRead > 0);
	return totalFramesRead * bytesPerFrame;
}

uint8 CAudioDecoderMP3::getChannels()
{
	return _Decoder.channels;
}

uint CAudioDecoderMP3::getSamplesPerSec()
{
	return _Decoder.sampleRate;
}

uint8 CAudioDecoderMP3::getBitsPerSample()
{
	return MP3_BITS_PER_SAMPLE;
}

bool CAudioDecoderMP3::isMusicEnded()
{
	return _IsMusicEnded;
}

float CAudioDecoderMP3::getLength()
{
	// cached because drmp3_get_pcm_frame_count is reading full file
	if (_PCMFrameCount == 0)
	{
		_PCMFrameCount = drmp3_get_pcm_frame_count(&_Decoder);
	}

	return _PCMFrameCount / (float) _Decoder.sampleRate;
}

void CAudioDecoderMP3::setLooping(bool loop)
{
	 _Loop = loop;
}

} /* namespace NLSOUND */

#endif /* (NL_COMP_VC_VERSION > 90) */

/* end of file */
