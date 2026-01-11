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

#ifndef NL_MUSIC_SOUND_MANAGER_H
#define NL_MUSIC_SOUND_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"


namespace NLSOUND
{

class CMusicSource;
class CMusicSound;

// ***************************************************************************
/**
 * A manager that manage multiple CMusicSound that want to play at the same time
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class CMusicSoundManager
{
public:

	/// Constructor
	CMusicSoundManager();

	/// Add a source that want to play music. Called by CMusicSource::play()
	void		addMusicSourcePlaying(CMusicSource *musicSource);
	/// Remove a source that want to play music. Called by CMusicSource::stop()
	void		removeMusicSourcePlaying(CMusicSource *musicSource);

	/// update concurrent music
	void		update();

	/// enable or disable the music sound manager (disable it if you want to play user MP3 for instance)
	void		enable(bool enable);

	/// set false to avoid getMinimumPlayTime() and getTimeBeforeCanReplay() behavior. default to true
	void		enableTimeConstraint(bool enable) {_TimeConstraintEnabled= enable;}

private:
	bool						_Enabled;
	bool						_TimeConstraintEnabled;
	// The current music sound actually playing
	CMusicSound					*_CurrentMusicPlaying;
	NLMISC::TTime				_PlayStartTime;
	// All music sources that want to play. Only one can really play at a time (in _CurrentMusicPlaying)
	std::set<CMusicSource*>		_Sources;
	// The sources that were already played in the current zones. Only not looped sounds can be in this set
	std::set<CMusicSource*>		_AlreadyPlayedSources;

	void	startMusic(CMusicSound *, CMusicSource *);
	void	stopMusic(bool allowFade);
};


} // NLSOUND


#endif // NL_MUSIC_SOUND_MANAGER_H

/* End of music_sound_manager.h */
