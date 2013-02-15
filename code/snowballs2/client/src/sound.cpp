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

#include "sound.h"
#include "entities.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

#ifdef SBCLIENT_WITH_SOUND

////
//// Variables
////
//
//UAudioMixer *AudioMixer = NULL;
//TSoundId SoundId;
//const vector<TSoundId> *SoundIdArray;
//static bool SoundEnabled;
//
////
//// Functions
////
//
//#ifdef NL_OS_WINDOWS
//void initSound2();
//void releaseSound2();
//#endif
//
//void cbConfigFileSoundMaxTracks(NLMISC::CConfigFile::CVar &var)
//{
////#ifdef NL_OS_WINDOWS
////	AudioMixer->changeMaxTrack(var.asInt());
////#endif
//}
//
//void cbConfigFileSoundEnabled(NLMISC::CConfigFile::CVar &var)
//{
////#ifdef NL_OS_WINDOWS
////	if (var.asBool() != SoundEnabled)
////	{
////		if (var.asBool()) initSound2();
////		else releaseSound2();
////	}
////#endif
//}
//
//void cbConfigFileFail(NLMISC::CConfigFile::CVar &var)
//{
//	//nlwarning("You can't modify the config variable '%s' at runtime for now, please restart the game", var.asString().c_str());
//}
//
////#ifdef NL_OS_WINDOWS
////void initSound2()
////{
////	AudioMixer = UAudioMixer::createAudioMixer ();
////	std::string driverName;
////	NLSOUND::UAudioMixer::TDriver driverType;
////	if (!ConfigFile->exists("SoundDriver")) 
////#ifdef NL_OS_WINDOWS
////		driverType = NLSOUND::UAudioMixer::DriverFMod;
////#elif defined (NL_OS_UNIX)
////		driverType = NLSOUND::UAudioMixer::DriverOpenAl;
////#else
////		driverType = NLSOUND::UAudioMixer::DriverAuto;
////#endif
////	else 
////	{
////		driverName = ConfigFile->getVar("SoundDriver").asString();
////		if (driverName == "Auto") driverType = NLSOUND::UAudioMixer::DriverAuto;
////		else if (driverName == "FMod") driverType = NLSOUND::UAudioMixer::DriverFMod;
////		else if (driverName == "DSound") driverType = NLSOUND::UAudioMixer::DriverDSound;
////		else if (driverName == "OpenAl") driverType = NLSOUND::UAudioMixer::DriverOpenAl;
////		else nlerror("SoundDriver value '%s' is invalid.", driverName.c_str());
////	}
////
////	AudioMixer->init(
////		ConfigFile->exists("SoundMaxTracks")
////		? ConfigFile->getVar("SoundMaxTracks").asInt() : 32,
////		ConfigFile->exists("SoundUseEax")
////		? ConfigFile->getVar("SoundUseEax").asBool() : true,
////		ConfigFile->exists("SoundUseADPCM")
////		? ConfigFile->getVar("SoundUseADPCM").asBool() : true,
////		NULL, false, driverType,
////		ConfigFile->exists("SoundForceSoftware")
////		? ConfigFile->getVar("SoundForceSoftware").asBool() : true);
////
////	ConfigFile->setCallback("SoundMaxTracks", cbConfigFileSoundMaxTracks);
////	ConfigFile->setCallback("SoundUseEax", cbConfigFileFail);
////	ConfigFile->setCallback("SoundUseADPCM", cbConfigFileFail);
////	ConfigFile->setCallback("SoundForceSoftware", cbConfigFileFail);
////	ConfigFile->setCallback("SoundDriver", cbConfigFileFail);
////
////	PlaylistManager = new SBCLIENT::CMusicPlaylistManager(AudioMixer, ConfigFile, "SoundPlaylist");
////
////	/* AudioMixer->loadSoundBuffers ("sounds.nss", &SoundIdArray);
////	nlassert( SoundIdArray->size() == 2 );
////	SoundId = (*SoundIdArray)[0];
////	// StSoundId = (*SoundIdArray)[1]; */
////	
////	SoundEnabled = true;
////}
////#endif
//
//void initSound()
//{
////#ifdef NL_OS_WINDOWS
////	if (ConfigFile->exists("SoundEnabled") ? ConfigFile->getVar("SoundEnabled").asBool() : false) initSound2();
////	ConfigFile->setCallback("SoundEnabled", cbConfigFileSoundEnabled);
////#endif
//}
//
//void playSound (CEntity &entity, TSoundId id)
//{
///*	entity.Source = AudioMixer->createSource (id);
//	entity.Source->setLooping (true);
//	entity.Source->play ();
//*/}
//
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
//
//void updateSound()
//{
////#ifdef NL_OS_WINDOWS
////	if (SoundEnabled)
////	{
////		PlaylistManager->update(DiffTime);
////		AudioMixer->update();
////	}
////#endif
//}
//
////#ifdef NL_OS_WINDOWS
////void releaseSound2()
////{		
////	SoundEnabled = false;
////	ConfigFile->setCallback("SoundMaxTracks", NULL);
////	ConfigFile->setCallback("SoundUseEax", NULL);
////	ConfigFile->setCallback("SoundUseADPCM", NULL);
////	ConfigFile->setCallback("SoundForceSoftware", NULL);
////	ConfigFile->setCallback("SoundDriver", NULL);
////	delete PlaylistManager;
////	PlaylistManager = NULL;
////	delete AudioMixer;
////	AudioMixer = NULL;
////}
////#endif
//
//void releaseSound()
//{
////#ifdef NL_OS_WINDOWS
////	ConfigFile->setCallback("SoundEnabled", NULL);
////	if (SoundEnabled) releaseSound2();
////#endif
//}
//
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
////
////NLMISC_COMMAND(music_bg,"background music","")
////{
////	if (args.size() != 0) return false;
////	playMusic(SBCLIENT_MUSIC_BACKGROUND);
////	return true;
////}
////
////NLMISC_COMMAND(music_bg_beat,"background music with beat","")
////{
////	if (args.size() != 0) return false;
////	PlaylistManager->playMusic(SBCLIENT_MUSIC_BACKGROUND_BEAT);
////	return true;
////}
////
////NLMISC_COMMAND(music_wait,"loading music","")
////{
////	if (args.size() != 0) return false;
////	PlaylistManager->playMusic(SBCLIENT_MUSIC_WAIT);
////	return true;
////}
////
////NLMISC_COMMAND(music_login,"login screen music","")
////{
////	if (args.size() != 0) return false;
////	PlaylistManager->playMusic(SBCLIENT_MUSIC_LOGIN);
////	return true;
////}
////
////#endif 

#endif // SBCLIENT_WITH_SOUND
