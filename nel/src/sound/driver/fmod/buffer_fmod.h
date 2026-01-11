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

#ifndef NL_BUFFER_FMOD_H
#define NL_BUFFER_FMOD_H


#include "nel/sound/driver/buffer.h"


namespace NLSOUND {

/**
 *  Buffer for the FMod implementation of the audio driver.
 *
 * A buffer represents a sound file loaded in RAM.
 *
 * \author Peter Hanappe, Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CBufferFMod : public IBuffer
{
	friend class CSourceFMod;
	friend class CSoundDriverFMod;

private:
	/// The name of the buffer
	NLMISC::TStringId _Name;
	/// The sample format
    TSampleFormat _Format;
	/// Locked data
	void *_LockedData;
	/// The sample frequency
    uint _Freq;
	
	/// The sample data in this buffer.
	FSOUND_SAMPLE *_FModSample;
	/// The size of the buffer
	uint _Size;
	
public:
	/// Constructor
	CBufferFMod();
	/// Destructor
	virtual	~CBufferFMod();
	
private:
	/// (Internal) Read the audio data from a WAV format buffer.
	bool readWavBuffer(const std::string &name, uint8 *wavData, uint dataSize);
	/// (Internal) Read the audio data from a raw buffer.
	bool readRawBuffer(const std::string &name, uint8 *rawData, uint dataSize, TSampleFormat sampleFormat, uint32 frequency);
	
	void loadDataToFMod(const uint8 *data);
	
	void *lock();
	void unlock(void *ptr);
	
public:	
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
	
};


} // NLSOUND


#endif // NL_BUFFER_FMOD_H

