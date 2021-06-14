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




#ifndef RYAI_EGS_INTERFACE_H
#define RYAI_EGS_INTERFACE_H

// Include
#include "nel/net/unified_network.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"

//#include "ai_entity_id.h"


// the class
class CEGSInterface
{
public:
	// classic init(), update() and release()
	static void init();
	static void update();
	static void release();
};


class CAddHandledAIGroupImp : public CAddHandledAIGroupMsg
{
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

class CDelHandledAIGroupImp : public CDelHandledAIGroupMsg
{
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

class CCreatureDespawnImp : public CCreatureDespawnMsg
{
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

class CUserEventMsgImp : public CUserEventMsg
{	
	void callback (const std::string &name, NLNET::TServiceId id);
};

class CSetEscortTeamIdImp : public CSetEscortTeamId
{
	void callback (const std::string &name, NLNET::TServiceId id);
};



#endif
