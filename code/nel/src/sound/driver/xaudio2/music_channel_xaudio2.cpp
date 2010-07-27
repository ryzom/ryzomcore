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
#include "sound_driver_xaudio2.h"
#include "music_channel_xaudio2.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

CMusicChannelXAudio2::CMusicChannelXAudio2(CSoundDriverXAudio2 *soundDriver) 
: _MusicBuffer(NULL), _SourceVoice(NULL), _BufferPos(0), _SoundDriver(soundDriver), _Gain(1.0)
{
	nlwarning(NLSOUND_XAUDIO2_PREFIX "Initializing CMusicChannelXAudio2");
	
	stop();
}

CMusicChannelXAudio2::~CMusicChannelXAudio2()
{
	release();
	if (_SoundDriver) { _SoundDriver->removeMusicChannel(this); _SoundDriver = NULL; }
	
	nlwarning(NLSOUND_XAUDIO2_PREFIX "Destroying CMusicChannelXAudio2");
}

void CMusicChannelXAudio2::release()
{
	nlwarning(NLSOUND_XAUDIO2_PREFIX "Releasing CMusicChannelXAudio2");
	
	stop();
}

/** Play some music (.ogg etc...)
 *	NB: if an old music was played, it is first stop with stopMusic()
 *	\param filepath file path, CPath::lookup is done here
 *  \param async stream music from hard disk, preload in memory if false
 *	\param loop must be true to play the music in loop. 
 */
bool CMusicChannelXAudio2::play(const std::string &filepath, bool async, bool loop)
{
	// nlinfo(NLSOUND_XAUDIO2_PREFIX "play %s %u", filepath.c_str(), (uint32)loop);

	_SoundDriver->performanceIncreaseMusicPlayCounter();

	HRESULT hr;

	stop();

	_MusicBuffer = IMusicBuffer::createMusicBuffer(filepath, async, loop);
	
	if (_MusicBuffer)
	{
		WAVEFORMATEX wfe;
		wfe.cbSize = 0;
		wfe.wFormatTag = WAVE_FORMAT_PCM; // todo: getFormat();
		wfe.nChannels = _MusicBuffer->getChannels();
		wfe.wBitsPerSample = _MusicBuffer->getBitsPerSample();
		wfe.nSamplesPerSec = _MusicBuffer->getSamplesPerSec();
		wfe.nBlockAlign = wfe.nChannels * wfe.wBitsPerSample / 8;
		wfe.nAvgBytesPerSec = wfe.nSamplesPerSec * wfe.nBlockAlign;

		XAUDIO2_VOICE_DETAILS voice_details;
		_SoundDriver->getMasteringVoice()->GetVoiceDetails(&voice_details);

		// nlinfo(NLSOUND_XAUDIO2_PREFIX "Creating music voice with %u channels, %u bits per sample, %u samples per sec, "
		// 	"on mastering voice with %u channels, %u samples per sec", 
		// 	(uint32)wfe.nChannels, (uint32)wfe.wBitsPerSample, (uint32)wfe.nSamplesPerSec, 
		// 	(uint32)voice_details.InputChannels, (uint32)voice_details.InputSampleRate);

		if (FAILED(hr = _SoundDriver->getXAudio2()->CreateSourceVoice(&_SourceVoice, &wfe, XAUDIO2_VOICE_NOPITCH, 1.0f, this, NULL, NULL)))
		{ 
			nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED CreateSourceVoice"); 
			stop(); return false; 
		}

		_SourceVoice->SetVolume(_Gain);
		_SourceVoice->Start(0);
	}
	else
	{
		nlwarning(NLSOUND_XAUDIO2_PREFIX "no _MusicBuffer");
		return false;
	}
	
	return true;
}

/// Stop the music previously loaded and played (the Memory is also freed)
void CMusicChannelXAudio2::stop()
{
	if (_SourceVoice) { _SourceVoice->DestroyVoice(); _SourceVoice = NULL; }
	if (_MusicBuffer) { delete _MusicBuffer; _MusicBuffer = NULL; }
	// memset(_Buffer, 0, sizeof(_Buffer));
	_BufferPos = 0;
}

/** Pause the music previously loaded and played (the Memory is not freed)
 */
