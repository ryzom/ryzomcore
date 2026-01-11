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

#include "std3d.h"



//#include "nel/3d/u_particle_system_sound.h" we don't include this to avoid a link with NLSOUND
#include "nel/3d/particle_system.h"
#include "nel/3d/u_ps_sound_interface.h"
#include "nel/3d/u_ps_sound_impl.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

void assignSoundServerToPS(UPSSoundServer *soundServer)
{
	CParticleSystem::registerSoundServer(soundServer);
} // NL3D


/// init the particle system sound with the given AudioMixer
void UParticleSystemSound::setPSSound(NLSOUND::UAudioMixer *audioMixer)
{
	static CPSSoundServImpl soundServer;
	soundServer.init(audioMixer);
	if (audioMixer)
	{
		assignSoundServerToPS(&soundServer);
	}
	else
	{
		assignSoundServerToPS(NULL);
	}
}


void CPSSoundInstanceImpl::release(void)
{
	if (!_Spawned) // remove this source from the audio mixer if it hasn't been spawned
	{
		if (_SoundServImpl->getAudioMixer())
		{
			//			_SoundServImpl->getAudioMixer()->removeSource(_Source);
			delete _Source;
		}
	}
	else
	{
		if (_Source) // tells this spawned source not to notify us when it ends
		{
			_Source->unregisterSpawnCallBack();
		}
	}
	delete this;
}



}

/* End of particle_system_sound_user.cpp */
