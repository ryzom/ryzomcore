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

#include "ryzom_mirror_properties.h"
#include "mirrored_data_set.h"
#include "mode_and_behaviour.h"

// See doc in .h
// fe_temp dataset
TPropertyIndex DSPropertyPOSX = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyPOSY = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyPOSZ = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyORIENTATION = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyTICK_POS = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyAI_INSTANCE = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertySHEET = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertySHEET_SERVER = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyBEHAVIOUR = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyNAME_STRING_ID = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyTARGET_ID = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyCONTEXTUAL = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyMODE = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyVPA = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyVPB = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyVPC = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyENTITY_MOUNTED_ID = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyRIDER_ENTITY_ID = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyCELL = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyVISION_COUNTER = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyCURRENT_HIT_POINTS = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyMAX_HIT_POINTS = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyCURRENT_RUN_SPEED = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyCURRENT_WALK_SPEED = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyBEST_ROLE = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyBEST_ROLE_LEVEL = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertySTUNNED = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyCOMBAT_STATE = INVALID_PROPERTY_INDEX;
//TPropertyIndex DSPropertyKAMI_FAME = INVALID_PROPERTY_INDEX;
//TPropertyIndex DSPropertyKARAVAN_FAME = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyWHO_SEES_ME = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyBARS = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyTEAM_ID = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyACTION_FLAGS = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyTARGET_LIST = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyVISUAL_FX = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyGUILD_SYMBOL = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyGUILD_NAME_ID = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyIN_OUTPOST_ZONE_ALIAS = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyIN_OUTPOST_ZONE_SIDE = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyEVENT_FACTION_ID = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyPVP_MODE = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyPVP_CLAN = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyFUEL =  INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyOWNER_PEOPLE = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyOUTPOST_INFOS = INVALID_PROPERTY_INDEX;
TPropertyIndex DSPropertyNPC_ALIAS = INVALID_PROPERTY_INDEX;


std::string BehaviourToStringCb( void *value )
{
	MBEHAV::CBehaviour behaviour( *(TYPE_BEHAVIOUR*)value );
	return behaviour.toString();
}

std::string ModeToStringCb( void *value )
{
	MBEHAV::TMode mode( *(uint64*)value );
	return mode.toString();
}

std::string CoordToString( void *value )
{
	sint32 &coord = *((sint32*)value);
	return NLMISC::toString("%.3f", coord/1000.0);
}

std::string RowIdToStringCb(void *value)
{
	uint32 &index = *((uint32*) value);

	if ((index & 0xffffff) == 0xffffff)
		return "not valid";
	else
		return NLMISC::toString("E%u_%u", index & 0xffffff, index >> 24);
}

std::string Uint64HexToStringCb(void *value)
{
	uint64 &i64 = *((uint64*) value);
	uint16 *i16p = (uint16*)(&i64);

	return NLMISC::toString("%04x:%04x:%04x:%04x", i16p[0], i16p[1], i16p[2], i16p[3]);
}

#define assignProp( suffix, propName ) \
nlverify( (DSProperty##suffix = dataset.getPropertyIndex( std::string(#propName) )) != INVALID_PROPERTY_INDEX );

/*
 * Assign the property indices for fe_dataset
 */
void	initRyzomVisualPropertyIndices( CMirroredDataSet& dataset )
{
	assignProp( POSX, X );
	assignProp( POSY, Y );
	assignProp( POSZ, Z );
	assignProp( ORIENTATION, Theta );
	assignProp( TICK_POS, TickPos );
	assignProp( AI_INSTANCE, AIInstance );
	assignProp( SHEET, Sheet );
	assignProp( SHEET_SERVER, SheetServer );
	assignProp( BEHAVIOUR, Behaviour );
	assignProp( NAME_STRING_ID, NameIndex );
	assignProp( TARGET_ID, Target );
	assignProp( CONTEXTUAL, ContextualProperty );
	assignProp( MODE, Mode );
	assignProp( VPA, VisualPropertyA );
	assignProp( VPB, VisualPropertyB );
	assignProp( VPC, VisualPropertyC );
	assignProp( ENTITY_MOUNTED_ID, EntityMounted );
	assignProp( RIDER_ENTITY_ID, RiderEntity );
	assignProp( CELL, Cell );
	assignProp( VISION_COUNTER, VisionCounter );
	assignProp( CURRENT_HIT_POINTS, CurrentHitPoints );
	assignProp( MAX_HIT_POINTS, MaxHitPoints );
	assignProp( CURRENT_RUN_SPEED, CurrentRunSpeed );
	assignProp( CURRENT_WALK_SPEED, CurrentWalkSpeed );
	assignProp( BEST_ROLE, BestRole );
	assignProp( BEST_ROLE_LEVEL, BestRoleLevel );
	assignProp( STUNNED, Stunned );
	assignProp( COMBAT_STATE, CombatState );
	assignProp( BARS, Bars );

//	assignProp( KAMI_FAME, KamiFame );
//	assignProp( KARAVAN_FAME, KaravanFame );

	assignProp( WHO_SEES_ME, WhoSeesMe );

	assignProp( TEAM_ID, TeamId );
	assignProp( RIDER_ENTITY_ID, RiderEntity );

	assignProp( ACTION_FLAGS, ActionFlags );

	assignProp( TARGET_LIST, TargetList );
	assignProp( VISUAL_FX, VisualFX );
	assignProp( GUILD_SYMBOL, GuildSymbol );
	assignProp( GUILD_NAME_ID, GuildNameId );
	assignProp( IN_OUTPOST_ZONE_ALIAS, InOutpostZoneAlias );
	assignProp( IN_OUTPOST_ZONE_SIDE, InOutpostZoneSide );
	assignProp( EVENT_FACTION_ID, EventFactionId );
	assignProp( PVP_MODE, PvpMode );
	assignProp( PVP_CLAN, PvpClan );

	assignProp( FUEL, Fuel );

	assignProp( OWNER_PEOPLE, OwnerPeople );

	assignProp( OUTPOST_INFOS, OutpostInfos );

	assignProp( NPC_ALIAS, NPCAlias );

	dataset.setDisplayCallback( DSPropertyPOSX, CoordToString );
	dataset.setDisplayCallback( DSPropertyPOSY, CoordToString );
	dataset.setDisplayCallback( DSPropertyPOSZ, CoordToString );
	dataset.setDisplayCallback( DSPropertyBEHAVIOUR, BehaviourToStringCb );
	dataset.setDisplayCallback( DSPropertyMODE, ModeToStringCb );
	dataset.setDisplayCallback( DSPropertyTARGET_ID, RowIdToStringCb );
	dataset.setDisplayCallback( DSPropertyENTITY_MOUNTED_ID, RowIdToStringCb );
	dataset.setDisplayCallback( DSPropertyRIDER_ENTITY_ID, RowIdToStringCb );
	dataset.setDisplayCallback( DSPropertyVPA, Uint64HexToStringCb );
	dataset.setDisplayCallback( DSPropertyVPB, Uint64HexToStringCb );
	dataset.setDisplayCallback( DSPropertyVPC, Uint64HexToStringCb );
	dataset.setDisplayCallback( DSPropertyWHO_SEES_ME, Uint64HexToStringCb );

}

