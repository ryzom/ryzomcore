/**
 * \file group_controller_root.h
 * \brief CGroupControllerRoot
 * \date 2012-04-10 09:44GMT
 * \author Jan Boon (Kaetemi)
 * CGroupControllerRoot
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
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

#ifndef NLSOUND_GROUP_CONTROLLER_ROOT_H
#define NLSOUND_GROUP_CONTROLLER_ROOT_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/singleton.h>

// Project includes
#include <nel/sound/group_controller.h>

namespace NLSOUND {

/**
 * \brief CGroupControllerRoot
 * \date 2012-04-10 09:44GMT
 * \author Jan Boon (Kaetemi)
 * CGroupControllerRoot
 */
class CGroupControllerRoot : public CGroupController, public NLMISC::CManualSingleton<CGroupControllerRoot>
{
public:
	CGroupControllerRoot();
	virtual ~CGroupControllerRoot();

	/// Gets the group controller in a certain path with separator '/', if it doesn't exist yet it will be created.
	CGroupController *getGroupController(const std::string &path);

protected:
	virtual std::string getPath();
	virtual void calculateFinalGain();
	virtual void increaseSources();
	virtual void decreaseSources();
	static bool isReservedName(const std::string &nodeName);

}; /* class CGroupControllerRoot */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_GROUP_CONTROLLER_ROOT_H */

/* end of file */
