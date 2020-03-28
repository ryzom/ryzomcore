// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2010-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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
#include "nel/sound/stream_sound.h"

#if NLSOUND_SHEET_VERSION_BUILT < 2
#include "nel/sound/group_controller_root.h"
#endif

namespace NLSOUND
{

CStreamSound::CStreamSound()
    : m_Alpha(1.0f)
{
}

CStreamSound::~CStreamSound()
{
}

void CStreamSound::importForm(const std::string &filename, NLGEORGES::UFormElm &root)
{
	// cannot do this debug check because used also by CStreamFileSound
	/*NLGEORGES::UFormElm *psoundType;
	std::string dfnName;

	// some basic checking.
	root.getNodeByName(&psoundType, ".SoundType");
	nlassert(psoundType != NULL);
	psoundType->getDfnName(dfnName);
	nlassert(dfnName == "stream_sound.dfn");*/

	// Call the base class
	CSound::importForm(filename, root);

	// MaxDistance
	root.getValueByName(_MaxDist, ".SoundType.MaxDistance");

	// MinDistance
	root.getValueByName(_MinDist, ".SoundType.MinDistance");

	// Alpha
	root.getValueByName(m_Alpha, ".SoundType.Alpha");

#if NLSOUND_SHEET_VERSION_BUILT < 2
	_GroupController = CGroupControllerRoot::getInstance()->getGroupController(NLSOUND_SHEET_V1_DEFAULT_SOUND_STREAM_GROUP_CONTROLLER);
#endif
}

void CStreamSound::serial(NLMISC::IStream &s)
{
	CSound::serial(s);

	s.serial(_MinDist);
	s.serial(m_Alpha);

#if NLSOUND_SHEET_VERSION_BUILT < 2
	if (s.isReading())
		_GroupController = CGroupControllerRoot::getInstance()->getGroupController(NLSOUND_SHEET_V1_DEFAULT_SOUND_STREAM_GROUP_CONTROLLER);
#endif
}

} /* namespace NLSOUND */

/* end of file */
