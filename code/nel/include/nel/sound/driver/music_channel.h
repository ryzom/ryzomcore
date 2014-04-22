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

#ifndef NLSOUND_MUSIC_CHANNEL_H
#define NLSOUND_MUSIC_CHANNEL_H

#include "nel/misc/types_nl.h"

namespace NLSOUND
{

/**
 * \brief IMusicChannel
 * \date 2008-09-04 16:29GMT
 * \author Jan Boon (Kaetemi)
 * IMusicChannel
 */
class IMusicChannel
{
public:
	IMusicChannel() { }
	virtual ~IMusicChannel() { }

	/** Play some music (.ogg etc...)
	 *	NB: if an old music was played, it is first stop with stopMusic()
	 *	\param filepath file path, CPath::lookup is done here
	 *  \param async stream music from hard disk, preload in memory if false
	 *	\param loop must be true to play the music in loop. 
	 */
	virtual bool play(const std::string &filepath, bool async, bool loop) =0; 

	/// Stop the music previously loaded and played (the Memory is also freed)
	virtual void stop() =0;

	/// Makes sure any resources are freed, but keeps available for next play call
	virtual void reset() =0;

	/// Pause the music previously loaded and played (the Memory is not freed)
	virtual void pause() =0;
	
	/// Resume the music previously paused
	virtual void resume() =0;

	/// Return true if a song is finished.
	virtual bool isEnded() =0;

	/// Return true if the song is still loading asynchronously and hasn't started playing yet (false if not async), used to delay fading
	virtual bool isLoadingAsync() =0;
	
	/// Return the total length (in second) of the music currently played
	virtual float getLength() =0;
	
	/** Set the music volume (if any music played). (volume value inside [0 , 1]) (default: 1)
	 *	NB: the volume of music is NOT affected by IListener::setGain()
	 */
	virtual void setVolume(float gain) =0;

}; /* class IMusicChannel */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_MUSIC_CHANNEL_H */

/* end of file */
