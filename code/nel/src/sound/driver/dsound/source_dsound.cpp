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
#include "source_dsound.h"
#include "sound_driver_dsound.h"
#include "buffer_dsound.h"
#include "listener_dsound.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace std;


namespace NLSOUND {


#if NLSOUND_PROFILE

	#define INITTIME(_var)	 TTicks _var = CTime::getPerformanceTime()

	#define DEBUG_POSITIONS  1

	#if DEBUG_POSITIONS
	#define DBGPOS(_a) nldebug ## _a
	#else
	#define DBGPOS(_a)
	#endif

#else
	#define INITTIME(_var)
	#define DBGPOS(_a)
#endif



const uint32 CSourceDSound::_SecondaryBufferSize = 0x10000;
const uint32 CSourceDSound::_SizeMask = 0xffff;
const uint32 CSourceDSound::_SwapCopySize = 0x8000;
const uint32 CSourceDSound::_UpdateCopySize = 0x4000;
const uint32 CSourceDSound::_XFadeSize = 64;
const uint CSourceDSound::_DefaultChannels = 1;
const uint CSourceDSound::_DefaultSampleRate = 22050;
const uint CSourceDSound::_DefaultSampleSize = 16;



#define NLSOUND_MIN(_a,_b)	  (((_a) < (_b)) ? (_a) : (_b))
#define NLSOUND_DISTANCE(_from, _to, _period)	(((_to) > (_from)) ? (_to) - (_from) : (_period) + (_to) - (_from))


#if NLSOUND_PROFILE

// Static variables used for profiling
double CSourceDSound::_LastSwapTime = 0.0;
double CSourceDSound::_TotalSwapTime = 0.0;
double CSourceDSound::_MaxSwapTime = 0.0;
double CSourceDSound::_MinSwapTime = 1000000.0;
uint32 CSourceDSound::_SwapCount = 0;
double CSourceDSound::_PosTime = 0.0;
double CSourceDSound::_LockTime = 0.0;
double CSourceDSound::_CopyTime = 0.0;
double CSourceDSound::_UnlockTime = 0.0;
uint32 CSourceDSound::_CopyCount = 0;
double CSourceDSound::_TotalUpdateTime = 0.0;
double CSourceDSound::_MaxUpdateTime = 0.0;
double CSourceDSound::_MinUpdateTime = 1000000.0;
uint32 CSourceDSound::_UpdateCount = 0;
uint32 CSourceDSound::_TotalUpdateSize = 0;
#endif


uint32 getWritePosAndSpace(uint32 &nextWritePos, uint32 playPos, uint32 writePos, uint32 bufferSize);


// ******************************************************************

CSourceDSound::CSourceDSound( uint sourcename )
:	ISource(),
	_SourceName(sourcename)
{
#if EAX_AVAILABLE == 1
	_EAXSource = 0;
#endif
	_Sample = 0;
	_SampleSize = 0;
	_SampleOffset = 0;
	_Format = Mono8;
	_SampleFreq = _DefaultSampleRate;
	_FillOffset = 0;
	_State = source_stopped;
	_PlayOffset = 0;
	_LastPlayPos = 0;
	_PosRelative= false;

//	_BufferSize = 0;
//	_SwapBuffer = 0;
	_SecondaryBuffer = 0;
//	_SecondaryBufferState = NL_DSOUND_SILENCED;
	_3DBuffer = 0;
//	_NextWritePos = 0;
//	_BytesWritten = 0;
//	_SilenceWritten = 0;
	_Loop = false;
//	_EndPosition = 0;
//	_EndState = NL_DSOUND_TAIL1;
//	_UserState = NL_DSOUND_STOPPED;
	_Freq = 1.0f;
	_SampleRate = _DefaultSampleRate;
//	_IsUsed = false;
	_Gain = 1.0f;
	_Volume = 0;
	_Alpha = 0.0;
	InitializeCriticalSection(&_CriticalSection);
}


// ******************************************************************

CSourceDSound::~CSourceDSound()
{
	nldebug("Destroying DirectSound source");

	CSoundDriverDSound::instance()->removeSource(this);

	EnterCriticalSection(&_CriticalSection);

	// Release the DirectSound buffer within the critical zone
	// to avoid a call to update during deconstruction
	release();

	LeaveCriticalSection(&_CriticalSection);
	DeleteCriticalSection(&_CriticalSection);
}


// ******************************************************************

void CSourceDSound::release()
{
//	_Buffer = 0;

#if EAX_AVAILABLE == 1
	if (_EAXSource != 0)
	{
		_EAXSource->Release();
		_EAXSource = 0;
	}
#endif

	if (_SecondaryBuffer != 0)
	{
		_SecondaryBuffer->Stop();
	}

	if (_3DBuffer != 0)
	{
		_3DBuffer->Release();
		_3DBuffer = 0;
	}

	if (_SecondaryBuffer != 0)
	{
		_SecondaryBuffer->Release();
		_SecondaryBuffer = 0;
	}

}


uint32	CSourceDSound::getTime()
{
	if (_Sample == 0)
		return 0;

	TSampleFormat format;
	uint freq;

	_Sample->getFormat(format, freq);

	return uint32(1000.0f * (_PlayOffset+1) / (float)freq);
}

// ******************************************************************

void CSourceDSound::init(LPDIRECTSOUND directSound, bool useEax)
{

	// Initialize the buffer format
	WAVEFORMATEX format;

	format.cbSize = sizeof(WAVEFORMATEX);
	format.nChannels = _DefaultChannels;
	format.wBitsPerSample = _DefaultSampleSize;
	format.nSamplesPerSec = _DefaultSampleRate;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.wFormatTag = WAVE_FORMAT_PCM;


	// Initialize the buffer description

	DSBUFFERDESC desc;

	CSoundDriverDSound* driver = CSoundDriverDSound::instance();


	ZeroMemory(&desc, sizeof(DSBUFFERDESC));
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.lpwfxFormat = &format;
	desc.dwBufferBytes = _SecondaryBufferSize;
	desc.dwReserved = 0;

	if (driver->countHw3DBuffers() > 0)
	{
		//nldebug("Source: Allocating 3D buffer in hardware");
		desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCHARDWARE | DSBCAPS_GETCURRENTPOSITION2
						| DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_MUTE3DATMAXDISTANCE;
	}
	else
	{
		nldebug("Failed to create a 3D Hardware DirectX secondary buffer. Try 3D software one");

		if (useEax)
		{
			throw ESoundDriver("No 3d hardware sound buffer, but EAX support requested");
		}
		//nldebug("Source: Allocating 3D buffer in software");
		desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2
						| DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_MUTE3DATMAXDISTANCE;
		desc.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
		//desc.guid3DAlgorithm = DS3DALG_HRTF_FULL;
	}


	// Allocate the secondary buffer

	if (FAILED(directSound->CreateSoundBuffer(&desc, &_SecondaryBuffer, NULL)))
	{
		if (useEax)
		{
			throw ESoundDriver("Failed to create a 3d hardware sound buffer, but EAX support requested");
		}
		nlwarning("Source: Failed to create a buffer with 3D capabilities.");

		ZeroMemory(&desc, sizeof(DSBUFFERDESC));
		desc.dwSize = sizeof(DSBUFFERDESC);
		desc.lpwfxFormat = &format;
		desc.dwBufferBytes = _SecondaryBufferSize;
		desc.dwReserved = 0;

		if (driver->countHw2DBuffers() > 0)
		{
			//nldebug("Source: Allocating 2D buffer in hardware");
			desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCHARDWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
		}
		else
		{
			//nldebug("Source: Allocating 2D buffer in software");
			desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
		}

		if (FAILED(directSound->CreateSoundBuffer(&desc, &_SecondaryBuffer, NULL)))
		{
			throw ESoundDriver("Failed to allocate the DirectSound secondary buffer");
		}
	}


	nldebug("Created DirectX secondary buffer @ %p", _SecondaryBuffer);

	// Fill the buffer with silence
	LPVOID ptr;
	DWORD bytes;

	if (FAILED(_SecondaryBuffer->Lock(0, 0, &ptr, &bytes, NULL, NULL, DSBLOCK_ENTIREBUFFER)))
	{
		throw ESoundDriver("Failed to lock the DirectSound secondary buffer");
	}

	memset(ptr, 0, bytes);

	_SecondaryBuffer->Unlock(ptr, bytes, 0, 0);

	// Allocate the 3D interface, if necessary

	if (FAILED(_SecondaryBuffer->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID *) &_3DBuffer)))
	{
	   throw ESoundDriver("Failed to allocate the DirectSound 3D buffer");
	}


	if (FAILED(_SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING)))
	{
		throw ESoundDriver("Play failed");
	}

}

// ******************************************************************

void CSourceDSound::reset()
{
	setPitch(1.0f);
	setLooping(false);
	setGain(1.0f);
}

/// Enable or disable streaming mode. Source must be stopped to call this.
void CSourceDSound::setStreaming(bool streaming)
{
	if (streaming) throw ESoundDriverNoBufferStreaming();
}

// ******************************************************************

void CSourceDSound::setStaticBuffer( IBuffer *buffer )
{
	EnterCriticalSection(&_CriticalSection);

	if (_State == source_playing)
	{
		_State = source_swap_pending;
		_Sample = 0;
		_NextSample = buffer;
		_SampleOffset = 0;
		_PlayOffset = 0;
	}
	else
	{
		_Sample = buffer;
		_NextSample = 0;
		_SampleOffset = 0;
		_PlayOffset = 0;
		_ADPCMState.PreviousSample = 0;
		_ADPCMState.StepIndex = 0;
		if (buffer)
		{
//			_SampleSize = buffer->getSize();
			buffer->getFormat(_Format, _SampleFreq);
			switch(_Format)
			{
			case Mono8:
				_SampleSize = buffer->getSize();
				break;
			case Mono16:
				_SampleSize = buffer->getSize() / 2;
				break;
			case Mono16ADPCM:
				_SampleSize = buffer->getSize() * 2;
				break;
			case Stereo8:
				_SampleSize = buffer->getSize() / 2;
				break;
			case Stereo16:
				_SampleSize = buffer->getSize() / 4;
				break;
			}
		}
	}

/*
	// If the user calls setStaticBuffer with a null buffer,
	// stop the currently playing buffer and set it to null.
	// Otherwise, store the buffer in the swap buffer variable.
	// A crossfade between the current buffer and the swap buffer
	// will be done when the user calls play.
	if (buffer == 0)
	{
		stop();
		_Buffer = 0;
		_BufferSize = 0;
		_BytesWritten = 0;
	}

	_SwapBuffer = _Buffer = buffer;

	_ADPCMState.PreviousSample = 0;
	_ADPCMState.StepIndex = 0;
*/
	LeaveCriticalSection(&_CriticalSection);
}

IBuffer *CSourceDSound::getStaticBuffer()
{
	if (_State == source_swap_pending)
		return _NextSample;
	else
		return _Sample;

}

/// Add a buffer to the streaming queue.  A buffer of 100ms length is optimal for streaming.
/// Should be called by a thread which checks countStreamingBuffers every 100ms.
void CSourceDSound::submitStreamingBuffer(IBuffer * /* buffer */)
{
	throw ESoundDriverNoBufferStreaming();
}

