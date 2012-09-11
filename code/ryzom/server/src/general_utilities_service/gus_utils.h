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

#ifndef GUS_UTILS_H
#define GUS_UTILS_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"
#include "nel/misc/smart_ptr.h"
#include "nel/net/unified_network.h"

#include "game_share/utils.h"


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// handy utilities
	//-----------------------------------------------------------------------------

	// Clean a path performing the following operations:
	//	- convert '\\' characters to '/'
	//	- replace '//' strings in the middle of the path with '/'
	//	- remove '.' directory entries
	//	- colapse '..' directory entries (removing parent entries)
	//	- append a final '/' (optionally)
	//
	// examples:
	//	- a:/bcd/efg/		=>	a:/bcd/efg/ (no change)
	//	- a:\bcd\efg		=>	a:/bcd/efg/
	//	- \bcd\\efg			=>	/bcd/efg/
	//	- \\bcd\efg			=>	//bcd/efg/
	//	- \bcd\.\efg		=>	/bcd/efg/
	//	- \bcd\..\efg		=>	/efg/
	//	- bcd\..\efg		=>	efg/
	//	- bcd\..\..\efg		=>	../efg/
	//	- \bcd\..\..\efg	=>	/efg/		(NOTE: the redundant '..' entry is lost due to leading '\')
	//
	NLMISC::CSString cleanPath(const NLMISC::CSString& path,bool addTrailingSlash);


	// execute a command on a remote service
	void executeRemoteCommand(NLNET::TServiceId sid,const NLMISC::CSString& cmdLine);
	void executeRemoteCommand(const char* serviceName,const NLMISC::CSString& cmdLine);

}

//-----------------------------------------------------------------------------
#endif
