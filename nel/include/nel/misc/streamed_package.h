// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NLMISC_STREAMED_PACKAGE_H
#define NLMISC_STREAMED_PACKAGE_H

#include <nel/misc/types_nl.h>
#include <nel/misc/sha1.h>

namespace NLMISC {

class CStreamedPackage
{
public:
	struct CEntry
	{
		std::string Name;
		CHashKey Hash;
		uint32 Size;
		uint32 LastModified;

		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	};

public:
	CStreamedPackage();
	~CStreamedPackage();

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/// result: [out] ex. /00/00/000000000..
	/// hash: [in]
	static void makePath(std::string &result, const CHashKey &hash);

public:
	typedef std::vector<CEntry> TEntries;
	TEntries Entries;
	
}; /* class CStreamedPackage */

} /* namespace NLMISC */

#endif /* #ifndef NLMISC_STREAMED_PACKAGE_H */

/* end of file */
