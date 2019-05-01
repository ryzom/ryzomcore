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

#ifndef SBCLIENT_SOUND_H
#define SBCLIENT_SOUND_H

//
// Includes
//

#include <nel/misc/types_nl.h>
#include <nel/misc/config_file.h>

#include <nel/sound/u_audio_mixer.h>
#include <nel/sound/u_listener.h>
#include <nel/sound/u_source.h>

#include "snowballs_client.h"
#include "entities.h"

// 
// Defines
// 

#define SBCLIENT_MUSIC_WAIT "xtarsia_evil-snowballs_wait.ogg"
#define SBCLIENT_MUSIC_LOGIN "xtarsia_evil-snowballs_login.ogg"
#define SBCLIENT_MUSIC_BACKGROUND "xtarsia_evil-snowballs_beat.ogg" /* "xtarsia_evil-snowballs_game.ogg" */
#define SBCLIENT_MUSIC_BACKGROUND_BEAT "xtarsia_evil-snowballs_beat.ogg"

namespace SBCLIENT {

//
// External variables
//

#ifdef SBCLIENT_WITH_SOUND
extern NLSOUND::UAudioMixer *AudioMixer;
//extern NLSOUND::TSoundId SoundId;
#endif

//
// External functions
//

//void playMusic(sint32 playlist, sint32 track);
//void setMusicVolume(sint32 playlist, float volume);

void initSound();
void updateSound();
void releaseSound();

void playMusic(const char *file);

//// Set and play a sound on an entity
//void playSound(CEntity &entity, NLSOUND::TSoundId id);

//// Remove the sound system link to the entity
//void deleteSound(CEntity &entity);

} /* namespace SBCLIENT */

#endif // SBCLIENT_SOUND_H

/* End of sound.h */ // duh
