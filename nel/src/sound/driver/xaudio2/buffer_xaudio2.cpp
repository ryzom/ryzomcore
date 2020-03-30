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

#include "stdxaudio2.h"

// Project includes
#include "buffer_xaudio2.h"
#include "sound_driver_xaudio2.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

CBufferXAudio2::CBufferXAudio2(CSoundDriverXAudio2 *soundDriver) 
: _SoundDriver(soundDriver), _DataAligned(NULL), _DataPtr(NULL), 
_Size(0), _Name(NULL), _Format((IBuffer::TBufferFormat)~0),
_Channels(0), _BitsPerSample(0), _Frequency(0), _Capacity(0)
{
	
}

CBufferXAudio2::~CBufferXAudio2()
{
	release();
}

void CBufferXAudio2::release()
{
	if (_SoundDriver)
	{
		// Remove the buffer from the driver.
		_SoundDriver->removeBuffer(this);
		// Update stats.
		_SoundDriver->performanceUnregisterBuffer(_Format, _Size);
		_SoundDriver = NULL;
	}
	// Release possible _Data
	if (_DataPtr)
	{
		delete[] _DataPtr;
		_DataAligned = NULL;
		_DataPtr = NULL;
	}
}

/// Get a writable pointer to the buffer of specified size. Returns NULL in case of failure. It is only guaranteed that the original data is still available when using StorageSoftware and the specified size is not larger than the available data. Call setStorageMode() and setFormat() first.
uint8 *CBufferXAudio2::lock(uint capacity)
{
	if (_DataPtr)
	{
		_SoundDriver->performanceUnregisterBuffer(_Format, _Size);
		if (capacity > _Capacity) 
		{
			delete[] _DataPtr;
			_DataAligned = NULL;
			_DataPtr = NULL;
		}
	}
	
	if (!_DataPtr) 
	{
		_DataPtr = new uint8[capacity + 15];
		_DataAligned = (uint8 *)((size_t)_DataPtr + ((16 - ((size_t)_DataPtr % 16)) % 16));
		_Capacity = capacity;
		if (_Size > capacity) 
			_Size = capacity;
	}
	_SoundDriver->performanceRegisterBuffer(_Format, _Size);
	
	return _DataAligned;
}

/// Notify that you are done writing to this buffer, so it can be copied over to hardware if needed. Returns true if ok.
bool CBufferXAudio2::unlock(uint size)
{
	_SoundDriver->performanceUnregisterBuffer(_Format, _Size);
	if (size > _Capacity) 
	{
		_Size = _Capacity;
		_SoundDriver->performanceRegisterBuffer(_Format, _Size);
		return false;
	}
	else
	{
		_Size = size;
		_SoundDriver->performanceRegisterBuffer(_Format, _Size);
		return true;
	}
}

/// Copy the data with specified size into the buffer. A readable local copy is only guaranteed when OptionLocalBufferCopy is set. Returns true if ok.
bool CBufferXAudio2::fill(const uint8 *src, uint size)
{
	uint8 *dest = lock(size);
	if (dest == NULL) return false;
	CFastMem::memcpy(dest, src, size);
	return unlock(size);
}

/** Preset the name of the buffer. Used for async loading to give a name
 *	before the buffer is effectivly loaded.
 *	If the name after loading of the buffer doesn't match the preset name,
 *	the load will assert.
 */
void CBufferXAudio2::setName(NLMISC::TStringId bufferName)
{
	_Name = bufferName;
}

void CBufferXAudio2::setFormat(TBufferFormat format, uint8 channels, uint8 bitsPerSample, uint32 frequency)
{
	_Format = format;
	_Channels = channels;
	_Frequency = frequency;
	_BitsPerSample = bitsPerSample;
}

/// Return the sample format information.
void CBufferXAudio2::getFormat(TBufferFormat &format, uint8 &channels, uint8 &bitsPerSample, uint32 &frequency) const
{
	format = _Format;
	channels = _Channels;
	bitsPerSample = _BitsPerSample;
	frequency = _Frequency;
}

/// Return the size of the buffer, in bytes
uint CBufferXAudio2::getSize() const
{
	return _Size;
}

/// Return the duration (in ms) of the sample in the buffer
float CBufferXAudio2::getDuration() const
{
	switch (_Format) 
	{
	case FormatDviAdpcm:
		nlassert(_Channels == 1 && _BitsPerSample == 16);
		return 1000.0f * ((float)_Size * 2.0f) / (float)_Frequency;
		break;
	case FormatPcm:
		return 1000.0f * getDurationFromPCMSize(_Size, _Channels, _BitsPerSample, _Frequency);
		break;
	}
	return 0.0f;
}

/// Return true if the buffer is stereo, false if mono
bool CBufferXAudio2::isStereo() const
{
	return _Channels > 1;
}

/// Return the name of this buffer
NLMISC::TStringId CBufferXAudio2::getName() const
{
	return _Name;
}

/// Return true if the buffer is loaded. Used for async load/unload.
bool CBufferXAudio2::isBufferLoaded() const
{
	return _DataPtr != NULL;
}
	
/// Set the storage mode of this buffer, call before filling this buffer. Storage mode is always software if OptionSoftwareBuffer is enabled. Default is auto.
void CBufferXAudio2::setStorageMode(IBuffer::TStorageMode /* storageMode */)
{
	// software buffering, no hardware storage mode available
}

/// Get the storage mode of this buffer.
IBuffer::TStorageMode CBufferXAudio2::getStorageMode()
{
	// always uses software buffers
	return IBuffer::StorageSoftware;
}

} /* namespace NLSOUND */

/* end of file */
