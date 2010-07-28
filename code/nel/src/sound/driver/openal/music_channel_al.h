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

#ifndef NLSOUND_MUSIC_CHANNEL_AL_H
#define NLSOUND_MUSIC_CHANNEL_AL_H

#include "nel/sound/driver/music_channel.h"

namespace NLSOUND
{
	class CSoundDriverAL;
	class IMusicBuffer;

/**
 * \brief CMusicChannelAL
 * \date 2010-07-27 16:56GMT
 * \author Kervala
 * CMusicChannelAL is an implementation of the IMusicChannel interface to run on OpenAL.
 */
class CMusicChannelAL : public IMusicChannel, public NLMISC::IRunnable
{
protected:
	// outside pointers
	CSoundDriverAL*		_SoundDriver;
	
	// pointers
	IMusicBuffer*		_MusicBuffer;
	NLMISC::IThread*	_Thread;

	IBuffer*			_Buffer;
	CSourceAL*			_Source;
	bool				_Playing;
	bool				_Async;

	float				_Gain;

	/// Fill IBuffer with data from IMusicBuffer
	bool fillBuffer(IBuffer *buffer, uint length);

	/// Use buffer format from IMusicBuffer
	void setBufferFormat(IBuffer *buffer);

	/// Declared in NLMISC::IRunnable interface
	virtual void run();

public:
	CMusicChannelAL(CSoundDriverAL *soundDriver);
	virtual ~CMusicChannelAL();
	void release();

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
	 *	NB: in OpenAL driver, the volume of music IS affected by IListener::setGain()
	 */
	virtual void setVolume(float gain);

	/// Play sync music
	bool playSync();

	/// Update music
	void update();
}; /* class CMusicChannelAL */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_MUSIC_CHANNEL_AL_H */

/* end of file */
