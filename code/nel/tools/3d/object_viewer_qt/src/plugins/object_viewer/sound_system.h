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


#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#include <nel/misc/types_nl.h>

// NeL includes
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/path.h>
#include <nel/sound/sound_anim_manager.h>

// STL includes
#include <string>
#include <set>

namespace NLSOUND
{
class UAudioMixer;
class USource;
}

namespace NLMISC
{
class CMatrix;
}

namespace NLQT
{

/**
@class CSoundSystem
@brief This class init the sound system used by the object viewer
*/
class CSoundSystem
{
public:
	CSoundSystem();
	virtual ~CSoundSystem();

	/// Set the name of the file containing the sample bank
	void addSampleBank(const std::string &sampleBankFileName)
	{
		_SampleBanksFileName.insert(sampleBankFileName);
	}

	/// Sets the path which contains samples
	void setSamplePath(std::string &path)
	{
		_SamplePath = NLMISC::CPath::standardizePath(path, true);
	}

	/// Sets the path which contains packed sheet
	void setPackedSheetPath(std::string &path)
	{
		_PackedSheetPath = NLMISC::CPath::standardizePath(path, true);
	}

	/// Init the sound system this also load the sound bank. See setSoundBank
	void init(void);

	/// Release the sound system
	void release(void);

	/// Set the listener matrix.
	void setListenerMatrix(const NLMISC::CMatrix &m);

	/// Spawn a sound at the user position
	void play(const std::string &soundName);

	/// Create a sound at the user position (don't spawn it)
	NLSOUND::USource *create(const std::string &soundName);

	/// Load the sound animation with the specified name
	void loadAnimation(std::string &name)
	{
		_AnimManager->loadAnimation(name);
	}

	/// Start playing a sound animation.
	void playAnimation(std::string &name, float lastTime, float curTime, NLSOUND::CSoundContext &context);

	// Update the sound animations.
	//static void updateAnimations(float lastTime, float curTime)	{ _AnimManager->update(lastTime, curTime); };

	/// Get the audio mixer, or null if init failed
	NLSOUND::UAudioMixer *getAudioMixer(void)
	{
		return _AudioMixer;
	}

	/// Returns a reference to the animation manager
	NLSOUND::CSoundAnimManager *getSoundAnimManager()
	{
		return _AnimManager;
	}

	/// Init the particle system sound with the given AudioMixer
	void initGraphics();

	/// Release the particle system sound with the given AudioMixer
	void releaseGraphics();

	/// Update sound. Must be called periodically
	void update();

private:
	NLSOUND::UAudioMixer *_AudioMixer;
	std::set<std::string> _SampleBanksFileName;
	NLSOUND::CSoundAnimManager *_AnimManager;
	NLMISC::CVector _Zero;
	std::string	_SamplePath;
	std::string	_PackedSheetPath;
	NLMISC::CMatrix	oldMatrix;
}; /* class CSoundSystem */

} /* namespace NLQT */

#endif // SOUND_SYSTEM_H
