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

#include "std_afx.h"
#include "resource.h"
#include "sound_system.h"
#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/u_listener.h"
#include "nel/sound/sound_anim_manager.h"
#include "nel/sound/sound_animation.h"
#include "nel/misc/path.h"
#include "edit_ps_sound.h"

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;


UAudioMixer				*CSoundSystem::_AudioMixer = NULL;
//set<string>				CSoundSystem::_SoundBanksFileName;
set<string>				CSoundSystem::_SampleBanksFileName;
CSoundAnimManager		*CSoundSystem::_AnimManager = NULL;
//TSoundAnimId			CSoundSystem::_CurrentAnimation = CSoundAnimation::NoId;
//sint32					CSoundSystem::_CurrentPlayback = -1;
CVector					CSoundSystem::_Zero = CVector::Null;
string					CSoundSystem::_SamplePath;
string					CSoundSystem::_PackedSheetPath;
//sint					CSoundSystem::_AnimIndex = -1;

void CSoundSystem::setListenerMatrix(const NLMISC::CMatrix &m)
{
	if (_AudioMixer)
	{
		static CMatrix oldMatrix;
		if(m.getPos() != oldMatrix.getPos() || m.getJ() != oldMatrix.getJ() || m.getK() != oldMatrix.getK())
		{
			UListener *l = _AudioMixer->getListener();
			l->setPos(m.getPos());		
			l->setOrientation(m.getJ(), m.getK());
			oldMatrix = m;
		}
	}
}


void CSoundSystem::initSoundSystem ()
{		
	_AudioMixer = NULL;
	_AnimManager = NULL;
	_AudioMixer = NLSOUND::UAudioMixer::createAudioMixer();
	try
	{
		_AudioMixer->setSamplePath(_SamplePath);
		_AudioMixer->setPackedSheetOption(_PackedSheetPath, true);
		_AudioMixer->init(32, true, false, NULL, true);
	}
	catch(NLMISC::Exception &e)
	{
		// in case of exeption during mixer init, the mixer is destroyed !
		string mess = string("Unable to init sound :") + e.what();
		nlwarning ("Init sound: %s", mess.c_str());
		_AudioMixer = NULL;
		return;
	}

	// ok for the mixer, now create the sound anim manager
	try
	{
		// TODO : boris : Hum, as far as I know, throwing exeption in constructor is a very BAD idea...
		_AnimManager = new CSoundAnimManager(_AudioMixer);
	}
	catch (NLMISC::Exception &e)
	{
		string mess = string("Unable to init sound :") + e.what();
		nlwarning ("Init sound: %s", mess.c_str());
		if (_AnimManager)
		{
			delete _AnimManager;
			_AnimManager = NULL;
		}

		delete _AudioMixer;
		_AudioMixer = NULL;

		return;
	}
	setPSSoundSystem(_AudioMixer);

/*		
	for (set<string>::const_iterator it1 = _SampleBanksFileName.begin();
		 it1 != _SampleBanksFileName.end();
		 ++it1)
	{
		try
		{
			//_AudioMixer->loadSampleBank(NLMISC::CPath::lookup(*it).c_str());
			_AudioMixer->loadSampleBank(false, (*it1));
		}
		catch (NLMISC::Exception &e)
		{
			string mess = "Unable to load sound file :" + *it1
						+ "\n" + e.what();
			nlwarning ("Init sound: %s", mess.c_str());
		}
	}					
*/
/*	for (set<string>::const_iterator it2 = _SoundBanksFileName.begin();
		 it2 != _SoundBanksFileName.end();
		 ++it2)
	{
		try
		{
			//_AudioMixer->loadSoundBank(NLMISC::CPath::lookup(*it).c_str());
			_AudioMixer->loadSoundBank((*it2).c_str());
		}
		catch (NLMISC::Exception &e)
		{
			string mess = "Unable to load sound file :" + *it2
						+ "\n" + e.what();
			nlwarning ("Init sound: %s", mess.c_str());
		}
	}					
*/

}


void CSoundSystem::poll()
{
	if (_AudioMixer)
	{
		_AudioMixer->update();
	}
}



void CSoundSystem::releaseSoundSystem(void)
{
	setPSSoundSystem(NULL);
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


void CSoundSystem::play(const string &soundName)
{
	if (_AudioMixer)
	{
		NLSOUND::USource *src = _AudioMixer->createSource(NLMISC::CSheetId(soundName, "sound"), true);
		if (src)
		{
			src->setLooping(false);
			const CVector &pos = _AudioMixer->getListener()->getPos();
			src->setPos(pos);
			src->play();
		}
		else
		{
			MessageBox(NULL, "Can't play the sound (perhaps it's contextual sound)", "warning", MB_OK|MB_ICONWARNING );			
		}
	}	
}

USource *CSoundSystem::create(const std::string &soundName)
{
	if (_AudioMixer)
	{
		NLSOUND::USource *src = _AudioMixer->createSource(NLMISC::CSheetId(soundName, "sound"), false);
		if (src)
		{
			src->setLooping(false);
			const CVector &pos = _AudioMixer->getListener()->getPos();
			src->setPos(pos);
			src->play();
			return src;
		}	
		else
		{
			MessageBox(NULL, "Can't play the sound (perhaps it's contextual sound)", "warning", MB_OK|MB_ICONWARNING );			
		}	return NULL;
	}
	return NULL;
}	


void CSoundSystem::playAnimation(string& name, float lastTime, float curTime, CSoundContext &context)
{
	if (_AnimManager == NULL)
	{
		return;
	}

	TSoundAnimId id = _AnimManager->getAnimationFromName(name);

	if (id != CSoundAnimationNoId)
	{
		_AnimManager->playAnimation(id, lastTime, curTime, NULL, context);
	}
}
