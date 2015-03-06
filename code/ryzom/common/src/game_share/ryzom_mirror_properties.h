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



#ifndef NL_RYZOM_MIRROR_PROPERTIES_H
#define NL_RYZOM_MIRROR_PROPERTIES_H

#include "nel/misc/types_nl.h"
#include "base_types.h"


// AI Service Tag for Mirror Traffic Reduction. Must not collide with a tag of another service.
const TMTRTag AISTag = 1;


/*
 * Dataset property indices. Must be assigned in mirror initialization callback,
 * using initRyzomVisualPropertyIndices() (or manually, using CMirroredDataSet::getPropertyIndex(propName)).
 */

// fe_temp
extern TPropertyIndex DSPropertyPOSX;
extern TPropertyIndex DSPropertyPOSY;
extern TPropertyIndex DSPropertyPOSZ;
extern TPropertyIndex DSPropertyORIENTATION;
extern TPropertyIndex DSPropertyTICK_POS;
extern TPropertyIndex DSPropertyAI_INSTANCE;
extern TPropertyIndex DSPropertySHEET;
extern TPropertyIndex DSPropertySHEET_SERVER;
extern TPropertyIndex DSPropertyBEHAVIOUR;
extern TPropertyIndex DSPropertyNAME_STRING_ID;
extern TPropertyIndex DSPropertyTARGET_ID;
extern TPropertyIndex DSPropertyCONTEXTUAL;
extern TPropertyIndex DSPropertyMODE;
extern TPropertyIndex DSPropertyVPA;
extern TPropertyIndex DSPropertyVPB;
extern TPropertyIndex DSPropertyVPC;
extern TPropertyIndex DSPropertyENTITY_MOUNTED_ID;
extern TPropertyIndex DSPropertyRIDER_ENTITY_ID;
extern TPropertyIndex DSPropertyCELL;
extern TPropertyIndex DSPropertyVISION_COUNTER;
extern TPropertyIndex DSPropertyCURRENT_HIT_POINTS;
extern TPropertyIndex DSPropertyMAX_HIT_POINTS;
extern TPropertyIndex DSPropertyCURRENT_RUN_SPEED;
extern TPropertyIndex DSPropertyCURRENT_WALK_SPEED;
extern TPropertyIndex DSPropertyBEST_ROLE;
extern TPropertyIndex DSPropertyBEST_ROLE_LEVEL;
extern TPropertyIndex DSPropertySTUNNED;
extern TPropertyIndex DSPropertyCOMBAT_STATE;
extern TPropertyIndex DSPropertyKAMI_FAME;
extern TPropertyIndex DSPropertyKARAVAN_FAME;
extern TPropertyIndex DSPropertyWHO_SEES_ME;
extern TPropertyIndex DSPropertyBARS;
extern TPropertyIndex DSPropertyTEAM_ID;
extern TPropertyIndex DSPropertyACTION_FLAGS;
extern TPropertyIndex DSPropertyTARGET_LIST;
extern TPropertyIndex DSPropertyVISUAL_FX;
extern TPropertyIndex DSPropertyGUILD_SYMBOL;
extern TPropertyIndex DSPropertyGUILD_NAME_ID;
extern TPropertyIndex DSPropertyIN_OUTPOST_ZONE_ALIAS;
extern TPropertyIndex DSPropertyIN_OUTPOST_ZONE_SIDE;
extern TPropertyIndex DSPropertyEVENT_FACTION_ID;
extern TPropertyIndex DSPropertyPVP_MODE;
extern TPropertyIndex DSPropertyPVP_CLAN;
extern TPropertyIndex DSPropertyFUEL;
extern TPropertyIndex DSPropertyOWNER_PEOPLE;
extern TPropertyIndex DSPropertyOUTPOST_INFOS;
extern TPropertyIndex DSPropertyNPC_ALIAS;


class CMirroredDataSet;

/// Assign the property indices for fe_temp
void	initRyzomVisualPropertyIndices( CMirroredDataSet& dataset );


#define TYPE_POSX sint32
#define TYPE_POSY sint32
#define TYPE_POSZ sint32
#define TYPE_ORIENTATION float // please do not lower it or tell Olivier
#define TYPE_TICK_POS NLMISC::TGameCycle
#define TYPE_AI_INSTANCE sint32
#define TYPE_SHEET uint32
#define TYPE_SHEET_SERVER
#define TYPE_BEHAVIOUR uint64
#define TYPE_NAME_STRING_ID uint32 // please do not lower it or tell Olivier
#define TYPE_TARGET_ID TDataSetRow // same
#define TYPE_CONTEXTUAL uint16
#define TYPE_MODE uint64
#define TYPE_VPA uint64 // please keep the
#define TYPE_VPB uint64 // same type for
#define TYPE_VPC uint64 // VPA, VPB, VPC
#define TYPE_ENTITY_MOUNTED_ID TDataSetRow
#define TYPE_RIDER_ENTITY_ID TDataSetRow
#define TYPE_CELL sint32
#define TYPE_VISION_COUNTER uint8

#define TYPE_CURRENT_HIT_POINTS sint32
#define TYPE_MAX_HIT_POINTS sint32
#define TYPE_RUNSPEED float
#define TYPE_WALKSPEED float
#define TYPE_BEST_ROLE uint16
#define TYPE_BEST_ROLE_LEVEL uint16
#define TYPE_STUNNED bool
#define TYPE_COMBAT_STATE uint8
#define TYPE_KAMI_FAME sint32
#define TYPE_KARAVAN_FAME sint32
#define TYPE_WHO_SEES_ME uint64
#define TYPE_BARS uint32
#define TYPE_TEAM_ID uint16
#define TYPE_ACTION_FLAGS uint16
#define TYPE_TARGET_LIST sint32			// dirty hack?
#define TYPE_TARGET_LIST_ITEM sint32
#define TYPE_VISUAL_FX sint16
#define TYPE_AVAILABLE_IMPULSE_SIZE uint16

#define TYPE_GUILD_SYMBOL uint64
#define TYPE_GUILD_NAME_ID uint32

#define TYPE_IN_OUTPOST_ZONE_ALIAS uint32
#define TYPE_IN_OUTPOST_ZONE_SIDE uint8

#define TYPE_BOT_TRADE_SELECTOR1 uint64
#define TYPE_BOT_TRADE_SELECTOR2 uint64

#define TYPE_EVENT_FACTION_ID uint32
#define TYPE_PVP_MODE uint16
#define TYPE_PVP_CLAN uint32

#define TYPE_FUEL bool

#define TYPE_OWNER_PEOPLE uint8

#define TYPE_OUTPOST_INFOS uint16

#define TYPE_ALIAS uint32 // TAIAlias

#endif // NL_RYZOM_MIRROR_PROPERTIES_H

/* End of ryzom_mirror_properties.h */
