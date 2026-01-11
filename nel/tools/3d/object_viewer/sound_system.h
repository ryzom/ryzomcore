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


#ifndef OV_SOUND_SYTEM_H
#define OV_SOUND_SYTEM_H


#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>
#include <string>
#include <set>

#include "nel/sound/sound_anim_manager.h"


namespace NLSOUND
{
	class UAudioMixer;
	class USource;
}

namespace NLMISC
{
	class CMatrix;
}

/// this class init the sound system used by the object viewer
class CSoundSystem
{
public:
	/// set the name of the file containing the sound bank
/*	static void addSoundBank(const std::string &soundBankFileName)
	{
		_SoundBanksFileName.insert(soundBankFileName);
	}
*/	/// set the name of the file containing the sample bank
	static void addSampleBank(const std::string &sampleBankFileName)
	{
		_SampleBanksFileName.insert(sampleBankFileName);
	}

	static void setSamplePath(std::string& path)		{ _SamplePath = NLMISC::CPath::standardizePath(path, true); }
	static void setPackedSheetPath(std::string& path)		{ _PackedSheetPath = NLMISC::CPath::standardizePath(path, true); }

	/** Init the sound system this also load the sound bank
	  * See setSoundBank
	  */
	static void initSoundSystem(void);

	/// release the sound system
	static void releaseSoundSystem(void);

	/// set the listener matrix.
	static void setListenerMatrix(const NLMISC::CMatrix &m);

	/// poll sound. Must be called periodically
	static void poll(void);

	// spawn a sound at the user position
	static void play(const std::string &soundName);
	// create a sound at the user position (don't spawn it)
	static NLSOUND::USource *create(const std::string &soundName);

	// get the audio mixer, or null if init failed
	static NLSOUND::UAudioMixer *getAudioMixer(void)	{ return _AudioMixer; }	

	/// Load the sound animation with the specified name
	static void loadAnimation(std::string& name)		{ _AnimManager->loadAnimation(name); }

	/// Start playing a sound animation. 
	static void playAnimation(std::string& name, float lastTime, float curTime, NLSOUND::CSoundContext &context);

	/// Update the sound animations. 
	//static void updateAnimations(float lastTime, float curTime)	{ _AnimManager->update(lastTime, curTime); };


	/// Returns a reference to the animation manager
	static NLSOUND::CSoundAnimManager* getSoundAnimManager()		{ return _AnimManager; }

private:
	static NLSOUND::UAudioMixer			*_AudioMixer;
//	static std::set<std::string>		_SoundBanksFileName;
	static std::set<std::string>		_SampleBanksFileName;
	static NLSOUND::CSoundAnimManager	*_AnimManager;
	//static sint							_AnimIndex;
	//static NLSOUND::TSoundAnimId		_CurrentAnimation;
	//static NLSOUND::TSoundAnimPlayId	_CurrentPlayback;
	static NLMISC::CVector				_Zero;
	static std::string					_SamplePath;
	static std::string					_PackedSheetPath;

};


#endif
