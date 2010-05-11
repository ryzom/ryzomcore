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

#ifndef NLSOUND_MUSIC_CHANNEL_XAUDIO2_H
#define NLSOUND_MUSIC_CHANNEL_XAUDIO2_H
#include "stdxaudio2.h"

// STL includes

// NeL includes
#include "nel/sound/driver/music_channel.h"

// Project includes

namespace NLSOUND {
	class CSoundDriverXAudio2;
	class IMusicBuffer;

/**
 * \brief CMusicChannelXAudio2
 * \date 2008-08-30 13:31GMT
 * \author Jan Boon (Kaetemi)
 * CMusicChannelXAudio2 is an implementation of the IMusicChannel interface to run on XAudio2.
 * TODO: Properly decode the audio on a seperate thread.
 * TODO: Change IMusicChannel to IAudioStream, and fix the interface to make more sense.
 */
class CMusicChannelXAudio2 : public IMusicChannel, IXAudio2VoiceCallback
{
protected:
	// outside pointers
	CSoundDriverXAudio2 *_SoundDriver;
	
	// pointers
	IMusicBuffer *_MusicBuffer;
	IXAudio2SourceVoice *_SourceVoice;
	// todo: thread for async loading of music buffer and source voice
	// isasyncloading checks if thread exists
	// isended checks if not async loading too
	
	// instances
	uint8 _Buffer[64 * 1024]; // no specific reason, lol
	uint32 _BufferPos; // 0
	float _Gain;

public:
	CMusicChannelXAudio2(CSoundDriverXAudio2 *soundDriver);
	virtual ~CMusicChannelXAudio2();
	void release();

private:
	// XAudio2 Callbacks
	// Called just before this voice's processing pass begins.
	STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired);
	// Called just after this voice's processing pass ends.
	STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS);
	// Called when this voice has just finished playing a buffer stream
	// (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
	STDMETHOD_(void, OnStreamEnd) (THIS);
	// Called when this voice is about to start processing a new buffer.
	STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext);
	// Called when this voice has just finished processing a buffer.
	// The buffer can now be reused or destroyed.
	STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext);
	// Called when this voice has just reached the end position of a loop.
	STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext);
	// Called in the event of a critical error during voice processing,
	// such as a failing XAPO or an error from the hardware XMA decoder.
	// The voice may have to be destroyed and re-created to recover from
	// the error.  The callback arguments report which buffer was being
	// processed when the error occurred, and its HRESULT code.
	STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error);

public:
	/** Play some music (.ogg etc...)
	 *	NB: if an old music was played, it is first stop with stopMusic()
	 *	\param filepath file path, CPath::lookup is done here
	 *  \param async stream music from hard disk, preload in memory if false
	 *	\param loop must be true to play the music in loop. 
	 */
	virtual bool play(const std::string &filepath, bool async, bool loop); 

	/// Stop the music previously loaded and played (the Memory is also freed)
	virtual void stop();

	/// Pause the music previously loaded and played (the Memory is not freed)
	virtual void pause();
	
	/// Resume the music previously paused
	virtual void resume();

	/// Return true if a song is finished.
	virtual bool isEnded();

	/// Return true if the song is still loading asynchronously and hasn't started playing yet (false if not async), used to delay fading
	virtual bool isLoadingAsync();
	
	/// Return the total length (in second) of the music currently played
	virtual float getLength();
	
	/** Set the music volume (if any music played). (volume value inside [0 , 1]) (default: 1)
	 *	NB: the volume of music is NOT affected by IListener::setGain()
	 */
	virtual void setVolume(float gain);
}; /* class CMusicChannelXAudio2 */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_MUSIC_CHANNEL_XAUDIO2_H */

/* end of file */
