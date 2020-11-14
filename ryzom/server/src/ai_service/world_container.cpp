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



#include "stdpch.h"
#include "world_container.h"

#include "nel/misc/path.h"
#include "nel/misc/file.h"
//	#include "nel/misc/hierarchical_timer.h"
#include "server_share/msg_ai_service.h"


using namespace std;
using namespace NLMISC;
using namespace RYAI_MAP_CRUNCH;

RYAI_MAP_CRUNCH::CWorldMap	CWorldContainer::_WorldMaps/*[3]*/;

std::vector<std::string>	CWorldContainer::_ContinentNames;

extern	bool	EGSHasMirrorReady;

/*
 * Constructor
 */
CWorldContainer::CWorldContainer()
{
	clear();
}


/*
 * Clear
 */
void	CWorldContainer::clear()
{
	_WorldMaps.clear();
}


/*
 * Load Continent
 */
void	CWorldContainer::loadContinent(const string &name)
{
	nlinfo("Loading continent '%s' in WorldContainer", name.c_str());

	if (find(_ContinentNames.begin(), _ContinentNames.end(), name) != _ContinentNames.end())
	{
		nldebug("CWorldContainer::loadContinent : continent '%s' is already loaded, ignoring repeat load request", name.c_str());
		return;
	}

	try
	{
		CIFile		f0(CPath::lookup(name+"_0.cwmap2"));

		_WorldMaps.serial(f0);

		_ContinentNames.push_back(name);

		if (EGSHasMirrorReady)
		{
			// if EGS is up, send the new list of available continent
			CReportAICollisionAvailableMsg msg;
			msg.ContinentsCollision = _ContinentNames;
			msg.send("EGS");
		}
	}
	catch (const Exception &e)
	{
		nlwarning("Unable to load continent '%s', aborted (%s)", name.c_str(), e.what());
	}
}
