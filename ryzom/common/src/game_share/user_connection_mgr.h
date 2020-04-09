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


#ifndef DMS_USER_CONNECTION_MGR_H
#define DMS_USER_CONNECTION_MGR_H

#include "nel/misc/types_nl.h"

#include <string>

namespace R2
{


class CDynamicMapService;

class CUserConnectionMgr
{
public:
	CUserConnectionMgr(CDynamicMapService* server)
	{
		_Server = server;
	}
	uint32 getCurrentMap(const std::string& /* eid */) { return 1;}

private:
CDynamicMapService* _Server;

};
} // namespace DMS
#endif //DMS_USER_CONNECTION_MGR_H
