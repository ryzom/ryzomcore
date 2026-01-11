// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_BUFFER_H
#define NL_BUFFER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/string_mapper.h"
#include "sound_driver.h"

namespace NLMISC
{
	class IStream;
}

namespace NLSOUND
{

/**
 * Sound buffer interface (implemented in sound driver dynamic library)
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class IBuffer
{
public:
	/// Compression format of the sample.
	enum TBufferFormat
	{
		/// Unknown format.
		FormatUnknown = 0, 
		/// Standard PCM format.
		FormatPcm = 1, 
		/// Intel/DVI ADPCM format, only available for 1 channel at 16 bits per sample, encoded at 4 bits per sample.
		/// This is only implemented in the DSound and XAudio2 driver.
		FormatDviAdpcm = 11, 
		/// No format set. Used when a TBufferFormat value has not been set to any value yet.
		FormatNotSet = (~0),
	};
	/// The storage mode of this buffer. Also controls the X-RAM extension of OpenAL.
	enum TStorageMode
	{
		/// Put buffer in sound hardware memory if possible, else in machine ram.
		StorageAuto, 
		/// Put buffer in sound hardware memory (fails if not possible). It is recommended to use StorageAuto instead of StorageHardware.
		StorageHardware, 
		/// Put buffer in machine ram (used for streaming). It behaves as if OptionSoftwareBuffer and OptionLocalBufferCopy are both enabled.
		StorageSoftware
	};
	
	/** Preset the name of the buffer. Used for async loading to give a name
	 *	before the buffer is effectivly loaded.
	 *	If the name after loading of the buffer doesn't match the preset name,
	 *	the load will assert.
	 */
	virtual void setName(NLMISC::TStringId bufferName) = 0;
	/// Return the name of this buffer
	virtual NLMISC::TStringId getName() const = 0;

	/// Set the sample format. (channels = 1, 2, ...; bitsPerSample = 8, 16; frequency = samples per second, 44100, ...)
	virtual void setFormat(TBufferFormat format, uint8 channels, uint8 bitsPerSample, uint32 frequency) = 0;
	/// Return the sample format information.
	virtual void getFormat(TBufferFormat &format, uint8 &channels, uint8 &bitsPerSample, uint32 &frequency) const = 0;
		/// Set the storage mode of this buffer, call before filling this buffer. Storage mode is always software if OptionSoftwareBuffer is enabled. Default is auto.
	virtual void setStorageMode(TStorageMode storageMode = IBuffer::StorageAuto) = 0;
	/// Get the storage mode of this buffer.
	virtual TStorageMode getStorageMode() = 0;

	/// Get a writable pointer to the buffer of specified size. Use capacity to specify the required bytes. Returns NULL in case of failure. It is only guaranteed that the original data is still available when using StorageSoftware and the specified size is not larger than the size specified in the last lock. Call setStorageMode() and setFormat() first.
	virtual uint8 *lock(uint capacity) = 0;
	/// Notify that you are done writing to this buffer, so it can be copied over to hardware if needed. Set size to the number of bytes actually written to the buffer. Returns true if ok.
	virtual bool unlock(uint size) = 0;
	/// Copy the data with specified size into the buffer. A readable local copy is only guaranteed when OptionLocalBufferCopy is set. Returns true if ok.
	virtual bool fill(const uint8 *src, uint size) = 0;
	
	/// Return the size of the buffer, in bytes.
	virtual uint getSize() const = 0;
	/// Return the duration (in ms) of the sample in the buffer.
	virtual float getDuration() const = 0;
	/// Return true if the buffer is stereo (multi-channel), false if mono.
	virtual bool isStereo() const = 0;	
	/// Return true if the buffer is loaded. Used for async load/unload.
	virtual bool isBufferLoaded() const = 0;
	
	//@{
	//\name ***deprecated***
	/// Set the sample format. Example: freq=44100. ***deprecated***
	void setFormat(TSampleFormat format, uint freq);
	/// Return the format and frequency. ***deprecated***
	void getFormat(TSampleFormat& format, uint& freq) const;
	/// Convert old sample format to new buffer format
	static void sampleFormatToBufferFormat(TSampleFormat sampleFormat, TBufferFormat &bufferFormat, uint8 &channels, uint8 &bitsPerSample);
	/// Convert new buffer format to old sample format
	static void bufferFormatToSampleFormat(TBufferFormat bufferFormat, uint8 channels, uint8 bitsPerSample, TSampleFormat &sampleFormat);
	//@}

	//@{
	//\name Utility functions
	/// Return pcm size in bytes from duration in seconds.
	static uint getPCMSizeFromDuration(float duration, uint8 channels, uint8 bitsPerSample, uint32 frequency);
	/// Return duration in seconds from pcm size in bytes.
	static float getDurationFromPCMSize(uint size, uint8 channels, uint8 bitsPerSample, uint32 frequency);
	//@}
	
	//@{
	//\name ADPCM and sample bank building utility methods
	struct TADPCMState
	{
		/// Previous output sample
		sint16	PreviousSample;
		/// Stepsize table index
		uint8	StepIndex;
	};
	/// Encode 16bit Mono PCM buffer into Mono ADPCM.
	static void encodeADPCM(const sint16 *indata, uint8 *outdata, uint nbSample, TADPCMState &state);
	/// Decode Mono ADPCM into 16bit Mono PCM.
	static void decodeADPCM(const uint8 *indata, sint16 *outdata, uint nbSample, TADPCMState &state);	
	/// Read a wav file. Data type uint8 is used as unspecified buffer format.
	static bool readWav(const uint8 *wav, uint size, std::vector<uint8> &result, TBufferFormat &bufferFormat, uint8 &channels, uint8 &bitsPerSample, uint32 &frequency);
	/// Write a wav file. Data type uint8 does not imply a buffer of any format.
	static bool writeWav(const uint8 *buffer, uint size, TBufferFormat bufferFormat, uint8 channels, uint8 bitsPerSample, uint32 frequency, NLMISC::IStream &out);
	/// Convert buffer data to 16bit Mono PCM. Resulting vector size is in samples.
	static bool convertToMono16PCM(const uint8 *buffer, uint size, std::vector<sint16> &result, TBufferFormat bufferFormat, uint8 channels, uint8 bitsPerSample);
	/// Convert 16bit Mono PCM buffer data to ADPCM.
	static bool convertMono16PCMToMonoADPCM(const sint16 *buffer, uint samples, std::vector<uint8> &result);
	//@}
	
private:
	static const sint _IndexTable[16];
	static const uint _StepsizeTable[89];
	//@}
	
protected:
	/// Constructor
	IBuffer() { }
	
public:
	/// Destructor
	virtual ~IBuffer() { }
	
};

} // NLSOUND

#endif // NL_BUFFER_H

/* End of buffer.h */
