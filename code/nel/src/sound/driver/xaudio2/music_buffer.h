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

#ifndef NLSOUND_MUSIC_BUFFER_H
#define NLSOUND_MUSIC_BUFFER_H
#include "stdxaudio2.h"

// STL includes

// NeL includes

// Project includes

namespace NLMISC {
	class IStream;
	class CIFile;
}

namespace NLSOUND {

	/*
	 * TODO: Streaming
	 * Some kind of decent streaming functionality, to get rid of the current music implementation. Audio decoding should be done on nlsound level. IBuffer needs a writable implementation, it allocates and owns the data memory, which can be written to by nlsound. When buffer is written, a function needs to be called to 'finalize' the buffer (so it can be submitted to OpenAL for example). 
	 * Required interface functions, IBuffer:
	 * /// Allocate a new writable buffer. If this buffer was already allocated, the previous data is released.
	 * /// May return NULL if the format or frequency is not supported by the driver.
	 * uint8 *IBuffer::openWritable(uint size, TBufferFormat bufferFormat, uint8 channels, uint8 bitsPerSample, uint32 frequency);
	 * /// Tell that you are done writing to this buffer, so it can be copied over to hardware if needed.
	 * /// If keepLocal is true, a local copy of the buffer will be kept (so allocation can be re-used later).
	 * /// keepLocal overrides the OptionLocalBufferCopy flag. The buffer can use this function internally.
	 * void IBuffer::lockWritable(bool keepLocal);
	 * Required interface functions, ISource:
	 * /// Enable or disable the streaming facilities.
	 * void ISource::setStreaming(bool streaming);
	 * /// Submits a new buffer to the stream. A buffer of 100ms length is optimal for streaming.
	 * /// Should be called by a thread which checks countStreamingBuffers every 100ms
	 * void ISource::submitStreamingBuffer(IBuffer *buffer);
	 * /// Returns the number of buffers that are queued (includes playing buffer). 3 buffers is optimal.
	 * uint ISource::countStreamingBuffers();
	 * Other required interface functions, ISource:
	 * /// Enable or disable 3d calculations (to send directly to speakers).
	 * void ISource::set3DMode(bool enable);
	 * For compatibility with music trough fmod, ISoundDriver:
	 * /// Returns true if the sound driver has a native implementation of IMusicChannel (bad!).
	 * /// If this returns false, use the nlsound music channel, which goes trough Ctrack/ISource,
	 * /// The nlsound music channel requires support for IBuffer/ISource streaming.
	 * bool ISoundDriver::hasMusicChannel();
	 */

/**
 * \brief IMusicBuffer
 * \date 2008-08-30 11:38GMT
 * \author Jan Boon (Kaetemi)
 * IMusicBuffer is only used by the driver implementation to stream 
 * music files into a readable format (it's a simple decoder interface).
 * You should not call these functions (getSongTitle) on nlsound or user level, 
 * as a driver might have additional music types implemented.
 * TODO: Change IMusicBuffer to IAudioDecoder, and change the interface to make more sense.
 * TODO: Allow user application to register more decoders.
 * TODO: Look into libavcodec for decoding audio.
 */
class IMusicBuffer
{
private:
	// pointers
	/// Stream from file created by IMusicBuffer
	NLMISC::IStream *_InternalStream;

public:
	IMusicBuffer();
	virtual ~IMusicBuffer();

	/// Create a new music buffer, may return NULL if unknown type, destroy with delete. Filepath lookup done here. If async is true, it will stream from hd, else it will load in memory first.
	static IMusicBuffer *createMusicBuffer(const std::string &filepath, bool async, bool loop);

	/// Create a new music buffer from a stream, type is file extension like "ogg" etc.
	static IMusicBuffer *createMusicBuffer(const std::string &type, NLMISC::IStream *stream, bool loop);

	/// Get information on a music file (only artist and title at the moment).
	static bool getInfo(const std::string &filepath, std::string &artist, std::string &title);

	/// Get how many bytes the music buffer requires for output minimum.
	virtual uint32 getRequiredBytes() =0;

	/// Get an amount of bytes between minimum and maximum (can be lower than minimum if at end).
	virtual uint32 getNextBytes(uint8 *buffer, uint32 minimum, uint32 maximum) =0;

	/// Get the amount of channels (2 is stereo) in output.
	virtual uint16 getChannels() =0;

	/// Get the samples per second (often 44100) in output.
	virtual uint32 getSamplesPerSec() =0;

	/// Get the bits per sample (often 16) in output.
	virtual uint16 getBitsPerSample() =0;

	/// Get if the music has ended playing (never true if loop).
	virtual bool isMusicEnded() =0;

	/// Get the total time in seconds.
	virtual float getLength() =0;
}; /* class IMusicBuffer */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_MUSIC_BUFFER_H */

/* end of file */
