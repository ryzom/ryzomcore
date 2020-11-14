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

#include "stdpch.h"

#include	"ai_types.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;


std::map<std::string,bool>	strMap;

bool	StrToBool	(bool &res, const	std::string	&str, bool defaultVal)
{

	if (strMap.empty())
	{
		strMap.insert(std::make_pair(std::string("on"),true));
		strMap.insert(std::make_pair(std::string("ON"),true));
		strMap.insert(std::make_pair(std::string("true"),true));
		strMap.insert(std::make_pair(std::string("TRUE"),true));
		strMap.insert(std::make_pair(std::string("1"),true));
		strMap.insert(std::make_pair(std::string("off"),false));
		strMap.insert(std::make_pair(std::string("OFF"),false));
		strMap.insert(std::make_pair(std::string("false"),false));
		strMap.insert(std::make_pair(std::string("FALSE"),false));
		strMap.insert(std::make_pair(std::string("0"),false));
	}
	
	std::map<std::string,bool>::iterator	it=strMap.find(str);
	if (it==strMap.end())
	{
		res=defaultVal;		//	default value;
		return	false;
	}
	
	res=it->second;
	return true;
}

namespace	AITYPES	{


	//	have to create a new file for this ..
	
	template <> CDescType<TAIType>::CDescTypeEntry	CDescType<TAIType>::_entries[]	=
	{
		CDescTypeEntry(	"MANAGER",				AITypeManager	),
		CDescTypeEntry(	"NOGO",					AITypeNoGo	),
		CDescTypeEntry(	"GRP",					AITypeGrp	),
//		CDescTypeEntry(	"GRP_PARAMETERS",		AITypeGrpParameters	),
		CDescTypeEntry(	"PLACE",				AITypePlace	),
		CDescTypeEntry(	"FAUNA_PLACE",			AITypePlaceFauna	),
		CDescTypeEntry(	"GRP_FAUNA_POP",		AITypeGrpFaunaPop	),
		CDescTypeEntry(	"BOT_NPC",				AITypeBot	),
		CDescTypeEntry(	"NPC_STATE_ZONE",		AITypeNpcStateZone	),
		CDescTypeEntry(	"NPC_STATE_ROUTE",		AITypeNpcStateRoute	),
		CDescTypeEntry(	"NPC_PUNCTUAL_STATE",	AITypePunctualState	),
		CDescTypeEntry(	"NPC_STATE_PROFILE",	AITypeNpcStateProfile	),
		CDescTypeEntry(	"NPC_EVENT",			AITypeEvent			),
		CDescTypeEntry(	"NPC_EVENT_ACTION",		AITypeEventAction	),
		CDescTypeEntry(	"FOLDER",				AITypeFolder		),
		CDescTypeEntry(	"FAUNA_STATE",			AITypeState	),
		CDescTypeEntry(	"DEPOSIT_ZONE",			AITypeKamiDeposit	),		
		CDescTypeEntry(	"KARAVAN_STATE",		AITypeKaravanState	),
		CDescTypeEntry(	"NPC_STATE_CHAT",		AITypeNpcStateChat	),

		CDescTypeEntry(	"DYNAMIC_SYSTEM",		AITypeDynamicSystem	),
		CDescTypeEntry(	"DYNAMIC_REGION",		AITypeDynamicRegion ),
		CDescTypeEntry(	"CELL_ZONE",			AITypeCellZone	),
		CDescTypeEntry(	"CELL",					AITypeCell	),
		CDescTypeEntry(	"DYN_FAUNA_ZONE",		AITypeDynFaunaZone	),
		CDescTypeEntry(	"DYN_NPC_ZONE",			AITypeDynNpcZonePlace	),
		CDescTypeEntry(	"DYN_NPC_ZONE_PATATE",	AITypeDynNpcZoneShape	),
		CDescTypeEntry(	"DYN_ROAD",				AITypeDynRoad	),
		CDescTypeEntry(	"TRIGGER",				AITypeRoadTrigger	),
		CDescTypeEntry(	"GROUP_FAMILY",			AITypeGroupFamily	),
		
		CDescTypeEntry(	"GROUP_TEMPLATE",		AITypeGroupTemplate	),
		CDescTypeEntry(	"GROUP_TEMPLATE_ML",	AITypeGroupTemplateMultiLevel	),
		CDescTypeEntry(	"GROUP_TEMPLATE_FAUNA",	AITypeGroupTemplateFauna	),
		CDescTypeEntry(	"GROUP_TEMPLATE_NPC",	AITypeGroupTemplateNpc	    ),
	//	CDescTypeEntry(	"GROUP_TEMPLATE_GENERIC",	AITypeGroupTemplateGeneric	), // to test
		CDescTypeEntry(	"FAUNA_DYN_COMP",		AITypeGroupFamilyProfileFauna	),
		CDescTypeEntry(	"TRIBE_DYN_COMP",		AITypeGroupFamilyProfileTribe	),
		CDescTypeEntry(	"NPC_DYN_COMP",			AITypeGroupFamilyProfileNpc	    ),
	//	CDescTypeEntry(	"GENERIC_DYN_COMP",		AITypeGroupFamilyProfileGeneric	), // to test
		
		CDescTypeEntry(	"GROUP_CONFIG",			AITypeGroupConfig	),
		CDescTypeEntry(	"BOT_TEMPLATE",			AITypeBotTemplate ),
		CDescTypeEntry(	"BOT_TEMPLATE_ML",		AITypeBotTemplateMultiLevel ),
		CDescTypeEntry( "SQUAD_TEMPLATE",		AITypeSquadTemplate ),
		CDescTypeEntry( "SQUAD_TEMPLATE_VARIANT", AITypeSquadTemplateVariant ),
		CDescTypeEntry( "SQUAD_TEMPLATE_MEMBER", AITypeSquadTemplateMember ),
		CDescTypeEntry(	"OUTPOST",				AITypeOutpost ),
		CDescTypeEntry(	"OUTPOST_CHARGE",		AITypeOutpostCharge ),
		CDescTypeEntry(	"OUTPOST_SQUAD_FAMILY",	AITypeOutpostSquadFamily ),
		CDescTypeEntry(	"OUTPOST_SPAWN_ZONE",	AITypeOutpostSpawnZone ),
		CDescTypeEntry(	"OUTPOST_BUILDING",		AITypeOutpostBuilding ),
		CDescTypeEntry(	"NOGO_POINT",			AITypeNogoPoint ),
		CDescTypeEntry(	"NOGO_POINT_LIST",		AITypeNogoPointList ),
		CDescTypeEntry(	"CELL_ZONE_ENERGY",		AITypeCellZoneEnergy	),
		CDescTypeEntry(	"FAUNA_SPAWN_ATOM",		AITypeFaunaSpawnAtom	),
		
		CDescTypeEntry(	"FAUNA_ACTION_ZONE",	AITypeFaunaActionZone	),
		CDescTypeEntry(	"NPC_ACTION_ZONE",	    AITypeNpcActionZone	),
		CDescTypeEntry(	"SAFE_ZONE",			AITypeSafeZone	),
		
		CDescTypeEntry(	"SCRIPT",				AITypeScript	),
		
		CDescTypeEntry(	"USER_MODEL",			AITypeUserModel	),
		CDescTypeEntry(	"USER_MODEL_LIST",		AITypeUserModelList	),
		CDescTypeEntry( "CUSTOM_LOOT_TABLES",	AITypeCustomLootTables ),
		CDescTypeEntry( "CUSTOM_LOOT_TABLE",	AITypeCustomLootTable ),
		CDescTypeEntry( "CUSTOM_LOOT_SET",		AITypeCustomLootSet ),
		CDescTypeEntry(	"CELL_ZONES",			AITypeFolder ),
		CDescTypeEntry(	"GEOM_ITEMS",			AITypeFolder ),
		CDescTypeEntry(	"GROUP_FAMILY",			AITypeFolder	),
		CDescTypeEntry(	"GROUP_DESCRIPTIONS",	AITypeFolder	),


		CDescTypeEntry(	"KAMI_STATE",			AITypeNpcStateRoute	),
		CDescTypeEntry(	"FAUNA_EVENT",			AITypeEvent			),
		CDescTypeEntry(	"FAUNA_EVENT_ACTION",	AITypeEventAction	),		
		CDescTypeEntry(	"GRP_NPC",				AITypeGrp		),
//		CDescTypeEntry(	"GRP_PARAMETERS",		AITypeGrpParameters	),
		CDescTypeEntry(	"FAUNA_SPAWN",			AITypeGrpFaunaPop	),
		CDescTypeEntry(	"GROUP_FAUNA",			AITypeGrp	),
		CDescTypeEntry(	"GROUP_KARAVAN",		AITypeGrp	),
		CDescTypeEntry(	"GROUP_KAMI",			AITypeGrp	),
		CDescTypeEntry(	"GROUP_NPC",			AITypeGrp	),
		CDescTypeEntry(	"NPC_BOT",				AITypeBot	),
		CDescTypeEntry(	"NO_GO",				AITypeNoGo		),
		CDescTypeEntry(	"FAUNA_PUNCTUAL_STATE",	AITypePunctualState	),		// unused ?

		CDescTypeEntry(	"EVENT",				AITypeEvent			),
		CDescTypeEntry(	"EVENT_ACTION",			AITypeEventAction	),
		CDescTypeEntry(	"ACTION_ZONE",			AITypeActionZone	),
		CDescTypeEntry(	"STATE",				AITypeState			),
		
		CDescTypeEntry(	"SPIRE",				AITypeSpire	),
		
		CDescTypeEntry(	"BAD_TYPE",				AITypeBadType	)
	};

