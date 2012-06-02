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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

//nel
#include "nel/misc/command.h"

// game share
#include "game_share/utils.h"

// local
#include "gus_net.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace GUSNET;
using namespace GUS;


//-----------------------------------------------------------------------------
// CGusNet Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(GUSNET,gusnetDisplay,"display the net hubs, connections and remote modules","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CGusNet::getInstance()->display();

	return true;
}


//-----------------------------------------------------------------------------
