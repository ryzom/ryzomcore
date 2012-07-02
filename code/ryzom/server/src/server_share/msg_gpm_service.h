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




#ifndef RY_MSG_GPM_SERVICE_H
#define RY_MSG_GPM_SERVICE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"
#include "nel/net/unified_network.h"
#include "nel/net/transport_class.h"

#include "game_share/ryzom_entity_id.h"
#include "game_share/base_types.h"
#include "game_share/synchronised_message.h"


/****************************************************************\
 ****************************************************************
	  Messages definitions for GPM Service
 ****************************************************************
\****************************************************************/

class CEntitySheetId
{
public:
	NLMISC::CEntityId	EntityId;
	NLMISC::CSheetId	SheetId;
	CEntitySheetId() {}
	CEntitySheetId(NLMISC::CEntityId &eid, NLMISC::CSheetId &sheet) : EntityId(eid), SheetId(sheet) {}

	void	serial(NLMISC::IStream &f) { f.serial(EntityId, SheetId); }
};


class CGPMPlayerPrivilege : public CMirrorTransportClass
{
public:

	enum TType
	{
		Player = 0,	// basic player, no particular rights
		GM = 1,
		CS = 2
	};


	TDataSetRow			PlayerIndex;
	uint32				Type;

	virtual void description ()
	{
		className("CGPMPlayerPrivilege");
		property("PlayerIndex", PropDataSetRow, TDataSetRow(), PlayerIndex);
		property("Type", PropUInt32, (uint32)Player, Type);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id) {}
};


#endif // RY_MSG_GPM_SERVICE_H

/* End of msg_gpm_service.h */
