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


#ifndef R2_SERVER_ADMIN_MODULE_H
#define R2_SERVER_ADMIN_MODULE_H

#include "nel/misc/types_nl.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "game_share/persistent_data.h"

#include "game_share/object.h"

#include "game_share/r2_types.h"
#include "dms.h"

#include <string>

namespace R2
{
class CDynamicMapService;
class CPionieer;
class CAdventure;
class CIsland;
class CEntryPoint;
class CAct;

// from r2_share/r2_messages.h
class CEditionMessageAdventureCreation;
class CAdminMessageAdventureCreation;
class CAdminMessageAdventureUserConnection;
class CEditionMessageAdventureUserConnection;

/** Management of Adventure.
*
* Management of features as package upload features needed by users. Features that are the same for two adventures are only store once.
* Enable search upon adventures. (Database)
* Manage right upon users (a users of level X that pays Y is allowed to use sheet A, B, C and put N elements).
* Code to allow / refuse map creation, map edition, go Live ....
* Store path of rt-data and et-data.
* Store the xp policy (see David docs)
* FAKE DB (Session Manager)
* There is only one Admin Module but multiple Edition/Animation modules
*/
class CServerAdminModule :
public IServerAdminModule,
public NLNET::CEmptyModuleServiceBehav<NLNET::CEmptyModuleCommBehav<NLNET::CEmptySocketBehav <NLNET::CModuleBase> > >
{
public:

	// Simulation of Database Tables
	// :XXX: To remove when DB is Ok


public:

	/*! Initialize the module
	* plug the module to the gw
	* \param gateway the module use by the module to communicate.

	*/
	void init(NLNET::IModuleSocket* gateway, CDynamicMapService* server);

	CServerAdminModule();

	~CServerAdminModule();


	virtual void onModuleUp(NLNET::IModuleProxy *moduleProxy);

	virtual void onModuleDown(NLNET::IModuleProxy *moduleProxy);

	virtual bool onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message);

	virtual void onModuleSecurityChange(NLNET::IModuleProxy *moduleProxy);

	bool getPosition(TSessionId sessionId, double&x, double&y, double& orient, uint8& season, uint32 locationIndex = 0);

	TSessionId getSessionIdByCharId(uint32 charId) const;


	virtual bool isImmediateDispatchingSupported() const { return false; }
	static CServerAdminModule& getInstance() ;


	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CServerAdminModule, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CServerAdminModule, displayIslands, "display islands", "no args")
	NLMISC_COMMAND_HANDLER_TABLE_END

	/// Dump to stdout location infos
	NLMISC_CLASS_COMMAND_DECL(displayIslands);

private:


private:
	CDynamicMapService* _Server;


	static CServerAdminModule* _Instance;

	//:XXX: One Admin server but multiple Animation / Edition Modules???
	NLNET::TModuleProxyPtr	_ServerAnimationProxy;
	NLNET::TModuleProxyPtr	_ServerEditionProxy;

};

} // namespace DMS
#endif //R2_SERVER_ADMIN_MODULE_H

