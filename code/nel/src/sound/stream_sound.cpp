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

#include "stdsound.h"
#include "stream_sound.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

// using namespace std;
// using namespace NLMISC;

namespace NLSOUND {

CStreamSound::CStreamSound()
{
	
}

CStreamSound::~CStreamSound()
{
	
}

void CStreamSound::importForm(const std::string &filename, NLGEORGES::UFormElm &root)
{
	NLGEORGES::UFormElm *psoundType;
	std::string dfnName;

	// some basic checking.
	root.getNodeByName(&psoundType, ".SoundType");
	nlassert(psoundType != NULL);
	psoundType->getDfnName(dfnName);
	nlassert(dfnName == "stream_sound.dfn");

	// Call the base class
	CSound::importForm(filename, root);

	// MaxDistance
 	root.getValueByName(_MaxDist, ".SoundType.MaxDistance");

	// MinDistance
	root.getValueByName(_MinDist, ".SoundType.MinDistance");

	// Alpha
	root.getValueByName(m_Alpha, ".SoundType.Alpha");
}

void CStreamSound::serial(NLMISC::IStream &s)
{
	CSound::serial(s);

	s.serial(_MinDist);
	s.serial(m_Alpha);
}

} /* namespace NLSOUND */

/* end of file */
