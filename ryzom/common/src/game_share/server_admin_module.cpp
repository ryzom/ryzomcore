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

#include "stdpch.h"

#include "server_admin_module.h"

#include "dms.h"
#include "server_edition_module.h"
#include "server_animation_module.h"

#include "game_share/object.h"
#include "game_share/r2_messages.h"
#include "game_share/scenario_entry_points.h"

#include "nel/net/service.h"
#include "nel/net/unified_network.h"
#include "nel/net/module_message.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"
#include "nel/net/module_builder_parts.h"


#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"
#include "nel/misc/variable.h"
#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace R2;

//static std::string AdminModuleSavePath;

//NLMISC_VARIABLE( std::string, AdminModuleSavePath, "File where users adventure are stored." );

//const std::string CServerAdminModule::_AdminModuleSaveFilename( "server_admin_module_data.xml");

NLNET_REGISTER_MODULE_FACTORY(CServerAdminModule,"ServerAdminModule");



namespace R2
{
CServerAdminModule* CServerAdminModule::_Instance = 0;

//------------------------------------------------------------------------------



CServerAdminModule& CServerAdminModule::getInstance()
{
	nlassert(_Instance);
	return *_Instance;
}

TSessionId CServerAdminModule::getSessionIdByCharId(uint32 charId) const
{
	TSessionId editionSessionId = _Server->getEditionModule()->getSessionIdByCharId(charId);
	if (editionSessionId.asInt()!=0) return editionSessionId;
	TSessionId animationSessionId = _Server->getAnimationModule()->getSessionIdByCharId(charId);
	if (animationSessionId.asInt()!=0) return animationSessionId;
	return TSessionId(0);
}


bool CServerAdminModule::getPosition(TSessionId sessionId, double&x, double&y, double& orient, uint8& season, uint32 actId)
{


	IServerAnimationModule* anim = _Server->getAnimationModule();
	nlassert(anim);
	IServerEditionModule* edit = _Server->getEditionModule();
	nlassert(edit);

	x=0;
	y=0;
	orient=0;
	season=0;

	CScenario* scenario = edit->getScenarioById(sessionId);

	if (!scenario)
	{
		return false;
	}

	if (scenario->isEditing())
	{
		return edit->getPosition(sessionId, x, y, orient, season, actId);
	}

	if (scenario->isRunning())
	{
		bool ok = anim->getPosition(sessionId, x, y, orient, season, actId);
		if (!ok)
		{
			return edit->getPosition(sessionId, x, y, orient, season, actId);// return start position
		}
		return ok;

	}
	return false;
}




void CServerAdminModule::onModuleUp(NLNET::IModuleProxy *moduleProxy)
{
	if (moduleProxy->getModuleClassName() == "ServerAnimationModule")
	{
		_ServerAnimationProxy = moduleProxy;
	}
	else if (moduleProxy->getModuleClassName() == "ServerEditionModule")
	{
		_ServerEditionProxy = moduleProxy;
	}
}


void CServerAdminModule::onModuleDown(NLNET::IModuleProxy *moduleProxy)
{
	if (moduleProxy->getModuleClassName() == "ServerAnimationModule")
	{
		_ServerAnimationProxy = NULL;
	}
	else if (moduleProxy->getModuleClassName() == "ServerEditionModule")
	{
		_ServerEditionProxy = NULL;
	}
}


void CServerAdminModule::init(NLNET::IModuleSocket* gateway, CDynamicMapService* server)
{
	_Server = server;
	this->plugModule(gateway);
}


bool CServerAdminModule::onProcessModuleMessage(IModuleProxy * /* senderModuleProxy */, const CMessage &msgin)
{
	std::string operationName = msgin.getName();


	nlwarning("R2Admin: Invalid Operation Name '%s'", operationName.c_str() );

	return false;
}


void CServerAdminModule::onModuleSecurityChange(IModuleProxy * /* moduleProxy */)
{
}


CServerAdminModule::CServerAdminModule()
{
	_Instance = this;
}


CServerAdminModule::~CServerAdminModule()
{
	_Instance = 0;
}





NLMISC_CLASS_COMMAND_IMPL(CServerAdminModule, displayIslands)
{
	if (args.size() != 0)
		return false;


	CScenarioEntryPoints&  epManager = CScenarioEntryPoints::getInstance();
	const CScenarioEntryPoints::TCompleteIslands& islands =  epManager.getCompleteIslands();



	CScenarioEntryPoints::TCompleteIslands::const_iterator first(islands.begin()), last(islands.end());


	uint32 id = 0;
	for (; first != last; ++first)
	{
		++id;
		log.displayNL("%u: '%s' '%s' %s'",  id, first->Package.c_str(), first->Island.c_str(), first->Continent.c_str());

		CScenarioEntryPoints::TShortEntryPoints::const_iterator firstPoint(first->EntryPoints.begin()), lastPoint(first->EntryPoints.end());

		for (; firstPoint != lastPoint; ++firstPoint)
		{
			log.displayNL("\t-> %u: '%s' '%d' '%d'",
				id,
				firstPoint->Location.c_str(),
				firstPoint->X,
				firstPoint->Y
				);
		}
	}

	return true;
}



}
