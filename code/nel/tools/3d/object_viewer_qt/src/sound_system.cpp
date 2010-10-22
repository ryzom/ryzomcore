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


#include "stdpch.h"
#include "sound_system.h"

// NeL includes
#include <nel/sound/u_audio_mixer.h>
#include <nel/sound/u_listener.h>
#include <nel/sound/sound_anim_manager.h>
#include <nel/sound/sound_animation.h>
#include <nel/3d/u_particle_system_sound.h>
#include <nel/misc/path.h>

// Project includes
#include "modules.h"

namespace NLQT {

CSoundSystem::CSoundSystem()
		:_AudioMixer(NULL),
		_AnimManager(NULL),
		_Zero(NLMISC::CVector::Null)
{
	
}

CSoundSystem::~CSoundSystem()
{
}

void CSoundSystem::setListenerMatrix(const NLMISC::CMatrix &m)
{
	if (_AudioMixer)
	{
		if(m.getPos() != oldMatrix.getPos() || m.getJ() != oldMatrix.getJ() || m.getK() != oldMatrix.getK())
		{
			NLSOUND::UListener *l = _AudioMixer->getListener();
			l->setPos(m.getPos());
			l->setOrientation(m.getJ(), m.getK());
			oldMatrix = m;
		}
	}
}

void CSoundSystem::init ()
{
  	//H_AUTO2
	nldebug("CSoundSystem::init");
	
	// create audiomixer
	_AudioMixer = NULL;
	_AnimManager = NULL;
	NL3D::UParticleSystemSound::setPSSound(NULL);
	_AudioMixer = NLSOUND::UAudioMixer::createAudioMixer();
	nlassert(_AudioMixer);

	try
	{ 
		// init audiomixer
		_PackedSheetPath = Modules::config().getValue("SoundPackedSheetPath", std::string(""));
		_SamplePath = Modules::config().getValue("SoundSamplePath", std::string(""));
		_AudioMixer->setSamplePath(_SamplePath);
		_AudioMixer->setPackedSheetOption(_PackedSheetPath, true);
		std::vector<std::string> devices;
		_AudioMixer->initDriver(Modules::config().getValue("SoundDriver", std::string("Auto")));
		_AudioMixer->getDevices(devices);
		NLSOUND::UAudioMixer::CInitInfo audioInfo;
		audioInfo.AutoLoadSample = Modules::config().getValue("SoundAutoLoadSample", true);
		audioInfo.EnableOccludeObstruct = Modules::config().getValue("SoundEnableOccludeObstruct", true);
		audioInfo.EnableReverb = Modules::config().getValue("SoundEnableReverb", true);
		audioInfo.ManualRolloff = Modules::config().getValue("SoundManualRolloff", true);
		audioInfo.ForceSoftware = Modules::config().getValue("SoundForceSoftware", false);
		audioInfo.MaxTrack = Modules::config().getValue("SoundMaxTrack", 48);
		audioInfo.UseADPCM = Modules::config().getValue("SoundUseADPCM", false);
		_AudioMixer->initDevice(Modules::config().getValue("SoundDevice", std::string("")), audioInfo, NULL);
		_AudioMixer->setLowWaterMark(1);
	}
	catch(NLMISC::Exception &e)
	{
		// in case of exeption during mixer init, the mixer is destroyed !
		std::string mess = std::string("Unable to init sound :") + e.what();
		nlwarning ("Init sound: %s", mess.c_str());
		_AudioMixer = NULL;
		return;
	}

	// ok for the mixer, now create the sound anim manager
	try
	{
		// TODO : boris : Hum, as far as I know, throwing exeption in constructor is a very BAD idea...
		_AnimManager = new NLSOUND::CSoundAnimManager(_AudioMixer);
	}
	catch (NLMISC::Exception &e)
	{
		std::string mess = std::string("Unable to init sound :") + e.what();
		nlwarning ("Init sound: %s", mess.c_str());
		delete _AudioMixer;
		_AudioMixer = NULL;

		return;
	}
	NL3D::UParticleSystemSound::setPSSound(_AudioMixer);
}

void CSoundSystem::release(void)
{
	//H_AUTO2
	nldebug("CSoundSystem::release");

	NL3D::UParticleSystemSound::setPSSound(NULL);
	if (_AnimManager)
	{
		delete _AnimManager;
		_AnimManager = NULL;
	}
	if (_AudioMixer)
	{
		delete _AudioMixer;
		_AudioMixer = NULL;
	}
}

void CSoundSystem::play(const std::string &soundName)
{
	if (_AudioMixer)
	{
		NLSOUND::USource *src = _AudioMixer->createSource(NLMISC::CStringMapper::map(soundName), true);
		if (src)
		{
			src->setLooping(false);
			const NLMISC::CVector &pos = _AudioMixer->getListener()->getPos();
			src->setPos(pos);
			src->play();
		}
		else
		{
			nlwarning("Can't play the sound (perhaps it's contextual sound)");
		}
	}	
}

NLSOUND::USource *CSoundSystem::create(const std::string &soundName)
{
	if (_AudioMixer)
	{
		NLSOUND::USource *src = _AudioMixer->createSource(NLMISC::CStringMapper::map(soundName), false);
		if (src)
		{
			src->setLooping(false);
			const NLMISC::CVector &pos = _AudioMixer->getListener()->getPos();
			src->setPos(pos);
			src->play();
			return src;
		}	
		else
		{
			nlwarning("Can't play the sound (perhaps it's contextual sound)");
		}	return NULL;
	}
	return NULL;
}	

void CSoundSystem::playAnimation(std::string& name, float lastTime, float curTime, NLSOUND::CSoundContext &context)
{
	if (_AnimManager == NULL)
	{
		return;
	}

	NLSOUND::TSoundAnimId id = _AnimManager->getAnimationFromName(name);

	if (id != NLSOUND::CSoundAnimationNoId)
	{
		_AnimManager->playAnimation(id, lastTime, curTime, NULL, context);
	}
}

void CSoundSystem::update()
{
	if (_AudioMixer)
	{
		_AudioMixer->update();
	}
}

void CSoundSystem::initGraphics()
{
	//H_AUTO2
	nldebug("CSoundSystem::initGraphics");

	// set particle system sound
	NL3D::UParticleSystemSound::setPSSound(_AudioMixer);

	// ...
	// todo: displayers for all the test sound sources :)
}

void CSoundSystem::releaseGraphics()
{
	//H_AUTO2
	nldebug("CSoundSystem::releaseGraphics");

	// ..
	
	// clear particle system sound
	NL3D::UParticleSystemSound::setPSSound(NULL);
}

} /* namespace NLQT */