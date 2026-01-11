// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_MUSIC_CHANNEL_FMOD_H
#define NL_MUSIC_CHANNEL_FMOD_H

#include "nel/sound/driver/music_channel.h"

struct FSOUND_STREAM;

namespace NLSOUND
{
	class CSoundDriverFMod;

/**
 * \brief CMusicChannelFMod
 * \date 2004
 * \author Lionel Berenguier
 * \author Nevrax France
 * A player of music in sound driver, allowing fade across music
 * \date 2008-09-05 13:10GMT
 * \author Jan Boon (Kaetemi)
 * Stuff for fading moved into nlsound, and modified for new music interface
 */
class CMusicChannelFMod : public IMusicChannel
{
protected:
	/// Volume set by user
	float _Gain;
	/// The FMod stream
	FSOUND_STREAM *_MusicStream;
	/// the RAM buffer (representation of a MP3 file, only for sync play)
	uint8 *_MusicBuffer;
	/// channel played for music. CAN BE -1 while _MusicStream!=NULL in case of Async Loading
	sint _MusicChannel;
	/// true if the fmod end callback said the stream is ended
	bool _CallBackEnded;
	/// see stopMusicFader()
	std::list<FSOUND_STREAM *> _WaitingForClose;
	/// Sound driver that created this
	CSoundDriverFMod *_SoundDriver;

public:
	/// Constructor
	CMusicChannelFMod(CSoundDriverFMod *soundDriver);
	virtual ~CMusicChannelFMod();

	/// From callback
	void markMusicChannelEnded(void *stream);

	/// Play async, if bnp give path of bnp and position and size of file inside, else just path to file.
	bool playAsync(const std::string &filepath, bool loop, uint fileOffset = 0, uint fileSize = 0);

	/// Play from memory.
	bool playSync(const std::string &filepath, bool loop);

	/// Play the stream
	bool playStream();

	/// updateWaitingForClose
	void update();

	/// Close async streams
	void updateWaitingForClose();

	/** Play some music (.ogg etc...)
	 *	NB: if an old music was played, it is first stop with stopMusic()
	 *	\param filepath file path, CPath::lookup is done here
	 *  \param async stream music from hard disk, preload in memory if false
	 *	\param loop must be true to play the music in loop. 
	 */
	virtual bool play(const std::string &filepath, bool async, bool loop); 

	/// Stop the music previously loaded and played (the Memory is also freed)
	virtual void stop();

	/// Makes sure any resources are freed, but keeps available for next play call
	virtual void reset();

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
}; /* class CMusicChannelFMod */

} /* namespace NLSOUND */

#endif /* #ifndef NL_MUSIC_CHANNEL_FMOD_H */

/* end of file */
