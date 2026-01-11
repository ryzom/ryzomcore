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


#include "stdfmod.h"
#include "buffer_fmod.h"
#include "sound_driver_fmod.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#	include <mmsystem.h>
#endif

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace std;

namespace NLSOUND
{

static const std::string	EmptyString;


// ***************************************************************************
CBufferFMod::CBufferFMod() : _LockedData(NULL)
{
	_Name = CStringMapper::map(EmptyString);
    _Size = 0;
    _Format = Mono16;
    _Freq = 0;
	_FModSample = NULL;
}

// ***************************************************************************
CBufferFMod::~CBufferFMod()
{
    if (_FModSample!=NULL)
    {
		// delete FMod sample
		loadDataToFMod(NULL);
    }
}

// ***************************************************************************
void CBufferFMod::setName(NLMISC::TStringId bufferName)
{
	_Name = bufferName;
}


// ***************************************************************************
/// Set the sample format. (channels = 1, 2, ...; bitsPerSample = 8, 16; frequency = samples per second, 44100, ...)
void CBufferFMod::setFormat(TBufferFormat format, uint8 channels, uint8 bitsPerSample, uint frequency)
{
	bufferFormatToSampleFormat(format, channels, bitsPerSample, _Format);
	_Freq = frequency;
}

/// Get a writable pointer to the buffer of specified size. Returns NULL in case of failure. It is only guaranteed that the original data is still available when using StorageSoftware and the specified size is not larger than the available data. Call setStorageMode() and setFormat() first.
uint8 *CBufferFMod::lock(uint capacity)
{
	// not fully implemented in fmod driver
	nlassert(!_LockedData);
	if (capacity != _Size) return NULL;
	else return (uint8 *)lock();
}

/// Notify that you are done writing to this buffer, so it can be copied over to hardware if needed. Returns true if ok.
bool CBufferFMod::unlock(uint size)
{
	// not fully implemented in fmod driver
	nlassert(_LockedData);
	unlock(_LockedData);
	_LockedData = NULL;
	if (size != _Size) return false;
	else return true;
}

/// Copy the data with specified size into the buffer. A readable local copy is only guaranteed when OptionLocalBufferCopy is set. Returns true if ok.
bool CBufferFMod::fill(const uint8 *src, uint size)
{
	_Size = size;
	loadDataToFMod(src);
	return _FModSample != NULL;
}

/// Return the sample format information.
void CBufferFMod::getFormat(TBufferFormat &format, uint8 &channels, uint8 &bitsPerSample, uint &frequency) const
{
	sampleFormatToBufferFormat(_Format, format, channels, bitsPerSample);
	frequency = _Freq;
}

/// Return the size of the buffer, in bytes.
uint CBufferFMod::getSize() const
{
	return _Size;
}

/// Return the duration (in ms) of the sample in the buffer.
float CBufferFMod::getDuration() const
{
    float frames = (float) _Size;

    switch (_Format)
	{
    case Mono8:
        break;
    case Mono16ADPCM:
        frames *= 2.0f;
        break;
    case Mono16:
        frames /= 2.0f;
        break;
    case Stereo8:
        frames /= 2.0f;
        break;
    case Stereo16:
        frames /= 4.0f;
        break;
    }

    return 1000.0f * frames / (float) _Freq;
}

/// Return true if the buffer is stereo (multi-channel), false if mono.
bool CBufferFMod::isStereo() const
{
	return (_Format == Stereo8) || (_Format == Stereo16);
}

/// Return the name of this buffer
NLMISC::TStringId CBufferFMod::getName() const
{
	return _Name;
}

/// Return true if the buffer is loaded. Used for async load/unload.
bool CBufferFMod::isBufferLoaded() const
{
	return _FModSample != 0;
}

/// Set the storage mode of this buffer, call before filling this buffer. Storage mode is always software if OptionSoftwareBuffer is enabled. Default is auto.
void CBufferFMod::setStorageMode(TStorageMode /* storageMode */)
{
	// not implemented
}

/// Get the storage mode of this buffer.
IBuffer::TStorageMode CBufferFMod::getStorageMode()
{
	// not implemented
	return IBuffer::StorageAuto;
}

// ***************************************************************************
void	CBufferFMod::loadDataToFMod(const uint8 *data)
{
	// Delete old one
	if(_FModSample)
	{
		FSOUND_Sample_Free(_FModSample);
		_FModSample= NULL;
	}

	// if some data, create new one
	if(data)
	{
		uint8 *tmpData = NULL;
		const uint8	*uploadData = data;

		// if format is ADPCM, decode into Mono16
		if(_Format==Mono16ADPCM)
		{
			// create Mono16 dest buffer
			uint	nbSample= _Size*2;
			tmpData= new uint8 [nbSample * sizeof(sint16)];

			// uncompress
			TADPCMState	state;
			state.PreviousSample = 0;
			state.StepIndex = 0;
			decodeADPCM(data, (sint16*)tmpData, nbSample, state);

			// change upload data, _Format and _Size to fit the new format
			uploadData= tmpData;
			_Format= Mono16;
			_Size= nbSample*2;
		}

		// create FMod sample and upload according to format
		uint32	type= 0;
		switch(_Format)
		{
		case Mono8: type= FSOUND_8BITS|FSOUND_MONO; break;
		case Mono16: type= FSOUND_16BITS|FSOUND_MONO; break;
		case Stereo8: type= FSOUND_8BITS|FSOUND_STEREO; break;
		case Stereo16: type= FSOUND_16BITS|FSOUND_STEREO; break;
		default: nlstop;
		};
		uint32	commonType= FSOUND_LOADRAW|FSOUND_LOOP_NORMAL|FSOUND_LOADMEMORY;
		// if can use hardware buffer
		if(!CSoundDriverFMod::getInstance()->forceSofwareBuffer())
			commonType|= FSOUND_HW3D;
		// create FMod sample
		_FModSample= FSOUND_Sample_Load(FSOUND_FREE, (const char*)uploadData, commonType|type, 0, _Size);

		// clear any temporary data
		if(tmpData)
		{
			delete [] tmpData;
		}
	}
}


// ***************************************************************************
void		*CBufferFMod::lock()
{
	if(!_FModSample)
		return NULL;

	void	*ptr0, *ptr1;
	uint	len0, len1;
	if(!FSOUND_Sample_Lock(_FModSample, 0, _Size, &ptr0, &ptr1, &len0, &len1))
		return 0;
	nlassert(ptr1== NULL && len1==0);
	nlassert(len0==_Size);

	return ptr0;
}

// ***************************************************************************
void		CBufferFMod::unlock(void *ptr)
{
	if(!_FModSample || !ptr)
		return;

	FSOUND_Sample_Unlock(_FModSample, ptr, NULL, _Size, 0);
}


} // NLSOUND



