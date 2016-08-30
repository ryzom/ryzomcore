/**
 * \file audio_decoder_vorbis.cpp
 * \brief CAudioDecoderVorbis
 * \date 2012-04-11 09:35GMT
 * \author Jan Boon (Kaetemi)
 * CAudioDecoderVorbis
 */

/* 
 * Copyright (C) 2008-2012  by authors
 * 
 * This file is part of RYZOM CORE.
 * RYZOM CORE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * RYZOM CORE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

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

	if (audio_decoder_vorbis->getStream()->seek(SEEK_SET ? audio_decoder_vorbis->getStreamOffset() + (sint32)offset : (sint32)offset, origin)) return 0;
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
  (int (*)(void *))							 NULL, //vorbisCloseFunc,
  (long (*)(void *))							vorbisTellFunc
};

CAudioDecoderVorbis::CAudioDecoderVorbis(NLMISC::IStream *stream, bool loop) 
: _Stream(stream), _Loop(loop), _IsMusicEnded(false), _StreamSize(0)
{
	_StreamOffset = stream->getPos();
	stream->seek(0, NLMISC::IStream::end);
	_StreamSize = stream->getPos();
	stream->seek(_StreamOffset, NLMISC::IStream::begin);
	ov_open_callbacks(this, &_OggVorbisFile, NULL, 0, OV_CALLBACKS_NLMISC_STREAM);
}

CAudioDecoderVorbis::~CAudioDecoderVorbis()
{
	ov_clear(&_OggVorbisFile);
}

/// Get information on a music file (only artist and title at the moment).
bool CAudioDecoderVorbis::getInfo(NLMISC::IStream *stream, std::string &artist, std::string &title, float &length)
{
	CAudioDecoderVorbis mbv(stream, false); // just opens and closes the oggvorbisfile thing :)
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

uint32 CAudioDecoderVorbis::getNextBytes(uint8 *buffer, uint32 minimum, uint32 maximum)
{
	sint current_section = 0; // ???
	if (_IsMusicEnded) return 0;
	nlassert(minimum <= maximum); // can't have this..
	uint32 bytes_read = 0;
#ifdef NL_BIG_ENDIAN
	sint endianness = 1;
#else
	sint endianness = 0;
#endif
	do
	{
		// signed 16-bit or unsigned 8-bit little-endian samples
		sint br = ov_read(&_OggVorbisFile, (char *)&buffer[bytes_read], maximum - bytes_read, 
			endianness, // Specifies big or little endian byte packing. 0 for little endian, 1 for b ig endian. Typical value is 0.
			getBitsPerSample() == 8 ? 1 : 2, 
			getBitsPerSample() == 8 ? 0 : 1, // Signed or unsigned data. 0 for unsigned, 1 for signed. Typically 1.
			&current_section);
		// nlinfo(NLSOUND_XAUDIO2_PREFIX "current_section: %i", current_section);
		if (br > 0)
		{
			bytes_read += (uint32)br;
		}
		else if (br == 0) // EOF
		{
			if (_Loop)
			{
				ov_pcm_seek(&_OggVorbisFile, 0);
				//_Stream->seek(0, NLMISC::IStream::begin);
			}
			else 
			{
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
				nlwarning("ov_read returned OV_HOLE");
				break;
			case OV_EINVAL:
				nlwarning("ov_read returned OV_EINVAL");
				break;
			case OV_EBADLINK:
				nlwarning("ov_read returned OV_EBADLINK");
				break;
			default:
				nlwarning("ov_read returned %d", br);
			}
		}
	} while (bytes_read < minimum);
	return bytes_read;
}

uint8 CAudioDecoderVorbis::getChannels()
{
	vorbis_info *vi = ov_info(&_OggVorbisFile, -1);
	if (vi) return (uint8)vi->channels;
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
