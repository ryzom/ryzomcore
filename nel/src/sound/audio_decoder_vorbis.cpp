/**
 * \file audio_decoder_vorbis.cpp
 * \brief CAudioDecoderVorbis
 * \date 2012-04-11 09:35GMT
 * \author Jan Boon (Kaetemi)
 * CAudioDecoderVorbis
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2008-2022  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2016  Winch Gate Property Limited
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
#include <nel/sound/audio_decoder_vorbis.h>

// STL includes

// NeL includes
#include <nel/misc/debug.h>

// Project includes

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

size_t vorbisReadFunc(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	CAudioDecoderVorbis *audio_decoder_vorbis = (CAudioDecoderVorbis *)datasource;
	NLMISC::IStream *stream = audio_decoder_vorbis->getStream();
	nlassert(stream->isReading());
	sint32 length = (sint32)(size * nmemb);
	if (length > audio_decoder_vorbis->getStreamSize() - stream->getPos())
		length = audio_decoder_vorbis->getStreamSize() - stream->getPos();
	stream->serialBuffer((uint8 *)ptr, length);
	return length;
}

int vorbisSeekFunc(void *datasource, ogg_int64_t offset, int whence)
{
	if (whence == SEEK_CUR && offset == 0)
	{
		// nlwarning(NLSOUND_XAUDIO2_PREFIX "This seek call doesn't do a damn thing, wtf.");
		return 0; // ooookkaaaaaayyy
	}

	CAudioDecoderVorbis *audio_decoder_vorbis = (CAudioDecoderVorbis *)datasource;

	NLMISC::IStream::TSeekOrigin origin;
	switch (whence)
	{
	case SEEK_SET:
		origin = NLMISC::IStream::begin;
		break;
	case SEEK_CUR:
		origin = NLMISC::IStream::current;
		break;
	case SEEK_END:
		origin = NLMISC::IStream::end;
		break;
	default:
		// nlwarning(NLSOUND_XAUDIO2_PREFIX "Seeking to fake origin.");
		return -1;
	}

	sint32 actual_offset = (origin == NLMISC::IStream::begin)
		? audio_decoder_vorbis->getStreamOffset() + (sint32)offset
		: (sint32)offset;
	if (audio_decoder_vorbis->getStream()->seek(actual_offset, origin)) return 0;
	else return -1;
}

//int vorbisCloseFunc(void *datasource)
//{
//	//CAudioDecoderVorbis *audio_decoder_vorbis = (CAudioDecoderVorbis *)datasource;
//}

long vorbisTellFunc(void *datasource)
{
	CAudioDecoderVorbis *audio_decoder_vorbis = (CAudioDecoderVorbis *)datasource;
	return (long)(audio_decoder_vorbis->getStream()->getPos() - audio_decoder_vorbis->getStreamOffset());
}

static ov_callbacks OV_CALLBACKS_NLMISC_STREAM = {
  (size_t (*)(void *, size_t, size_t, void *))  vorbisReadFunc,
  (int (*)(void *, ogg_int64_t, int))		  vorbisSeekFunc,
  (int (*)(void *)) nullptr, //vorbisCloseFunc,
  (long (*)(void *))							vorbisTellFunc
};

/// The share of a channel in the left and in the right channel of a stereo mix
struct CVorbisMixDownGain
{
	float Left;
	float Right;
};

/// -3 dB, the share of the front center and of a side or rear channel
#define NL_VORBIS_MIX_REDUCED 0.70710678f
/// -6 dB, the share of the rear center on each side
#define NL_VORBIS_MIX_REAR_CENTER 0.5f

/** The channels of a stream of 3 to 8 channels, in the order the Vorbis I specification gives them, and their share in
 *  the left and in the right channel of a stereo mix. The shares are those of ITU-R BS.775 for the layouts it has, and
 *  of the resampler of ffmpeg with its default levels (center and surround at -3 dB, no low frequency channel) for all
 *  of them:
 *   - front left and front right: 1 on their own side
 *   - front center: 0.7071 on both sides
 *   - side or rear left, side or rear right: 0.7071 on their own side
 *   - rear center (7 channels): a surround at 0.7071, split over both sides at another 0.7071, so 0.5 on both sides
 *   - low frequency: left out
 *  mixDownToStereo scales the shares of a side to a sum of 1.
 */
