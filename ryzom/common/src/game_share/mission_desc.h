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



#ifndef RY_MISSION_DESC_H
#define RY_MISSION_DESC_H

#include "nel/misc/types_nl.h"

// Number of missions a player can take
const uint	MAX_NUM_MISSIONS = 15;

// Number of mission tagret in a mission
const uint	MAX_NUM_MISSION_TARGETS = 8;

namespace MISSION_DESC
{
	enum TMissionType
	{
		Solo = 0,
		Group,
		Guild
	};

	enum TStepType
	{
		KillCreature = 0,
		EndDynSteps = KillCreature,
		Talk,
		Target,
		SkillProgress,
		Craft,
		Harvest,
		BuyItem,
		SellItem,
		KillNpc,
		GiveItem,
		NbStepTypes,
	};
	enum TDifficulty
	{
		Easy = 0,
		Medium,
		Hard,
		NbDifficulties
	};

	enum TLength
	{
		Short = 0,
		Normal,
		Long,
		NbLength
	};

	enum TRewardType
	{
		Seeds = 0,
		Sp,
		ZCBuilding,
		NbReward,
	};

	/** This enum describe the visual on client (for window title text for instance)
	 *	You must change botchat_v4.xml
	 */
	enum TClientMissionType
	{
		Mission= 0,				// Describe a standard mission
		ZCRequirement,			// A Guild Mission, requirement to take a ZC pacifically
		BuildingRequirement,	// A Guild Mission, requirement to
		ZCCharge,				// Fake Mission. Represent a ZC Charge acquiring
		Building,				// Fake Mission. Represent a Building acquiring
		RMBuy,					// Fake Mission. Represent a BotMaster buying
		RMUpgrade,				// Fake Mission. Represent a BotMaster upgrading

		NumClientMissionType
	};

	/** This enum describe the visual on client. This is what filled in ICON Database field
	 *	If you change this enum, you MUST also change <options type="mission_icons" name="mission_icons">
	 *	in client config.xml.
	 *	You must also change the IconToClientMissionType below
	 */
	enum TIconId
	{
		IconNone= 0,
		IconMNCraft,					// 01) BK_generic.tga	ICO_Task_Craft.tga
		IconMNFight,					// 02) BK_generic.tga	ICO_Task_Fight.tga
		IconMNForage,					// 03) BK_generic.tga	ICO_Task_Forage.tga
		IconMNTravel,					// 04) BK_generic.tga	ICO_Task_Travel.tga
		IconMNGeneric,					// 05) BK_generic.tga	ICO_Task_Generic.tga
		IconMNRite,						// 06) BK_generic.tga	ICO_Task_Rite.tga
		IconMGGuild,					// 07) BK_guild.tga		ICO_Task_Guild.tga
		IconMGCraft,					// 08) BK_guild.tga		ICO_Task_Craft.tga
		IconMGFight,					// 09) BK_guild.tga		ICO_Task_Fight.tga
		IconMGForage,					// 10) BK_guild.tga		ICO_Task_Forage.tga
		IconMGTravel,					// 11) BK_guild.tga		ICO_Task_Travel.tga
		IconZCCharge,
		IconRMMagicBuy,
		IconRMFightBuy,
		IconRMCraftBuy,
		IconRMMagicUpgrade,
		IconRMFightUpgrade,
		IconRMCraftUpgrade,
		IconZCRequirement,
		IconBuildingRequirement,
		IconBuilding,
		NumIcons
	};

	// For each IconId, gives the ClientMissionType
	const	TClientMissionType		IconToClientMissionType[]=
	{
		Mission,
		Mission,
		Mission,
		Mission,
		Mission,
		Mission,
		Mission,
		Mission,
		Mission,
		Mission,
		Mission,
		Mission,
		ZCCharge,
		RMBuy,
		RMBuy,
		RMBuy,
		RMUpgrade,
		RMUpgrade,
		RMUpgrade,
		ZCRequirement,
		BuildingRequirement,
		Building
	};

	// The pre-requesit state of a mission (the client know why he can't do a mission)
	enum TPreReqState
	{
		PreReqSuccess = 0,
		PreReqFailAlreadyDone,
		PreReqFail,
		PreReqFailRunning
	};

	/// get the TClientMissionType according to TIconId. return Mission if error.
	TClientMissionType	getClientMissionType(TIconId iconId);

	const std::string & toString(TStepType type);
	TStepType toStepType( const std::string & str );

	const std::string & toString(TRewardType type);
	TRewardType toRewardType( const std::string & str );
}


#endif // RY_MISSION_DESC_H

/* End of mission_desc.h */