	template <> CDescType<TAITypeSpec>::CDescTypeEntry	CDescType<TAITypeSpec>::_entries[]	=
	{
		//	made for debug.
		CDescTypeEntry(	"GRP_FAUNA",			AITypeSpecFauna	),
		CDescTypeEntry(	"GRP_NPC",				AITypeSpecNpc	),
		CDescTypeEntry(	"GROUP_KAMI",			AITypeSpecKami	),
		CDescTypeEntry(	"GROUP_KARAVAN",		AITypeSpecKaravan	),
		CDescTypeEntry(	"GRP_FAUNA_POP",		AITypeSpecFauna	),
		CDescTypeEntry(	"BOT_NPC",				AITypeSpecNpc	),
		CDescTypeEntry(	"NPC_STATE_ZONE",		AITypeSpecNpc	),
		CDescTypeEntry(	"NPC_STATE_ROUTE",		AITypeSpecNpc	),
		CDescTypeEntry(	"NPC_PUNCTUAL_STATE",	AITypeSpecNpc	),
		CDescTypeEntry(	"NPC_STATE_PROFILE",	AITypeSpecNpc	),
		CDescTypeEntry(	"FAUNA_SPAWN",			AITypeSpecFauna	),
		CDescTypeEntry(	"GROUP_FAUNA",			AITypeSpecFauna	),
		CDescTypeEntry(	"FAUNA_STATE",			AITypeSpecFauna	),
		CDescTypeEntry(	"FAUNA_EVENT",			AITypeSpecFauna	),
		CDescTypeEntry(	"FAUNA_EVENT_ACTION",	AITypeSpecFauna	),
		CDescTypeEntry(	"GROUP_NPC",			AITypeSpecNpc	),
		CDescTypeEntry(	"NPC_BOT",				AITypeSpecNpc	),
		CDescTypeEntry(	"KARAVAN_STATE",		AITypeSpecKaravan	),
		CDescTypeEntry(	"BAD_TYPE",				AITypeSpecBadType	)
	};
	