// clang-format off
static const CVorbisMixDownGain s_MixDownGains[6][8] = {
	// 3: left, center, right
	{ { 1.f, 0.f }, { NL_VORBIS_MIX_REDUCED, NL_VORBIS_MIX_REDUCED }, { 0.f, 1.f } },
	// 4: front left, front right, rear left, rear right
	{ { 1.f, 0.f }, { 0.f, 1.f }, { NL_VORBIS_MIX_REDUCED, 0.f }, { 0.f, NL_VORBIS_MIX_REDUCED } },
	// 5: front left, center, front right, rear left, rear right
	{ { 1.f, 0.f }, { NL_VORBIS_MIX_REDUCED, NL_VORBIS_MIX_REDUCED }, { 0.f, 1.f },
	  { NL_VORBIS_MIX_REDUCED, 0.f }, { 0.f, NL_VORBIS_MIX_REDUCED } },
	// 6: front left, center, front right, rear left, rear right, low frequency
	{ { 1.f, 0.f }, { NL_VORBIS_MIX_REDUCED, NL_VORBIS_MIX_REDUCED }, { 0.f, 1.f },
	  { NL_VORBIS_MIX_REDUCED, 0.f }, { 0.f, NL_VORBIS_MIX_REDUCED }, { 0.f, 0.f } },
	// 7: front left, center, front right, side left, side right, rear center, low frequency
	{ { 1.f, 0.f }, { NL_VORBIS_MIX_REDUCED, NL_VORBIS_MIX_REDUCED }, { 0.f, 1.f },
	  { NL_VORBIS_MIX_REDUCED, 0.f }, { 0.f, NL_VORBIS_MIX_REDUCED },
	  { NL_VORBIS_MIX_REAR_CENTER, NL_VORBIS_MIX_REAR_CENTER }, { 0.f, 0.f } },
	// 8: front left, center, front right, side left, side right, rear left, rear right, low frequency
	{ { 1.f, 0.f }, { NL_VORBIS_MIX_REDUCED, NL_VORBIS_MIX_REDUCED }, { 0.f, 1.f },
	  { NL_VORBIS_MIX_REDUCED, 0.f }, { 0.f, NL_VORBIS_MIX_REDUCED },
	  { NL_VORBIS_MIX_REDUCED, 0.f }, { 0.f, NL_VORBIS_MIX_REDUCED }, { 0.f, 0.f } },
};
// clang-format on

/// The most channels a stream can have: the specification gives the channels of a larger stream no meaning
static const uint s_MaxChannels = 8;

/// One sample of the mix as 16 bits. The range is checked on the float: a cast of a value that does not fit the integer
/// is undefined.
static sint16 mixToSint16(float sample)
{
	if (!(sample > -32768.f))
		return -32768; // and for a NaN
	if (sample >= 32767.f)
		return 32767;
	return (sint16)floorf(sample + 0.5f);
}

/** Mixes samples of 3 to 8 channels down to stereo of 16 bits. The shares of the channels in a side are scaled to a
 *  sum of 1, the same on both sides: channels at full scale do not clip, and the mix is quieter than its loudest
 *  channel was.
 */
static void mixDownToStereo(float **pcm, uint channels, uint samples, sint16 *stereo)
{
	const CVorbisMixDownGain *gains = s_MixDownGains[channels - 3];
	float sum = 0.f;
	for (uint c = 0; c < channels; ++c)
		sum += gains[c].Left;
	const float scale = 32768.f / sum;
	for (uint i = 0; i < samples; ++i)
	{
		float left = 0.f;
		float right = 0.f;
		for (uint c = 0; c < channels; ++c)
		{
			left += pcm[c][i] * gains[c].Left;
			right += pcm[c][i] * gains[c].Right;
		}
		stereo[i * 2] = mixToSint16(left * scale);
		stereo[i * 2 + 1] = mixToSint16(right * scale);
	}
}

