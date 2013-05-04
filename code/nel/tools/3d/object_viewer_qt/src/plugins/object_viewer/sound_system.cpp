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

// Project includes
#include "modules.h"
#include "object_viewer_constants.h"
#include "../core/icore.h"

// NeL includes
#include <nel/sound/u_audio_mixer.h>
#include <nel/sound/u_listener.h>
#include <nel/sound/sound_anim_manager.h>
#include <nel/sound/sound_animation.h>
#include <nel/3d/u_particle_system_sound.h>
#include <nel/misc/path.h>
#include <nel/misc/sheet_id.h>


// Qt includes
#include <QtCore/QSettings>

namespace NLQT
{

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

void CSoundSystem::init()
{
	//H_AUTO2
	nldebug("CSoundSystem::init");
	
	// require sheet id without sheet id bin
	NLMISC::CSheetId::initWithoutSheet();

	// create audiomixer
	_AudioMixer = NULL;
	_AnimManager = NULL;
	NL3D::UParticleSystemSound::setPSSound(NULL);
	_AudioMixer = NLSOUND::UAudioMixer::createAudioMixer();
	nlassert(_AudioMixer);

	try
	{
		// init audiomixer
		QSettings *settings = Core::ICore::instance()->settings();
		settings->beginGroup(Constants::OBJECT_VIEWER_SECTION);

		_PackedSheetPath = settings->value(Constants::SOUND_PACKED_SHEET_PATH, "").toString().toUtf8().constData();
		_SamplePath = settings->value(Constants::SOUND_SAMPLE_PATH, "").toString().toUtf8().constData();
		_AudioMixer->setSamplePath(_SamplePath);
		_AudioMixer->setPackedSheetOption(_PackedSheetPath, true);
		std::vector<std::string> devices;
		_AudioMixer->initDriver(settings->value(Constants::SOUND_DRIVER, "Auto").toString().toUtf8().constData());
		_AudioMixer->getDevices(devices);
		NLSOUND::UAudioMixer::CInitInfo audioInfo;
		audioInfo.AutoLoadSample = settings->value(Constants::SOUND_AUTO_LOAD_SAMPLE, true).toBool();
		audioInfo.EnableOccludeObstruct = settings->value(Constants::SOUND_ENABLE_OCCLUDE_OBSTRUCT, true).toBool();
		audioInfo.EnableReverb = settings->value(Constants::SOUND_ENABLE_REVERB, true).toBool();
		audioInfo.ManualRolloff = settings->value(Constants::SOUND_MANUAL_ROLL_OFF, true).toBool();
		audioInfo.ForceSoftware = settings->value(Constants::SOUND_FORCE_SOFTWARE, false).toBool();
		audioInfo.MaxTrack = settings->value(Constants::SOUND_MAX_TRACK, 48).toInt();
		audioInfo.UseADPCM = settings->value(Constants::SOUND_USE_ADCPM, false).toBool();
		_AudioMixer->initDevice(settings->value(Constants::SOUND_DEVICE, "").toString().toUtf8().constData(), audioInfo, NULL);
		_AudioMixer->setLowWaterMark(1);

		settings->endGroup();
	}
	catch(NLMISC::Exception &e)
	{
		// in case of exeption during mixer init, the mixer is destroyed !
		std::string mess = std::string("Unable to init sound :") + e.what();
		nlwarning ("Init sound: %s", mess.c_str());
		_AudioMixer = NULL;
		QSettings *settings = Core::ICore::instance()->settings();
		if (settings->group() == Constants::OBJECT_VIEWER_SECTION)
			settings->endGroup();
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
		NLSOUND::USource *src = _AudioMixer->createSource(NLMISC::CSheetId(soundName, "sound"), true);
		if (src)
		{
			// FIXME: Use relative positioning, and set pos to 0,0,0
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
		NLSOUND::USource *src = _AudioMixer->createSource(NLMISC::CSheetId(soundName, "sound"), false);
		if (src)
		{
			// FIXME: Use relative positioning, and set pos to 0,0,0
			src->setLooping(false);
			const NLMISC::CVector &pos = _AudioMixer->getListener()->getPos();
			src->setPos(pos);
			src->play();
			return src;
		}
		else
		{
			nlwarning("Can't play the sound (perhaps it's contextual sound)");
		}
		return NULL;
	}
	return NULL;
}

void CSoundSystem::playAnimation(std::string &name, float lastTime, float curTime, NLSOUND::CSoundContext &context)
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
