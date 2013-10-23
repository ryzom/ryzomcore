/**
 * \file audio_decoder_vorbis.h
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

#ifndef NLSOUND_AUDIO_DECODER_VORBIS_H
#define NLSOUND_AUDIO_DECODER_VORBIS_H
#include <nel/misc/types_nl.h>

// STL includes

// 3rd Party includes
#ifdef NL_OS_WINDOWS
#	pragma warning( push )
#	pragma warning( disable : 4244 )
#endif
#include <vorbis/vorbisfile.h>
#ifdef NL_OS_WINDOWS
#	pragma warning( pop )
#endif

// NeL includes
#include <nel/sound/audio_decoder.h>

// Project includes

namespace NLSOUND {

/**
 * \brief CAudioDecoderVorbis
 * \date 2008-08-30 11:38GMT
 * \author Jan Boon (Kaetemi)
 * CAudioDecoderVorbis
 * Create trough IAudioDecoder, type "ogg"
 */
class CAudioDecoderVorbis : public IAudioDecoder
{
protected:
	// outside pointers
	NLMISC::IStream *_Stream;

	// pointers
	
	// instances
	OggVorbis_File _OggVorbisFile;
	bool _Loop;
	bool _IsMusicEnded;
	sint32 _StreamOffset;
	sint32 _StreamSize;
public:
	CAudioDecoderVorbis(NLMISC::IStream *stream, bool loop);
	virtual ~CAudioDecoderVorbis();
	inline NLMISC::IStream *getStream() { return _Stream; }
	inline sint32 getStreamSize() { return _StreamSize; }
	inline sint32 getStreamOffset() { return _StreamOffset; }

	/// Get information on a music file (only artist and title at the moment).
	static bool getInfo(NLMISC::IStream *stream, std::string &artist, std::string &title);

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
}; /* class CAudioDecoderVorbis */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_AUDIO_DECODER_VORBIS_H */

/* end of file */