/// Return the amount of buffers in the queue (playing and waiting). 3 buffers is optimal.
uint CSourceDSound::countStreamingBuffers() const
{
	throw ESoundDriverNoBufferStreaming();
}

void CSourceDSound::getCursors(TCursors &cursors)
{
	_SecondaryBuffer->GetCurrentPosition((DWORD*)&cursors.PlayCursor, (DWORD*)&cursors.WriteCursor);
	// add a security margin to the write cursor
/*	cursors.WriteCursor += _UpdateCopySize;
	if (cursors.WriteCursor > _SecondaryBufferSize)
		cursors.WriteCursor -= _SecondaryBufferSize;
*/
	// compute the available write size
//	cursors.WriteSize = std::min(_UpdateCopySize, cursors.PlayCursor + _SecondaryBufferSize - cursors.WriteCursor);
	cursors.WriteSize = std::min(_UpdateCopySize, (cursors.PlayCursor - cursors.WriteCursor) & _SizeMask);
}

void CSourceDSound::fillData(sint16 *dst, uint nbSample)
{
	nlassert((nbSample & 0xfffffffe) == nbSample);
	if (_Sample != 0)
	{
		const void *data = ((CBufferDSound*) _Sample)->getData();
		const sint8 *data8;
		const uint8 *dataAdpcm;
		const sint16 *data16;
		uint	i;

//nldebug("Filling from %p to %p (%p sample, %p bytes)", dst, dst+nbSample, nbSample, nbSample*2);


		switch(_Format)
		{
		case Mono8:
			data8 = (sint8*) data;
			data8 += _SampleOffset;
//	nldebug(" with data (Mono8) from %p to %p (sample : base = %p, size = %p)", data8, data8+nbSample, data, _SampleSize);
			for (i=0; i<nbSample; ++i)
			{
				dst[i] = sint16(data8[i])*256;
			}
			_SampleOffset += nbSample;
			break;
		case Mono16ADPCM:
			dataAdpcm = (uint8*) data;
			dataAdpcm += _SampleOffset/2;
//nldebug("Filling ADPCM : %p => %p with %p to %p", dst, dst+nbSample, dataAdpcm - (uint8*)data, dataAdpcm + (nbSample/2)-(uint8*)data);
//nldebug(" with data (Mono16ADPCM) from %p to %p (sample : base = %p, size = %p)", dataAdpcm, dataAdpcm+(nbSample/2), data, _SampleSize*2);
			IBuffer::decodeADPCM(dataAdpcm, dst, nbSample, _ADPCMState);
			_SampleOffset += nbSample;
			break;
		case Mono16:
			data16 = (sint16*)data;
			data16 += _SampleOffset;
//nldebug("Filling Mono16 : %p => %p with %p to %p", dst, dst+nbSample, data16 - (sint16*)data, data16 + (nbSample)-(sint16*)data);
//	nldebug(" with data (Mono16) from %p to %p (sample : base = %p, size = %p)", data16, data16+nbSample, data, _SampleSize/2);
			CFastMem::memcpy(dst, data16, nbSample*2);
			_SampleOffset += nbSample;
			break;
		case Stereo8:
			data8 = (sint8*) data;
			data8 += _SampleOffset*2;
			for (i=0; i<nbSample; ++i)
			{
				dst[i] = (sint16(data8[i*2])*128) + (sint16(data8[i*2+1])*128);
			}
			_SampleOffset += nbSample*2;
			break;
		case Stereo16:
			data16 = (sint16*) data;
			data16 += _SampleOffset*2;
			for (i=0; i<nbSample; ++i)
			{
				dst[i] = (data16[i*2]>>1) + (data16[i*2+1]>>1);
			}
			_SampleOffset += nbSample*2;
			break;
		}

/*		if (_SampleOffset == _SampleSize)
		{
			_SampleOffset = 0;
			_ADPCMState.PreviousSample = 0;
			_ADPCMState.StepIndex = 0;
		}
*/

		nlassert(_SampleOffset <= _SampleSize);
	}
	else
	{
	nldebug("Filling : NO DATA from %p to %p (%p sample, %p bytes)", dst, dst+nbSample, nbSample, nbSample*2);

		// write silence in the dst.
		while (nbSample)
		{
			*dst++ = 0;
			--nbSample;
		}
	}
}

void CSourceDSound::fillData(const TLockedBufferInfo &lbi, int nbSample)
{
	nlassert((nbSample & 0x1) == 0);
/*	nlassert(lbi.Size1 != 0);
	nlassert(lbi.Ptr1 != NULL);
*/	uint size = std::min(uint32(nbSample), lbi.Size1>>1);
	fillData(lbi.Ptr1, size);
	nbSample -= size;

	if (nbSample)
	{
/*		nlassert(lbi.Size2 != 0);
		nlassert(lbi.Ptr2 != NULL);
*/		size = min(uint32(nbSample), lbi.Size2>>1);
		fillData(lbi.Ptr2, size);
		nbSample -= size;
	}
	nlassert(nbSample == 0);
}

void CSourceDSound::fillSilence(const TLockedBufferInfo &lbi, int nbSample)
{
	uint size = min(uint32(nbSample), lbi.Size1>>1);
	uint tmp = size;
	sint16	*ptr = lbi.Ptr1;
//	nldebug("Silencing from %p to %p (%p sample, %p bytes)", ptr, ptr+size, size, size*2);

	for (; size != 0; --size)
		*ptr++ = 0;
	nbSample -= tmp;

	if (nbSample)
	{
		size = std::min(uint32(nbSample), lbi.Size2>>1);
		tmp = size;
		ptr = lbi.Ptr2;
//	nldebug("Silencing from %p to %p (%p sample, %p bytes)", ptr, ptr+size, size, size*2);
		for (; size != 0; --size)
			*ptr++ = 0;
		nbSample -= tmp;
	}
	nlassert(nbSample == 0);

}

void CSourceDSound::xfade(const TLockedBufferInfo &lbi, sint16 *src)
{
	// do the XFade in integer fixed point arithmetic

	nlassert((_XFadeSize & 0x1) == 0);
	uint	fade = _XFadeSize;
	sint16	*ptr = lbi.Ptr1;
	uint	count = lbi.Size1 /2;
	sint	alpha, invAlpha;

	while (fade && count)
	{
		alpha = (fade<<16) / _XFadeSize;
		invAlpha = 0x10000 - alpha;
		*ptr = (sint(*ptr)*alpha + sint(*src) * invAlpha) >> 16;
		++src;
		++ptr;
		--count;
		--fade;
	}

	ptr = lbi.Ptr2;
	count = lbi.Size2 /2;

	while (fade && count)
	{
		alpha = (fade<<16) / _XFadeSize;
		invAlpha = 0x10000 - alpha;
		*ptr = (sint(*ptr)*alpha + sint(*src) * invAlpha) >> 16;
		++src;
		++ptr;
		--count;
		--fade;
	}
}

void CSourceDSound::fadeOut(const TLockedBufferInfo &lbi)
{
	nlassert((_XFadeSize & 0x1) == 0);
	uint	fade = _XFadeSize;
	sint16	*ptr = lbi.Ptr1;
	uint	count = lbi.Size1/2;
	sint	alpha;

	while (fade && count)
	{
		alpha = (fade<<16) / _XFadeSize;
		*ptr = (*ptr*alpha) >> 16;
		++ptr;
		--count;
		--fade;
	}

	ptr = lbi.Ptr2;
	count = lbi.Size2/2;

	while (fade && count)
	{
		alpha = (fade<<16) / _XFadeSize;
		*ptr = (*ptr*alpha) >> 16;
		++ptr;
		--count;
		--fade;
	}
}
void CSourceDSound::fadeIn(const TLockedBufferInfo &lbi)
{
	// do the XFade in integer fixed point arithmetic

	nlassert((_XFadeSize & 0x1) == 0);
	uint	fade = _XFadeSize;
	sint16	*ptr = lbi.Ptr1;
	uint	count = lbi.Size1 /2;
	sint	alpha, invAlpha;

	while (fade && count)
	{
		alpha = (fade<<16) / _XFadeSize;
		invAlpha = 0x10000 - alpha;
		*ptr = (*ptr*invAlpha) >> 16;
		++ptr;
		--count;
		--fade;
	}

	ptr = lbi.Ptr2;
	count = lbi.Size2 /2;

	while (fade && count)
	{
		alpha = (fade<<16) / _XFadeSize;
		invAlpha = 0x10000 - alpha;
		*ptr = (*ptr*invAlpha) >> 16;
		++ptr;
		--count;
		--fade;
	}
}


void CSourceDSound::advanceFill(TLockedBufferInfo &lbi, uint nbSample)
{
	uint32 size = nbSample * 2;
	if (lbi.Size1 < size)
	{
		size -= lbi.Size1;
		lbi.Size1 = lbi.Size2;
		lbi.Size2 = 0;
		lbi.Ptr1 = lbi.Ptr2;
		lbi.Ptr2 = 0;
	}

	nlassert(lbi.Size1 >= size);
	lbi.Size1 -= size;
	lbi.Ptr1 += size/2;

	_FillOffset += nbSample*2;
	nlassert(_FillOffset == (_FillOffset & 0xfffffffC));
	_FillOffset &= _SizeMask;
//	if (_FillOffset >= _SecondaryBufferSize)
//		_FillOffset -= _SecondaryBufferSize;
	nlassert(_FillOffset < _SecondaryBufferSize);
}



