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



#ifndef EGS_ENTITIES_GAME_SERVICE_H
#define EGS_ENTITIES_GAME_SERVICE_H

#include "nel/misc/time_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/log.h"
#include "nel/misc/displayer.h"
#include "nel/misc/hierarchical_timer.h"

#include "nel/net/service.h"

#include "game_share/backup_service_interface.h"
#include "world_instances.h"
#include "cdb_group.h"

// Callback connection / disconnection management
void cbConnection( const std::string &serviceName, NLNET::TServiceId sid, void *arg );
void cbDisconnection( const std::string &serviceName, NLNET::TServiceId sid, void *arg );
void cbMirrorUp( const std::string &serviceName, NLNET::TServiceId sid, void *arg );
void cbMirrorDn( const std::string &serviceName, NLNET::TServiceId sid, void *arg );

/**
 * CPlayerService
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CPlayerService : 
	public NLNET::IService, 
	public CWorldInstances::IAIInstanceReady
{
public:

	/** 
	 * init the service
	 */
	void init();

	/**
	 * load the sart position and other parameters
	 */
	void initConfigFileVars();

	/**
	 * main loop
	 */
	bool update();

	/**
	 * EGS update
	 */
	static void egsUpdate();

	/**
	 * EGS synchronization
	 */
	static void egsSync();

	/**
	 * update regen due to variable change in cfg
	 */
	static void updateRegen(NLMISC::IVariable &var);

	/**
	 * release
	 */
	void release();

	// ai instance ready callback
	void onAiInstanceReady(const CReportStaticAIInstanceMsg &msg);

	// ai instance down callback
	void onAiInstanceDown(const CReportStaticAIInstanceMsg &msg);

	// call back character ready for monkey loader
	void egsAddMonkeyPlayerCallback(uint32 userId);

private:
	// monkey players load simulation
	static void egsLoadMonkey();
	
	// add a random player
	static void egsAddMonkeyPlayer();

	// remove a random player
	static void removeMonkeyPlayer();
};

extern std::string				StatPath;
extern NLMISC::CLog				EgsStat;
extern CCDBGroup				DbGroupGlobal;

#endif //EGS_ENTITIES_GAME_SERVICE_H



