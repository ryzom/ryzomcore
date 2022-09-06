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



#ifndef GE_GPMS_H
#define GE_GPMS_H

#include "nel/misc/types_nl.h"

#include <string>
#include <map>
#include <set>
#include <list>
#include <vector>

#include "nel/misc/common.h"
#include "nel/misc/time_nl.h"
#include "nel/net/service.h"

#include "game_share/ryzom_entity_id.h"
#include "game_share/entity_types.h"
#include "game_share/tick_event_handler.h"
#include "game_share/mirror.h"

#include "server_share/r2_vision.h"

#include "move_checker.h"
#include "world_position_manager.h"
#include "vision_delta_manager.h"
#include "messages.h"
#include "variables.h"


#define MAX_MSG_ELT 400

class CGlobalPositionManagerService;
extern CGlobalPositionManagerService	*pCGPMS;



/**
 * CGlobalPositionManagerService
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CGlobalPositionManagerService : public NLNET::IService
{
public:

	/** 
	 * init the service
	 */
	void init(void);

	/// Init after the mirror init
	void initMirror();

	/**
	 * main loop
	 */
	bool update(void);

	/**
	 * update, called at each tick
	 */
	static void gpmsUpdate();

	/**
	 * method called to treat mirror updates on non-ring shards
	 */
	static void	processMirrorUpdates();

	/**
	 * method called to treat mirror updates on ring shards
	 */
	static void ringShardProcessMirrorUpdates();

	/**
	 * release
	 */
	void release(void);

	/**
	 * private method used to manage creation of instances in the ring vision universe
	 */
	void _checkAddCharacterToRingAIInstance(sint32 aiInstance);

	/**
	 * private method used to manage deletion of instances from the ring vision universe
	 */
	void _checkRemoveCharacterFromRingAIInstance(sint32 aiInstance);

	// Set of updated AI
	//std::set< NLMISC::CEntityId > EntityIAUpdated;

	std::vector<NLMISC::CEntityId>	Tracked;

	NLMISC::CEntityId	EntityTrack0;
	NLMISC::CEntityId	EntityTrack1;

	CMirror				Mirror;
	CMirroredDataSet	*DataSet;

	NLMISC::CSmartPtr<R2_VISION::CUniverse> RingVisionUniverse;
	NLMISC::CSmartPtr<CVisionDeltaManager>  RingVisionDeltaManager;
	NLMISC::CSmartPtr<CMoveChecker> MoveChecker;

	typedef std::map<uint32,uint32> TCharactersPerAIInstance;
	TCharactersPerAIInstance _CharactersPerAIInstance;
};


#define TheMirror (pCGPMS->Mirror)
#define TheDataset (*(pCGPMS->DataSet))


#endif //GE_GPMS_H