bool CSourceDSound::play()
{
//	nldebug("Play");
	EnterCriticalSection(&_CriticalSection);

	_SilenceWriten = 0;

//	uint32 writeSize = checkFillCursor();
	TCursors cursors;
	getCursors(cursors);

	// set a new filling point
	_FillOffset = cursors.WriteCursor;
	_FillOffset = (_FillOffset+3) & 0xfffffffC;
	cursors.WriteCursor = _FillOffset;

	TLockedBufferInfo lbi;
	if (lock(_FillOffset, cursors.WriteSize, lbi))
	{
		TLockedBufferInfo unlockInfo(lbi);
		// ok, the buffer is locked, write data
		if (_State == source_swap_pending)
		{
			// we swap the buffer.
			_Sample = _NextSample;
			_NextSample = 0;
			if (_Sample != 0)
			{
				_Sample->getFormat(_Format, _SampleFreq);
				switch(_Format)
				{
				case Mono8:
					_SampleSize = _Sample->getSize();
					break;
				case Mono16:
					_SampleSize = _Sample->getSize() / 2;
					break;
				case Mono16ADPCM:
					_SampleSize = _Sample->getSize() * 2;
					break;
				case Stereo8:
					_SampleSize = _Sample->getSize() / 2;
					break;
				case Stereo16:
					_SampleSize = _Sample->getSize() / 4;
					break;
				}
				_State = source_playing;
			}
			else
			{
				_SampleSize = 0;
				_State = source_silencing;
			}
		}
		_LastPlayPos = cursors.PlayCursor;
		_SampleOffset = 0;
		_PlayOffset = 0;
		_ADPCMState.PreviousSample = 0;
		_ADPCMState.StepIndex = 0;
		// Compute the size of data to write.
		uint dataToFill = std::min(uint(cursors.WriteSize / 2), _SampleSize - _SampleOffset);
		dataToFill &= 0xfffffffe;
		// ok, the buffer is locked, write data
		if (_State == source_playing || _State == source_silencing)
		{
			// we need a little XFade
			sint16	fadeBuffer[_XFadeSize];
			fillData(fadeBuffer, _XFadeSize);
			xfade(lbi, fadeBuffer);
			advanceFill(lbi, _XFadeSize);
			cursors.WriteSize -= _XFadeSize*2;
			dataToFill -= _XFadeSize;
		}
		else
		{
			// we need a little FadeIn
			fillData(lbi, _XFadeSize);
			fadeIn(lbi);
			advanceFill(lbi, _XFadeSize);
			cursors.WriteSize -= _XFadeSize*2;
			dataToFill -= _XFadeSize;
		}
		fillData(lbi, dataToFill);
		cursors.WriteSize -= dataToFill * 2;
		advanceFill(lbi, dataToFill);
		_State = source_playing;
		if (_Loop)
		{
			while (cursors.WriteSize >= 4)
			{
				if (_SampleOffset == _SampleSize)
				{
					// rewind the sample
					_SampleOffset = 0;
					_ADPCMState.PreviousSample = 0;
					_ADPCMState.StepIndex = 0;
				}
		nlassert(_SampleOffset < _SampleSize);
				dataToFill = std::min(uint(cursors.WriteSize / 2), _SampleSize - _SampleOffset);
				dataToFill &= 0xfffffffe;
				fillData(lbi, dataToFill);
				advanceFill(lbi, dataToFill);
				cursors.WriteSize -= dataToFill*2;
			}
		}
		else
		{
			if (_SampleOffset == _SampleSize)
			{
				// begin to write silence, but stil in play state until all sample are played
//				_State = source_silencing;
				fillSilence(lbi, cursors.WriteSize/2);
				advanceFill(lbi, cursors.WriteSize/2);
				_SilenceWriten = cursors.WriteSize;
				cursors.WriteSize = 0;
			}
//			else
//				_State = source_playing;
		}


		unlock(unlockInfo);
	}
	else
	{
		nlwarning("Couldn't lock the sound buffer for %u bytes", cursors.WriteSize);
	}

	// set the volume NOW
	CListenerDSound* listener = CListenerDSound::instance();

	updateVolume(listener->getPos());

	LeaveCriticalSection(&_CriticalSection);

	return true;
}

void CSourceDSound::stop()
{
//	nldebug("Stop");
	EnterCriticalSection(&_CriticalSection);

	if (_State != source_stopped && _State != source_silencing)
	{
		// retreive the cursors;
		TCursors	cursors;
		getCursors(cursors);

		_FillOffset = cursors.WriteCursor;
		_FillOffset = (_FillOffset+3) & 0xfffffffC;

		TLockedBufferInfo lbi;
		if (lock(_FillOffset, cursors.WriteSize, lbi))
		{
			TLockedBufferInfo unlockInfo(lbi);

 			fadeOut(lbi);
			advanceFill(lbi, _XFadeSize);
			cursors.WriteSize -= _XFadeSize*2;
			fillSilence(lbi, cursors.WriteSize/2);
			advanceFill(lbi, cursors.WriteSize/2);
			_SilenceWriten = cursors.WriteSize;

			_State = source_silencing;

			unlock(unlockInfo);
		}
	}

	LeaveCriticalSection(&_CriticalSection);
}



// ******************************************************************

void CSourceDSound::setLooping( bool l )
{
	_Loop = l;
}


// ******************************************************************

bool CSourceDSound::getLooping() const
{
	return _Loop;
}


// ******************************************************************
/*
void CSourceDSound::swap()
{
	_Buffer = _SwapBuffer;
	_BufferSize = _Buffer->getSize();
	_BytesWritten = 0;
	_SwapBuffer = 0;
}
*/
// ******************************************************************
/*
bool CSourceDSound::play()
{
	EnterCriticalSection(&_CriticalSection);

	if (_Buffer == 0 || ((CBufferDSound*) _Buffer)->getData() == 0)
	{
		// the sample has been unloaded, can't play!

		LeaveCriticalSection(&_CriticalSection);
		return false;
	}


	DBGPOS(("[%p] PLAY: Enter, buffer state = %u", this, _SecondaryBufferState));
	switch (_SecondaryBufferState)
	{
	case NL_DSOUND_FILLING:
		if (_SwapBuffer != 0)
		{
			// crossfade to the new sound
			DBGPOS(("[%p] PLAY: XFading 1", this));
			crossFade();
		}
		break;

	case NL_DSOUND_SILENCING:
		if (_SwapBuffer != 0)
		{
			if ((_Buffer != 0) && (_UserState == NL_DSOUND_PLAYING))
			{
				// crossfade to the new sound
				DBGPOS(("[%p] PLAY: XFading 2", this));
				crossFade();
			}
			else
			{
				DBGPOS(("[%p] PLAY: Swap & fadein 1", this));
				swap();
				fadeIn();
			}
		}
		else
		{
			DBGPOS(("[%p] PLAY: Fadein", this));
			_BytesWritten = 0;
			// start the old sound again
			fadeIn();
		}

		break;


	case NL_DSOUND_SILENCED:
		if (_SwapBuffer != 0)
		{
			// fade in to the new sound
				DBGPOS(("[%p] PLAY: Swap & fadein 2", this));
			swap();
			fadeIn();
		}
		else
		{
			DBGPOS(("[%p] PLAY: Fadein", this));
			_BytesWritten = 0;
			// start the old sound again
			fadeIn();
		}
	}

	_UserState = NL_DSOUND_PLAYING;
	DBGPOS(("[%p] PLAY: PLAYING", this));

	//nldebug ("NLSOUND: %p play", this);

	LeaveCriticalSection(&_CriticalSection);

	return true;
}
*/

// ******************************************************************
/*
void CSourceDSound::stop()
{
	EnterCriticalSection(&_CriticalSection);

	TSourceDSoundUserState old = _UserState;

	_UserState = NL_DSOUND_STOPPED;
	DBGPOS(("[%p] STOP: STOPPED", this));

	//nldebug ("NLSOUND: %p stop", this);

	if (old == NL_DSOUND_PLAYING)
	{
		fadeOut();
	}

	_BytesWritten = 0;

	LeaveCriticalSection(&_CriticalSection);
}
*/
// ******************************************************************

void CSourceDSound::pause()
{
	// TODO : recode this !
	nlassert(false);
/*	EnterCriticalSection(&_CriticalSection);

	TSourceDSoundUserState old = _UserState;

	_UserState = NL_DSOUND_PAUSED;
	DBGPOS(("[%p] PAUZ: PAUSED", this));

	//nldebug ("NLOUND: pause %p", this);

	if (old == NL_DSOUND_PLAYING)
	{
		fadeOut();
	}

	LeaveCriticalSection(&_CriticalSection);
*/
}

// ******************************************************************

bool CSourceDSound::isPlaying() const
{
//	return (_UserState == NL_DSOUND_PLAYING);
	return _State == source_playing || _State == source_swap_pending;
}


// ******************************************************************

bool CSourceDSound::isPaused() const
{
	// TODO
	nlassert(false);
	return false;
//	return (_UserState == NL_DSOUND_PAUSED);
}


// ******************************************************************

bool CSourceDSound::isStopped() const
{
	return _State == source_silencing || _State == source_stopped;
//	return (_UserState == NL_DSOUND_STOPPED);
}


// ******************************************************************

bool CSourceDSound::needsUpdate()
{
	return _State == source_silencing || _State == source_playing || _State == source_swap_pending;
}



// ******************************************************************

bool CSourceDSound::update()
{
	H_AUTO(NLSOUND_SourceDSoundUpdate)
	bool updateDone = false;

	EnterCriticalSection(&_CriticalSection);

	// fill some data into the buffer
	TCursors cursors;
	getCursors(cursors);
	uint32	writeSize;

	// The total size available for fill (between fillOffset and play cusors)
	uint32 fillSize = (cursors.PlayCursor - _FillOffset) & _SizeMask;
	// The play margin (between play and write cursor)
	uint32 margin = (cursors.WriteCursor - cursors.PlayCursor) & _SizeMask;
	// The number of sample played since previous update
	uint32 samplePlayed = ((cursors.PlayCursor - _LastPlayPos) & _SizeMask) / 2;
	_LastPlayPos = cursors.PlayCursor;


//	if (_FillOffset < cursors.WriteCursor && _FillOffset >cursors.PlayCursor)
	if (fillSize + margin > _SecondaryBufferSize)
	{
		// arg !
		nlwarning("FillOffset is between play and write cursor (P = %p F = %p W = %p!", cursors.PlayCursor, _FillOffset, cursors.WriteCursor);
		_FillOffset = cursors.WriteCursor;
		_FillOffset = (_FillOffset+3) & 0xfffffffC;
		_SilenceWriten = 0;
	nlassert(_FillOffset < _SecondaryBufferSize);
	}

	// advance of the fill offset over the write cursor
	uint32 advance = (_FillOffset - cursors.WriteCursor) & _SizeMask;

/*	nldebug("Play = %p, Write = %p, Fill = %p, FillSize = %p, Margin = %p, Advance = %p",
		cursors.PlayCursor, cursors.WriteCursor, _FillOffset, fillSize, margin, advance);
*/
	if (advance > _UpdateCopySize)
	{
		// enougth data wrote, wait until next update
		cursors.WriteSize = 0;
	}
/*	if (cursors.WriteSize)
	{
		if (_FillOffset < cursors.WriteCursor)
		{
			if (_FillOffset > cursors.WriteCursor + _UpdateCopySize - _SecondaryBufferSize )
			{
//				nlwarning("Enougth data wrote");
				// already fill enough data
				cursors.WriteSize = 0;
			}
		}
		else
		{
			if (_FillOffset > cursors.WriteCursor + _UpdateCopySize)
			{
//				nlwarning("Enougth data wrote");
				// already fill enough data
				cursors.WriteSize = 0;
			}
		}
	}
*/
//	nldebug("Cursors : Play = %p, Write = %p, Fill = %p", cursors.PlayCursor, cursors.WriteCursor, _FillOffset);

	cursors.WriteCursor = _FillOffset;
	writeSize = cursors.WriteSize;	// compute the real played sample offset

	// update the real number of played sample
	if (_State == source_playing)
		_PlayOffset += samplePlayed;

	if (writeSize >= _UpdateCopySize)
	{
//		nldebug("Update %p bytes", _UpdateCopySize);
		writeSize = _UpdateCopySize;
		updateDone = true;
		TLockedBufferInfo lbi;
		if (lock(_FillOffset, writeSize, lbi))
		{
			TLockedBufferInfo unlockInfo(lbi);
			if (_State == source_playing)
			{
		nlassert(_SampleOffset <= _SampleSize);
				uint32	updateSize = min(_SampleSize - _SampleOffset, uint(writeSize/2));
				updateSize &= 0xfffffffe;
				fillData(lbi, updateSize);
				advanceFill(lbi, updateSize);
				writeSize -= updateSize*2;

				if (_Loop)
				{
					while (_PlayOffset >= _SampleSize)
						_PlayOffset -= _SampleSize;

					// repeat until we have at least 2 sample to write
					while (writeSize >= 4)
					{
						if (_SampleOffset == _SampleSize)
						{
							// rewind the sample
							_SampleOffset = 0;
							_ADPCMState.PreviousSample = 0;
							_ADPCMState.StepIndex = 0;
						}

						updateSize = min(_SampleSize - _SampleOffset, uint(writeSize/2));
						updateSize &= 0xfffffffe;
						fillData(lbi, updateSize);
						advanceFill(lbi, updateSize);
						writeSize -= updateSize*2;
					}
				}
				else
				{
					if (_SampleOffset == _SampleSize)
					{
						fillSilence(lbi, writeSize/2);
						advanceFill(lbi, writeSize/2);
						_SilenceWriten += writeSize;
					}
					if (_PlayOffset >= _SampleSize)
					{
						// all the sample is played, no we are in silencing state !
						_PlayOffset = _SampleSize;
						_State = source_silencing;
					}
				}

			}
			else if (_State == source_swap_pending)
			{
				// the sample has been changed but not replayed yet ? so we 'stop' the old buffer

				fadeOut(lbi);
				advanceFill(lbi, _XFadeSize);
				writeSize -= _XFadeSize*2;
				fillSilence(lbi, writeSize/2);
				advanceFill(lbi, writeSize/2);
				_SilenceWriten = writeSize;
				_State = source_silencing;
			}
			else if (_State == source_silencing)
			{
				// write silence into the buffer.
				uint32 updateSize = min(writeSize, _SecondaryBufferSize - _SilenceWriten);
				updateSize &= 0xfffffffC;
				fillSilence(lbi, updateSize/2);
				advanceFill(lbi, updateSize/2);
				_SilenceWriten += updateSize;

				if (_SilenceWriten == _SecondaryBufferSize)
					_State = source_stopped;
			}
			else
			{
				nlwarning("Update not needed !");
			}

			unlock(unlockInfo);
		}
	}

	LeaveCriticalSection(&_CriticalSection);

	return updateDone;
}

