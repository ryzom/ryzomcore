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

#ifndef NL_U_PARTICLE_SYSTEM_SOUND_H
#define NL_U_PARTICLE_SYSTEM_SOUND_H

#include "nel/misc/types_nl.h"

#include "u_ps_sound_interface.h"
#include "u_ps_sound_impl.h"





namespace NL3D {

// if you include this, you must also have the NLSOUND library








/// for private use only..
void assignSoundServerToPS(UPSSoundServer *soundServer);


/**
 * This init the sound for particle systems
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class UParticleSystemSound
{
public:
	/// init the particle system sound with the given AudioMixer
	static void setPSSound(NLSOUND::UAudioMixer *audioMixer);
};


} // NL3D


#endif // NL_U_PARTICLE_SYSTEM_SOUND_H

/* End of u_particle_system_sound.h */
