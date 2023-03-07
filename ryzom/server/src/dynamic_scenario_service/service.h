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

#ifndef R2_SERVICE_H
#define R2_SERVICE_H

#include "nel/misc/types_nl.h"
#include "nel/net/service.h"
#include "game_share/dyn_chat.h"


namespace NLNET
{
	class IModuleSocket;
}
namespace NLMISC
{
	class CConfigFile;
	struct CEntityId;
}
namespace R2
{
class CDynamicMapService;

class CDynamicScenarioService : public NLNET::IService
{

public:
	//CDynamicScenarioService();
	//~CDynamicScenarioService();
	// Initialisation of service
	void init ();

	// Update net processing 
	bool update ();

	// Update service processing
	static void serviceUpdate();

	// Release the service
	void release ();

	void addTestClient(const std::string & clientId);

	void removeTestClient(const std::string & clientId);

	void listTestClient() const;

	void runTestClientLuaScript(const std::string & clientId, const std::string & cmd);
	

	static R2::CDynamicScenarioService & instance()  { return (CDynamicScenarioService&)*IService::getInstance(); }

	R2::CDynamicMapService & getDynamicMapService() const { return *_Dms; }
	
	void forwardToStringManagerModule (NLNET::CMessage &msgin);
	

	void forwardIncarnChat(TChanID id,TDataSetRow senderId,ucstring sentence);
	

private:
	R2::CDynamicMapService* _Dms;


};	
} //namespace R2

#endif //R2_SERVICE_H

