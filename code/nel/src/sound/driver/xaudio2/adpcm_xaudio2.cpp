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

#include "stdxaudio2.h"

// Project includes
#include "buffer_xaudio2.h"
#include "adpcm_xaudio2.h"

using namespace std;
// using namespace NLMISC;

namespace NLSOUND {

CAdpcmXAudio2::CAdpcmXAudio2(bool loop) 
: _SourceVoice(NULL), _SourceData(NULL), _SourceSize(0), _SampleRate(0), 
_Loop(loop), _BufferNext(0), _SampleNb(0), _AdpcmSize(0),
_AdpcmIndex(0), _LastBufferContext(0)
{
	_State.PreviousSample = 0;
	_State.StepIndex = 0;
	for (uint i = 0; i < _BufferNb; ++i) 
		_ValidBufferContext[i] = 0;
}

CAdpcmXAudio2::~CAdpcmXAudio2()
{
	
}

/// Submit the next ADPCM buffer, only 1 buffer can be submitted at a time!
void CAdpcmXAudio2::submitSourceBuffer(CBufferXAudio2 *buffer)
{
	_Mutex.enter();
	if (_SourceData) nlerror(NLSOUND_XAUDIO2_PREFIX "Only 1 ADPCM buffer can be submitted at a time! Call flushSourceBuffers first in CSourceXAudio2 stop().");
	_SourceSize = buffer->getSize();
	if (_SourceSize == 0)
	{
		nlwarning(NLSOUND_XAUDIO2_PREFIX "Empty ADPCM buffer!");
	}
	else
	{
		_AdpcmSize = buffer->getFrequency() / 40;
		_SampleNb = _AdpcmSize * 2;
		nlassert(_SampleNb < _BufferMax);
		_SourceData = buffer->getData();
		// nldebug("submit called!!!");
		processBuffers();
	}
	_Mutex.leave();
}

/// Reset the decoder, clear the queued buffer
void CAdpcmXAudio2::flushSourceBuffers()
{
	_Mutex.enter();
	_SourceData = NULL;
	for (uint i = 0; i < _BufferNb; ++i) 
		_ValidBufferContext[i] = 0;
	//_SourceSize = 0;
	_State.PreviousSample = 0;
	_State.StepIndex = 0;
	_AdpcmIndex = 0;
	//_PcmSize = 0;
	//_AdpcmSize = 0;
	_Mutex.leave();
}

void CAdpcmXAudio2::processBuffers()
{
	if (_SourceData)
	{
		XAUDIO2_VOICE_STATE voice_state;
		_SourceVoice->GetState(&voice_state);
		while (voice_state.BuffersQueued < _BufferNb)
		{
			// ADPCM = 4bit, PCM = 16bit // 1 adpcm byte = 2 samples
			
			uint maxinbytes = _SourceSize - _AdpcmIndex;
			uint inbytes = min(maxinbytes, _AdpcmSize);

			// nldebug("queue: %u", (uint32)voice_state.BuffersQueued);
			// nldebug("yeah, i'm decoding adpcm! %s, the size is %u and I'm at index %u, run %u, do %u bytes, source data %s!!!", _Loop ? "it's LOOPING" : "it's NOT looping", (uint32)_SourceSize, (uint32)_AdpcmIndex, (uint32)0, (uint32)inbytes, _SourceData ? "EXISTS" : "does NOT exist");

			if (inbytes > 0)
			{
				IBuffer::decodeADPCM(_SourceData + _AdpcmIndex, _Buffer[_BufferNext], inbytes * 2, _State);
				
				XAUDIO2_BUFFER xbuffer;
				xbuffer.AudioBytes = inbytes * 4;
				xbuffer.Flags = 0;
				xbuffer.LoopBegin = 0;
				xbuffer.LoopCount = 0;
				xbuffer.LoopLength = 0;
				xbuffer.pAudioData = (BYTE *)_Buffer[_BufferNext];
				xbuffer.pContext = (void *)(++_LastBufferContext);
				xbuffer.PlayBegin = 0;
				xbuffer.PlayLength = 0;
				_SourceVoice->SubmitSourceBuffer(&xbuffer);
				// nlwarning("submit %u", (uint32)xbuffer.pContext);

				_AdpcmIndex += inbytes;
				_BufferNext = (_BufferNext + 1) % _BufferNb;
				++voice_state.BuffersQueued;
			}

			if (inbytes != _AdpcmSize) // end of buffer
			{
				_State.PreviousSample = 0;
				_State.StepIndex = 0;
				_AdpcmIndex = 0;
				if (!_Loop)
				{
					_SourceData = NULL;
					return;
				}
			}

			nlassert(_SourceData);
		}
	}
}

void CAdpcmXAudio2::OnVoiceProcessingPassStart(UINT32 /* BytesRequired */)
{	
	
}

void CAdpcmXAudio2::OnVoiceProcessingPassEnd()
{ 

}

void CAdpcmXAudio2::OnStreamEnd()
{
	
}

void CAdpcmXAudio2::OnBufferStart(void *pBufferContext)
{
	// set a flag that this buffer has started
	// nlwarning("start %u", (uint32)pBufferContext);
	for (uint i = 0; i < _BufferNb; ++i)  if (!_ValidBufferContext[i])
	{
		_ValidBufferContext[i] = pBufferContext;
		return;
	}
	// nlwarning("No valid buffer context available for: %u", (uint32)pBufferContext);
}

void CAdpcmXAudio2::OnBufferEnd(void *pBufferContext)
{
	// verify if this buffer has started
	// nlwarning("end %u", (uint32)pBufferContext);
	for (uint i = 0; i < _BufferNb; ++i) if (_ValidBufferContext[i] == pBufferContext)
	{
		_ValidBufferContext[i] = 0;
		goto ProcessBuffers;
	}
	// nlwarning("Not a valid buffer context: %u", (uint32)pBufferContext);
	return;
ProcessBuffers:
	// The "correct" way would be to decode the buffer on a seperate thread,
	// but since ADPCM decoding should be really fast, there is no problem
	// with doing it here directly.
	_Mutex.enter();
	processBuffers();
	_Mutex.leave();
}

void CAdpcmXAudio2::OnLoopEnd(void * /* pBufferContext */)
{
	
}

void CAdpcmXAudio2::OnVoiceError(void * /* pBufferContext */, HRESULT /* Error */)
{ 
	
}

} /* namespace NLSOUND */

/* end of file */
