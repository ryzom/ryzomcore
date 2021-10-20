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

#include "stdmisc.h"

#include "nel/misc/class_id.h"

#include <sstream>
#include <iomanip>

#include "nel/misc/stream.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace	NLMISC
{


const	CClassId	CClassId::Null(0);

void CClassId::serial(NLMISC::IStream &s)
{
	// s.serial(Uid);
	// Backwards.
	uint32 va = a();
	uint32 vb = b();
	s.serial(va);
	s.serial(vb);
	setA(va);
	setB(vb);
}

std::string CClassId::toString() const
{
	std::stringstream ss;
	ss << "(0x";
	{
		std::stringstream ss1;
		ss1 << std::hex << std::setfill('0');
		ss1 << std::setw(8) << a();
		ss << ss1.str();
	}
	ss << ", 0x";
	{
		std::stringstream ss1;
		ss1 << std::hex << std::setfill('0');
		ss1 << std::setw(8) << b();
		ss << ss1.str();
	}
	ss << ")";
	return ss.str();
}


}