void CMusicChannelXAudio2::pause()
{
	if (_SourceVoice) _SourceVoice->Stop(0);
}

/// Resume the music previously paused
void CMusicChannelXAudio2::resume()
{
	if (_SourceVoice) _SourceVoice->Start(0);
}

/// Return true if a song is finished.
bool CMusicChannelXAudio2::isEnded()
{
	if (_MusicBuffer)
	{
		if (!_MusicBuffer->isMusicEnded())
			return false;
	}
	if (_SourceVoice)
	{
		XAUDIO2_VOICE_STATE voice_state;
		_SourceVoice->GetState(&voice_state);
		if (voice_state.BuffersQueued)
		{
			// nldebug(NLSOUND_XAUDIO2_PREFIX "isEnded() -> voice_state.BuffersQueued, wait ...");
			return false;
		}
	}
	// nldebug(NLSOUND_XAUDIO2_PREFIX "isEnded() -> stop()");
	stop();
	return true;
}

/// Return true if the song is still loading asynchronously and hasn't started playing yet (false if not async), used to delay fading
bool CMusicChannelXAudio2::isLoadingAsync()
{
	return false;
}

/// Return the total length (in second) of the music currently played
float CMusicChannelXAudio2::getLength()
{
	if (_MusicBuffer) return _MusicBuffer->getLength(); 
	else return .0f;
}

/** Set the music volume (if any music played). (volume value inside [0 , 1]) (default: 1)
 *	NB: the volume of music is NOT affected by IListener::setGain()
 */
void CMusicChannelXAudio2::setVolume(float gain)
{
	_Gain = gain;
	if (_SourceVoice) _SourceVoice->SetVolume(gain);
}

void CMusicChannelXAudio2::OnVoiceProcessingPassStart(UINT32 BytesRequired)
{	
	if (BytesRequired > 0)
	{
		// nlwarning(NLSOUND_XAUDIO2_PREFIX "Bytes Required: %u", BytesRequired); // byte req to not have disruption
		
		if (_MusicBuffer)
		{
			uint32 minimum = BytesRequired * 2; // give some more than required :p
			if (_MusicBuffer->getRequiredBytes() > minimum) minimum = _MusicBuffer->getRequiredBytes();
			if (minimum > sizeof(_Buffer) - _BufferPos) _BufferPos = 0;
			uint32 maximum = sizeof(_Buffer) - _BufferPos;
			uint8 *buffer = &_Buffer[_BufferPos];
			uint32 length = _MusicBuffer->getNextBytes(buffer, minimum, maximum);
			_BufferPos += length;

			if (length)
			{
				XAUDIO2_BUFFER xbuffer;
				xbuffer.AudioBytes = length;
				xbuffer.Flags = 0;
				xbuffer.LoopBegin = 0;
				xbuffer.LoopCount = 0;
				xbuffer.LoopLength = 0;
				xbuffer.pAudioData = buffer;
				xbuffer.pContext = NULL; // nothing here for now
				xbuffer.PlayBegin = 0;
				xbuffer.PlayLength = 0;

				_SourceVoice->SubmitSourceBuffer(&xbuffer);
			}
			else
			{
				// nldebug(NLSOUND_XAUDIO2_PREFIX "!length -> delete _MusicBuffer");
				// set member var to null before deleting it to avoid crashing main thread
				IMusicBuffer *music_buffer = _MusicBuffer;
				_MusicBuffer = NULL; delete music_buffer;
				_SourceVoice->Discontinuity();
			}
		}
	}
}

void CMusicChannelXAudio2::OnVoiceProcessingPassEnd()
{ 
	
}

void CMusicChannelXAudio2::OnStreamEnd()
{ 
	
}

void CMusicChannelXAudio2::OnBufferStart(void * /* pBufferContext */)
{	
	
}

void CMusicChannelXAudio2::OnBufferEnd(void * /* pBufferContext */)
{ 
	
}

void CMusicChannelXAudio2::OnLoopEnd(void * /* pBufferContext */)
{	
	
}

void CMusicChannelXAudio2::OnVoiceError(void * /* pBufferContext */, HRESULT /*  Error */)
{ 
	
}

} /* namespace NLSOUND */

/* end of file */