// ******************************************************************
/*
bool CSourceDSound::update2()
{
	bool res = false;



	INITTIME(start);

	//
	// Enter critical region
	//
	EnterCriticalSection(&_CriticalSection);


	switch (_SecondaryBufferState)
	{


	case NL_DSOUND_SILENCED:
		{
			// Just pretend were writing silence by advancing the next
			// write position.
			DWORD playPos, writePos;
			uint32 space;

			_SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);

			if (playPos == _NextWritePos)
			{
				LeaveCriticalSection(&_CriticalSection);
				return false;
			}

			space = getWritePosAndSpace(_NextWritePos, playPos, writePos, _SecondaryBufferSize);

			// not enougth space available
			if (space < _UpdateCopySize)
				break;

			_NextWritePos += _UpdateCopySize;

			if (_NextWritePos >= _SecondaryBufferSize)
			{
				_NextWritePos  -= _SecondaryBufferSize;
			}
		}
		break;


	case NL_DSOUND_FILLING:
		res = fill();
		break;


	case NL_DSOUND_SILENCING:
		res = silence();
		break;

	}


	//
	// Leave critical region
	//
	LeaveCriticalSection(&_CriticalSection);


#if NLSOUND_PROFILE
	double dt = CTime::ticksToSecond(CTime::getPerformanceTime() - start);;
	_TotalUpdateTime += dt;
	_MaxUpdateTime = (dt > _MaxUpdateTime) ? dt : _MaxUpdateTime;
	_MinUpdateTime = (dt < _MinUpdateTime) ? dt : _MinUpdateTime;
	_UpdateCount++;
#endif

	return res;
}
*/


// ******************************************************************

void CSourceDSound::setPos( const NLMISC::CVector& pos, bool deferred )
{
	_Pos = pos;
	// Coordinate system: conversion from NeL to OpenAL/GL:
	if (_3DBuffer != NULL)
	{
		if (_3DBuffer->SetPosition(pos.x, pos.z, pos.y, deferred ? DS3D_DEFERRED : DS3D_IMMEDIATE) != DS_OK)
		{
			nlwarning ("SetPosition failed");
		}
		else
		{
			//nlwarning ("%p set source NEL(p:%.2f/%.2f/%.2f) DS(p:%.2f/%.2f/%.2f)", this, pos.x, pos.y, pos.z, pos.x, pos.z, pos.y);
		}
	}
}


// ******************************************************************

const NLMISC::CVector &CSourceDSound::getPos() const
{
	return _Pos;
}


// ******************************************************************

void CSourceDSound::setVelocity( const NLMISC::CVector& vel, bool deferred )
{
	if (_3DBuffer != NULL)
	{
		if (_3DBuffer->SetVelocity(vel.x, vel.z, vel.y, deferred ? DS3D_DEFERRED : DS3D_IMMEDIATE) != DS_OK)
		{
			nlwarning ("SetVelocity failed");
		}
	}
}


// ******************************************************************

void CSourceDSound::getVelocity( NLMISC::CVector& vel ) const
{
	if (_3DBuffer != NULL)
	{
		D3DVECTOR v;

		if (_3DBuffer->GetVelocity(&v) != DS_OK)
		{
			nlwarning ("GetVelocity failed");
			vel.set(0, 0, 0);
		}
		else
		{
			vel.set(v.x, v.z, v.y);
		}
	}
	else
	{
		vel.set(0, 0, 0);
	}
}


// ******************************************************************

void CSourceDSound::setDirection( const NLMISC::CVector& dir )
{
	if (_3DBuffer != 0)
	{
		if (_3DBuffer->SetConeOrientation(dir.x, dir.z, dir.y, DS3D_DEFERRED) != DS_OK)
		{
			nlwarning ("SetConeOrientation failed (x=%.2f, y=%.2f, z=%.2f)", dir.x, dir.y, dir.z);
		}
		else
		{
			//nlwarning ("NLSOUND: %p set source direction NEL(p:%.2f/%.2f/%.2f) DS(p:%.2f/%.2f/%.2f)", this, dir.x, dir.y, dir.z, dir.x, dir.z, dir.y);
		}
	}
}


// ******************************************************************

void CSourceDSound::getDirection( NLMISC::CVector& dir ) const
{
	if (_3DBuffer != NULL)
	{
		D3DVECTOR v;

		if (_3DBuffer->GetConeOrientation(&v) != DS_OK)
		{
			nlwarning("GetConeOrientation failed");
			dir.set(0, 0, 1);
		}
		else
		{
			dir.set(v.x, v.z, v.y);
		}
	}
	else
	{
		dir.set(0, 0, 1);
	}
}


// ******************************************************************

void CSourceDSound::setGain( float gain )
{
	clamp(gain, 0.00001f, 1.0f);
	_Gain = gain;

	/* convert from linear amplitude to hundredths of decibels */
	_Volume = (uint32)(100.0 * 20.0 * log10(gain));
	clamp(_Volume, DSBVOLUME_MIN, DSBVOLUME_MAX);

	//nlwarning ("set gain %f vol %d", gain, _Volume);

	/*
	if ((_SecondaryBuffer != 0) && (_SecondaryBuffer->SetVolume(_Volume) != DS_OK))
	{
		nlwarning("SetVolume failed");
	}
	*/
}


/*
 * Get the gain
 */
float CSourceDSound::getGain() const
{
	return _Gain;
}


// ******************************************************************

void CSourceDSound::setPitch( float coeff )
{
//	_Freq = coeff;

	if ((_Sample != 0) && (_SecondaryBuffer != 0))
	{
		TSampleFormat format;
		uint freq;

		_Sample->getFormat(format, freq);

		_SampleRate = (uint32) (coeff * (float) freq);

		//nlwarning("Freq=%d", newfreq);

		if (_SecondaryBuffer->SetFrequency(_SampleRate) != DS_OK)
		{
//			nlwarning("SetFrequency failed (buffer freq=%d, NeL freq=%.5f, DSound freq=%d)", freq, coeff, newfreq);
			nlwarning("SetFrequency");
		}
	}
}


// ******************************************************************

float CSourceDSound::getPitch() const
{
	if ((_Sample != 0) && (_SecondaryBuffer != 0))
	{
		TSampleFormat format;
		uint freq0;
		DWORD freq;

		_Sample->getFormat(format, freq0);

		if (_SecondaryBuffer->GetFrequency(&freq) != DS_OK)
		{
			nlwarning("GetFrequency failed");
			return 1.0;
		}

		return ((float) freq / (float) freq0);
	}

	return 1.0;
}


// ******************************************************************

void CSourceDSound::setSourceRelativeMode( bool mode )
{
	if (_3DBuffer != 0)
	{
		HRESULT hr;

		if (mode)
		{
			hr = _3DBuffer->SetMode(DS3DMODE_HEADRELATIVE, DS3D_IMMEDIATE);
		}
		else
		{
			hr = _3DBuffer->SetMode(DS3DMODE_NORMAL, DS3D_IMMEDIATE);
		}

		// cache
		if (hr == DS_OK)
			_PosRelative= mode;
		else
			nlwarning("SetMode failed");
	}
	else
	{
		nlwarning("Requested setSourceRelativeMode on a non-3D source");
	}
}


// ******************************************************************

bool CSourceDSound::getSourceRelativeMode() const
{
	return _PosRelative;
}


// ******************************************************************
void CSourceDSound::setMinMaxDistances( float mindist, float maxdist, bool deferred )
{
	if (_3DBuffer != 0)
	{
		if (_3DBuffer->SetMinDistance(std::max(DS3D_DEFAULTMINDISTANCE, mindist), deferred ? DS3D_DEFERRED : DS3D_IMMEDIATE) != DS_OK)
		{
			nlwarning("SetMinDistance (%f) failed", mindist);
		}
		if (_3DBuffer->SetMaxDistance(std::min(DS3D_DEFAULTMAXDISTANCE, maxdist), deferred ? DS3D_DEFERRED : DS3D_IMMEDIATE) != DS_OK)
		{
			nlwarning("SetMaxDistance (%f) failed", maxdist);
		}
	}
	else
	{
		nlwarning("Requested setMinMaxDistances on a non-3D source");
	}
}


// ******************************************************************

void CSourceDSound::getMinMaxDistances( float& mindist, float& maxdist ) const
{
	if (_3DBuffer != 0)
	{
		D3DVALUE min, max;

		if ((_3DBuffer->GetMinDistance(&min) != DS_OK)
			|| (_3DBuffer->GetMaxDistance(&max) != DS_OK))
		{
			mindist = 0.0f;
			maxdist = 0.0f;
			nlwarning("GetMinDistance or GetMaxDistance failed");
		}
		else
		{
			mindist = min;
			maxdist = max;
		}
	}
	else
	{
		mindist = 0.0f;
		maxdist = 0.0f;
		nlwarning("Requested getMinMaxDistances on a non-3D source");
	}
}

// ******************************************************************
void CSourceDSound::updateVolume( const NLMISC::CVector& listener )
{
	if (!CSoundDriverDSound::instance()->getOption(ISoundDriver::OptionManualRolloff))
	{
		// API controlled rolloff => return (just set the volume)
		_SecondaryBuffer->SetVolume(_Volume);
	}
	else // manual rolloff
	{
		CVector pos = getPos();
		// make relative to listener (if not already!)
		if(!_PosRelative)
			pos -= listener;
		float sqrdist = pos.sqrnorm();

		float mindist, maxdist;
		getMinMaxDistances(mindist, maxdist);

		// attenuate the volume according to distance and alpha
		sint32 volumeDB = ISource::computeManualRollOff(_Volume, DSBVOLUME_MIN, DSBVOLUME_MAX, _Alpha, sqrdist, mindist, maxdist);

		// set attenuated volume
		_SecondaryBuffer->SetVolume(volumeDB);
	}
}

