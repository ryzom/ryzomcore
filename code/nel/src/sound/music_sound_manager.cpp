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

#include "stdsound.h"

#include "nel/sound/audio_mixer_user.h"
#include "nel/sound/music_sound_manager.h"
#include "nel/sound/music_sound.h"
#include "nel/sound/music_source.h"


using namespace NLMISC;
using namespace std;


namespace NLSOUND {


// ***************************************************************************
CMusicSoundManager::CMusicSoundManager()
{
	_Enabled= true;
	_CurrentMusicPlaying= NULL;
	_PlayStartTime= INT_MIN;
	_TimeConstraintEnabled= true;
}

// ***************************************************************************
void		CMusicSoundManager::addMusicSourcePlaying(CMusicSource *musicSource)
{
	if(!musicSource)
		return;
	_Sources.insert(musicSource);
}

// ***************************************************************************
void		CMusicSoundManager::removeMusicSourcePlaying(CMusicSource *musicSource)
{
	_Sources.erase(musicSource);
	// may remove from the already played source
	_AlreadyPlayedSources.erase(musicSource);
}

// ***************************************************************************
void		CMusicSoundManager::update()
{
	// if disabled, then just quit
	if(!_Enabled)
		return;

	// update playing music each frame
	NLMISC::TTime		currentTime= CTime::getLocalTime();
	CAudioMixerUser		*mixer= CAudioMixerUser::instance();

	// **** First, see if the current music played is cut-able
	bool	canPlayNewMusic= true;
	// if the current played music has not ended his "minimum play time"
	if(_TimeConstraintEnabled && _CurrentMusicPlaying && currentTime<=_PlayStartTime+_CurrentMusicPlaying->getMinimumPlayTime())
		canPlayNewMusic= false;

	// if cannot play new music, continue the current one
	if(!canPlayNewMusic)
		return;

	// **** Search a music to replace the currently played one
	CMusicSound		*bestSound= _CurrentMusicPlaying;
	CMusicSource	*bestSource= NULL;
	std::set<CMusicSource*>::iterator	it= _Sources.begin();
	// for all possibles music sources
	for(;it!=_Sources.end();it++)
	{
		CMusicSource	*src= *it;
		CMusicSound		*snd= dynamic_cast<CMusicSound*>(src->getSound());
		// error, -> skip
		if(!snd)
			continue;
		// If the source was already played (this is not a loop sound), skip it
		// NB: It may be the current one but in this case it doesn't matters....
		if(_AlreadyPlayedSources.find(src)!=_AlreadyPlayedSources.end())
			continue;
		// verify that this sound can be played again from the last time it has been played
		if(_TimeConstraintEnabled && snd->LastStopTime>INT_MIN && currentTime<=snd->LastStopTime+snd->getTimeBeforeCanReplay())
			continue;
		// if no sound yet, take it
		if(!bestSound)
		{
			bestSource= src;
			bestSound= snd;
		}
		// else compare sound
		else
		{
			// take the higher priority (priority value is inversed: 0 is the best priority)
			if(snd->getPriority()<bestSound->getPriority())
			{
				bestSound= snd;
				bestSource= src;
			}
			// else, other criteria
			else if(snd->getPriority()==bestSound->getPriority())
			{
				/* if the new sound is not looping and the best is, consider the new sound as an "instant sound"
					which is prioritary
				*/
				if(!snd->getLooping() && bestSound->getLooping())
				{
					bestSound= snd;
					bestSource= src;
				}
				else if(snd->getLooping() == bestSound->getLooping())
				{
					// if the bestSound is the current sound played, prefer to not change the sound
					if(bestSound!=_CurrentMusicPlaying)
					{
						/* NB: here, bestSound can be != from _CurrentMusicPlaying in the following cases:
							- _CurrentMusicPlaying= NULL
							- bestSound was assigned to a higher priority sound than _CurrentMusicPlaying
								thereFore snd should be different from _CurrentMusicPlaying, since this one is of
								lower priority...
						 */
						// compare name to avoid full random jitter
						string	snd0= CStringMapper::unmap(bestSound->getFileName());
						string	snd1= CStringMapper::unmap(snd->getFileName());
						if(snd1>snd0)
						{
							bestSound= snd;
							bestSource= src;
						}
					}
				}
			}
		}
	}

	// if some new music found (different from the currently played one)
	if(bestSound && bestSound!= _CurrentMusicPlaying)
	{
		// then launch the new music
		startMusic(bestSound, bestSource);
	}
	// else, no new music found => if the music is currently playing
	else if(_CurrentMusicPlaying)
	{
		// if the music has ended (and not loop), stop
		if(_CurrentMusicPlaying->getLooping()==false && mixer->isMusicEnded())
		{
			// without fade (no need since ended)
			stopMusic(false);
		}
		else
		{
			// verify that a source with this sound still exist. If not, we have to cut this sound too
			bool	found= false;
			std::set<CMusicSource*>::iterator	it= _Sources.begin();
			for(;it!=_Sources.end();it++)
			{
				CMusicSource	*src= *it;
				CMusicSound		*snd= dynamic_cast<CMusicSound*>(src->getSound());
				if(snd && snd==_CurrentMusicPlaying)
				{
					found= true;
					break;
				}
			}
			// if not found, cut the music
			if(!found)
			{
				// with fade
				stopMusic(true);
			}
		}
	}
}

// ***************************************************************************
void		CMusicSoundManager::enable(bool enable)
{
	// if disabled, stop any music (without any fade)
	if(!enable)
		stopMusic(false);

	_Enabled= enable;
}

// ***************************************************************************
void	CMusicSoundManager::startMusic(CMusicSound *newMs, CMusicSource *newSrc)
{
	nlassert(newMs && newSrc);
	// fade with the current. Take the min of new FadeIn and old FadeOut
	sint32	xFade= newMs->getFadeInLength();
	if(_CurrentMusicPlaying)
		xFade= min(xFade, _CurrentMusicPlaying->getFadeOutLength());

	// start play the new music, xFade with the old
	CAudioMixerUser::instance()->playMusic(CStringMapper::unmap(newMs->getFileName()), uint(xFade), true, newMs->getLooping());

	// Mark the old one as stopped
	if(_CurrentMusicPlaying)
	{
		_CurrentMusicPlaying->LastStopTime= CTime::getLocalTime();
	}

	// update markers
	_CurrentMusicPlaying= newMs;
	_PlayStartTime= CTime::getLocalTime();

	// The source is played this time. Avoid replay it for infinite time if the player stay in the zone
	if(!newMs->getLooping())
		_AlreadyPlayedSources.insert(newSrc);
}

// ***************************************************************************
void	CMusicSoundManager::stopMusic(bool allowFade)
{
	if(_CurrentMusicPlaying)
	{
		// stop with or without fadeout
		if(allowFade)
			CAudioMixerUser::instance()->stopMusic(_CurrentMusicPlaying->getFadeOutLength());
		else
			CAudioMixerUser::instance()->stopMusic(0);
		// Mark the last stop time
		_CurrentMusicPlaying->LastStopTime= CTime::getLocalTime();
		// no more music playing
		_CurrentMusicPlaying= NULL;
	}
}


} // NLSOUND