	template <> CDescType<TMgrType>::CDescTypeEntry	CDescType<TMgrType>::_entries[]	=
	{
		CDescTypeEntry(	"FAUNA",		MgrTypeFauna),
		CDescTypeEntry(	"KARAVAN",		MgrTypeKaravan),
		CDescTypeEntry(	"KAMI",			MgrTypeKami),
		CDescTypeEntry(	"NPC",			MgrTypeNpc),
//		CDescTypeEntry(	"TRIBE",		MgrTypeTribe),
		CDescTypeEntry(	"PET",			MgrTypePet),
		CDescTypeEntry(	"GUILD_NPC",	MgrTypeGuildNpc),
		CDescTypeEntry(	"OUTPOST",		MgrTypeOutpost),
		CDescTypeEntry(	"BAD_TYPE",		MgrTypeBadType)
	};	
	
	template <> CDescType<TFaunaType>::CDescTypeEntry	CDescType<TFaunaType>::_entries[]	=
	{					
		CDescTypeEntry(	"HERBIVORE",	FaunaTypeHerbivore),
		CDescTypeEntry(	"PREDATOR",		FaunaTypePredator),
		CDescTypeEntry(	"PLANT",		FaunaTypePlant),
		CDescTypeEntry(	"SCAVENGER",	FaunaTypePredator),
		CDescTypeEntry(	"OMNIVORE",			FaunaTypePredator),		// !!?
		CDescTypeEntry(	"PREDATOR_PLANT",	FaunaTypePredator),		// !!?
		CDescTypeEntry(	"PET",				FaunaTypeHerbivore),	// !!?
		CDescTypeEntry(	"BAD_TYPE",		FaunaTypeBadType)
	};	
	
