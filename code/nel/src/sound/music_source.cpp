// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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

#include "stdsound.h"

#include "nel/sound/music_source.h"
#include "nel/sound/music_sound.h"
#include "nel/sound/audio_mixer_user.h"
#include "nel/sound/music_sound_manager.h"


namespace NLSOUND {


// ***************************************************************************
CMusicSource::CMusicSource(CMusicSound *musicSound, bool spawn, TSpawnEndCallback cb, void *cbUserParam, NL3D::CCluster *cluster, CGroupController *groupController)
	:	CSourceCommon(musicSound, spawn, cb, cbUserParam, cluster, groupController)
{
	_MusicSound= musicSound;
}

// ***************************************************************************
CMusicSource::~CMusicSource()
{
	if(isPlaying())
		stop();

	// avoid any bug, ensure the source is removed
	CAudioMixerUser::instance()->getBackgroundMusicManager()->removeMusicSourcePlaying(this);
}

// ***************************************************************************
TSoundId			CMusicSource::getSound()
{
	return _MusicSound;
}

// ***************************************************************************
void				CMusicSource::play()
{
	// if already playing, no-op (don't restart)
	if(isPlaying())
		return;

	// append and play common
	CAudioMixerUser::instance()->getBackgroundMusicManager()->addMusicSourcePlaying(this);
	CSourceCommon::play();
}

// ***************************************************************************
void				CMusicSource::stop()
{
	// if already non-playing, no-op (don't restop)
	if(!isPlaying())
		return;

	// remove and stop common
	CAudioMixerUser::instance()->getBackgroundMusicManager()->removeMusicSourcePlaying(this);
	CSourceCommon::stop();
}


} // NLSOUND