// ******************************************************************

void CSourceDSound::setCone( float innerAngle, float outerAngle, float outerGain )
{
	if (_3DBuffer != 0)
	{
		// Set the cone angles

		// Convert from radians to degrees
		DWORD inner = (DWORD)(180.0 * innerAngle / Pi);
		DWORD outer = (DWORD)(180.0 * outerAngle / Pi);


		// Sanity check: wrap the angles in the [0,360] interval
		if (outer < inner)
		{
			outer = inner;
		}

		while (inner < DS3D_MINCONEANGLE)
		{
			inner += 360;
		}

		while (inner > DS3D_MAXCONEANGLE)
		{
			inner -= 360;
		}

		while (outer < DS3D_MINCONEANGLE)
		{
			outer += 360;
		}

		while (outer > DS3D_MAXCONEANGLE)
		{
			outer -= 360;
		}

		if (_3DBuffer->SetConeAngles(inner, outer, DS3D_DEFERRED) != DS_OK)
		{
			nlwarning("SetConeAngles failed");
		}

		// Set the outside volume
		if (outerGain < 0.00001f)
		{
			outerGain = 0.00001f;
		}

		// convert from linear amplitude to hundredths of decibels
		LONG volume = (LONG)(100.0 * 20.0 * log10(outerGain));

		if (volume < DSBVOLUME_MIN)
		{
			volume = DSBVOLUME_MIN;
		}
		else if (volume > DSBVOLUME_MAX)
		{
			volume = DSBVOLUME_MAX;
		}

		if (_3DBuffer->SetConeOutsideVolume(volume, DS3D_DEFERRED) != DS_OK)
		{
			nlwarning("SetConeOutsideVolume failed");
		}

	}
	else
	{
		nlwarning("Requested setCone on a non-3D source");
	}
}

// ******************************************************************

void CSourceDSound::getCone( float& innerAngle, float& outerAngle, float& outerGain ) const
{
	if (_3DBuffer != 0)
	{
		DWORD inner, outer;
		LONG volume;

		if (_3DBuffer->GetConeAngles(&inner, &outer) != DS_OK)
		{
			nlwarning("GetConeAngles failed");
			innerAngle = outerAngle = (float)(2.0 * Pi);
		}
		else
		{
			innerAngle = (float)(Pi * inner / 180.0);
			outerAngle = (float)(Pi * outer / 180.0);
		}

		if (_3DBuffer->GetConeOutsideVolume(&volume) != DS_OK)
		{
			nlwarning("GetConeOutsideVolume failed");
			outerGain = 0.0f;
		}
		else
		{
			outerGain = (float) pow((double)10, (double) volume / 20.0 / 100.0);
		}
	}
	else
	{
		nlwarning("Requested getCone on a non-3D source");
	}
}

// ******************************************************************

//void CSourceDSound::setEAXProperty( uint prop, void *value, uint valuesize )
//{
//#if EAX_AVAILABLE == 1
//	if (_EAXSource == 0)
//	{
//		_EAXSource = CSoundDriverDSound::instance()->createPropertySet(this);
//	}
//	if ( _EAXSource != NULL )
//	{
//		H_AUTO(NLSOUND_EAXPropertySet_Set)
//		HRESULT res = _EAXSource->Set( DSPROPSETID_EAX_BufferProperties, prop, NULL, 0, value, valuesize );
//		if (res != S_OK)
//		{
////			nlwarning("Setting EAX Param #%u fail : %x", prop, res);
//		}
//	}
//#endif
//}

/** Set the alpha value for the volume-distance curve
 *
 *	Useful only with OptionManualRolloff. value from -1 to 1 (default 0)
 *
 *  alpha.0: the volume will decrease linearly between 0dB and -100 dB
 *  alpha = 1.0: the volume will decrease linearly between 1.0 and 0.0 (linear scale)
 *  alpha = -1.0: the volume will decrease inversely with the distance (1/dist). This
 *                is the default used by DirectSound/OpenAL
 *
 *  For any other value of alpha, an interpolation is be done between the two
 *  adjacent curves. For example, if alpha equals 0.5, the volume will be halfway between
 *  the linear dB curve and the linear amplitude curve.
 */
void CSourceDSound::setAlpha(double a)
{
	_Alpha = a;
}

/// Enable or disable direct output [true/false], default: true
void CSourceDSound::setDirect(bool /* enable */)
{
	
}

/// Return if the direct output is enabled
bool CSourceDSound::getDirect() const
{
	return true;
}

/// Set the gain for the direct path
void CSourceDSound::setDirectGain(float /* gain */)
{
	
}

/// Get the gain for the direct path
float CSourceDSound::getDirectGain() const
{
	return NLSOUND_DEFAULT_DIRECT_GAIN;
}

/// Enable or disable the filter for the direct channel
void CSourceDSound::enableDirectFilter(bool /* enable */)
{
	
}

/// Check if the filter on the direct channel is enabled
bool CSourceDSound::isDirectFilterEnabled() const
{
	return false;
}

/// Set the filter parameters for the direct channel
void CSourceDSound::setDirectFilter(TFilter /*filterType*/, float /*lowFrequency*/, float /*highFrequency*/, float /*passGain*/)
{
	
}

/// Get the filter parameters for the direct channel
void CSourceDSound::getDirectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const
{
	filterType = FilterLowPass;
	lowFrequency = NLSOUND_DEFAULT_FILTER_PASS_LF; 
	highFrequency = NLSOUND_DEFAULT_FILTER_PASS_HF; 
	passGain = NLSOUND_DEFAULT_FILTER_PASS_GAIN;
}

/// Set the direct filter gain
void CSourceDSound::setDirectFilterPassGain(float /*passGain*/)
{
	
}

/// Get the direct filter gain
float CSourceDSound::getDirectFilterPassGain() const
{
	return 0.0f;
}

/// Set the effect send for this source, NULL to disable. [IEffect], default: NULL
void CSourceDSound::setEffect(IReverbEffect * /* reverbEffect */)
{
	
}

/// Get the effect send for this source
IEffect *CSourceDSound::getEffect() const
{
	return NULL;
}

/// Set the gain for the effect path
void CSourceDSound::setEffectGain(float /* gain */)
{
	
}

/// Get the gain for the effect path
float CSourceDSound::getEffectGain() const
{
	return NLSOUND_DEFAULT_EFFECT_GAIN;
}

/// Enable or disable the filter for the effect channel
void CSourceDSound::enableEffectFilter(bool /* enable */)
{
	
}

/// Check if the filter on the effect channel is enabled
bool CSourceDSound::isEffectFilterEnabled() const
{
	return false;
}

/// Set the filter parameters for the effect channel
void CSourceDSound::setEffectFilter(TFilter /*filterType*/, float /*lowFrequency*/, float /*highFrequency*/, float /*passGain*/)
{
	
}

/// Get the filter parameters for the effect channel
void CSourceDSound::getEffectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const
{
	filterType = FilterLowPass;
	lowFrequency = NLSOUND_DEFAULT_FILTER_PASS_LF; 
	highFrequency = NLSOUND_DEFAULT_FILTER_PASS_HF; 
	passGain = NLSOUND_DEFAULT_FILTER_PASS_GAIN;
}

/// Set the effect filter gain
void CSourceDSound::setEffectFilterPassGain(float /*passGain*/)
{
	
}

/// Get the effect filter gain
float CSourceDSound::getEffectFilterPassGain() const
{
	return 0.0f;
}

// ******************************************************************

IBuffer *CSourceDSound::getBuffer()
{
	return _Sample;
}


// ******************************************************************

bool CSourceDSound::lock(uint32 offset, uint32 size, TLockedBufferInfo &lbi)
{
	HRESULT hr = _SecondaryBuffer->Lock(offset, size, (LPVOID*) &lbi.Ptr1, (DWORD*) &lbi.Size1, (LPVOID*) &lbi.Ptr2, (DWORD*) &lbi.Size2, 0);

	if (hr == DSERR_BUFFERLOST)
	{
		// If the buffer got lost somehow, try to restore it.
		if (FAILED(_SecondaryBuffer->Restore()))
		{
			nlwarning("Lock failed (1)");
			return false;
		}
		if (FAILED(_SecondaryBuffer->Lock(offset, size, (LPVOID*) &lbi.Ptr1, (DWORD*)&lbi.Size1, (LPVOID*) &lbi.Ptr2, (DWORD*)&lbi.Size2, 0)))
		{
			nlwarning("Lock failed (2)");
			return false;
		}
	}
	else if (hr != DS_OK)
	{
		nlwarning("Lock failed (3)");
		return false;
	}

	return true;
}
/*

bool CSourceDSound::lock(uint32 writePos, uint32 size, uint8* &ptr1, DWORD &bytes1, uint8* &ptr2, DWORD &bytes2)
{
	HRESULT hr = _SecondaryBuffer->Lock(writePos, size, (LPVOID*) &ptr1, &bytes1, (LPVOID*) &ptr2, &bytes2, 0);

	if (hr == DSERR_BUFFERLOST)
	{
		// If the buffer got lost somehow, try to restore it.
		if (FAILED(_SecondaryBuffer->Restore()))
		{
			nlwarning("Lock failed (1)");
			return false;
		}
		if (FAILED(_SecondaryBuffer->Lock(_NextWritePos, _UpdateCopySize, (LPVOID*) &ptr1, &bytes1, (LPVOID*) &ptr2, &bytes2, 0)))
		{
			nlwarning("Lock failed (2)");
			return false;
		}
	}
	else if (hr != DS_OK)
	{
		nlwarning("Lock failed (3)");
		return false;
	}

	return true;
}
*/
// ******************************************************************

bool CSourceDSound::unlock(const TLockedBufferInfo &lbi)
{
 	_SecondaryBuffer->Unlock(lbi.Ptr1, lbi.Size1, lbi.Ptr2, lbi.Size2);
	return true;
}


