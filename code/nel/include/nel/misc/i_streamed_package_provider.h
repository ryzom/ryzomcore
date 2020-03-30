// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
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

#ifndef NLMISC_STREAMED_PACKAGE_PROVIDER_H
#define NLMISC_STREAMED_PACKAGE_PROVIDER_H

#include <nel/misc/types_nl.h>
#include <nel/misc/sha1.h>
#include <string>

namespace NLMISC {

class IStreamedPackageProvider
{
public:
	IStreamedPackageProvider();
	virtual ~IStreamedPackageProvider();

	/// Download a file. This call is blocking
	/// filePath: [out] ex. /games/nel/stream/00/00/000000000..
	/// hash: [in]
	/// name: [in] name for debugging purposes
	virtual bool getFile(std::string &filePath, const CHashKey &hash, const std::string &name = "") = 0;
	
}; /* class IStreamedPackageProvider */

} /* namespace NLMISC */

#endif /* #ifndef NLMISC_STREAMED_PACKAGE_PROVIDER_H */

/* end of file */
