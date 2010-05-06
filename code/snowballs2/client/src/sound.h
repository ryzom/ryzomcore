/** \file sound.h
 * Sound interface between the game and NeL
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

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

//#define SBCLIENT_MUSIC_WAIT (0), (0)
//#define SBCLIENT_MUSIC_LOGIN (1), (0)
//#define SBCLIENT_MUSIC_BACKGROUND (2), (0)
//#define SBCLIENT_MUSIC_BACKGROUND_BEAT (2), (1)

//
// External variables
//

//extern NLSOUND::UAudioMixer *AudioMixer;
//extern NLSOUND::TSoundId SoundId;
//
////
//// External functions
////
//
//void playMusic(sint32 playlist, sint32 track);
//void setMusicVolume(sint32 playlist, float volume);
//
//void initSound();
//void updateSound();
//void releaseSound();
//
//// Set and play a sound on an entity
//void playSound(CEntity &entity, NLSOUND::TSoundId id);
//
//// Remove the sound system link to the entity
//void deleteSound(CEntity &entity);

#endif // SBCLIENT_SOUND_H

/* End of sound.h */ // duh