/*
bool CSourceDSound::unlock(uint8* ptr1, DWORD bytes1, uint8* ptr2, DWORD bytes2)
{
	_SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);
	return true;
}
*/
/*
void CSourceDSound::copySampleTo16BitsTrack(void *dst, void *src, uint nbSample, TSampleFormat sourceFormat)
{
	if (sourceFormat == Mono8 || sourceFormat == Stereo8)
	{
		nlassert("8 bit mixing is not supported now !");
		return;
		// convert sample from 8 to 16 inside the dst buffer
		sint8 *psrc = (sint8*)src;
		sint16 *pdst = (sint16*)dst;
		sint8 *endSrc = psrc + nbSample;

		for (; psrc != endSrc; psrc += 2)
		{
			// write the high word
			*pdst++ = sint16(*psrc++) * 256;
		}
	}
	if (sourceFormat == Mono16ADPCM)
	{
		// unpack ADPCM data
		nldebug("Mixing %u samples in ADPCM", nbSample);
		IBuffer::decodeADPCM((uint8*)src, (sint16*)dst, nbSample, _ADPCMState);
	}
	else
	{
		// use the fastmem copy buffer
		CFastMem::memcpy(dst, src, nbSample*2);
	}
}
*/
/***************************************************************************


  Buffer operations

  There are five buffer operation: fill, silence, fadeOut, crossFade
  and fadeIn. fill and silence are called by the update function. The
  others are called by the user state functions (play, stop, pause).


			NW	   P   W
   +--------+------+---+-----------------------+
   |........|	   |xxx|.......................|
   +--------+------+---+-----------------------+

  The source maintains the next write position (_NextWritePos, NW in figure
  above). That is the position at which new samples or silemce is written.
  DirectSound maintaines a play cursor and a write cursor (P and W in figure).
  The data between P and W is scheduled for playing and cannot be touched.
  The data between W and NW are unplayed sample data that the source copied
  into the DirectSound buffer.

  The update functions (fill, silence) refresh the buffer with new samples
  or silence. That insert the sample data at the next write position NW. This
  write position is maintained as closely behind the DirectSound play cursor
  as possible to keep the buffer filled with data.

  The user functions (the fades) modify the sample data that is right after
  the write cursor W maintained by DirectSound. The data has to be written
  after W to hear the changes as soon as possible. When a fade is done, the
  data already written in the buffer has to be overwritten. The function
  getFadeOutSize() helps to found out how many samples are writen between
  W and NW and to what section of the original audio data they correspond.

  All the buffer functions below have the same pattern:

	- get current play and write cursors (P and W)
	- lock the buffer
	- copy samples
	- unlock buffer
	- update state variables

  The differences between the functions are due to different operation
  (fades), position and size of the buffer to lock, handling of silence
  and looping.

  Enjoy!

  PH


************************************************************************/
/*
uint32 getWritePosAndSpace(uint32 &nextWritePos, uint32 playPos, uint32 writePos, uint32 bufferSize)
{
	uint32 space;
	if (playPos < writePos) //_NextWritePos)
	{
		// the 'forbiden' interval is continuous
		if (nextWritePos > playPos && nextWritePos < writePos)
		{
			// We have a problem here because our write pointer is in the forbiden zone
			// This is mainly due to cpu overload.
			nextWritePos = writePos;
		}
//		space = playPos - _NextWritePos;
	}
	else
	{
		// The forbiden interval is wrapping
		if (nextWritePos > playPos || nextWritePos < writePos)
		{
			// We have a problem here because our write pointer is in the forbiden zone
			// This is mainly due to cpu overload.
			nextWritePos = writePos;
		}
//		space = _SecondaryBufferSize + playPos - _NextWritePos;
	}

	// compute the available space to write to
	if (nextWritePos > playPos)
	{
		space = bufferSize + playPos - nextWritePos;
	}
	else
	{
		space = playPos - nextWritePos;
	}

	return space;
}
*/
/*
bool CSourceDSound::fill()
{
	bool res = false;
	uint8 *ptr1, *ptr2;
	DWORD bytes1, bytes2;
	DWORD playPos, writePos;
	uint32 space;


	if (_Buffer == NULL)
	{
		_SecondaryBufferState = NL_DSOUND_SILENCING;
		_UserState = NL_DSOUND_STOPPED;
		return false;
	}

	if (_SecondaryBuffer == 0)
	{
		return false;
	}

	TSampleFormat	sampleFormat;
	uint			freq;
	_Buffer->getFormat(sampleFormat, freq);


	INITTIME(startPos);


	_SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);

	if (playPos == _NextWritePos)
	{
		return false;
	}

	uint32 nextWritePos = _NextWritePos;
	space = getWritePosAndSpace(nextWritePos, playPos, writePos, _SecondaryBufferSize);

	// Don't bother if the number of samples that can be written is too small.
	if (space < _UpdateCopySize)
	{
		return false;
	}

	_NextWritePos = nextWritePos;

	uint8* data = ((CBufferDSound*) _Buffer)->getData();
	uint32 available = (_BytesWritten < _BufferSize) ? _BufferSize - _BytesWritten : 0;


	// The number of samples bytes that will be copied. If bytes is 0
	// than write silence to the buffer.
	uint32 bytes = std::min(_UpdateCopySize, available);
//	uint32 clear = _UpdateCopySize - available;


	// Lock the buffer

	INITTIME(startLock);


	if (!lock(_NextWritePos, _UpdateCopySize, ptr1, bytes1, ptr2, bytes2))
	{
		return false;
	}


	INITTIME(startCopy);

	// Start copying the samples


	if (bytes1 <= bytes) {

//		CFastMem::memcpy(ptr1, data + _BytesWritten, bytes1);
		copySampleTo16BitsTrack(ptr1, data + _BytesWritten, bytes1/2, sampleFormat);
		_BytesWritten += bytes1;
		bytes -= bytes1;

		if (ptr2)
		{
			if (bytes > 0)
			{
//				CFastMem::memcpy(ptr2, data + _BytesWritten, bytes);
				copySampleTo16BitsTrack(ptr2, data + _BytesWritten, bytes/2, sampleFormat);
				_BytesWritten += bytes;
			}

			if (bytes < bytes2)
			{
				if (_Loop)
				{
					DBGPOS(("[%p] FILL: LOOP", this));

					//CFastMem::memcpy(ptr2 + bytes, data, bytes2 - bytes);
					copySampleTo16BitsTrack(ptr2 + bytes, data, (bytes2 - bytes)/2, sampleFormat);
					_BytesWritten = bytes2 - bytes;
				}
				else
				{
					memset(ptr2 + bytes, 0, bytes2 - bytes);
					_SilenceWritten = bytes2 - bytes;
				}
			}
		}
	}
	else
	{
		if (bytes > 0)
		{
//			CFastMem::memcpy(ptr1, data + _BytesWritten, bytes);
			copySampleTo16BitsTrack(ptr1, data + _BytesWritten, bytes/2, sampleFormat);
			_BytesWritten += bytes;
		}

		if (_Loop)
		{
			DBGPOS(("[%p] FILL: LOOP", this));

//			CFastMem::memcpy(ptr1 + bytes, data, bytes1 - bytes);
			copySampleTo16BitsTrack(ptr1 + bytes, data, (bytes1 - bytes)/2, sampleFormat);
			_BytesWritten = bytes1 - bytes;

			if (ptr2)
			{
//				CFastMem::memcpy(ptr2, data + _BytesWritten, bytes2);
				copySampleTo16BitsTrack(ptr2, data + _BytesWritten, bytes2 / 2, sampleFormat);
				_BytesWritten += bytes2;
			}

		}
		else
		{
			memset(ptr1 + bytes, 0, bytes1 - bytes);
			_SilenceWritten = bytes1 - bytes;

			if (ptr2)
			{
				memset(ptr2, 0, bytes2);
				_SilenceWritten += bytes2;
			}
		}

	}




	INITTIME(startUnlock);

	// Unlock the buffer
	_SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);


	// Update the state variables

	// Check if we've reached the end of the file
	if (_BytesWritten == _BufferSize)
	{
		if (_Loop)
		{
			// If we're looping, start all over again
			DBGPOS(("[%p] FILL: LOOP", this));
			_BytesWritten = 0;
		}
		else
		{
			_SecondaryBufferState = NL_DSOUND_SILENCING;

			// Keep track of where tha last sample was written and the position
			// of the play cursor relative to the end position. if the _EndState
			// is 0, the play cursor is after the end position, 1 otherwise.
			_EndPosition = writePos + bytes;
			if (_EndPosition >= _SecondaryBufferSize)
			{
				_EndPosition -= _SecondaryBufferSize;
			}

			_EndState = (playPos > _EndPosition)? NL_DSOUND_TAIL1 : NL_DSOUND_TAIL2;

			DBGPOS(("[%p] FILL: SILENCING", this));
			DBGPOS(("[%p] FILL: ENDSTATE=%d, E=%d, P=%d", this, (int) _EndState, _EndPosition, playPos));
		}
	}


	// Update the write pointer
	_NextWritePos += bytes1 + bytes2;
	if (_NextWritePos >= _SecondaryBufferSize)
	{
		_NextWritePos  -= _SecondaryBufferSize;
	}

	DBGPOS(("[%p] FILL: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", this, playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));


#if NLSOUND_PROFILE
	_TotalUpdateSize += bytes1 + bytes2;
	_PosTime += CTime::ticksToSecond(startLock - startPos);
	_LockTime += CTime::ticksToSecond(startCopy - startLock);
	_CopyTime += CTime::ticksToSecond(startUnlock - startCopy);
	_UnlockTime += CTime::ticksToSecond(CTime::getPerformanceTime() - startUnlock);
	_CopyCount++;
#endif


	return true;
}
*/



// ******************************************************************
/*
bool CSourceDSound::silence()
{
	uint8 *ptr1, *ptr2;
	DWORD bytes1, bytes2;
	DWORD playPos, writePos;
	uint32 space;

	if (_SecondaryBuffer == 0)
	{
		return false;
	}

	INITTIME(startPos);

	_SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);

	if (playPos == _NextWritePos)
	{
		return false;
	}

	space = getWritePosAndSpace(_NextWritePos, playPos, writePos, _SecondaryBufferSize);
*/
/*	else if (playPos > _NextWritePos)
	{
		space = playPos - _NextWritePos;
	}
	else
	{
		space = _SecondaryBufferSize + playPos - _NextWritePos;
	}
*/
/*	// Don't bother if the number of samples that can be written is too small.
	if (space < _UpdateCopySize)
	{
		return false;
	}

		// Lock the buffer

	INITTIME(startLock);

	if (!lock(_NextWritePos, _UpdateCopySize, ptr1, bytes1, ptr2, bytes2))
	{
		return false;
	}


	INITTIME(startCopy);

	// Silence the buffer
	memset(ptr1, 0, bytes1);
	memset(ptr2, 0, bytes2);

		// Unlock the buffer

	INITTIME(startUnlock);

	_SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);


	// Update the next write position
	_NextWritePos += bytes1 + bytes2;
	if (_NextWritePos >= _SecondaryBufferSize)
	{
		_NextWritePos  -= _SecondaryBufferSize;
	}


	// Check if all the samples in the buffer are played
	if ((playPos > _EndPosition) && (_EndState == NL_DSOUND_TAIL2))
	{
		// The buffer played passed the last sample. Flag the source as stopped.
		_EndState = NL_DSOUND_ENDED;
		DBGPOS(("[%p] SLNC: ENDED", this));

		// If the buffer was playing, mark it as stopped
		if (_UserState == NL_DSOUND_PLAYING)
		{
			_UserState = NL_DSOUND_STOPPED;
			DBGPOS(("[%p] SLNC: STOPPED", this));
		}
	}
	else if ((playPos < _EndPosition) && (_EndState == NL_DSOUND_TAIL1))
	{
		// The play cursor wrapped around the buffer and is now before the end position
		_EndState = NL_DSOUND_TAIL2;
		DBGPOS(("[%p] FILL: ENDSTATE=%d, E=%d, P=%d", this, (int) _EndState, _EndPosition, playPos));
	}


	// Update the amount of silence written
	_SilenceWritten += bytes1 + bytes2;
	if (_SilenceWritten >= _SecondaryBufferSize)
	{
		// The buffer is now completely filled with silence
		_SecondaryBufferState = NL_DSOUND_SILENCED;
		DBGPOS(("[%p] SLNC: SILENCED", this));

		// If the buffer was playing, mark it as stopped
		if (_UserState == NL_DSOUND_PLAYING)
		{
			_UserState = NL_DSOUND_STOPPED;
			DBGPOS(("[%p] SLNC: STOPPED*", this));
		}
	}

	DBGPOS(("[%p] SLNC: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", this, playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));


#if NLSOUND_PROFILE
	_TotalUpdateSize += bytes1 + bytes2;
	_PosTime += CTime::ticksToSecond(startLock - startPos);
	_LockTime += CTime::ticksToSecond(startCopy - startLock);
	_CopyTime += CTime::ticksToSecond(startUnlock - startCopy);
	_UnlockTime += CTime::ticksToSecond(CTime::getPerformanceTime() - startUnlock);
	_CopyCount++;
#endif

	return true;
}
*/

