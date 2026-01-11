// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdsound.h"

#include "nel/sound/music_sound.h"
#include "nel/misc/path.h"
#include "nel/georges/u_form_elm.h"

#if NLSOUND_SHEET_VERSION_BUILT < 2
#	include "nel/sound/group_controller_root.h"
#endif

using namespace std;
using namespace NLMISC;

namespace NLSOUND {


// ***************************************************************************
CMusicSound::CMusicSound()
{
	// init with NULL in case of unexcepted access
	_FileName= NULL;
	_FadeInLength= 2000;
	_FadeOutLength= 2000;
	_MinimumPlayTime= 10000;
	_TimeBeforeCanReplay= 0;
	LastStopTime= INT_MIN;
}

// ***************************************************************************
CMusicSound::~CMusicSound()
{
}


// ***************************************************************************
void		CMusicSound::importForm(const std::string& filename, NLGEORGES::UFormElm& root)
{
	NLGEORGES::UFormElm *psoundType;
	std::string dfnName;

	// some basic checking.
	root.getNodeByName(&psoundType, ".SoundType");
	nlassert(psoundType != NULL);
	psoundType->getDfnName(dfnName);
	nlassert(dfnName == "music_sound.dfn");

	// Call the base class
	CSound::importForm(filename, root);

	// fileName
	std::string musicFileName;
	root.getValueByName(musicFileName, ".SoundType.FileName");
	musicFileName = CFile::getFilename(musicFileName);
	_FileName = CStringMapper::map(musicFileName);

	// Other params
	root.getValueByName(_FadeInLength, ".SoundType.FadeInLength");
	root.getValueByName(_FadeOutLength, ".SoundType.FadeOutLength");
	root.getValueByName(_MinimumPlayTime, ".SoundType.MinimumPlayTime");
	root.getValueByName(_TimeBeforeCanReplay, ".SoundType.TimeBeforeCanReplay");

#if NLSOUND_SHEET_VERSION_BUILT < 2
	_GroupController = CGroupControllerRoot::getInstance()->getGroupController(NLSOUND_SHEET_V1_DEFAULT_SOUND_MUSIC_GROUP_CONTROLLER);
#endif

}

// ***************************************************************************
uint32		CMusicSound::getDuration()
{
	// Cannot know the length of this music sound.
	// Since its not really a sound (played in another "channel"), suppose 0
	return 0;
}

// ***************************************************************************
void		CMusicSound::getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const
{
	subsounds.clear();
}

// ***************************************************************************
void		CMusicSound::serial(NLMISC::IStream &s)
{
	s.serialVersion(0);
	CSound::serial(s);

	CStringMapper::serialString(s, _FileName);
	s.serial(_FadeInLength, _FadeOutLength);
	s.serial(_MinimumPlayTime, _TimeBeforeCanReplay);
	
#if NLSOUND_SHEET_VERSION_BUILT < 2
	if (s.isReading()) _GroupController = CGroupControllerRoot::getInstance()->getGroupController(NLSOUND_SHEET_V1_DEFAULT_SOUND_MUSIC_GROUP_CONTROLLER);
#endif
	
}

// ***************************************************************************
float		CMusicSound::getMaxDistance() const
{
	// used in background_sound_manager, since 2D sound, return 0 because
	// the sound must be cut once out of the patat
	return 0.f;
}

// ***************************************************************************
bool		CMusicSound::isDetailed() const
{
	return false;
}



} // NLSOUND
