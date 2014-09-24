// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014  Jan BOON (jan.boon@kaetemi.be)
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

#ifndef NLMISC_STREAMED_PACKAGE_MANAGER_H
#define NLMISC_STREAMED_PACKAGE_MANAGER_H

#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/streamed_package.h>

namespace NLMISC {

class CStreamedPackageManager
{
	NLMISC_SAFE_RELEASABLE_SINGLETON_DECL(CStreamedPackageManager);

public:
	CStreamedPackageManager();
	~CStreamedPackageManager();

	/// Loads a package into the package manager
	/// package: ex. /games/nel/data/characters_maps_hr.snp
	bool loadPackage(const std::string &package);

	/// Get a file list
	void list(std::vector<std::string> &fileNames, const std::string &package);

	/// Unload all packages
	void unloadAll();

	/// Get an existing file or download if necessary
	/// filePath: [out] ex. /games/nel/stream/00/00/000000000..
	/// fileName: ex. fy_hof_underwear.dds
	bool getFile(std::string &filePath, const std::string &fileName);

	/// Get file size
	bool getFileSize(uint32 &fileSize, const std::string &fileName);

private:
	std::map<std::string, CStreamedPackage> m_Packages;
	std::map<std::string, const CStreamedPackage::CEntry *> m_Entries;
	
}; /* class CStreamedPackageManager */

} /* namespace NLMISC */

#endif /* #ifndef NLMISC_STREAMED_PACKAGE_MANAGER_H */

/* end of file */
