/**
 * \file stream_file_sound.cpp
 * \brief CStreamFileSound
 * \date 2012-04-11 09:57GMT
 * \author Jan Boon (Kaetemi)
 * CStreamFileSound
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2012-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include <nel/sound/stream_file_sound.h>

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/sound/group_controller_root.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace NLSOUND {

CStreamFileSound::CStreamFileSound() : m_Async(true)
{
	
}

CStreamFileSound::~CStreamFileSound()
{
	
}

void CStreamFileSound::importForm(const std::string &filename, NLGEORGES::UFormElm &root)
{
	// Call the base class
	CStreamSound::importForm(filename, root);

	// Async
	root.getValueByName(m_Async, ".SoundType.Async");

	// FilePath
	root.getValueByName(m_FilePath, ".SoundType.FilePath");
}

void CStreamFileSound::serial(NLMISC::IStream &s)
{
	CStreamSound::serial(s);

	s.serial(m_Async);
	s.serial(m_FilePath);
}

void CStreamFileSound::setMusicFilePath(const std::string &filePath, bool async, bool loop)
{
#if !FINAL_VERSION
	_Name = NLMISC::CStringMapper::map(std::string("<MusicChannel:") + NLMISC::CFile::getFilenameWithoutExtension(filePath) + ">");
#else
	_Name = NLMISC::CStringMapper::map("<MusicChannel>");
#endif
	_ConeInnerAngle = NLMISC::Pi * 2;
	_ConeOuterAngle = NLMISC::Pi * 2;
	_Looping = loop;
	_Gain = 1.0f;
	_ConeOuterGain = 1.0f;
	_Direction = NLMISC::CVector(0.f, 0.f, 0.f);
	_Pitch = 1.0f;
	_Priority = HighestPri;
	_MaxDist = 9000.0f;
	_MinDist = 1000.0f;
	m_Async = async;
	m_FilePath = filePath;
	_GroupController = CGroupControllerRoot::getInstance()->getGroupController(NLSOUND_SHEET_V1_DEFAULT_SOUND_MUSIC_GROUP_CONTROLLER);
}

} /* namespace NLSOUND */

/* end of file */