CAudioDecoderVorbis::CAudioDecoderVorbis(NLMISC::IStream *stream, bool loop) 
: _Stream(stream), _Loop(loop), _IsMusicEnded(false), _StreamSize(0), m_IsSupported(false), m_Channels(0)
{
	_StreamOffset = stream->getPos();
	stream->seek(0, NLMISC::IStream::end);
	_StreamSize = stream->getPos();
	stream->seek(_StreamOffset, NLMISC::IStream::begin);

	// a stream that is not Ogg Vorbis leaves the file closed, and every call on a closed file fails
	int res = ov_open_callbacks(this, &_OggVorbisFile, nullptr, 0, OV_CALLBACKS_NLMISC_STREAM);
	if (res != 0)
	{
		nlwarning("VORBIS: Cannot open '%s' as Ogg Vorbis (ov_open_callbacks returned %d)", stream->getStreamName().c_str(), res);
		_IsMusicEnded = true;
		return;
	}

	// more than 2 channels are mixed down to stereo, which takes a known channel layout
	vorbis_info *vi = ov_info(&_OggVorbisFile, -1);
	m_Channels = vi ? (uint)vi->channels : 0;
	if (m_Channels > s_MaxChannels)
	{
		nlwarning("VORBIS: Cannot play '%s', it has %u channels and no more than %u are supported", stream->getStreamName().c_str(), m_Channels, s_MaxChannels);
		_IsMusicEnded = true;
		return;
	}
	m_IsSupported = true;
}

CAudioDecoderVorbis::~CAudioDecoderVorbis()
{
	ov_clear(&_OggVorbisFile);
}

/// Get information on a music file (only artist and title at the moment).
bool CAudioDecoderVorbis::getInfo(NLMISC::IStream *stream, std::string &artist, std::string &title, float &length)
{
	CAudioDecoderVorbis mbv(stream, false); // just opens and closes the oggvorbisfile thing :)
	if (!mbv.isFormatSupported())
	{
		title.clear();
		artist.clear();
		length = 0.f;
		return false;
	}
	vorbis_comment *vc = ov_comment(&mbv._OggVorbisFile, -1);
	char *title_c = vorbis_comment_query(vc, "title", 0);
	if (title_c) title = title_c; else title.clear();
	char *artist_c = vorbis_comment_query(vc, "artist", 0);
	if (artist_c) artist = artist_c; else artist.clear();
	length = (float)ov_time_total(&mbv._OggVorbisFile, -1);
	return true;
}

uint32 CAudioDecoderVorbis::getRequiredBytes()
{
	return 0; // no minimum requirement of bytes to buffer out
}

/// How many interruptions in the data are passed over in a row before the stream ends
static const uint s_MaxHoles = 32;

