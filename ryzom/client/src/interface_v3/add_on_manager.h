// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef NL_ADD_ON_MANAGER_H
#define NL_ADD_ON_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/progress_callback.h"


// ***************************************************************************
/**
 * <Class description>
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class CAddOnManager
{
public:

	/// Constructor
	CAddOnManager();

	/** Parse File in rootPath, take only one that match posFilterList, and that don't match negFilterList
	 *	eg: addSearchFiles("uiaddon", "*.xml;*.lua;*.tga", "login_*.xml;out_v2_*.xml");
	 */
	void	addSearchFiles(const std::string &path, const std::string &posFilterList, const std::string &negFilterList, NLMISC::IProgressCallback *progressCallBack= NULL);

	/** Get All files added in addSearchFiles, that match one filter in filterList
	 *	eg: getFiles("*.xml;*.tga", vec);
	 *	NB: only filenames are returned (eg clock.xml), not full path
	 */
	void	getFiles(const std::string &filterList, std::vector<std::string> &files);

private:
	std::set<std::string>	_FileSet;
};


// ***************************************************************************
// Interface AddOnManager
extern CAddOnManager	InterfaceAddOnManager;


#endif // NL_ADD_ON_MANAGER_H

/* End of add_on_manager.h */
