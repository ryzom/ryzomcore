// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2021  Winch Gate Property Limited
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

// Project includes
#include <nel/misc/streamed_package.h>
#include <nel/misc/stream.h>

namespace NLMISC {

CStreamedPackage::CStreamedPackage()
{
	// init
}

CStreamedPackage::~CStreamedPackage()
{
	// release
}

void CStreamedPackage::serial(NLMISC::IStream &f)
{
	f.serialCheck(NELID("SNPK"));

	uint version = 1;
	f.serialVersion(version);

	f.serialCont(Entries);
}

void CStreamedPackage::CEntry::serial(NLMISC::IStream &f)
{
	uint version = 1;
	f.serialVersion(version);

	f.serial(Name);
	f.serial(Hash);
	f.serial(Size);
	f.serial(LastModified);
}

void CStreamedPackage::makePath(std::string &result, const CHashKey &hash)
{
	std::string lowerHash = NLMISC::toLowerAscii(hash.toString());
	result = std::string("/") + lowerHash.substr(0, 2)
		+ "/" + lowerHash.substr(2, 2)
		+ "/" + lowerHash.substr(4);
}

} /* namespace NLMISC */

/* end of file */
