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

#ifndef NL_AIS_CONTROL_H
#define NL_AIS_CONTROL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"
#include "game_share/far_position.h"
#include "ais_control.h"

class IAisControl
:	public NLMISC::CManualSingleton<IAisControl>
{
public:

	virtual void activateEasterEgg(uint32 easterEggId, TSessionId scenarioId, uint32 actId, const std::string &items, float x, float y, float z, float heading, const std::string &grpCtrl, const std::string& name="", const std::string& look="") =0;
	
	virtual void deactivateEasterEgg(uint32 easterEggId, TSessionId scenarioId, uint32 actId) =0;
	
	virtual void dssMessage(TSessionId sessionId, const std::string& mode, const std::string& who, const std::string &msg) =0;

	virtual void giveRewardMessage(TDataSetRow characterRowId, TDataSetRow creatureRowId,
								const std::string& rewardText,
								const std::string& rareRewardText,
								const std::string& inventoryFullText,
								const std::string& notEnoughPointsText) = 0;

	virtual void teleportNearMessage(const NLMISC::CEntityId& entity, float x, float y, float z) = 0;

	virtual void reportNpcControl(const NLMISC::CEntityId& playerEntityId, const NLMISC::CEntityId& botEntityId) = 0;
	virtual void reportStopNpcControl(const NLMISC::CEntityId& playerEntityId, const NLMISC::CEntityId& botEntityId) = 0;
	virtual void setScenarioPoints(TSessionId sessionId, float scenarioPoints) = 0;
	virtual void startScenarioTiming(TSessionId sessionId) = 0; 
	virtual void endScenarioTiming(TSessionId sessionId) = 0;
	

};


#endif // NL_AIS_CONTROL_H

/* End of ais_control.h */
