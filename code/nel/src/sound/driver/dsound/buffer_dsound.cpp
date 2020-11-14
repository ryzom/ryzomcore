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


#include "stddsound.h"
#include "buffer_dsound.h"
#include "sound_driver_dsound.h"

#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace std;

namespace NLSOUND {

static const std::string	EmptyString;

// Custom mutimedia IO proc.
/*LRESULT NelIOProc(LPSTR lpmmioinfo, UINT uMsg, LONG lParam1, LONG lParam2)
{
	MMIOINFO *mmioinfo = (MMIOINFO*) lpmmioinfo;

	switch (uMsg)
	{
	case MMIOM_OPEN:
		{
			// do some validity checking.
			nlassert((mmioinfo->dwFlags & MMIO_CREATE) == 0);

			char *fileName = (char*)lParam1;
			std::string fullName = NLMISC::CPath::lookup(fileName, false);
			if (fullName.empty())
			{
				mmioinfo->adwInfo[0] = NULL;
				return MMIOERR_CANNOTOPEN;
			}

			NLMISC::CIFile	*pfile = new NLMISC::CIFile(fullName);

			mmioinfo->adwInfo[0] = (DWORD)pfile;
			return MMSYSERR_NOERROR ;
		}
		break;
	case MMIOM_CLOSE:
		{
			NLMISC::CIFile *file = (NLMISC::CIFile *)mmioinfo->adwInfo[0];
			delete file;
			return 0;
		}
		break;
	case MMIOM_READ:
		{
			uint8 *pdst = (uint8*) lParam1;
			uint  bytes = (uint) lParam2;

			nlassert(mmioinfo->adwInfo[0] != NULL);
			NLMISC::CIFile *file = (NLMISC::CIFile *)mmioinfo->adwInfo[0];
			bytes = std::min(uint(file->getFileSize() - file->getPos()), bytes);
			file->serialBufferWithSize(pdst, bytes);

			mmioinfo->lBufOffset = file->getPos();

			return bytes;
		}
		break;
	case MMIOM_SEEK:
		{
			uint newPos = (uint) lParam1;
			uint seekMode = lParam2;

			nlassert(mmioinfo->adwInfo[0] != NULL);
			NLMISC::CIFile *file = (NLMISC::CIFile *)mmioinfo->adwInfo[0];

			switch(seekMode)
			{
			case SEEK_CUR:
				file->seek(newPos, NLMISC::IStream::current);
				break;
			case SEEK_END:
				file->seek(newPos, NLMISC::IStream::end);
				break;
			case SEEK_SET:
				file->seek(newPos, NLMISC::IStream::begin);
				break;
			}

			mmioinfo->lBufOffset = file->getPos();

			return mmioinfo->lBufOffset;
		}
		break;
	case MMIOM_WRITE:
		nlassert("Mutimedia IO write is not supported !");
		break;
	case MMIOM_WRITEFLUSH:
		nlassert("Mutimedia IO write is not supported !");
		break;
	}
}
*/


CBufferDSound::CBufferDSound() : _Data(NULL), _Capacity(0), _Size(0)
{
	_Name = CStringMapper::map(EmptyString);
    _Format = Mono16;
    _Freq = 0;
}

CBufferDSound::~CBufferDSound()
{
//	nldebug("Destroying DirectSound buffer %s (%p)", CSoundDriverDSound::instance()->getStringMapper()->unmap(_Name).c_str(), this);

	if (_Data)
	{
		delete[] _Data;
		_Data = NULL;
	}
}

void CBufferDSound::setName(NLMISC::TStringId bufferName)
{
	_Name = bufferName;
}

/// Set the sample format. (channels = 1, 2, ...; bitsPerSample = 8, 16; frequency = samples per second, 44100, ...)
void CBufferDSound::setFormat(TBufferFormat format, uint8 channels, uint8 bitsPerSample, uint32 frequency)
{
	bufferFormatToSampleFormat(format, channels, bitsPerSample, _Format);
	_Freq = frequency;
}

/// Get a writable pointer to the buffer of specified size. Returns NULL in case of failure. It is only guaranteed that the original data is still available when using StorageSoftware and the specified size is not larger than the available data. Call setStorageMode() and setFormat() first.
uint8 *CBufferDSound::lock(uint capacity)
{
	if (_Data)
	{
		if (capacity > _Capacity) 
		{
			delete[] _Data;
			_Data = NULL;
		}
	}
	
	if (!_Data) 
	{
		_Data = new uint8[capacity];
		_Capacity = capacity;
		if (_Size > capacity) 
			_Size = capacity;
	}
	
	return _Data;
}

/// Notify that you are done writing to this buffer, so it can be copied over to hardware if needed. Returns true if ok.
bool CBufferDSound::unlock(uint size)
{
	if (size > _Capacity) 
	{
		_Size = _Capacity;
		return false;
	}
	else
	{
		_Size = size;
		return true;
	}
}

/// Copy the data with specified size into the buffer. A readable local copy is only guaranteed when OptionLocalBufferCopy is set. Returns true if ok.
bool CBufferDSound::fill(const uint8 *src, uint size)
{
	uint8 *dest = lock(size);
	if (dest == NULL) return false;
	CFastMem::memcpy(dest, src, size);
	return unlock(size);
}

/// Return the sample format information.
void CBufferDSound::getFormat(TBufferFormat &format, uint8 &channels, uint8 &bitsPerSample, uint32 &frequency) const
{
	sampleFormatToBufferFormat(_Format, format, channels, bitsPerSample);
	frequency = _Freq;
}

/// Return the size of the buffer, in bytes.
uint CBufferDSound::getSize() const
{
	return _Size;
}

float CBufferDSound::getDuration() const
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

bool CBufferDSound::isStereo() const
{
	return (_Format == Stereo8) || (_Format == Stereo16);
}

/// Return the name of this buffer
NLMISC::TStringId CBufferDSound::getName() const
{
	return _Name;
}

/// Return true if the buffer is loaded. Used for async load/unload.
bool CBufferDSound::isBufferLoaded() const
{
	return _Data != NULL;
}

	
/// Set the storage mode of this buffer, call before filling this buffer. Storage mode is always software if OptionSoftwareBuffer is enabled. Default is auto.
void CBufferDSound::setStorageMode(TStorageMode /* storageMode */)
{
	// software buffering, no hardware storage mode available
}

/// Get the storage mode of this buffer.
IBuffer::TStorageMode CBufferDSound::getStorageMode()
{
	// always uses software buffers
	return IBuffer::StorageSoftware;
}

} // NLSOUND



