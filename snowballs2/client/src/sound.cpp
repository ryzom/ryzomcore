// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

//
// Includes
//

#include <nel/misc/types_nl.h>

#include <cmath>
#include <vector>

#include <nel/misc/vector.h>
#include <nel/misc/command.h>

#include <nel/sound/u_audio_mixer.h>
#include <nel/sound/u_listener.h>
#include <nel/sound/u_source.h>

#include "snowballs_client.h"
#include "sound.h"
#include "entities.h"
#include "configuration.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

namespace SBCLIENT
{

//
// Variables
///

#ifdef SBCLIENT_WITH_SOUND
UAudioMixer *AudioMixer = NULL;
//TSoundId SoundId;
//const vector<TSoundId> *SoundIdArray;
static bool SoundEnabled;
#endif

//
// Functions
//

#ifdef SBCLIENT_WITH_SOUND

void initSound2();
void releaseSound2();

void cbConfigFileSoundMaxTracks(NLMISC::CConfigFile::CVar &var)
{
	if (AudioMixer)
		AudioMixer->changeMaxTrack(var.asInt());
}

void cbConfigFileSoundEnabled(NLMISC::CConfigFile::CVar &var)
{
	if (var.asBool() != SoundEnabled)
	{
		if (var.asBool())
			initSound2();
		else
			releaseSound2();
	}
}

void cbConfigFileMusicVolume(NLMISC::CConfigFile::CVar &var)
{
	if (AudioMixer)
		AudioMixer->setMusicVolume(var.asFloat());
}

void cbConfigFileFail(NLMISC::CConfigFile::CVar &var)
{
	nlwarning("You can't modify the config variable '%s' at runtime for now, please restart the game", var.asString().c_str());
}

void initSound2()
{
	AudioMixer = UAudioMixer::createAudioMixer();
	std::string driverName;
	NLSOUND::UAudioMixer::TDriver driverType;
	if (!ConfigFile->exists("SoundDriver"))
#ifdef NL_OS_WINDOWS
		driverType = NLSOUND::UAudioMixer::DriverFMod;
#elif defined(NL_OS_UNIX)
		driverType = NLSOUND::UAudioMixer::DriverOpenAl;
#else
		driverType = NLSOUND::UAudioMixer::DriverAuto;
#endif
	else
	{
		driverName = ConfigFile->getVar("SoundDriver").asString();
		if (driverName == "Auto")
			driverType = NLSOUND::UAudioMixer::DriverAuto;
		else if (driverName == "FMod")
			driverType = NLSOUND::UAudioMixer::DriverFMod;
		else if (driverName == "DSound")
			driverType = NLSOUND::UAudioMixer::DriverDSound;
		else if (driverName == "OpenAl")
			driverType = NLSOUND::UAudioMixer::DriverOpenAl;
		else
			nlerror("SoundDriver value '%s' is invalid.", driverName.c_str());
	}

	AudioMixer->init(
	    ConfigFile->exists("SoundMaxTracks")
	        ? ConfigFile->getVar("SoundMaxTracks").asInt()
	        : 32,
	    ConfigFile->exists("SoundUseEax")
	        ? ConfigFile->getVar("SoundUseEax").asBool()
	        : true,
	    ConfigFile->exists("SoundUseADPCM")
	        ? ConfigFile->getVar("SoundUseADPCM").asBool()
	        : true,
	    NULL, true, driverType,
	    ConfigFile->exists("SoundForceSoftware")
	        ? ConfigFile->getVar("SoundForceSoftware").asBool()
	        : true);

	ConfigFile->setCallback("SoundMaxTracks", cbConfigFileSoundMaxTracks);
	ConfigFile->setCallback("SoundUseEax", cbConfigFileFail);
	ConfigFile->setCallback("SoundUseADPCM", cbConfigFileFail);
	ConfigFile->setCallback("SoundForceSoftware", cbConfigFileFail);
	ConfigFile->setCallback("SoundDriver", cbConfigFileFail);
	CConfiguration::setAndCallback("MusicVolume", cbConfigFileMusicVolume);

	// PlaylistManager = new SBCLIENT::CMusicPlaylistManager(AudioMixer, ConfigFile, "SoundPlaylist");

	/* AudioMixer->loadSoundBuffers ("sounds.nss", &SoundIdArray);
	nlassert( SoundIdArray->size() == 2 );
	SoundId = (*SoundIdArray)[0];
	// StSoundId = (*SoundIdArray)[1]; */

	SoundEnabled = true;
}

void releaseSound2()
{
	SoundEnabled = false;
	ConfigFile->setCallback("SoundMaxTracks", NULL);
	ConfigFile->setCallback("SoundUseEax", NULL);
	ConfigFile->setCallback("SoundUseADPCM", NULL);
	ConfigFile->setCallback("SoundForceSoftware", NULL);
	ConfigFile->setCallback("SoundDriver", NULL);
	// delete PlaylistManager;
	// PlaylistManager = NULL;
	delete AudioMixer;
	AudioMixer = NULL;
}

#endif

void initSound()
{
#ifdef SBCLIENT_WITH_SOUND
	if (ConfigFile->exists("SoundEnabled") ? ConfigFile->getVar("SoundEnabled").asBool() : false)
		initSound2();
	ConfigFile->setCallback("SoundEnabled", cbConfigFileSoundEnabled);
#endif
}

//void playSound (CEntity &entity, TSoundId id)
//{
///*	entity.Source = AudioMixer->createSource (id);
//	entity.Source->setLooping (true);
//	entity.Source->play ();
//*/}

//void deleteSound (CEntity &entity)
//{
///*	if (entity.Source != NULL)
//	{
//		if (entity.Source->isPlaying ())
//			entity.Source->stop ();
//
//		AudioMixer->removeSource (entity.Source);
//		entity.Source = NULL;
//	}
//*/}

void updateSound()
{
#ifdef SBCLIENT_WITH_SOUND
	if (SoundEnabled)
	{
		// PlaylistManager->update(DiffTime);
		AudioMixer->update();
	}
	#endif
}

void releaseSound()
{
#ifdef SBCLIENT_WITH_SOUND
	ConfigFile->setCallback("SoundEnabled", NULL);
	if (SoundEnabled) releaseSound2();
#endif
}

void playMusic(const char *file)
{
#ifdef SBCLIENT_WITH_SOUND
	if (AudioMixer)
		AudioMixer->playMusic(file, 1000, true, true);
#endif
}

} /* namespace SBCLIENT */

