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

#include <nel/misc/types_nl.h>
#include "sound_utilities.h"

// STL includes

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/hierarchical_timer.h>
#include <nel/misc/config_file.h>
#include <nel/sound/u_audio_mixer.h>
#include <nel/sound/sound_anim_manager.h>
#include <nel/misc/progress_callback.h>
#include <nel/3d/u_particle_system_sound.h>

// Project includes
#include "configuration.h"
#include "internationalization.h"
#include "graphics_viewport.h"

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

namespace NLQT {

CSoundUtilities::CSoundUtilities()
	: m_Configuration(NULL), 
	m_Internationalization(NULL), 
	m_GraphicsViewport(NULL), 
	//m_LandscapeUtilities(NULL), 
	//m_PacsUtilities(NULL), 
	m_AudioMixer(NULL), 
	m_SoundAnimManager(NULL)
{
	
}

CSoundUtilities::~CSoundUtilities()
{
	// release();
}

void CSoundUtilities::init(CConfiguration *configuration, CInternationalization *internationalization)
{
	//H_AUTO2
	nldebug("CSoundUtilities::init");

	// copy parameters
	m_Configuration = configuration;
	m_Internationalization = internationalization;
	
	// check stuff we need
	nlassert(m_Configuration);
	nlassert(m_Internationalization);

	// create audiomixer	
	NL3D::UParticleSystemSound::setPSSound(NULL);
	nlassert(!m_AudioMixer);
	m_AudioMixer = UAudioMixer::createAudioMixer();
	nlassert(m_AudioMixer);

	// init audiomixer
	std::vector<std::string> devices;
	m_AudioMixer->initDriver(m_Configuration->getValue("SoundDriver", string("Auto")));
	m_AudioMixer->getDevices(devices);
	UAudioMixer::CInitInfo audioInfo;
	audioInfo.AutoLoadSample = m_Configuration->getValue("SoundAutoLoadSample", true);
	audioInfo.EnableOccludeObstruct = m_Configuration->getValue("SoundEnableOccludeObstruct", true);
	audioInfo.EnableReverb = m_Configuration->getValue("SoundEnableReverb", true);
	audioInfo.ManualRolloff = m_Configuration->getValue("SoundManualRolloff", true);
	audioInfo.ForceSoftware = m_Configuration->getValue("SoundForceSoftware", false);
	audioInfo.MaxTrack = m_Configuration->getValue("SoundMaxTrack", 48);
	audioInfo.UseADPCM = m_Configuration->getValue("SoundUseADPCM", false);
	m_AudioMixer->initDevice(m_Configuration->getValue("SoundDevice", string("")), audioInfo, NULL);
	m_AudioMixer->setLowWaterMark(1);

	// config callbacks
	// ...

	// sound anim manager
	nlassert(!m_SoundAnimManager);
	m_SoundAnimManager = new CSoundAnimManager(m_AudioMixer);
	nlassert(m_SoundAnimManager);
	
	// temp listener pos
	m_AudioMixer->setListenerPos(CVector(0.0f, 0.0f, 0.0f));

	// init sources
	// ...
}

void CSoundUtilities::release()
{
	//H_AUTO2
	nldebug("CSoundUtilities::release");

	// release sources
	// ...

	// release sound anim manager
	if (m_SoundAnimManager)
	{
		delete m_SoundAnimManager;
		m_SoundAnimManager = NULL;
	}
	else nlwarning("!m_SoundAnimManager");

	// drop config callbacks
	// ...

	// release audiomixer (todo: +sources!!!)
	if (m_AudioMixer)
	{
		delete m_AudioMixer;
		m_AudioMixer = NULL;
	}
	else nlwarning("!m_AudioMixer");
	
	// reset parameters
	m_Configuration = NULL;
	m_Internationalization = NULL;
}

void CSoundUtilities::updateSound()
{
	m_AudioMixer->update();
}

void CSoundUtilities::initGraphics(CGraphicsViewport *graphicsViewport)
{
	//H_AUTO2
	nldebug("CSoundUtilities::initGraphics");

	// copy parameters
	m_GraphicsViewport = graphicsViewport;

	// check stuff we need
	nlassert(m_GraphicsViewport);

	// set particle system sound
	NL3D::UParticleSystemSound::setPSSound(m_AudioMixer);

	// ...
	// todo: displayers for all the test sound sources :)
}

void CSoundUtilities::releaseGraphics()
{
	//H_AUTO2
	nldebug("CSoundUtilities::releaseGraphics");

	// ..
	
	// clear particle system sound
	NL3D::UParticleSystemSound::setPSSound(NULL);

	// reset parameters
	m_GraphicsViewport = NULL;
}

} /* namespace NLQT */

/* end of file */
