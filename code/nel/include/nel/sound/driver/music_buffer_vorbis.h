// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef NLSOUND_MUSIC_BUFFER_VORBIS_H
#define NLSOUND_MUSIC_BUFFER_VORBIS_H

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

// Project includes
#include "music_buffer.h"

namespace NLSOUND
{

/**
 * \brief CMusicBufferVorbis
 * \date 2008-08-30 11:38GMT
 * \author Jan Boon (Kaetemi)
 * CMusicBufferVorbis
 * Create trough IMusicBuffer, type "ogg"
 */
class CMusicBufferVorbis : public IMusicBuffer
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
	CMusicBufferVorbis(NLMISC::IStream *stream, bool loop);
	virtual ~CMusicBufferVorbis();
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
	virtual uint32 getSamplesPerSec();

	/// Get the bits per sample (often 16) in output.
	virtual uint8 getBitsPerSample();

	/// Get if the music has ended playing (never true if loop).
	virtual bool isMusicEnded();

	/// Get the total time in seconds.
	virtual float getLength();

	/// Get the size of uncompressed data in bytes.
	virtual uint getUncompressedSize();
}; /* class CMusicBufferVorbis */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_MUSIC_BUFFER_VORBIS_H */

/* end of file */