// ******************************************************************

/**
 *	Calculate the size of the crossfade and set the pointer in the sample buffer
 *	to the sample that comes after the write cursor in the DirectSound buffer.
 *	This method also returns the number of samples that have been written after
 *	the write cursor.
 *
 */
/*
void CSourceDSound::getFadeOutSize(uint32 writePos, uint32 &xfadeSize, sint16* &in1, uint32 &writtenTooMuch)
{
	// The number of samples over which we will do the crossfade
	xfadeSize = _XFadeSize;


	// The tricky part of this method is to figger out how many samples of the old
	// buffer were written after the write cursor and figger our with what position
	// in the old buffer the write cursor corresponds. We have to consider the following
	// cases:
	//
	// - the old buffer just looped,
	// - the old buffer is finished, but stil has samples in the DirectSound buffer
	// - the default case, i.e. the old buffer is playing somewhere in the middle.
	//

	// How many bytes did we write after the write position?

	writtenTooMuch = NLSOUND_DISTANCE(writePos, _NextWritePos, _SecondaryBufferSize);

	DBGPOS(("[%p] FADE: TOO=%d", this, writtenTooMuch));


	uint8* data = ((CBufferDSound*) _Buffer)->getData();

	// If the sound is finished and all the samples are written in the
	// buffer, it's possible that there are still samples after the write
	// position. If this is the case, we have to do a fade out. If there is
	// only silence, we only need to copy without fade.

	if (_SilenceWritten > 0)
	{

		if (writtenTooMuch > _SilenceWritten)
		{
			writtenTooMuch -= _SilenceWritten;
			in1 = (sint16*) (data + _BufferSize - writtenTooMuch) ;

			if (writtenTooMuch < 2 * xfadeSize)
			{
				xfadeSize = writtenTooMuch / 2;
			}
		}
		else
		{
			xfadeSize = 0;
		}

		DBGPOS(("[%p] FADE: END, TOO=%d, S=%d, F=%d", this, writtenTooMuch, _SilenceWritten, xfadeSize));
	}

	// If the sound looped, it's possible that the number of samples
	// written is small. In that case, the write cursor is still inside
	// the previous loop. All we have to do is fade out the last part
	// of the previous loop.

	else if (writtenTooMuch >= _BytesWritten)
	{

		writtenTooMuch -= _BytesWritten;

		in1 = (sint16*) (data + _BufferSize - writtenTooMuch) ;

		if (writtenTooMuch < 2 * xfadeSize)
		{
			xfadeSize = writtenTooMuch / 2;
		}

		DBGPOS(("[%p] FADE: LOOPED, TOO=%d, F=%d", this, writtenTooMuch, xfadeSize));

	}

	// This is the default case. Simply fade from the previous to the next buffer.
	// The variable writtenTooMuch tells us how much of the previous sound has
	// been written after the current write cursor. We just have to check there
	// are enough samples available for the fade.

	else
	{
		in1 = (sint16*) (data + (sint32) _BytesWritten - writtenTooMuch);

		if (xfadeSize > _BufferSize - _BytesWritten)
		{
			xfadeSize = _BufferSize - _BytesWritten;
		}

		DBGPOS(("[%p] FADE: STD, TOO=%d, F=%d", this, writtenTooMuch, xfadeSize));
	}
}
*/


// ******************************************************************
/*
void CSourceDSound::crossFade()
{
	uint8 *ptr1, *ptr2;
	DWORD bytes1, bytes2;
	DWORD playPos, writePos;
	uint32 i;



	if (_Buffer == NULL)
	{
		return;
	}

	if (_SecondaryBuffer == 0)
	{
		return;
	}


	INITTIME(start);


	EnterCriticalSection(&_CriticalSection);

	TSampleFormat	sampleFormat;
	uint			freq;
	_Buffer->getFormat(sampleFormat, freq);


	// The source is currently playing another buffer. We will do a hot
	// swap between the old and the new buffer. DirectSound maintains two
	// cursors into the buffer: the play cursor and the write cursor.
	// The write cursor indicates where we can start writing the new samples.
	// To avoid clicks, we have to do a cross fade between the old buffer and
	// the new buffer.

	_SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);


	// The number of bytes we will write to the DirectSound buffer
	uint32 bytes = _SwapCopySize;
	if (bytes > _SwapBuffer->getSize())
	{
		bytes = _SwapBuffer->getSize();
	}


	// Lock the DirectSound buffer

	if (FAILED(_SecondaryBuffer->Lock(writePos, bytes, (LPVOID*) &ptr1, &bytes1, (LPVOID*) &ptr2, &bytes2, 0)))
	{
		LeaveCriticalSection(&_CriticalSection);
		throw ESoundDriver("Failed to lock the DirectSound secondary buffer");
	}


	sint16* in1;
	uint8* data1 = ((CBufferDSound*) _Buffer)->getData();
	uint8* data2 = ((CBufferDSound*) _SwapBuffer)->getData();
	sint16* in2 = (sint16*) data2;
	sint16* out = (sint16*) ptr1;

	// The number of samples over which we will do the crossfade
	uint32 xfadeSize;
	uint32 xfadeByteSize;
	uint32 writtenTooMuch;

	getFadeOutSize(writePos, xfadeSize, in1, writtenTooMuch);
	xfadeByteSize = 2 * xfadeSize;

#define MIX 1


#if MIX
	float incr, amp1, amp2;

	if (xfadeSize == 0)
	{
		amp1 = 0.0f;
		amp2 = 1.0f;
		incr = 0.0f;
	}
	else
	{
		amp1 = 1.0f;
		amp2 = 0.0f;
		incr = 1.0f / xfadeSize;
	}

#else
	float incr, amp1;

	if (xfadeSize == 0)
	{
		amp1 = 0.0f;
		incr = 0.0f;
	}
	else
	{
		amp1 = 1.0f;
		incr = 1.0f / xfadeSize;
	}

#endif


	// Start copying the samples


	// In the first case, the cross fade is completely contained in the first buffer
	// pointed to by ptr1.
	if (xfadeByteSize < bytes1)
	{

		// Do cross fade

		for (i = 0; i < xfadeSize; i++)
		{
#if MIX
			out[i] = (sint16) (amp1 * in1[i] + amp2 * in2[i]);
			amp1 -= incr;
			amp2 += incr;
#else
			out[i] = (sint16) (amp1 * in1[i]);
			amp1 -= incr;
#endif
		}

		// Copy remaining samples

#if MIX
//		CFastMem::memcpy(ptr1 + xfadeByteSize, data2 + xfadeByteSize, bytes1 - xfadeByteSize);
		copySampleTo16BitsTrack(ptr1 + xfadeByteSize, data2 + xfadeByteSize, (bytes1 - xfadeByteSize)/2, sampleFormat);

		_BytesWritten = bytes1;
#else
//		CFastMem::memcpy(ptr1 + xfadeByteSize, data2, bytes1 - xfadeByteSize);
		copySampleTo16BitsTrack(ptr1 + xfadeByteSize, data2, (bytes1 - xfadeByteSize)/2, sampleFormat);
		_BytesWritten = bytes1 - xfadeByteSize;
#endif

		if (ptr2)
		{
			//CFastMem::memcpy(ptr2, data2 + _BytesWritten, bytes2);
			copySampleTo16BitsTrack(ptr2, data2 + _BytesWritten, bytes2/2, sampleFormat);
			_BytesWritten += bytes2;
		}

	}

	// In the second case, the cross fade stretches over the first and the second buffers.
	else
	{

		uint32 fade1 = bytes1 / 2;
		uint32 fade2 = xfadeSize - fade1;

		// Do cross fade

		// Part 1, start at ptr1
		for (i = 0; i < fade1; i++)
		{
#if MIX
			out[i] = (sint16) (amp1 * in1[i] + amp2 * in2[i]);
			amp1 -= incr;
			amp2 += incr;
#else
			out[i] = (sint16) (amp1 * in1[i]);
			amp1 -= incr;
#endif
		}
#if MIX
		_BytesWritten = bytes1;
#else
		_BytesWritten = 0;
#endif


		if (ptr2)
		{
			out = (sint16*) ptr2;

			// Part 2, ontinue at ptr2
			for (uint32 k = 0; i < xfadeSize; i++, k++)
			{
#if MIX
				out[k] = (sint16) (amp1 * in1[i] + amp2 * in2[i]);
				amp1 -= incr;
				amp2 += incr;
#else
				out[k] = (sint16) (amp1 * in1[i]);
				amp1 -= incr;
#endif
			}

			// Copy remaining samples
#if MIX
//			CFastMem::memcpy(ptr2 + 2 * k, data2 + _BytesWritten + 2 * k, bytes2 - 2 * k);
			copySampleTo16BitsTrack(ptr2 + 2 * k, data2 + _BytesWritten + 2 * k, (bytes2 - 2 * k)/2, sampleFormat);
			_BytesWritten += bytes2;
#else
			//CFastMem::memcpy(ptr2 + 2 * k, data2, bytes2 - 2 * k);
			copySampleTo16BitsTrack(ptr2 + 2 * k, data2, (bytes2 - 2 * k)/2, sampleFormat);
			_BytesWritten = bytes2 - 2 * k;
#endif
		}
	}


	// Unlock the DirectSound buffer
	_SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);


	// Update the state variables

	_SilenceWritten = 0;

	_NextWritePos = (writePos + bytes1 + bytes2);
	if (_NextWritePos >= _SecondaryBufferSize)
	{
		_NextWritePos -= _SecondaryBufferSize;
	}



	_SecondaryBufferState = NL_DSOUND_FILLING;


	// Swap the two buffers
	_BufferSize = _SwapBuffer->getSize();
	_Buffer = _SwapBuffer;
	_SwapBuffer = 0;

	setPitch(_Freq);

	// Check if we've reached the end of the file
	if (_BytesWritten == _BufferSize)
	{
		if (_Loop)
		{
			_BytesWritten = 0;
		}
		else
		{
			_SecondaryBufferState = NL_DSOUND_SILENCING;
		}
	}


	DBGPOS(("[%p] XFADE", this));
	DBGPOS(("[%p] P=%d, W=%d, NW=%d, SZ=%d, BW=%d, B=%d", this, playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, bytes1 + bytes2));



	LeaveCriticalSection(&_CriticalSection);


#if NLSOUND_PROFILE
	_LastSwapTime = CTime::ticksToSecond(CTime::getPerformanceTime() - start);
	_TotalSwapTime += _LastSwapTime;
	_MaxSwapTime = (_LastSwapTime > _MaxSwapTime) ? _LastSwapTime : _MaxSwapTime;
	_MinSwapTime = (_LastSwapTime < _MinSwapTime) ? _LastSwapTime : _MinSwapTime;
	_SwapCount++;
#endif

}
*/


