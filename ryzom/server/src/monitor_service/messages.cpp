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



// include files
#include "stdpch.h"

#include "messages.h"

using namespace NLMISC;
using namespace NLNET;

// ***************************************************************************
// the callback table
// ***************************************************************************

static void cbAddEntities( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId );

TUnifiedCallbackItem CbArray[] = 
{
	{	"ADDED_ENTITIES",		cbAddEntities,			},
};

// ***************************************************************************
// Messages from the world editor
// ***************************************************************************

//
static void cbAddEntities( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
//	This is where we should deal with player record creation
}