uint32 CAudioDecoderVorbis::getNextBytes(uint8 *buffer, uint32 minimum, uint32 maximum)
{
	sint current_section = 0; // ???
	if (_IsMusicEnded) return 0;
	nlassert(minimum <= maximum); // can't have this..
	uint32 bytes_read = 0;
	// OV_HOLE results in a row
	uint holes = 0;
	// true from the rewind of a looping stream until samples are read: a stream without samples is rewound once
	bool rewound = false;
#ifdef NL_BIG_ENDIAN
	sint endianness = 1;
#else
	sint endianness = 0;
#endif
	do
	{
		sint br;
		if (m_Channels > 2)
		{
			// more than 2 channels are read as floats and mixed down to stereo of 16 bits, 4 bytes per sample
			float **pcm = nullptr;
			if (maximum - bytes_read < 4)
			{
				// No room for a sample. ov_read below fails with OV_EINVAL for that, which ends the stream: the same
				// here, so that both paths return the minimum at least or end the stream (ov_read_float would return
				// 0 samples, which reads as the end of the file).
				br = OV_EINVAL;
			}
			else
			{
				br = (sint)ov_read_float(&_OggVorbisFile, &pcm, (int)((maximum - bytes_read) / 4), &current_section);
			}
			if (br > 0)
			{
				// the next stream of a chained file can have another channel count than the one the output was set up for
				vorbis_info *vi = ov_info(&_OggVorbisFile, -1);
				if (!vi || vi->channels != (int)m_Channels)
				{
					nlwarning("VORBIS: '%s' goes on with %d channels after %u, which is not supported, the stream ends", _Stream->getStreamName().c_str(), vi ? vi->channels : 0, m_Channels);
					_IsMusicEnded = true;
					break;
				}
				mixDownToStereo(pcm, m_Channels, (uint)br, (sint16 *)&buffer[bytes_read]);
				br *= 4;
			}
		}
		else
		{
			// signed 16-bit or unsigned 8-bit little-endian samples
			br = ov_read(&_OggVorbisFile, (char *)&buffer[bytes_read], maximum - bytes_read, 
				endianness, // Specifies big or little endian byte packing. 0 for little endian, 1 for b ig endian. Typical value is 0.
				getBitsPerSample() == 8 ? 1 : 2, 
				getBitsPerSample() == 8 ? 0 : 1, // Signed or unsigned data. 0 for unsigned, 1 for signed. Typically 1.
				&current_section);
		}
		// nlinfo(NLSOUND_XAUDIO2_PREFIX "current_section: %i", current_section);
		if (br > 0)
		{
			bytes_read += (uint32)br;
			holes = 0;
			rewound = false;
		}
		else if (br == 0) // EOF
		{
			if (_Loop && !rewound)
			{
				int res = ov_pcm_seek(&_OggVorbisFile, 0);
				if (res != 0)
				{
					nlwarning("VORBIS: Cannot rewind '%s' to loop it (ov_pcm_seek returned %d)", _Stream->getStreamName().c_str(), res);
					_IsMusicEnded = true;
					break;
				}
				rewound = true;
				//_Stream->seek(0, NLMISC::IStream::begin);
			}
			else 
			{
				if (_Loop)
					nlwarning("VORBIS: '%s' has no samples to loop", _Stream->getStreamName().c_str());
				_IsMusicEnded = true;
				break; 
			}
		}
		else
		{ 
			// error
			switch(br)
			{
			case OV_HOLE:
				// an interruption in the data (a damaged page), after which the decoder goes on
				nlwarning("ov_read returned OV_HOLE");
				++holes;
				if (holes >= s_MaxHoles)
				{
					nlwarning("VORBIS: '%s' has %u interruptions in a row, the stream ends", _Stream->getStreamName().c_str(), holes);
					_IsMusicEnded = true;
				}
				break;
			case OV_EINVAL:
				_IsMusicEnded = true;
				nlwarning("ov_read returned OV_EINVAL");
				break;
			case OV_EBADLINK:
				_IsMusicEnded = true;
				nlwarning("ov_read returned OV_EBADLINK");
				break;
			default:
				_IsMusicEnded = true;
				nlwarning("ov_read returned %d", br);
				break;
			}
			// the break above leaves the switch only: after an error the same read would fail again, forever
			if (_IsMusicEnded)
				break;
		}
	} while (bytes_read < minimum);
	return bytes_read;
}

uint8 CAudioDecoderVorbis::getChannels()
{
	vorbis_info *vi = ov_info(&_OggVorbisFile, -1);
	// getNextBytes mixes more than 2 channels down to stereo
	if (vi) return (uint8)std::min(vi->channels, 2);
	nlwarning("ov_info returned NULL");
	return 0;
}

uint CAudioDecoderVorbis::getSamplesPerSec()
{
	vorbis_info *vi = ov_info(&_OggVorbisFile, -1);
	if (vi) return (uint)vi->rate;
	nlwarning("ov_info returned NULL");
	return 0;
}

uint8 CAudioDecoderVorbis::getBitsPerSample()
{
	return 16;
}

bool CAudioDecoderVorbis::isMusicEnded()
{
	return _IsMusicEnded;
}

float CAudioDecoderVorbis::getLength()
{
	return (float)ov_time_total(&_OggVorbisFile, -1);
}

void CAudioDecoderVorbis::setLooping(bool loop)
{
	 _Loop = loop;
}

} /* namespace NLSOUND */

/* end of file */
