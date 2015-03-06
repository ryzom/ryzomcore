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

#include "stdopenal.h"
#include "sound_driver_al.h"
#include "buffer_al.h"

using namespace NLMISC;

namespace NLSOUND
{

CBufferAL::CBufferAL(ALuint buffername) :
	IBuffer(), _BufferName(buffername), _Name(NULL), _SampleFormat(AL_INVALID), _Frequency(0),
	_DataAligned(NULL), _DataPtr(NULL), _Capacity(0), _Size(0), _StorageMode(IBuffer::StorageAuto), _IsLoaded(false)
{
	
}

CBufferAL::~CBufferAL()
{
	// delete local copy
	if (_DataPtr != NULL)
	{
		delete[] _DataPtr;
		_DataAligned = NULL;
		_DataPtr = NULL;
	}

	// delete OpenAL copy
	CSoundDriverAL *sdal = CSoundDriverAL::getInstance();
	//nlinfo("AL: Deleting buffer (name %u)", _BufferName );
	sdal->removeBuffer( this );
}

/** Preset the name of the buffer. Used for async loading to give a name
 *	before the buffer is effectivly loaded.
 *	If the name after loading of the buffer doesn't match the preset name,
 *	the load will assert.
 */
void CBufferAL::setName(NLMISC::TStringId bufferName)
{
	_Name = bufferName;
}

/// Set the sample format. (channels = 1, 2, ...; bitsPerSample = 8, 16; frequency = samples per second, 44100, ...)
void CBufferAL::setFormat(TBufferFormat format, uint8 channels, uint8 bitsPerSample, uint32 frequency)
{
	TSampleFormat sampleFormat;
	bufferFormatToSampleFormat(format, channels, bitsPerSample, sampleFormat);
	switch (sampleFormat) 
	{
		case Mono8: _SampleFormat = AL_FORMAT_MONO8; break;
		case Mono16: _SampleFormat = AL_FORMAT_MONO16; break;
		case Stereo8: _SampleFormat = AL_FORMAT_STEREO8; break;
		case Stereo16: _SampleFormat = AL_FORMAT_STEREO16; break;
		default: nlstop; _SampleFormat = AL_INVALID;
	}
	_Frequency = frequency;
}

/// Get a writable pointer to the buffer of specified size. Returns NULL in case of failure. It is only guaranteed that the original data is still available when using StorageSoftware and the specified size is not larger than the available data. Call setStorageMode() and setFormat() first.
uint8 *CBufferAL::lock(uint capacity)
{
	nlassert((_SampleFormat != AL_INVALID) && (_Frequency != 0));

	_IsLoaded = false;

	if (_DataPtr)
	{
		if (capacity > _Capacity) 
		{
			delete[] _DataPtr;
			_DataAligned = NULL;
			_DataPtr = NULL;
		}
	}
	
	if (!_DataPtr) _DataPtr = new uint8[capacity + 15];
	_DataAligned = (uint8 *)((size_t)_DataPtr + ((16 - ((size_t)_DataPtr % 16)) % 16));
	if (_Size > capacity) _Size = capacity;
	_Capacity = capacity;
	
	return _DataAligned;
}

/// Notify that you are done writing to this buffer, so it can be copied over to hardware if needed. Returns true if ok.
bool CBufferAL::unlock(uint size)
{
	if (size > _Capacity) 
	{
		_Size = _Capacity;
		return false;
	}
	
	// Fill buffer (OpenAL one)
	_Size = size;
	alBufferData(_BufferName, _SampleFormat, _DataAligned, _Size, _Frequency);
	
	if (_StorageMode != IBuffer::StorageSoftware && !CSoundDriverAL::getInstance()->getOption(ISoundDriver::OptionLocalBufferCopy))
	{
		delete[] _DataPtr;
		_DataAligned = NULL;
		_DataPtr = NULL;
		_Capacity = 0;
	}

	// Error handling
	if (alGetError() == AL_NO_ERROR)
		_IsLoaded = true; // ->lock() set it to false

	return _IsLoaded;
}

/// Copy the data with specified size into the buffer. A readable local copy is only guaranteed when OptionLocalBufferCopy is set. Returns true if ok.
bool CBufferAL::fill(const uint8 *src, uint size)
{
	nlassert((_SampleFormat != AL_INVALID) && (_Frequency != 0));

	bool localBufferCopy = CSoundDriverAL::getInstance()->getOption(ISoundDriver::OptionLocalBufferCopy);

	if (_DataPtr)
	{
		if ((!localBufferCopy) || (size > _Capacity)) 
		{
			delete[] _DataPtr;
			_DataAligned = NULL;
			_DataPtr = NULL;
			_Capacity = 0;
		}
	}
	
	_Size = size;
	
	if (localBufferCopy)
	{
		// Force a local copy of the buffer
		if (!_DataPtr) 
		{
			_DataPtr = new uint8[size + 15];
			_DataAligned = (uint8 *)((size_t)_DataPtr + ((16 - ((size_t)_DataPtr % 16)) % 16));
			_Capacity = size;
		}
		CFastMem::memcpy(_DataAligned, src, size);
	}
	
	// Fill buffer (OpenAL one)
	alBufferData(_BufferName, _SampleFormat, src, size, _Frequency);

	// Error handling
	_IsLoaded = (alGetError() == AL_NO_ERROR);

	return _IsLoaded;
}

/// Return the sample format information.
void CBufferAL::getFormat(TBufferFormat &format, uint8 &channels, uint8 &bitsPerSample, uint32 &frequency) const
{
	TSampleFormat sampleFormat;
	switch (_SampleFormat)
	{
		case AL_FORMAT_MONO8: sampleFormat = Mono8; break;
		case AL_FORMAT_MONO16: sampleFormat = Mono16; break;
		case AL_FORMAT_STEREO8: sampleFormat = Stereo8; break;
		case AL_FORMAT_STEREO16: sampleFormat = Stereo16; break;
		default: sampleFormat = SampleFormatUnknown;
	}
	sampleFormatToBufferFormat(sampleFormat, format, channels, bitsPerSample);
	frequency = _Frequency;
}


/*
 * Return the size of the buffer, in bytes
 */
uint32 CBufferAL::getSize() const
{
	return _Size;
	/*ALint value;
	alGetBufferi(_BufferName, AL_SIZE, &value);
	nlassert(alGetError() == AL_NO_ERROR);
	return value;*/
}

/*
 * Return the duration (in ms) of the sample in the buffer
 */
float CBufferAL::getDuration() const
{
	if ( _Frequency == 0 )
		return 0;

	uint32 bytespersample;
	switch ( _SampleFormat ) {
		case AL_FORMAT_MONO8:
			bytespersample = 1;
			break;

		case AL_FORMAT_MONO16:
		case AL_FORMAT_STEREO8:
			bytespersample = 2;
			break;

		case AL_FORMAT_STEREO16:
			bytespersample = 4;
			break;

		default:
			return 0;
	}

	return (float)(getSize()) * 1000.0f / (float)_Frequency / (float)bytespersample;
}

/*
 * Return true if the buffer is stereo, false if mono
 */
bool CBufferAL::isStereo() const
{
	return (_SampleFormat==AL_FORMAT_STEREO8) || (_SampleFormat==AL_FORMAT_STEREO16);
}

/// Return the name of this buffer
NLMISC::TStringId CBufferAL::getName() const
{
	return _Name;
}

bool CBufferAL::isBufferLoaded() const
{
	return _IsLoaded;
}

	
/// Set the storage mode of this buffer, call before filling this buffer. Storage mode is always software if OptionSoftwareBuffer is enabled. Default is auto.
void CBufferAL::setStorageMode(TStorageMode storageMode)
{
	_StorageMode = storageMode;
}

/// Get the storage mode of this buffer.
IBuffer::TStorageMode CBufferAL::getStorageMode()
{
	return _StorageMode;
}

} // NLSOUND