	template <> CDescType<TEffectType>::CDescTypeEntry	CDescType<TEffectType>::_entries[]	=
	{					
		CDescTypeEntry(	"NEGATIVE",	EffectNegative),
		CDescTypeEntry(	"POSITIVE",	EffectPositive),
		CDescTypeEntry(	"STUN",		EffectStun),
		CDescTypeEntry(	"ROOT",		EffectRoot),
		CDescTypeEntry(	"BLIND",	EffectBlind),
		CDescTypeEntry(	"MAZE",		EffectMaze),
		CDescTypeEntry(	"SLOW",		EffectSlow),
		CDescTypeEntry(	"BAD_TYPE",	EffectTypeBadType)
	};	
	
	template <> CDescType<TSpawnType>::CDescTypeEntry	CDescType<TSpawnType>::_entries[]	=
	{					
		CDescTypeEntry(	"NEVER",	SpawnTypeNever	),
		CDescTypeEntry(	"DAY",		SpawnTypeDay	),
		CDescTypeEntry(	"NIGHT",	SpawnTypeNight	),
		CDescTypeEntry(	"ALWAYS",	SpawnTypeAlways	),
		CDescTypeEntry(	"BAD_TYPE",	SpawnTypeBadType	)
	};	

	template <> CDescType<TProfiles>::CDescTypeEntry	CDescType<TProfiles>::_entries[]	=
	{					
		CDescTypeEntry(	"BOT_FOLLOW_POS",		BOT_FOLLOW_POS),
		CDescTypeEntry(	"BOT_STAND_AT_POS",		BOT_STAND_AT_POS),
		CDescTypeEntry(	"FIGHT",		BOT_FIGHT),
		CDescTypeEntry(	"FLEE",			BOT_FLEE),
		CDescTypeEntry(	"GO_AWAY",		BOT_GO_AWAY),
		
		CDescTypeEntry(	"MOVE_FOLLOW_ROUTE",		MOVE_FOLLOW_ROUTE),
		CDescTypeEntry(	"MOVE_GOTO_POINT",			MOVE_GOTO_POINT),
		CDescTypeEntry(	"MOVE_IDLE",				MOVE_IDLE),
		CDescTypeEntry(	"MOVE_STAND_AT_POINT",		MOVE_STAND_AT_POINT),
		CDescTypeEntry(	"MOVE_STAND_ON_VERTICES",	MOVE_STAND_ON_VERTICES	),
		CDescTypeEntry(	"MOVE_WAIT",				MOVE_WAIT),
		CDescTypeEntry(	"ACTIVITY_NORMAL",			ACTIVITY_NORMAL),
		CDescTypeEntry(	"ACTIVITY_GUARD",			ACTIVITY_GUARD),
		CDescTypeEntry(	"ACTIVITY_TRIBU",			ACTIVITY_TRIBU),
		CDescTypeEntry(	"ACTIVITY_ESCORTED",		ACTIVITY_ESCORTED),
		
		CDescTypeEntry(	"ACTIVITY_BANDIT",			ACTIVITY_BANDIT),
		CDescTypeEntry(	"FIGHT_NORMAL",				FIGHT_NORMAL),
		

		CDescTypeEntry(	"WANDER",		ACTIVITY_WANDERING),
		CDescTypeEntry(	"GRAZE",		ACTIVITY_GRAZING),
		CDescTypeEntry(	"REST",			ACTIVITY_RESTING),
		CDescTypeEntry(	"PLANT_IDLE",	ACTIVITY_PLANTIDLE),
		CDescTypeEntry(	"CORPSE",		ACTIVITY_CORPSE),
		CDescTypeEntry(	"EAT_CORPSE",	ACTIVITY_EAT_CORPSE),		
		CDescTypeEntry(	"CURIOSITY",	ACTIVITY_CURIOSITY),
		CDescTypeEntry(	"CONTACT",		ACTIVITY_CONTACT),
		CDescTypeEntry(	"HARVEST",		ACTIVITY_HARVEST),	
		CDescTypeEntry(	"FIGHT",		ACTIVITY_FIGHT),
		CDescTypeEntry(	"FACTION",		ACTIVITY_FACTION),
		CDescTypeEntry(	"SQUAD",		ACTIVITY_SQUAD),
		
		CDescTypeEntry(	"MOVE_DYN_FOLLOW_PATH",	MOVE_DYN_FOLLOW_PATH),
		CDescTypeEntry(	"MOVE_CAMPING",	MOVE_CAMPING),
		CDescTypeEntry(	"ZONE_WAIT",	ZONE_WAIT),

		CDescTypeEntry(	"CURIOSITY",	PET_STAND),
		CDescTypeEntry(	"CURIOSITY",	PET_FOLLOW),
		CDescTypeEntry(	"CURIOSITY",	PET_GOTO),
		CDescTypeEntry(	"CURIOSITY",	PET_GOTO_AND_DESPAWN),

		CDescTypeEntry(	"BAD_TYPE",		BAD_TYPE)
	};
	
//	CDescType<TFaunaActivity>::CDescTypeEntry	CDescType<TFaunaActivity>::_entries[]	=
//	{					
//		CDescTypeEntry(	"WANDER",		FaunaActivityWander),
//		CDescTypeEntry(	"GRAZE",		FaunaActivityGraze),
//		CDescTypeEntry(	"REST",			FaunaActivityRest),
//		CDescTypeEntry(	"FIGHT",		FaunaActivityFight),
//		CDescTypeEntry(	"FLEE",			FaunaActivityFlee),
//		CDescTypeEntry(	"PURSUE",		FaunaActivityPursue),
//		CDescTypeEntry(	"PLANT_IDLE",	FaunaActivityPlantIdle),
//		CDescTypeEntry(	"CORPSE",		FaunaActivityCorpse),
//		CDescTypeEntry(	"BAD_TYPE",		FaunaActivityBadType)
//	};


/*	NL_BEGIN_STRING_CONVERSION_TABLE(TFaunaZoneActivity)
	{	"activity_fauna_spawn", act_fz_spawn},
	{	"activity_fauna_food_herb", act_fz_food_herb},
	{	"activity_fauna_food_carn", act_fz_food_carn},
	{	"activity_fauna_rest_herb", act_fz_rest_herb},
	{	"activity_fauna_rest_carn", act_fz_rest_carn}
	NL_END_STRING_CONVERSION_TABLE(TFaunaZoneActivity, faunaActivityConversionTable, act_fz_none)
	const std::string &toString(TFaunaZoneActivity activity)
	{
		return faunaActivityConversionTable.toString(activity);
	}

	NL_BEGIN_STRING_CONVERSION_TABLE(TNpcZoneActivity)
	{ "activity_npc_harvest", act_nz_harvest},
	{	"activity_npc_ambush", act_nz_ambush},			
	{	"activity_npc_rest", act_nz_rest},			
	{	"activity_npc_outpost", act_nz_outpost},
	{	"activity_npc_spawn", act_nz_spawn},	
	{	"activity_npc_outpost_def", act_nz_outpost_def},
	{	"activity_npc_outpost_atk", act_nz_outpost_atk},	
	{	"activity_npc_kami_wander", act_nz_kami_wander}	
	NL_END_STRING_CONVERSION_TABLE(TNpcZoneActivity, npcActivityConversionTable, act_nz_none)
	const std::string &toString(TNpcZoneActivity activity)
	{
		return npcActivityConversionTable.toString(activity);
	}
*/
	
//	NL_BEGIN_STRING_CONVERSION_TABLE(TZoneActivity)
//	{	"activity_fauna_spawn", act_fz_spawn},
//	{	"activity_fauna_food_herb", act_fz_food_herb},
//	{	"activity_fauna_food_carn", act_fz_food_carn},
//	{	"activity_fauna_rest_herb", act_fz_rest_herb},
//	{	"activity_fauna_rest_carn", act_fz_rest_carn},
//
//	{   "activity_npc_harvest",		act_nz_harvest},
//	{	"activity_npc_ambush",		act_nz_ambush},			
//	{	"activity_npc_rest",		act_nz_rest},			
//	{	"activity_npc_outpost",		act_nz_outpost},
//	{	"activity_npc_spawn",		act_nz_spawn},	
//	{	"activity_npc_outpost_def", act_nz_outpost_def},
//	{	"activity_npc_outpost_atk", act_nz_outpost_atk},	
//	{	"activity_npc_kami_wander", act_nz_kami_wander}	,
//	{	"activity_npc_escort",		act_nz_escort},
//	{	"activity_npc_convoy",		act_nz_convoy},
//	{	"activity_npc_contact",		act_nz_contact},
//	{	"activity_npc_fight",		act_nz_fight},
//
//	{	"activity_npc_contact_camp",	act_nz_contact_camp},
//	{	"activity_npc_contact_outpost", act_nz_contact_outpost},
//	{	"activity_npc_contact_city",	act_nz_contact_city},
//	{	"activity_npc_fight_boss",		act_nz_contact_city},
//	{	"act_nz_fight_boss",			act_nz_fight_boss},
//
//	{	"act_fz_food_kitin",		act_fz_food_kitin},
//	{	"act_fz_food_kitin_invasion",	act_fz_food_kitin_invasion},
//	{	"act_fz_rest_kitin_invasion",	act_fz_rest_kitin_invasion},
//	{	"act_fz_food_degen",		act_fz_food_degen},
//	{	"act_fz_plant",				act_fz_plant	},
//	{	"act_fz_rest_kitin",		act_fz_rest_kitin},
//	{	"act_fz_rest_degen",		act_fz_rest_degen}
//	NL_END_STRING_CONVERSION_TABLE(TZoneActivity, activityConversionTable, act_none)
//	const std::string &toString(TZoneActivity activity)
//	{
//		return activityConversionTable.toString(activity);
//	}

//	std::vector<std::pair<std::string, NLMISC::TStringId> > TPopulationFamily::_TribeNames;
//
//	NL_BEGIN_STRING_CONVERSION_TABLE(TFamilyTag)
//		{ "fauna_herbivore",	family_fauna_herbivore},
//		{ "fauna_carnivore",	family_fauna_carnivore},
//		{ "flora",				family_flora},
//		{ "civil", 	family_civil},
//		{ "bandit", family_bandit},
//		{ "tribe",	family_tribe},
//		{ "kitin",	family_kitin},
//		{ "kitin_invasion",	family_kitin_invasion},
//		{ "kami",	family_kami},
//		{ "karavan", family_karavan},
//		{ "degen",	family_degen},
//		{ "goo",	family_goo},
//		{ "mp",		family_mp},
//	NL_END_STRING_CONVERSION_TABLE(TFamilyTag, familyConversionTable, family_bad)
//
//	void TPopulationFamily::init()
//	{
//		static bool inited = false;
//		if (!inited)
//		{
//			try
//			{
//				// read the list of existing tribe
//				CConfigFile::CVar &var = IService::getInstance()->ConfigFile.getVar("TribeNamePath");
//				vector<string> files;
//				CPath::getPathContent(var.asString(), false, false, true, files);
//
//				for (uint i=0; i<files.size(); ++i)
//				{
//					string fn = CFile::getFilename(files[i]);
//					if (CFile::getExtension(fn) == "html")
//					{
//						if (fn.find("tribe_") == 0)
//						{
//							// this is a tribe name
//							string tn = CFile::getFilenameWithoutExtension(fn);
//							_TribeNames.push_back(make_pair(tn, CStringMapper::map(tn)));
//						}
//					}
//				}
//			}
//			catch(...)
//			{
//				nlwarning("Can't find var 'TribeNamePath' in config file. Tribe name will not be available.");
//			}
//
//			inited = true;
//		}
//	}
//
//
//	TPopulationFamily::TPopulationFamily(const std::string &familyName)
//	{
//		init();
//
//		FamilyTag = family_bad;
//		TribeName = CStringMapper::emptyId();
//
//		// the tribe name list is initilised, now look for a tribe name
//		if	(familyName.find("tribe_") == 0)
//		{
//			// this is a tribe
//			string tn = familyName.substr(6);
//			
//			for (uint i=0; i<_TribeNames.size(); ++i)
//			{
//				if (_TribeNames[i].first == familyName)
//				{
//					// ok, we found the tribe name
//					FamilyTag = family_tribe;
//					TribeName = _TribeNames[i].second;
//					return;
//				}
//
//			}
//
//		}
//		else
//			FamilyTag = familyConversionTable.fromString(familyName);
//	}
//
//	const std::string &TPopulationFamily::getFamilyName() const
//	{
//		if (FamilyTag == family_tribe)
//		{
//			return CStringMapper::unmap(TribeName);
//		}
//		else
//		{
//			return familyConversionTable.toString(FamilyTag);
//		}
//
//	}
//
//	const std::vector<std::pair<std::string, NLMISC::TStringId> > &TPopulationFamily::getTribesNames()
//	{
//		init();
//		return _TribeNames;
//	}
//
//	void TPopulationFamily::getFamilyNames(std::vector<std::string> &result)
//	{
//		for (uint i=0; i<family_bad; ++i)
//		{
//			result.push_back(familyConversionTable.toString(static_cast<TFamilyTag>(i)));
//		}
//
//	}

	NL_BEGIN_STRING_CONVERSION_TABLE(TVerticalPos)
		{ "auto",	vp_auto },
		{ "upper", 	vp_upper},
		{ "middle", vp_middle},
		{ "lower", 	vp_lower},
	NL_END_STRING_CONVERSION_TABLE(TVerticalPos, verticalPosConversionTable, vp_auto)

	TVerticalPos verticalPosFromString(const std::string &vpName)
	{
		return verticalPosConversionTable.fromString(vpName);
	}

	const std::string &verticalPosToString(TVerticalPos vPos)
	{
		return verticalPosConversionTable.toString(vPos);
	}

}