// ******************************************************************
/*
void CSourceDSound::fadeOut()
{
	uint8 *ptr1, *ptr2;
	DWORD bytes1, bytes2;
	DWORD playPos, writePos;
	uint32 i;


	if (_Buffer == NULL)
	{
		_SecondaryBufferState = NL_DSOUND_SILENCING;
		_UserState = NL_DSOUND_STOPPED;
		return;
	}

	if (_SecondaryBuffer == 0)
	{
		return;
	}

	INITTIME(start);


	_SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);


	// Lock the DirectSound buffer

	if (FAILED(_SecondaryBuffer->Lock(writePos, _SwapCopySize, (LPVOID*) &ptr1, &bytes1, (LPVOID*) &ptr2, &bytes2, 0)))
	{
		throw ESoundDriver("Failed to lock the DirectSound secondary buffer");
	}



	// in1 points to the position in the old buffer where the fade out starts
	sint16* in1;
	sint16* out = (sint16*) ptr1;

	// The number of samples over which we will do the crossfade
	uint32 xfadeSize;
	uint32 xfadeByteSize;
	uint32 writtenTooMuch;

	getFadeOutSize(writePos, xfadeSize, in1, writtenTooMuch);
	xfadeByteSize = 2 * xfadeSize;

	float amp1, incr;

	if (xfadeSize == 0)
	{
		amp1 = 0.0f;
		incr = 0.0f;
	}
	else
	{
		amp1 = 1.0f;
		incr = 1.0f / xfadeSize;
	}


	if (writtenTooMuch > _BytesWritten)
	{
		// The buffer looped. Count backwards from the end of the file.
		_BytesWritten = _BufferSize - writtenTooMuch;
	}
	else
	{
		_BytesWritten -= writtenTooMuch;
	}


	// Start copying the samples


	// In the first case, the fade out is completely contained in the first buffer
	// pointed to by ptr1.
	if (xfadeByteSize < bytes1)
	{

		// Do cross fade

		for (i = 0; i < xfadeSize; i++)
		{
			out[i] = (sint16) (amp1 * in1[i]);
			amp1 -= incr;
		}

		// Copy remaining samples

		memset(ptr1 + xfadeByteSize, 0, bytes1 - xfadeByteSize);
		_SilenceWritten = bytes1 - xfadeByteSize;

		if (ptr2)
		{
			memset(ptr2, 0, bytes2);
			_SilenceWritten += bytes2;
		}

	}

	// In the second case, the fade out stretches over the first and the second buffers.
	else
	{

		uint32 fade1 = bytes1 / 2;
		uint32 fade2 = xfadeSize - fade1;

		// Do cross fade

		// Part 1, start at ptr1
		for (i = 0; i < fade1; i++)
		{
			out[i] = (sint16) (amp1 * in1[i]);
			amp1 -= incr;
		}


		if (ptr2)
		{
			out = (sint16*) ptr2;

			// Part 2, continue at ptr2
			for (uint32 k = 0; i < xfadeSize; i++, k++)
			{
				out[k] = (sint16) (amp1 * in1[i]);
				amp1 -= incr;
			}

			// Clear remaining samples
			memset(ptr2 + 2 * k, 0, bytes2 - 2 * k);
			_SilenceWritten = bytes2 - 2 * k;
		}

	}


	_BytesWritten += xfadeByteSize;


	// Unlock the DirectSound buffer
	_SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);


	// Update the next write position
	_NextWritePos = (writePos + bytes1 + bytes2);
	if (_NextWritePos >= _SecondaryBufferSize)
	{
		_NextWritePos -= _SecondaryBufferSize;
	}


	_SecondaryBufferState = NL_DSOUND_SILENCING;
	DBGPOS(("[%p] FDOU: SILENCING", this));

	// Keep track of where tha last sample was written and the position
	// of the play cursor relative to the end position. if the _EndState
	// is 0, the play cursor is after the end position, 1 otherwise.
	_EndPosition = writePos + xfadeSize;
	if (_EndPosition >= _SecondaryBufferSize)
	{
		_EndPosition -= _SecondaryBufferSize;
	}

	_EndState = (playPos > _EndPosition)? NL_DSOUND_TAIL1 : NL_DSOUND_TAIL2;
	DBGPOS(("[%p] FDOU: ENDSTATE=%d, E=%d, P=%d", this, (int) _EndState, _EndPosition, playPos));


	DBGPOS(("[%p] FDOU: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", this, playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));



#if NLSOUND_PROFILE
	_LastSwapTime = CTime::ticksToSecond(CTime::getPerformanceTime() - start);
	_TotalSwapTime += _LastSwapTime;
	_MaxSwapTime = (_LastSwapTime > _MaxSwapTime) ? _LastSwapTime : _MaxSwapTime;
	_MinSwapTime = (_LastSwapTime < _MinSwapTime) ? _LastSwapTime : _MinSwapTime;
	_SwapCount++;
#endif

}
*/
// ******************************************************************
/*
void CSourceDSound::fadeIn()
{
	bool res = false;
	uint8 *ptr1, *ptr2;
	DWORD bytes1, bytes2;
	DWORD playPos, writePos;


	if (_Buffer == NULL)
	{
		_SecondaryBufferState = NL_DSOUND_SILENCING;
		_UserState = NL_DSOUND_STOPPED;
		return;
	}

	if (_SecondaryBuffer == 0)
	{
		return;
	}

	INITTIME(startPos);

	TSampleFormat	sampleFormat;
	uint			freq;
	_Buffer->getFormat(sampleFormat, freq);

	// Set the correct pitch for this sound
//	setPitch(_Freq);
//	setPitch(_SecondaryBuffer->GetFrequency());

	// Set the correct volume
	// FIXME: a bit of a hack
	const CVector &pos = CListenerDSound::instance()->getPos();
	updateVolume(pos);


	_SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);

	uint8* data = ((CBufferDSound*) _Buffer)->getData();
	uint32 available = (_BytesWritten < _BufferSize) ? _BufferSize - _BytesWritten : 0;
	uint32 bytes = NLSOUND_MIN(_SwapCopySize, available);
//	uint32 clear = _SwapCopySize - available;


	_SilenceWritten = 0;

	// Lock the buffer

	INITTIME(startLock);


	if (!lock(writePos, _SwapCopySize, ptr1, bytes1, ptr2, bytes2))
	{
		return;
	}


	INITTIME(startCopy);

	// Start copying the samples

	if (bytes1 <= bytes) {

//		CFastMem::memcpy(ptr1, data + _BytesWritten, bytes1);
		copySampleTo16BitsTrack(ptr1, data + _BytesWritten, bytes1/2, sampleFormat);
		_BytesWritten += bytes1;
		bytes -= bytes1;

		if (ptr2)
		{
			if (bytes > 0)
			{
				//CFastMem::memcpy(ptr2, data + _BytesWritten, bytes);
				copySampleTo16BitsTrack(ptr2, data + _BytesWritten, bytes/2, sampleFormat);
				_BytesWritten += bytes;
			}

			if (bytes < bytes2)
			{
				if (_Loop)
				{
					DBGPOS(("[%p] FDIN: LOOP", this));

					//CFastMem::memcpy(ptr2 + bytes, data, bytes2 - bytes);
					copySampleTo16BitsTrack(ptr2 + bytes, data, (bytes2 - bytes)/2, sampleFormat);
					_BytesWritten = bytes2 - bytes;
				}
				else
				{
					memset(ptr2 + bytes, 0, bytes2 - bytes);
					_SilenceWritten = bytes2 - bytes;
				}
			}
		}
	}
	else
	{
		if (bytes > 0)
		{
			//CFastMem::memcpy(ptr1, data + _BytesWritten, bytes);
			copySampleTo16BitsTrack(ptr1, data + _BytesWritten, bytes/2, sampleFormat);
			_BytesWritten += bytes;
		}

		if (_Loop)
		{
			DBGPOS(("[%p] FDIN: LOOP", this));

			//CFastMem::memcpy(ptr1 + bytes, data, bytes1 - bytes);
			copySampleTo16BitsTrack(ptr1 + bytes, data, (bytes1 - bytes) / 2, sampleFormat);
			_BytesWritten = bytes1 - bytes;

			if (ptr2)
			{
				//CFastMem::memcpy(ptr2, data + _BytesWritten, bytes2);
				copySampleTo16BitsTrack(ptr2, data + _BytesWritten, bytes2/2, sampleFormat);
				_BytesWritten += bytes2;
			}
		}
		else
		{
			memset(ptr1 + bytes, 0, bytes1 - bytes);
			_SilenceWritten = bytes1 - bytes;

			if (ptr2)
			{
				memset(ptr2, 0, bytes2);
				_SilenceWritten += bytes2;
			}
		}
	}


	INITTIME(startUnlock);

	// Unlock the buffer
	_SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);


	// Update the state variables

	_SecondaryBufferState = NL_DSOUND_FILLING;
	DBGPOS(("[%p] FDIN: FILLING", this));


	// Check if we've reached the end of the file
	if (_BytesWritten == _BufferSize)
	{
		if (_Loop)
		{
			// If we're looping, start all over again
			DBGPOS(("[%p] FDIN: LOOP", this));
			_BytesWritten = 0;
		}
		else
		{
			_SecondaryBufferState = NL_DSOUND_SILENCING;

			// Keep track of where tha last sample was written and the position
			// of the play cursor relative to the end position. if the _EndState
			// is NL_DSOUND_TAIL1, the play cursor is after the end position,
			// NL_DSOUND_TAIL2 otherwise.
			_EndPosition = writePos + bytes;
			if (_EndPosition >= _SecondaryBufferSize)
			{
				_EndPosition -= _SecondaryBufferSize;
			}

			_EndState = (playPos > _EndPosition)? NL_DSOUND_TAIL1 : NL_DSOUND_TAIL2;

			DBGPOS(("[%p] FDIN: SILENCING", this));
			DBGPOS(("[%p] FDIN: ENDSTATE=%d, E=%d, P=%d", this, (int) _EndState, _EndPosition, playPos));
		}
	}


	// Update the write pointer
	_NextWritePos = writePos + bytes1 + bytes2;
	if (_NextWritePos >= _SecondaryBufferSize)
	{
		_NextWritePos  -= _SecondaryBufferSize;
	}

	DBGPOS(("[%p] FDIN: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", this, playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));


#if NLSOUND_PROFILE
	_TotalUpdateSize += bytes1 + bytes2;
	_PosTime += CTime::ticksToSecond(startLock - startPos);
	_LockTime += CTime::ticksToSecond(startCopy - startLock);
	_CopyTime += CTime::ticksToSecond(startUnlock - startCopy);
	_UnlockTime += CTime::ticksToSecond(CTime::getPerformanceTime() - startUnlock);
	_CopyCount++;
#endif


}

*/

} // NLSOUND
