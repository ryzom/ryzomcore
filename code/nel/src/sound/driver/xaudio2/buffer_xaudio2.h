// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2008  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NLSOUND_BUFFER_XAUDIO2_H
#define NLSOUND_BUFFER_XAUDIO2_H

#include "nel/sound/driver/buffer.h"

namespace NLSOUND {
	class CSoundDriverXAudio2;

/**
 * \brief CBufferXAudio2
 * \date 2008-08-20 17:21GMT
 * \author Jan Boon (Kaetemi)
 * CBufferXAudio2 is an implementation of the IBuffer interface to run on XAudio2.
 */
class CBufferXAudio2 : public IBuffer
{
protected:
	// outside pointers
	/// The sound driver that owns this buffer, used for stats. (artificial limit)
	CSoundDriverXAudio2 *_SoundDriver;
	
	// pointers
	/// The sample data in this buffer. Aligned 16 byte.
	uint8 *_DataAligned;
	/// The actual pointer used for deletion.
	uint8 *_DataPtr;
	// instances
	// XAudio2 buffer structure, could have one pre-configured 
	// here for optimization (looping state unknown).
	// XAUDIO2_BUFFER _Buffer;

	/// The capacity of the buffer
	uint _Capacity;
	/// The size of the data in the buffer
	uint _Size;
	/// The name of the buffer
	NLMISC::TStringId _Name;
	/// The sample format
	TBufferFormat _Format;
	/// The number of channels	
	uint8 _Channels;
	/// Bits per sample
	uint8 _BitsPerSample;
	/// The sample frequency
	uint _Frequency;
public:
	CBufferXAudio2(CSoundDriverXAudio2 *soundDriver);
	virtual ~CBufferXAudio2();
	void release();

	/// Returns a pointer to the PCM or ADPCM bytes.
	inline const uint8 *getData() { return _DataAligned; }
	/// Returns the sample format.
	inline TBufferFormat getFormat() { return _Format; }
	/// Returns the number of channels
	inline uint8 getChannels() { return _Channels; }
	/// Returns the bits per sample
	inline uint8 getBitsPerSample() { return _BitsPerSample; }
	/// Returns the sample rate.
	inline uint getFrequency() { return _Frequency; }
	
	/** Preset the name of the buffer. Used for async loading to give a name
	 *	before the buffer is effectivly loaded.
	 *	If the name after loading of the buffer doesn't match the preset name,
	 *	the load will assert.
	 */
	virtual void setName(NLMISC::TStringId bufferName);
	/// Return the name of this buffer
	virtual NLMISC::TStringId getName() const;

	/// Set the sample format. (channels = 1, 2, ...; bitsPerSample = 8, 16; frequency = samples per second, 44100, ...)
	virtual void setFormat(TBufferFormat format, uint8 channels, uint8 bitsPerSample, uint32 frequency);
	/// Return the sample format information.
	virtual void getFormat(TBufferFormat &format, uint8 &channels, uint8 &bitsPerSample, uint32 &frequency) const;
		/// Set the storage mode of this buffer, call before filling this buffer. Storage mode is always software if OptionSoftwareBuffer is enabled. Default is auto.
	virtual void setStorageMode(TStorageMode storageMode = IBuffer::StorageAuto);
	/// Get the storage mode of this buffer.
	virtual TStorageMode getStorageMode();

	/// Get a writable pointer to the buffer of specified size. Use capacity to specify the required bytes. Returns NULL in case of failure. It is only guaranteed that the original data is still available when using StorageSoftware and the specified size is not larger than the size specified in the last lock. Call setStorageMode() and setFormat() first.
	virtual uint8 *lock(uint capacity);
	/// Notify that you are done writing to this buffer, so it can be copied over to hardware if needed. Set size to the number of bytes actually written to the buffer. Returns true if ok.
	virtual bool unlock(uint size);
	/// Copy the data with specified size into the buffer. A readable local copy is only guaranteed when OptionLocalBufferCopy is set. Returns true if ok.
	virtual bool fill(const uint8 *src, uint size);
	
	/// Return the size of the buffer, in bytes.
	virtual uint getSize() const;
	/// Return the duration (in ms) of the sample in the buffer.
	virtual float getDuration() const;
	/// Return true if the buffer is stereo (multi-channel), false if mono.
	virtual bool isStereo() const;	
	/// Return true if the buffer is loaded. Used for async load/unload.
	virtual bool isBufferLoaded() const;
	
}; /* class CBufferXAudio2 */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_BUFFER_XAUDIO2_H */

/* end of file */
