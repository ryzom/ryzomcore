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



#ifndef RY_MISSION_MESSAGES_H
#define RY_MISSION_MESSAGES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/net/transport_class.h"
#include "game_share/base_types.h"
#include "game_share/synchronised_message.h"
#include "game_share/mission_desc.h"

/**
 * Message send by EGS to AIS to request a dynamic mission
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
/*class CDynMissionRequestMsg : public CMirrorTransportClass
{
public:

	/// Id of the request being answered
	uint32			RequestId;

	/// the id of the mission giver
	TDataSetRow		MissionGiver;

	uint8	MissionType;
	uint8	Difficulty;
	uint8	Length;

	virtual void description ()
	{
		className ("CDynMissionRequestMsg");
		property ("RequestId", PropUInt32, (uint32)0,RequestId);
		property ("MissionGiver", PropDataSetRow, TDataSetRow(),MissionGiver);
		property ("MissionType", PropUInt8, (uint8)0,MissionType);
		property ("Difficulty", PropUInt8, (uint8)0,Difficulty);
		property ("Length", PropUInt8, (uint8)0,Length);
	}

	virtual void callback (const std::string &name, uint8 id) {}

};
*/
/**
 * Message send by AIS to answer an EGS dynamic mission request
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
/*class CDynMissionDescMsg : public CMirrorTransportClass
{
public:

	/// Id of the request being answered
	uint32 RequestId;

	/// steps : 
	/// bot ending the step
	std::vector<TDataSetRow>		EndingStepBot;
	/// type of the step
	std::vector<uint8>				StepTypes;
	/// SheetParameter
	std::vector<NLMISC::CSheetId>	StepParamSheet1;
	/// uint parameter
	std::vector<uint32>				StepParamUint1;
	std::vector<uint32>				StepParamUint2;

	/// type of the reward
	uint8							RewardType;
	/// parameter of the reward
	uint64							RewardParam;

	void addKillCreatureStep( const TDataSetRow & endingBot, const NLMISC::CSheetId & sheet, uint32 quantity )
	{
		StepTypes.push_back( MISSION_DESC::KillCreature );
		EndingStepBot.push_back( endingBot );
		StepParamSheet1.push_back( sheet );
		StepParamUint1.push_back( quantity );
	}
	
	

	virtual void description ()
	{
		className ("CMissionDescMsg");

		property ("RequestId", PropUInt32, (uint32)0, RequestId);

		propertyCont ("EndingStepBot", PropDataSetRow, EndingStepBot);
		propertyCont ("StepTypes", PropUInt8,StepTypes);
		propertyCont ("StepParamSheet1", PropSheetId,StepParamSheet1);
		propertyCont ("StepParamUint1", PropUInt32,StepParamUint1);
		propertyCont ("StepParamUint2", PropUInt32,StepParamUint2);

		property ("RewardParam", PropUInt64, (uint64)0, RewardParam);
		property ("RewardType", PropUInt8, (uint8)MISSION_DESC::NbReward, RewardType);
	}
	
	virtual void callback (const std::string &name, uint8 id) {}
};
*/


#endif // RY_MISSION_MESSAGES_H

/* End of mission_messages.h */
