/**
 * \file audio_decoder.h
 * \brief IAudioDecoder
 * \date 2012-04-11 09:34GMT
 * \author Jan Boon (Kaetemi)
 * IAudioDecoder
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

#ifndef NLSOUND_AUDIO_DECODER_H
#define NLSOUND_AUDIO_DECODER_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

namespace NLSOUND {

/**
 * \brief IAudioDecoder
 * \date 2008-08-30 11:38GMT
 * \author Jan Boon (Kaetemi)
 * IAudioDecoder is only used by the driver implementation to stream 
 * music files into a readable format (it's a simple decoder interface).
 * You should not call these functions (getSongTitle) on nlsound or user level, 
 * as a driver might have additional music types implemented.
 * TODO: Split IAudioDecoder into IAudioDecoder (actual decoding) and IMediaDemuxer (stream splitter), and change the interface to make more sense.
 * TODO: Allow user application to register more decoders.
 * TODO: Look into libavcodec for decoding audio?
 */
class IAudioDecoder
{
private:
	// pointers
	/// Stream from file created by IAudioDecoder
	NLMISC::IStream *_InternalStream;

public:
	IAudioDecoder();
	virtual ~IAudioDecoder();

	/// Create a new music buffer, may return NULL if unknown type, destroy with delete. Filepath lookup done here. If async is true, it will stream from hd, else it will load in memory first.
	static IAudioDecoder *createAudioDecoder(const std::string &filepath, bool async, bool loop);

	/// Create a new music buffer from a stream, type is file extension like "ogg" etc.
	static IAudioDecoder *createAudioDecoder(const std::string &type, NLMISC::IStream *stream, bool loop);

	/// Get information on a music file (only artist and title at the moment).
	static bool getInfo(const std::string &filepath, std::string &artist, std::string &title);
	
	/// Get audio/container extensions that are currently supported by the nel sound library.
	static void getMusicExtensions(std::vector<std::string> &extensions);

	/// Return if a music extension is supported by the nel sound library.
	static bool isMusicExtensionSupported(const std::string &extension);

	/// Get how many bytes the music buffer requires for output minimum.
	virtual uint32 getRequiredBytes() = 0;

	/// Get an amount of bytes between minimum and maximum (can be lower than minimum if at end).
	virtual uint32 getNextBytes(uint8 *buffer, uint32 minimum, uint32 maximum) = 0;

	/// Get the amount of channels (2 is stereo) in output.
	virtual uint8 getChannels() = 0;

	/// Get the samples per second (often 44100) in output.
	virtual uint getSamplesPerSec() = 0;

	/// Get the bits per sample (often 16) in output.
	virtual uint8 getBitsPerSample() = 0;

	/// Get if the music has ended playing (never true if loop).
	virtual bool isMusicEnded() = 0;

	/// Get the total time in seconds.
	virtual float getLength() = 0;

	/// Set looping
	virtual void setLooping(bool loop) = 0;
}; /* class IAudioDecoder */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_AUDIO_DECODER_H */

/* end of file */