////#ifdef NL_OS_WINDOWS
////
////void playMusic(sint32 playlist, sint32 track)
////{
////	if (SoundEnabled)
////		PlaylistManager->playMusic(playlist, track);
////}
////
////void setMusicVolume(sint32 playlist, float volume)
////{
////	if (SoundEnabled)
////		PlaylistManager->setVolume(playlist, volume);
////}

NLMISC_COMMAND(music_bg,"background music","")
{
	if (args.size() != 0) return false;
	SBCLIENT::playMusic(SBCLIENT_MUSIC_BACKGROUND);
	return true;
}

NLMISC_COMMAND(music_bg_beat,"background music with beat","")
{
	if (args.size() != 0)
		return false;
	SBCLIENT::playMusic(SBCLIENT_MUSIC_BACKGROUND_BEAT);
	return true;
}

NLMISC_COMMAND(music_wait,"loading music","")
{
	if (args.size() != 0)
		return false;
	SBCLIENT::playMusic(SBCLIENT_MUSIC_WAIT);
	return true;
}

NLMISC_COMMAND(music_login,"login screen music","")
{
	if (args.size() != 0)
		return false;
	SBCLIENT::playMusic(SBCLIENT_MUSIC_LOGIN);
	return true;
}

////#endif 

/* end of file */
