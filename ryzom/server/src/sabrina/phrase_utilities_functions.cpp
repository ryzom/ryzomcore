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

#include "phrase_utilities_functions.h"
#include "egs_mirror.h"

// game share
#include "game_share/mode_and_behaviour.h"
#include "game_share/tick_event_handler.h"
#include "game_share/msg_combat_move_service.h"
#include "game_share/characteristics.h"
// georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
// misc
#include "nel/misc/command.h"
// std
#include <map>

//
#include "player_manager.h"
#include "phrase_manager.h"
#include "creature_manager.h"
#include "egs_globals.h"


extern uint8					CMSIsUp;
extern CPlayerManager			PlayerManager;
extern CCreatureManager			CreatureManager;
extern CGameItemManager			GameItemManager;
extern CPhraseManager			*PhraseManager;
extern bool						AllowPVP;


extern uint8					EntityForcedDefaultLevel; // 0 by default, it's the level of an entity when it has a level 0 (for tests purposes only)

extern CGenericXmlMsgHeaderManager	GenericMsgManager;

using namespace std;
using namespace NLGEORGES;
using namespace NLMISC;
using namespace NLNET;


bool VerboseBrickManagerInfo = false;
bool VerboseBrickManagerDbg = false;
bool AggroLog = false;

NLMISC_COMMAND(verboseBrickManagerInfo,"Turn on or off or check the state of verbose brick manager logging (for info)","")
{
	if(args.size()>1)
		return false;
	if(args.size()==1)
	{
		if(args[0]==string("on")||args[0]==string("ON")||args[0]==string("true")||args[0]==string("TRUE")||args[0]==string("1"))
			VerboseBrickManagerInfo=true;

		if(args[0]==string("off")||args[0]==string("OFF")||args[0]==string("false")||args[0]==string("FALSE")||args[0]==string("0"))
			VerboseBrickManagerInfo=false;
	}

	log.displayNL("VerboseBrickManagerInfo is %s",VerboseBrickManagerInfo?"ON":"OFF");
	return true;
}

// debug msg
NLMISC_COMMAND(verboseBrickManagerDbg,"Turn on or off or check the state of verbose brick manager logging (for debug)","")
{
	if(args.size()>1)
		return false;
	if(args.size()==1)
	{
		if(args[0]==string("on")||args[0]==string("ON")||args[0]==string("true")||args[0]==string("TRUE")||args[0]==string("1"))
			VerboseBrickManagerDbg=true;

		if(args[0]==string("off")||args[0]==string("OFF")||args[0]==string("false")||args[0]==string("FALSE")||args[0]==string("0"))
			VerboseBrickManagerDbg=false;
	}

	log.displayNL("VerboseBrickManagerDbg is %s",VerboseBrickManagerDbg?"ON":"OFF");
	return true;
}

// aggro msg
NLMISC_COMMAND(aggroLog,"Turn on or off aggro log display or check the state of the flag","")
{
	if(args.size()>1)
		return false;
	if(args.size()==1)
	{
		if(args[0]==string("on")||args[0]==string("ON")||args[0]==string("true")||args[0]==string("TRUE")||args[0]==string("1"))
			AggroLog=true;

		if(args[0]==string("off")||args[0]==string("OFF")||args[0]==string("false")||args[0]==string("FALSE")||args[0]==string("0"))
			AggroLog=false;
	}

	log.displayNL("AggroLog is %s",AggroLog?"ON":"OFF");
	return true;
}



namespace PHRASE_UTILITIES
{
std::string toString(ERange range)
{
	switch(range)
	{
	case POINT_BLANK:
		return std::string("POINT_BLANK");
	case SHORT_RANGE:
		return std::string("SHORT_RANGE");
	case MEDIUM_RANGE:
		return std::string("MEDIUM_RANGE");
	case LONG_RANGE:
		return std::string("LONG_RANGE");
	default:
		return std::string("UNDEFINED");
	};
}

typedef set< SLocalisation >				TLocalisationSet;

/// table of damage localisation (take shield into account)
TLocalisationSet		LocalisationTable;

/// the table giving the localisation adjustement for the specified attacker and defender size
sint8					LocalisationAdjustments[CREATURE_SIZE::NB_SIZE][CREATURE_SIZE::NB_SIZE];


static string OrientationStrings[]=
{
	"W",
	"WSW",
	"SW",
	"SSW",
	"S",
	"SSE",
	"SE",
	"ESE",
	"E",
	"ENE",
	"NE",
	"NNE",
	"N",
	"NNW",
	"NW",
	"WNW",
};


//--------------------------------------------------------------
//			entityPtrFromId()  
//--------------------------------------------------------------
CEntityBase* entityPtrFromId( const CEntityId &id )
{
	switch( id.getType() )
	{
	case RYZOMID::player:
		return PlayerManager.getChar( id );
		break;
	
	case RYZOMID::npc:
	case RYZOMID::creature:
	case RYZOMID::kami:
		return CreatureManager.getCreature( id );
		break;

	default:
		nlwarning( "<entityPtrFromId> entity type %d unknown", id.getType(), id.toString().c_str() );
		return 0;
	}
}

//--------------------------------------------------------------
//					loadLocalisationTable()  
//--------------------------------------------------------------
void loadLocalisationTable( const std::string &tableName )
{
	UFormLoader *loader = UFormLoader::createLoader ();

	CSmartPtr<UForm> form = loader->loadForm (tableName.c_str());
	if ( !form)
	{
		nlwarning("<loadLocalisationTable> Failed to load the form %s", tableName.c_str() );
		return;
	}

	// Get the root node, always exist
    UFormElm &root = form->getRootNode ();

	const UFormElm *array = NULL;
	// lines
	if (root.getNodeByName (&array, "Lines") && array)
    {
		 // Get array size
        uint size;
		array->getArraySize (size);

        // Get a array value
        for (uint i=0; i<size; ++i)
        {
			const UFormElm *line = NULL;
			if ( array->getArrayNode( &line, i) && line)
			{
				SLocalisation localisation;
				// the localisation number
				line->getValueByName( localisation.LocalisationNumber, "ValueNumber" );

				string shield;
				// the type of shield
				line->getValueByName( shield, "ShieldType" );
				localisation.ShieldType = SHIELDTYPE::stringToShieldType( shield );

				string slot;
				line->getValueByName( slot, "Localisation" );
				localisation.Slot = SLOT_EQUIPMENT::stringToSlotEquipment( slot);
				
				line->getValueByName( localisation.ShieldIsEffective, "ProtectionWorks" );

				LocalisationTable.insert( localisation );

//nlinfo("loadLocalisationTable : for value %d and shield %s, localisation=%s, shield=%d",localisation.LocalisationNumber, shield.c_str(),  slot.c_str(),  localisation.ShieldIsEffective);
			}
        }
	}
} // loadLocalisationTable //





//--------------------------------------------------------------
//					loadLocalisationSizeAdjusmentTable()  
//--------------------------------------------------------------
void loadLocalisationSizeAdjusmentTable( const std::string &tableName )
{
	UFormLoader *loader = UFormLoader::createLoader ();

	CSmartPtr<UForm> form = loader->loadForm (tableName.c_str());
	if ( !form)
	{
		nlwarning("<loadLocalisationSizeAdjusmentTable> Failed to load the form %s", tableName.c_str() );
		return;
	}

	// Get the root node, always exist
    UFormElm &root = form->getRootNode ();

	const UFormElm *array = NULL;
	// lines
	if (root.getNodeByName (&array, "Table") && array)
    {
		 // Get array size
        uint size;
		array->getArraySize (size);

        // Get a array value
        for (uint i=0; i<size; ++i)
        {
			const UFormElm *line = NULL;
			if ( array->getArrayNode( &line, i) && line)
			{
				string attacker;
				line->getValueByName( attacker, "AttackerSize" );
				CREATURE_SIZE::ECreatureSize attackerSize = CREATURE_SIZE::stringToCreatureSize( attacker );

				string defender;
				line->getValueByName( defender, "DefenderSize" );
				CREATURE_SIZE::ECreatureSize defenderSize = CREATURE_SIZE::stringToCreatureSize( defender );

				sint8 modifier;
				line->getValueByName( modifier, "Adjustment" );

				if ( attackerSize > CREATURE_SIZE::NB_SIZE || defenderSize > CREATURE_SIZE::NB_SIZE || attackerSize < 0 || defenderSize < 0)
				{
					nlwarning("<loadLocalisationSizeAdjusmentTable> Invalid line, attacker size = %s, defender size = %s", attacker.c_str(), defender.c_str() );
					continue;
				}

				///
				LocalisationAdjustments[attackerSize][defenderSize] = modifier;
			}
		}
	}
} // loadLocalisationSizeAdjusmentTable //


//--------------------------------------------------------------
//					getSuccessChance()  
//--------------------------------------------------------------
uint8 getSuccessChance( sint32 playerRelativeLevel )
{
	// clip delta level to min / max value
	if( playerRelativeLevel > MAX_DELTA_LVL ) playerRelativeLevel = MAX_DELTA_LVL;
	if( playerRelativeLevel < MIN_DELTA_LVL ) playerRelativeLevel = MIN_DELTA_LVL;
	
	return (uint8) CEntityBaseManager::_SuccessXpTable[ MIDDLE_DELTA_LVL - playerRelativeLevel ].SuccessProbability;
} // getSuccessChance //


//--------------------------------------------------------------
//					getSuccessFactor()  
// Return SF (Success Factor) > 1 if critical success
// Return SF = 1 if success
// Return  1 < SF < 0 if partial success
// Return SF = 0 if Failure
// Return SF < 0 if Critical Failure
//--------------------------------------------------------------
float getSucessFactor( uint8 chances, uint8 tirage )
{
	// Check if critical success
	if( float(CEntityBaseManager::_SuccessTable.CriticalSuccess * chances) / float( CEntityBaseManager::_SuccessTable.CriticalSuccess + CEntityBaseManager::_SuccessTable.Success ) > float(tirage) ) return 1.1f;
	
	// Check if Success
	if( chances > tirage ) return 1.0f;
	
	// Check if Partial Success
	const uint8 pSChance = (uint8) (float(CEntityBaseManager::_SuccessTable.PartialSuccess) / float(CEntityBaseManager::_SuccessTable.PartialSuccess + CEntityBaseManager::_SuccessTable.Failure + CEntityBaseManager::_SuccessTable.CriticalFailure) * (100.0f-chances));
	if ( pSChance > (tirage-chances) )
	{
		return ( float(pSChance - tirage + chances) / float(pSChance));
	}
	
	// Check if Failure
	if( ( float(CEntityBaseManager::_SuccessTable.PartialSuccess + CEntityBaseManager::_SuccessTable.Failure ) / float(CEntityBaseManager::_SuccessTable.PartialSuccess + CEntityBaseManager::_SuccessTable.Failure + CEntityBaseManager::_SuccessTable.CriticalFailure ) * (100.0f-chances) ) > float(tirage-chances) ) 
		return 0.0f;
	
	// stay Critical Failure
	return -1.0f;
}


//--------------------------------------------------------------
//					testTargetValidityForCombat()  
//--------------------------------------------------------------
bool testTargetValidityForCombat( const CEntityId &targetId, string &errorCode )
{
	switch ( targetId.getType() )
	{
	case RYZOMID::player:
		break;

	case RYZOMID::creature:
	case RYZOMID::kami:
	case RYZOMID::npc:
	case RYZOMID::mount:
		{
			CCreature* entity = CreatureManager.getCreature( targetId );
			if (entity == NULL)
			{
				nlwarning("<testTargetValidityForCombat> Invalid entity Id %s", targetId.toString().c_str() );
				errorCode = "BS_INVALID_TARGET";
				return false;
			}
			
			/*
			// get creature form
			const CSheetId &sheet = entity->getType();

			const CStaticCreatures *creature = CSheets::getCreaturesForm( sheet );
			if ( ! creature)
			{
				nlwarning("<testTargetValidityForCombat> can't found CStaticCreature for entity %s", targetId.toString().c_str() );
				errorCode = "BS_INVALID_TARGET";
				return false;
			}
			*/
			
			if ( ! entity->getContextualProperty().getValue().attackable() )
			{
				errorCode = "BS_TARGET_NOT_ATTACKABLE";
				return false;
			}
			
		}
		break;

	default:
		break;

	};

	return true;
} // testTargetValidityForCombat //



//--------------------------------------------------------------
//					getLocalisation()  
//--------------------------------------------------------------
TPairSlotShield  getLocalisation( SHIELDTYPE::EShieldType shield, sint8 adjustement, SLOT_EQUIPMENT::TSlotEquipment forcedSlot)
{
	TPairSlotShield slotShield;
	SLocalisation local;

	if ( forcedSlot == SLOT_EQUIPMENT::UNDEFINED)
	{
		sint32 randVal = RandomGenerator.rand() + adjustement;
		if (randVal < 0)
			randVal = 0;

		local.LocalisationNumber = randVal%10 + 1;
		local.ShieldType = shield;

		TLocalisationSet::iterator it = LocalisationTable.find( local );
		if (it != LocalisationTable.end() )
		{
			slotShield.first = (*it).Slot;
			slotShield.second = (*it).ShieldIsEffective;
		}
		else
		{
			slotShield.first = SLOT_EQUIPMENT::UNDEFINED;
			slotShield.second = false;
		}
	}
	else
	{
		TLocalisationSet subTable;
		TLocalisationSet::iterator it;
		for (it = LocalisationTable.begin() ; it != LocalisationTable.end() ; ++it)
		{
			uint8 val = 0;
			if ( (*it).ShieldType == shield && (*it).Slot == forcedSlot )
			{
				SLocalisation local;
				local.LocalisationNumber = ++val;
				local.ShieldType = shield;
				local.ShieldIsEffective = (*it).ShieldIsEffective;
				local.Slot = forcedSlot;

				subTable.insert( local );
			}
		}

		if ( subTable.empty() == true )
		{
			slotShield.first = SLOT_EQUIPMENT::UNDEFINED;
			slotShield.second = false;
		}
		else
		{
			local.LocalisationNumber = 1 + (uint8)RandomGenerator.rand(subTable.size()-1);
			local.ShieldType = shield;

			it = subTable.find( local );
			if (it != subTable.end() )
			{			
				slotShield.first = (*it).Slot;
				slotShield.second = (*it).ShieldIsEffective;
			}
			else
			{			
				slotShield.first = SLOT_EQUIPMENT::UNDEFINED;
				slotShield.second = false;
			}
		}
	}

	INFOLOG("<getLocalisation> with shield %d, with forced slot %d, localisation is slot %d, and shield is %d (0=inactive) (random value is %u)", shield, forcedSlot, slotShield.first, slotShield.second, local.LocalisationNumber );

	return slotShield;
} // getLocalisation //



//--------------------------------------------------------------
//					getLocalisationSizeAdjustement()  
//--------------------------------------------------------------
sint8 getLocalisationSizeAdjustement(  const CEntityId &attacker,  const CEntityId &defender )
{
	CREATURE_SIZE::ECreatureSize attackerSize, defenderSize;

	CEntityBase* entity = entityPtrFromId( attacker );
	if (entity == NULL)
	{
		nlwarning("<getLocalisationSizeAdjustement> Invalid entity Id %s", attacker.toString().c_str() );
		return 0;
	}

	attackerSize = entity->getSize();

	entity = entityPtrFromId( defender );
	if (entity == NULL)
	{
		nlwarning("<getLocalisationSizeAdjustement> Invalid entity Id %s", defender.toString().c_str() );
		return 0;
	}

	defenderSize = entity->getSize();

	if ( attackerSize > CREATURE_SIZE::NB_SIZE || defenderSize > CREATURE_SIZE::NB_SIZE || attackerSize < 0 || defenderSize < 0)
	{
		return 0;
	}

	return LocalisationAdjustments[attackerSize][defenderSize];
} // getLocalisationSizeAdjustement //


//--------------------------------------------------------------
//					getWeaponDamageFactor()  
//--------------------------------------------------------------
float getWeaponDamageFactor( WEAPONTYPE::EWeaponType type, uint16 quality, uint16 entityLevel )
{
	if (quality > 21)
		quality = 21;

	if (quality == 0)
		return float(0);

	switch(type)
	{
	case WEAPONTYPE::LIGHT:
	case WEAPONTYPE::LIGHT_GUN:
		{
			if ( entityLevel > 0 && entityLevel > quality)
				return TableLightMeleeWeaponGenerics[quality-1].secondaryDamageFactor;
			else
				return TableLightMeleeWeaponGenerics[quality-1].damageFactor;
		}
		break;

	case WEAPONTYPE::MEDIUM:
	case WEAPONTYPE::MEDIUM_GUN:
		{
			if ( entityLevel > 0 && entityLevel > quality)
				return TableMediumMeleeWeaponGenerics[quality-1].secondaryDamageFactor;
			else
				return TableMediumMeleeWeaponGenerics[quality-1].damageFactor;
		}
		break;

	case WEAPONTYPE::HEAVY:
	case WEAPONTYPE::HEAVY_GUN:
		{
			if ( entityLevel > 0 && entityLevel > quality)
				return TableHeavyMeleeWeaponGenerics[quality-1].secondaryDamageFactor;
			else
				return TableHeavyMeleeWeaponGenerics[quality-1].damageFactor;
		}		
		break;

	case WEAPONTYPE::HANDS:
		return TableLightMeleeWeaponGenerics[quality-1].damageFactor * (float) HANDS_DAMAGE_FACTOR_MULTIPLIER;

	default:
		nlwarning("<getWeaponDamageFactor> Unknown weapon type %d, return 0", type);
		return 0;
	};
} // getWeaponDamageFactor //






//--------------------------------------------------------------
//					getArmorCharacteristics()  
//--------------------------------------------------------------
TPairUInt8UInt16 getArmorCharacteristics( ARMORTYPE::EArmorType type, uint16 quality )
{
	if (quality > 21)
		quality = 21;

	TPairUInt8UInt16 ret;

	switch(type)
	{
	case ARMORTYPE::LIGHT:
		ret.first = TableLightArmorGenerics[quality-1].percentProtection;
		ret.second = TableLightArmorGenerics[quality-1].maxProtection;
		break;

	case ARMORTYPE::MEDIUM:
		ret.first = TableMediumArmorGenerics[quality-1].percentProtection;
		ret.second = TableMediumArmorGenerics[quality-1].maxProtection;
		break;

	case ARMORTYPE::HEAVY:
		ret.first = TableHeavyArmorGenerics[quality-1].percentProtection;
		ret.second = TableHeavyArmorGenerics[quality-1].maxProtection;
		break;

	default:
		nlwarning("<getArmorCharacteristics> Unknown armor type %d, return 0", type);
		ret.first = 0;
		ret.second = 0;

	};

	return ret;
} // getArmorCharacteristics //


//--------------------------------------------------------------
//					getEntityLevel()  
//--------------------------------------------------------------
uint32 getEntityLevel( const CEntityId &entityId, SKILLS::ESkills skill, sint32 skillModifier, float skillMultiplier )
{
	CEntityBase* entity = entityPtrFromId( entityId );
	if (entity == NULL)
	{
		nlwarning("<getEntityLevel> Invalid entity Id %s", entityId.toString().c_str() );
		return 0;
	}

	sint32 level = 0;

//	skillModifier += entity->getGlobalSkillModifier();

	if ( skill < SKILLS::NUM_SKILLS )
	{
		level = (sint32) ( skillMultiplier * float(entity->getSkills()._Skills[ skill ].Current) );
		level += skillModifier;
		if (level < 0)
			level = 0;
	}
	
	level = getLevelFromSkill(level);
	
	if (level == 0 && EntityForcedDefaultLevel > 0)
	{
		DEBUGLOG("<getEntityLevel> ONLY DEBUG : For entity %s, returns a level of %d instead of 0 for skill %s", entityId.toString().c_str(),EntityForcedDefaultLevel, SKILLS::toString(skill).c_str());
		return EntityForcedDefaultLevel;
	}

	return (uint32)level;

} // getEntityLevel //

//--------------------------------------------------------------
//					getEntityLevel()  
//--------------------------------------------------------------
uint32 getEntityLevel( const CEntityId &entityId, SCORES::TScores score, sint32 scoreModifier, float scoreMultiplier )
{
	CEntityBase* entity = entityPtrFromId( entityId );
	if (entity == NULL)
	{
		nlwarning("<getEntityLevel> Invalid entity Id %s", entityId.toString().c_str() );
		return 0;
	}

	uint32 level = 0;

	
	if ( score != SCORES::unknown )
	{
		level = (sint32) ( scoreMultiplier * float(entity->getScores()._PhysicalScores[ score ].Current ) );
		level += scoreModifier;
		if (score < 0)
			level = 0;
	}

	level = getLevelFromScore(level);
	
	if (level == 0 && EntityForcedDefaultLevel > 0)
	{
		DEBUGLOG("<getEntityLevel> ONLY DEBUG : For entity %s, returns a level of %d instead of 0 for score %s", entityId.toString().c_str(),EntityForcedDefaultLevel, SCORES::toString(score).c_str() );
		return EntityForcedDefaultLevel;
	}

	return level;

} // getEntityLevel //


//--------------------------------------------------------------
//					getDistance()  
//--------------------------------------------------------------
double getDistance( const CEntityId &entity1, const CEntityId &entity2 )
{
	TDataSetRow entityIndex1 = TheDataset.getDataSetRow( entity1 );
	if ( !entityIndex1.isValid() || !TheDataset.isDataSetRowStillValid(entityIndex1) )
		return -1;
	CMirrorPropValueRO<TYPE_POSX> posX( TheDataset, entityIndex1, DSPropertyPOSX );
	const double playerX = (double)posX / 1000.0f;
	CMirrorPropValueRO<TYPE_POSY> posY( TheDataset, entityIndex1, DSPropertyPOSY );
	const double playerY = (double)posY / 1000.0f;

	double targetX, targetY;//, targetZ;
	// get second entity position
/*	if (entity2.Type != RYZOMID::position)
	{*/

	TDataSetRow entityIndex2 = TheDataset.getDataSetRow( entity2 );
	if ( !entityIndex2.isValid() || !TheDataset.isDataSetRowStillValid(entityIndex2) )
		return -1;
	CMirrorPropValueRO<TYPE_POSX> targX( TheDataset, entityIndex2, DSPropertyPOSX );
	targetX = (double)targX() / 1000.0f;
	CMirrorPropValueRO<TYPE_POSY> targY( TheDataset, entityIndex2, DSPropertyPOSY );
	targetY = (double)targY() / 1000.0f;

	// 
	INFOLOG("Player position (in meters) : X = %g, Y = %g", playerX, playerY );
	INFOLOG("Target position (in meters) : X = %g, Y = %g", targetX, targetY );

	const double d = sqrt( sqr(playerX-targetX) + sqr(playerY-targetY) ); //+sqr (playerZ-targetZ) );
	INFOLOG("Distance = %g meters", d);

	return d;
} // getDistance //


//--------------------------------------------------------------
//					sendUpdateBehaviour()  
//--------------------------------------------------------------
void sendUpdateBehaviour( const CEntityId &entityId, const MBEHAV::CBehaviour &behaviour, bool forceUpdate )
{
	MBEHAV::CBehaviour b = behaviour;

	b.Combat.Time = uint16(CTickEventHandler::getGameCycle() >>3);

	if (behaviour.Behaviour > MBEHAV::COMBAT_BEHAVIOUR_BEGIN && behaviour.Behaviour < MBEHAV::COMBAT_BEHAVIOUR_END)
	{
		INFOLOG("<sendUpdateBehaviour>	entity %s param : attackintensity = %u, impact intensity = %u, behaviour %s",entityId.toString().c_str(), behaviour.Combat.AttackIntensity, behaviour.Combat.ImpactIntensity,  MBEHAV::behaviourToString(behaviour.Behaviour).c_str() );
	}
	else if (behaviour.Behaviour > MBEHAV::MAGIC_CASTING_BEHAVIOUR_BEGIN && behaviour.Behaviour < MBEHAV::MAGIC_END_CASTING_BEHAVIOUR_END)
	{
		INFOLOG("<sendUpdateBehaviour>	entity %s param : SpellPower = %u, resist = %d, impact intensity = %u, behaviour %s", entityId.toString().c_str(), behaviour.Magic.TargetResists, behaviour.Magic.SpellPower, behaviour.Magic.ImpactIntensity,  MBEHAV::behaviourToString(behaviour.Behaviour).c_str() );
	}
	else
	{
		INFOLOG("<sendUpdateBehaviour> entity %s, behaviour %s",entityId.toString().c_str(), MBEHAV::behaviourToString(behaviour.Behaviour).c_str() );
	}

/*	switch( entityId.getType() )
	{
	case RYZOMID::player:
		{
			//CMessage msgout("SET_BEHAVIOUR");
			//msgout.serial( const_cast<CEntityId&> (entityId) );
			//msgout.serial( b );
			//sendMessageViaMirror ("OPS", msgout);
		}
		break;
	
	case RYZOMID::npc:
	case RYZOMID::creature:
	case RYZOMID::kami:
		{
			//----------TEMP
//			CMessage msg("SET_BEHAVIOUR");
//			msg.serial( const_cast<CEntityId&> (entityId) );
//			msg.serial( b );
//			sendMessageViaMirror ("OPS", msg);
			//----------TEMP

//			CMessage msgout("U_FIGHT_BEHAVIOUR");
//			msgout.serial( const_cast<CEntityId&> (entityId) );
//			string bes = MBEHAV::behaviourToString( behaviour.Behaviour );
			//msgout.serialEnum( behaviour );
//			msgout.serial( bes );
//			sendMessageViaMirror ("AIS", msgout);
		}
		break;

	default:
		nlwarning( "<sendUpdateBehaviour> entity type %d unknown, fail to send update behaviour msg", entityId.getType() );
	}
*/
	CEntityBase* entity = PHRASE_UTILITIES::entityPtrFromId( entityId );
	if (entity == NULL)
	{
		nlwarning("<sendUpdateBehaviour> Invalid entity Id %s", entityId.toString().c_str() );
		return;
	}

	entity->setBehaviour( b, forceUpdate );
} // sendUpdateBehaviour //



//--------------------------------------------------------------
//					sendSimpleMessage()  
//--------------------------------------------------------------
void sendSimpleMessage( const CEntityId &entityId, const std::string &msgName )
{
	if ( entityId.getType() != RYZOMID::player && entityId.getType() != RYZOMID::chatGroup && entityId.getType() != RYZOMID::dynChatGroup )
		return;

	CMessage msg("STATIC_STRING");

	msg.serial( const_cast<CEntityId&> (entityId) );
	
	set<CEntityId> excluded;
	msg.serialCont( excluded );
	
	msg.serial( const_cast<string&> (msgName) );

	sendMessageViaMirror ("IOS", msg);

	INFOLOG("<sendSimpleMessage>send %s for entity %s",msgName.c_str(), entityId.toString().c_str());
} // sendSimpleMessage //


//--------------------------------------------------------------
//					sendMessage()  
//--------------------------------------------------------------
void sendMessage( const NLMISC::CEntityId &entityId, const std::string &msgName, const ucstring &txt )
{
	if ( entityId.getType() != RYZOMID::player && entityId.getType() != RYZOMID::chatGroup && entityId.getType() != RYZOMID::dynChatGroup )
		return;

	/// TEMP : convert the uctring to a string
	const string txtStr = txt.toString();

	CMessage msg("STATIC_STRING");

	msg.serial( const_cast<CEntityId&> (entityId) );
	
	set<CEntityId> excluded;
	msg.serialCont( excluded );
	
	msg.serial( const_cast<string&> (msgName) );
	msg.serial( const_cast<string&> (txtStr) );

	sendMessageViaMirror ("IOS", msg);

	INFOLOG("<sendMessage>send %s (param %s) for entity %s",msgName.c_str(), txt.toString().c_str(), entityId.toString().c_str());
} // sendMessage //

//--------------------------------------------------------------
//					sendMessage()  
//--------------------------------------------------------------
void sendMessage( const NLMISC::CEntityId &entityId, const std::string &msgName, const NLMISC::CEntityId &entityIdForText )
{
	if ( entityId.getType() != RYZOMID::player && entityId.getType() != RYZOMID::chatGroup && entityId.getType() != RYZOMID::dynChatGroup )
		return;

	CMessage msg("STATIC_STRING");

	msg.serial( const_cast<CEntityId&> (entityId) );
	
	set<CEntityId> excluded;
	msg.serialCont( excluded );
	
	msg.serial( const_cast<string&> (msgName) );
	msg.serial( const_cast<CEntityId&> (entityIdForText) );

	sendMessageViaMirror ("IOS", msg);

	INFOLOG("<sendMessage>send %s (param entity %s) for entity %s",msgName.c_str(), entityIdForText.toString().c_str(), entityId.toString().c_str());
} // sendMessage //


//--------------------------------------------------------------
//					sendSimpleDynamicChatMessage()
//--------------------------------------------------------------
void sendDynamicChatMessage( const CEntityId &entityId, const string &msgText )
{
	if ( msgText.empty() )
		return;

	if ( entityId.getType() != RYZOMID::player && entityId.getType() != RYZOMID::chatGroup && entityId.getType() != RYZOMID::dynChatGroup )
		return;

	CMessage msg("CHAT_MESSAGE");

	TDataSetRow dsr = TheDataset.getDataSetRow(entityId)
	msg.serial( dsr );
	msg.serial( const_cast<string&> (msgText) );
	
	sendMessageViaMirror ("IOS", msg);

	INFOLOG("<sendSimpleDynamicChatMessage>send %s for entity %s",msgText.c_str(), entityId.toString().c_str());
} // sendSimpleDynamicChatMessage //



//--------------------------------------------------------------
//					sendCombatResistMessages()  
//--------------------------------------------------------------
void sendCombatResistMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId )
{
	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	string msgName;

	if ( victimId.getType() == RYZOMID::player)
	{
		msgName ="BS_MISSES_YOU_E";
		CMessage msgVictim("STATIC_STRING");

		msgVictim.serial( const_cast<CEntityId&> (victimId) );
		set<CEntityId> excluded;
		msgVictim.serialCont( excluded );
		msgVictim.serial( msgName );

		msgVictim.serial( const_cast<CEntityId&> (aggressorId) );

		sendMessageViaMirror ("IOS", msgVictim);

		INFOLOG("<sendCombatResistMessages>send BS_MISSES_YOU_E with param e=%s for entity %s",aggressorId.toString().c_str(), victimId.toString().c_str());
	}

	if ( aggressorId.getType() == RYZOMID::player)
	{
		msgName ="BS_YOU_MISS_E";
		CMessage msgAggressor("STATIC_STRING");

		msgAggressor.serial( const_cast<CEntityId&> (aggressorId) );
		set<CEntityId> excluded;
		msgAggressor.serialCont( excluded );
		msgAggressor.serial( msgName );

		msgAggressor.serial( const_cast<CEntityId&> (victimId) );

		sendMessageViaMirror ("IOS", msgAggressor);

		INFOLOG("<sendCombatResistMessages>send BS_YOU_MISS with for entity %s", aggressorId.toString().c_str());
	}

// spectators
	// Send to 'speech' group
	msgName = "BS_MISSES_EE";

	CEntityId speechGroupId;
	if ( aggressorId.getType() == RYZOMID::player )
	{
		speechGroupId = aggressorId;
	}
	else if ( victimId.getType() == RYZOMID::player )
	{
		speechGroupId = victimId;
	}
	else
		return;

	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msgVictim("STATIC_STRING");
	msgVictim.serial( speechGroupId );
	
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	
	msgVictim.serialCont( excluded );
	msgVictim.serial( msgName );
	msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
	msgVictim.serial( const_cast<CEntityId&> (victimId) );

	sendMessageViaMirror ("IOS", msgVictim);
} // sendCombatResistMessages //


//--------------------------------------------------------------
//					sendDeathMessages()  
//--------------------------------------------------------------
void sendDeathMessages( const NLMISC::CEntityId &killerId, const NLMISC::CEntityId &deadId )
{
	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (killerId.getType() != RYZOMID::player) && ( deadId.getType() != RYZOMID::player ))
		return;

	string msgName;

	if ( killerId.getType() == RYZOMID::player )
	{
		msgName = "OPS_DEATH_KILLER_E";
		CMessage msgAggressor("STATIC_STRING");

		msgAggressor.serial( const_cast<CEntityId&> (killerId) );
		set<CEntityId> excluded;
		msgAggressor.serialCont( excluded );
		msgAggressor.serial( msgName );
		
		msgAggressor.serial( const_cast<CEntityId&> (deadId) );

		sendMessageViaMirror ("IOS", msgAggressor);
	}

	if ( deadId.getType() == RYZOMID::player )
	{
		msgName = "OPS_DEATH_KILLED_E";
		CMessage msgVictim("STATIC_STRING");

		msgVictim.serial( const_cast<CEntityId&> (deadId) );
		set<CEntityId> excluded;
		msgVictim.serialCont( excluded );
		msgVictim.serial( msgName );
		msgVictim.serial( const_cast<CEntityId&> (killerId) );
		sendMessageViaMirror ("IOS", msgVictim);
	}

// spectators
	// Send to 'speech' group
	msgName = "OPS_DEATH_KILL_EE";

	CEntityId speechGroupId;
	if ( deadId.getType() == RYZOMID::player )
	{
		speechGroupId = deadId;
	}
	else
	{
		speechGroupId = killerId;
	}
	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msgSpectators("STATIC_STRING");

	msgSpectators.serial( speechGroupId );
	
	set<CEntityId> excluded;
	excluded.insert( deadId );
	excluded.insert( killerId );
	msgSpectators.serialCont( excluded );
	msgSpectators.serial( msgName );

	msgSpectators.serial( const_cast<CEntityId&> (killerId) );
	msgSpectators.serial( const_cast<CEntityId&> (deadId) );
	sendMessageViaMirror ("IOS", msgSpectators);

} // sendDeathMessages //

//--------------------------------------------------------------
//					sendHitMessages()  
//--------------------------------------------------------------
void sendHitMessages( const CEntityId &aggressorId, const CEntityId &victimId, sint32 amount, sint32 lostStamina, sint32 lostSap)
{
	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	if ( !amount && !lostStamina && !lostSap)
		return;

	amount = abs(amount);
	lostStamina = abs(lostStamina);
	lostSap = abs(lostSap);

	string msgName;

	if ( aggressorId.getType() == RYZOMID::player )
	{
		msgName = "BS_YOU_HIT_EI";

		CMessage msgAggressor("STATIC_STRING");

		msgAggressor.serial( const_cast<CEntityId&> (aggressorId) );
		set<CEntityId> excluded;
		msgAggressor.serialCont( excluded );
		msgAggressor.serial( msgName );
		
		msgAggressor.serial( const_cast<CEntityId&> (victimId) );
		msgAggressor.serial( amount );

		sendMessageViaMirror ("IOS", msgAggressor);
		INFOLOG("<sendHitMessages>send BS_YOU_HIT_EI with param e=%s and i=%i for entity %s",victimId.toString().c_str(), amount, aggressorId.toString().c_str());

		// lost stamina
		if (lostStamina>0)
		{
			msgName = "EGS_LOSE_STA_EI";
			CMessage msgSta("STATIC_STRING");
			msgSta.serial( const_cast<CEntityId&> (aggressorId) );
			msgSta.serialCont( excluded );
			msgSta.serial( msgName );
			msgSta.serial( const_cast<CEntityId&> (victimId) );
			msgSta.serial( lostStamina );
			sendMessageViaMirror ("IOS", msgSta);
		}
		// lost sap
		if (lostSap>0)
		{
			msgName = "EGS_LOSE_SAP_EI";
			CMessage msgSap("STATIC_STRING");
			msgSap.serial( const_cast<CEntityId&> (aggressorId) );
			msgSap.serialCont( excluded );
			msgSap.serial( msgName );
			msgSap.serial( const_cast<CEntityId&> (victimId) );
			msgSap.serial( lostSap );
			sendMessageViaMirror ("IOS", msgSap);
		}
	}

	if ( victimId.getType() == RYZOMID::player )
	{
		msgName = "BS_HITS_YOU_EI";

		CMessage msgVictim("STATIC_STRING");

		msgVictim.serial( const_cast<CEntityId&> (victimId) );
		set<CEntityId> excluded;
		msgVictim.serialCont( excluded );
		msgVictim.serial( msgName );
		msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
		msgVictim.serial( amount );
		
		sendMessageViaMirror ("IOS", msgVictim);

		INFOLOG("<sendHitMessages>send BS_HITS_YOU_EI with param e=%s and i=%i for entity %s",aggressorId.toString().c_str(), amount, victimId.toString().c_str());

		// lost stamina
		if (lostStamina>0)
		{
			msgName = "EGS_U_LOSE_STA_EI";
			CMessage msgSta("STATIC_STRING");
			msgSta.serial( const_cast<CEntityId&> (victimId) );
			msgSta.serialCont( excluded );
			msgSta.serial( msgName );
			msgSta.serial( const_cast<CEntityId&> (aggressorId) );
			msgSta.serial( lostStamina );
			sendMessageViaMirror ("IOS", msgSta);
		}
		// lost sap
		if (lostSap>0)
		{
			msgName = "EGS_U_LOSE_SAP_EI";
			CMessage msgSap("STATIC_STRING");
			msgSap.serial( const_cast<CEntityId&> (victimId) );
			msgSap.serialCont( excluded );
			msgSap.serial( msgName );
			msgSap.serial( const_cast<CEntityId&> (aggressorId) );
			msgSap.serial( lostSap );
			sendMessageViaMirror ("IOS", msgSap);
		}
	}

// spectators
	// Send to 'speech' group
	msgName = "BS_HIT_EEI";
	

	CEntityId speechGroupId;
	if ( aggressorId.getType() == RYZOMID::player )
	{
		speechGroupId = aggressorId;
	}
	else
	{
		speechGroupId = victimId;
	}
	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msgVictim("STATIC_STRING");

	msgVictim.serial( speechGroupId );
	
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	msgVictim.serialCont( excluded );

	msgVictim.serial( msgName );

	msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
	msgVictim.serial( const_cast<CEntityId&> (victimId) );
	msgVictim.serial( amount );

	sendMessageViaMirror ("IOS", msgVictim);
} // sendHitMessages //



//--------------------------------------------------------------
//					sendCriticalHitMessage()  
//--------------------------------------------------------------
void sendCriticalHitMessage( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId )
{
	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	string msgName;

	if ( aggressorId.getType() == RYZOMID::player )
	{
		msgName = "EGS_YOU_CRITICAL_HIT_E";
		CMessage msgAggressor("STATIC_STRING");

		msgAggressor.serial( const_cast<CEntityId&> (aggressorId) );
		set<CEntityId> excluded;
		msgAggressor.serialCont( excluded );
		msgAggressor.serial( msgName );
		
		msgAggressor.serial( const_cast<CEntityId&> (victimId) );

		sendMessageViaMirror ("IOS", msgAggressor);
	}

	if ( victimId.getType() == RYZOMID::player )
	{
		msgName = "EGS_CRITICAL_HIT_YOU_E";
		CMessage msgVictim("STATIC_STRING");

		msgVictim.serial( const_cast<CEntityId&> (victimId) );
		set<CEntityId> excluded;
		msgVictim.serialCont( excluded );
		msgVictim.serial( msgName );
		msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
		sendMessageViaMirror ("IOS", msgVictim);
	}

// spectators
	// Send to 'speech' group
	msgName = "EGS_CRITICAL_HIT_EE";

	CEntityId speechGroupId;
	if ( aggressorId.getType() == RYZOMID::player )
	{
		speechGroupId = aggressorId;
	}
	else
	{
		speechGroupId = victimId;
	}
	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msgVictim("STATIC_STRING");

	msgVictim.serial( speechGroupId );
	
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	msgVictim.serialCont( excluded );

	msgVictim.serial( msgName );

	msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
	msgVictim.serial( const_cast<CEntityId&> (victimId) );

	sendMessageViaMirror ("IOS", msgVictim);

} // sendCriticalHitMessage //

//--------------------------------------------------------------
//					sendHitNullMessages()  
//--------------------------------------------------------------
void sendHitNullMessages( const CEntityId &aggressorId, const CEntityId &victimId )
{	
	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	string msgName;

	if ( aggressorId.getType() == RYZOMID::player )
	{
		msgName = "BS_YOU_HIT_NULL_E";
		CMessage msgAggressor("STATIC_STRING");

		msgAggressor.serial( const_cast<CEntityId&> (aggressorId) );
		set<CEntityId> excluded;
		msgAggressor.serialCont( excluded );
		msgAggressor.serial( msgName );
		
		msgAggressor.serial( const_cast<CEntityId&> (victimId) );

		sendMessageViaMirror ("IOS", msgAggressor);

		INFOLOG("<sendHitMessages>send BS_YOU_HIT_NULL_E with param e=%s for entity %s",victimId.toString().c_str(), aggressorId.toString().c_str());
	}

	if ( victimId.getType() == RYZOMID::player )
	{
		msgName = "BS_HITS_YOU_NULL_E";
		CMessage msgVictim("STATIC_STRING");

		msgVictim.serial( const_cast<CEntityId&> (victimId) );
		set<CEntityId> excluded;
		msgVictim.serialCont( excluded );
		msgVictim.serial( msgName );
		msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
		sendMessageViaMirror ("IOS", msgVictim);

		INFOLOG("<sendHitMessages>send BS_HITS_YOU_EI with param e=%s for entity %s",aggressorId.toString().c_str(), victimId.toString().c_str());
	}

// spectators
	// Send to 'speech' group
	msgName = "BS_HIT_NULL_EE";

	CEntityId speechGroupId;
	if ( aggressorId.getType() == RYZOMID::player )
	{
		speechGroupId = aggressorId;
	}
	else
	{
		speechGroupId = victimId;
	}
	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msgVictim("STATIC_STRING");

	msgVictim.serial( speechGroupId );
	
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	msgVictim.serialCont( excluded );

	msgVictim.serial( msgName );

	msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
	msgVictim.serial( const_cast<CEntityId&> (victimId) );

	sendMessageViaMirror ("IOS", msgVictim);
} // sendHitNullMessages //



//--------------------------------------------------------------
//					sendFumbleMessage()  
//--------------------------------------------------------------
void sendFumbleMessage( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId )
{
	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	string msgName;

	if ( aggressorId.getType() == RYZOMID::player )
	{
		msgName = "EGS_YOU_FUMBLE";
		CMessage msgAggressor("STATIC_STRING");

		msgAggressor.serial( const_cast<CEntityId&> (aggressorId) );
		set<CEntityId> excluded;
		msgAggressor.serialCont( excluded );
		msgAggressor.serial( msgName );

		sendMessageViaMirror ("IOS", msgAggressor);
	}

	if ( victimId.getType() == RYZOMID::player )
	{
		msgName = "EGS_ENEMY_FUMBLE_E";
		CMessage msgVictim("STATIC_STRING");

		msgVictim.serial( const_cast<CEntityId&> (victimId) );
		set<CEntityId> excluded;
		msgVictim.serialCont( excluded );
		msgVictim.serial( msgName );
		msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
		sendMessageViaMirror ("IOS", msgVictim);
	}

// spectators
	// Send to 'speech' group
	msgName = "EGS_FUMBLE_E";

	CEntityId speechGroupId;
	if ( aggressorId.getType() == RYZOMID::player )
	{
		speechGroupId = aggressorId;
	}
	else
	{
		speechGroupId = victimId;
	}
	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msgVictim("STATIC_STRING");

	msgVictim.serial( speechGroupId );
	
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	msgVictim.serialCont( excluded );

	msgVictim.serial( msgName );

	sendMessageViaMirror ("IOS", msgVictim);

} // sendFumbleMessage //

//--------------------------------------------------------------
//					sendEngageMessages()  
//--------------------------------------------------------------
void sendEngageMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId )
{
	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	string msgName;

	if ( aggressorId.getType() == RYZOMID::player )
	{
		msgName = "BS_YOU_ATTACK_E";
		CMessage msgAggressor("STATIC_STRING");

		msgAggressor.serial( const_cast<CEntityId&> (aggressorId) );
		set<CEntityId> excluded;
		msgAggressor.serialCont( excluded );
		msgAggressor.serial( msgName );
		
		msgAggressor.serial( const_cast<CEntityId&> (victimId) );

		sendMessageViaMirror ("IOS", msgAggressor);
	}

	if ( victimId.getType() == RYZOMID::player )
	{
		msgName = "BS_ATTACKS_YOU_E";
		CMessage msgVictim("STATIC_STRING");

		msgVictim.serial( const_cast<CEntityId&> (victimId) );
		set<CEntityId> excluded;
		msgVictim.serialCont( excluded );
		msgVictim.serial( msgName );
		msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
		sendMessageViaMirror ("IOS", msgVictim);
	}

// spectators
	// Send to 'speech' group
	msgName = "BS_ATTACKS_EE";

	CEntityId speechGroupId;
	if ( aggressorId.getType() == RYZOMID::player )
	{
		speechGroupId = aggressorId;
	}
	else
	{
		speechGroupId = victimId;
	}
	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msgVictim("STATIC_STRING");

	msgVictim.serial( speechGroupId );
	
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	msgVictim.serialCont( excluded );

	msgVictim.serial( msgName );

	msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
	msgVictim.serial( const_cast<CEntityId&> (victimId) );

	sendMessageViaMirror ("IOS", msgVictim);

} // sendEngageMessages //


//--------------------------------------------------------------
//					sendDisengageMessages()  
//--------------------------------------------------------------
void sendDisengageMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId )
{
	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	string msgName;

	if ( aggressorId.getType() == RYZOMID::player )
	{
		msgName = "BS_YOU_END_ATTACK_E";
		CMessage msgAggressor("STATIC_STRING");

		msgAggressor.serial( const_cast<CEntityId&> (aggressorId) );
		set<CEntityId> excluded;
		msgAggressor.serialCont( excluded );
		msgAggressor.serial( msgName );
		
		msgAggressor.serial( const_cast<CEntityId&> (victimId) );

		sendMessageViaMirror ("IOS", msgAggressor);
	}

	if ( victimId.getType() == RYZOMID::player )
	{
		msgName = "BS_END_ATTACKS_YOU_E";
		CMessage msgVictim("STATIC_STRING");

		msgVictim.serial( const_cast<CEntityId&> (victimId) );
		set<CEntityId> excluded;
		msgVictim.serialCont( excluded );
		msgVictim.serial( msgName );
		msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
		sendMessageViaMirror ("IOS", msgVictim);
	}

// spectators
	// Send to 'speech' group
	msgName = "BS_END_ATTACKS_EE";

	CEntityId speechGroupId;
	if ( aggressorId.getType() == RYZOMID::player )
	{
		speechGroupId = aggressorId;
	}
	else
	{
		speechGroupId = victimId;
	}
	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msgVictim("STATIC_STRING");

	msgVictim.serial( speechGroupId );
	
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	msgVictim.serialCont( excluded );

	msgVictim.serial( msgName );

	msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
	msgVictim.serial( const_cast<CEntityId&> (victimId) );

	sendMessageViaMirror ("IOS", msgVictim);

} // sendDisengageMessages //


//--------------------------------------------------------------
//					engageTargetInMelee()  
//--------------------------------------------------------------
bool engageTargetInMelee( const CEntityId &entityId , const CEntityId &targetId , sint8 mode)
{
	if ( targetId == CEntityId::Unknown )
	{
		return false;
	}

	// check target is alive??

	// indicate the new engaged target (for player only)
	if ( entityId.getType() == RYZOMID::player)
	{
		CCharacter *ch = PlayerManager.getChar( entityId );
		if (ch == NULL)
		{
			nlwarning("<engageTargetInMelee> Invalid char Id %s", entityId.toString().c_str() );
			return false;
		}
		ch->setFightingTarget( targetId );
	}

	//CBrickSentenceManager::engageMelee( entityId, targetId );
	PhraseManager->engageMelee( TheDataset.getDataSetRow(entityId), TheDataset.getDataSetRow(targetId) );
	return true;

} // engageTargetInMelee //





//--------------------------------------------------------------
//					engageTargetRange()  
//--------------------------------------------------------------
bool engageTargetRange( const CEntityId &entityId , const CEntityId &targetId )
{
	if ( targetId == CEntityId::Unknown )
	{
		return false;
	}

	// check target is alive??

	//CBrickSentenceManager::engageRange( entityId, targetId );
	PhraseManager->engageRange( TheDataset.getDataSetRow(entityId), TheDataset.getDataSetRow(targetId) );

	// 
	INFOLOG("<engageTargetRange> entity %s engage entity %s in range combat",entityId.toString().c_str(),targetId.toString().c_str() );		

	// indicate the new engaged target (for player only)
	if ( entityId.getType() == RYZOMID::player)
	{
		CCharacter *ch = PlayerManager.getChar( entityId );
		if (ch == NULL)
		{
			nlwarning("<engageTargetRange> Invalid char Id %s", entityId.toString().c_str() );
			return false;
		}
		ch->setFightingTarget( targetId );
	}

	return true;
} // engageTargetRange //



//--------------------------------------------------------------
//					getImpactIntensity()  
//--------------------------------------------------------------
INTENSITY_TYPE::TImpactIntensity getImpactIntensity( sint32 impact, const TDataSetRow &victimRowId, SCORES::TScores score )
{
	if (impact == 0)
		return INTENSITY_TYPE::IMPACT_NONE;

	if (impact < 0)
		impact = impact * (-1);

	if ( score >= SCORES::NUM_SCORES)
	{
		nlwarning("<getImpactIntensity> Unknown score %d", score);
		return INTENSITY_TYPE::IMPACT_NONE;
	}

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( victimRowId );
	if (entity == NULL)
	{
		nlwarning("<getImpactIntensity> Invalid entity row %u", victimRowId.getIndex() );
		return INTENSITY_TYPE::IMPACT_NONE;
	}	
	
	const sint32 scoreMax = entity->getScores()._PhysicalScores[ score ].Max;
	
	// percentage
	uint32 percent = (uint32) fabs(100 * (double)impact / (double)scoreMax );

	if (percent < 2)
		return INTENSITY_TYPE::IMPACT_INSIGNIFICANT;
	else if (percent < 5)
		return INTENSITY_TYPE::IMPACT_VERY_WEAK;
	else if (percent < 10)
		return INTENSITY_TYPE::IMPACT_WEAK;
	else if (percent < 20)
		return INTENSITY_TYPE::IMPACT_AVERAGE;
	else if (percent < 30)
		return INTENSITY_TYPE::IMPACT_STRONG;
	else if (percent < 40)
		return INTENSITY_TYPE::IMPACT_VERY_STRONG;
	else
		return INTENSITY_TYPE::IMPACT_HUGE;
} // getImpactIntensity //


//--------------------------------------------------------------
//					getAttackIntensity()  
//--------------------------------------------------------------
INTENSITY_TYPE::TAttackIntensity getAttackIntensity( uint32 attackLevel )
{
	if (attackLevel < 2)
		return INTENSITY_TYPE::NONE;
	else if (attackLevel < 5)
		return INTENSITY_TYPE::WEAK;
	else if (attackLevel < 10)
		return INTENSITY_TYPE::STANDARD;
	else
		return INTENSITY_TYPE::STRONG;
} // getAttackIntensity //

//--------------------------------------------------------------
//					getOrientationString()  
//--------------------------------------------------------------
std::string getOrientationString( const NLMISC::CVector &dir)
{
	return getOrientationString( (sint32)dir.x, (sint32)dir.y );
} // getOrientationString //


//--------------------------------------------------------------
//					getOrientationString()  
//--------------------------------------------------------------
std::string getOrientationString( sint32 x, sint32 y)
{
	float angle = (float)(atan2(y, x));

	//get the direction
	uint direction =(uint) ( 8.0f* (angle + Pi)/(Pi) );
	
	if (direction>15)
	{
		nlwarning("Direction for compass should never be >15");
		return "";
	}

	return OrientationStrings[direction];
} // getOrientationString //


//--------------------------------------------------------------
//					getEntityCurrentAction()  
//--------------------------------------------------------------
ENTITY_ACTION::EEntityAction getEntityCurrentAction( const NLMISC::CEntityId &entityId)
{
	// test melee combat
	TDataSetRow targetRowId = PhraseManager->getEntityEngagedMeleeBy( TheDataset.getDataSetRow(entityId) );
	//if (targetId != CEntityId::Unknown )
	if (targetRowId.isValid() && TheDataset.isDataSetRowStillValid(targetRowId))
	{
		return ENTITY_ACTION::MELEE_COMBAT;
	}

	// test range combat
	//targetId = CBrickSentenceManager::getEntityEngagedRangeBy( targetId );
	targetRowId = PhraseManager->getEntityEngagedRangeBy( TheDataset.getDataSetRow(entityId) );
	//if (targetId != CEntityId::Unknown )
	if (targetRowId.isValid() && TheDataset.isDataSetRowStillValid(targetRowId))
	{
		return ENTITY_ACTION::RANGE_COMBAT;
	}


	/// \todo Malkav : test faber/tracking/casting/walking and running
	CEntityBase* entity = entityPtrFromId( entityId );
	if (entity == NULL)
	{
		nlwarning("<getEntityCurrentAction> Invalid entity Id %s", entityId.toString().c_str() );
		return ENTITY_ACTION::IDLE;
	}

	const MBEHAV::CBehaviour &behaviour = entity->getBehaviour();
	switch( behaviour.Behaviour )
	{
		case MBEHAV::HARVESTING:
			return ENTITY_ACTION::HARVESTING;
			break;

		case MBEHAV::FABER:
		case MBEHAV::REPAIR:
		case MBEHAV::REFINE:
			return ENTITY_ACTION::DOING_FABER;
			break;

		default:
			{
				/// Problem : how to detect a casting entity when this entity use the suffix tryker brick 'hide' ??? :'(
				if (behaviour.Behaviour >= MBEHAV::MAGIC_CASTING_BEHAVIOUR_BEGIN && behaviour.Behaviour < MBEHAV::MAGIC_END_CASTING_BEHAVIOUR_BEGIN)
					return ENTITY_ACTION::CASTING;
			}
			break;
	};


	// by default : return IDLE
	return ENTITY_ACTION::IDLE;

} // getEntityCurrentAction //


//--------------------------------------------------------------
//						testOffensiveActionAllowed
//--------------------------------------------------------------
bool testOffensiveActionAllowed( const NLMISC::CEntityId &actorId, const NLMISC::CEntityId &targetId, string &errorCode )
{
	switch ( targetId.getType() )
	{
	case RYZOMID::player:
		{
			if (actorId.getType() == RYZOMID::player)
			{
				if (!AllowPVP)
					errorCode = "EGS_PVP_NOT_ALLOWED";

				return AllowPVP;
			}
		}
		break;

	case RYZOMID::creature:	
	case RYZOMID::mount:
	case RYZOMID::kami:
	case RYZOMID::npc:
		{
			CEntityBase* entity = CreatureManager.getCreature( targetId );
			if (entity == NULL)
			{
				nlwarning("<testOffensiveActionAllowed> Invalid entity Id %s", targetId.toString().c_str() );
				errorCode = "BS_INVALID_TARGET";
				return false;
			}
			
			/*// get creature form
			const CSheetId &sheet = entity->getType();

			const CStaticCreatures *creature = CSheets::getCreaturesForm( sheet );
			if ( ! creature)
			{
				nlwarning("<testOffensiveActionAllowed> can't found CStaticCreature for entity %s", targetId.toString().c_str() );
				errorCode = "BS_INVALID_TARGET";
				return false;
			}
			
			if ( ! creature->properties.attackable() )
			{
				errorCode = "BS_TARGET_NOT_ATTACKABLE";
				return false;
			}
			*/

			if (!entity->getContextualProperty().getValue().attackable())
			{
				errorCode = "BS_TARGET_NOT_ATTACKABLE";
				return false;
			}
		}
		break;

	default:
		break;

	};
	return true;
} // testOffensiveActionAllowed //

//--------------------------------------------------------------
//					sendScoreModifierSpellMessage
//--------------------------------------------------------------
void sendScoreModifierSpellMessage( CEntityBase * aggressor, CEntityBase* victim, sint32 value, SCORES::TScores score, ACTNATURE::EActionNature nature )
{
	nlassert(aggressor);
	nlassert(victim);

	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressor->getId().getType() != RYZOMID::player) && ( victim->getId().getType() != RYZOMID::player ))
		return;
	
	string msgAgressor;
	string msgVictim;
	
	switch (nature )
	{
	case ACTNATURE::OFFENSIVE:
		switch(score)
		{
		case SCORES::sap:
			msgVictim	= "MAGIC_U_SUFFER_OFFENSIVE_SPELL_E_SAP";
			msgAgressor = "MAGIC_U_CAST_OFFENSIVE_SPELL_E_SAP";
			break;
		case SCORES::stamina:
			msgVictim	= "MAGIC_U_SUFFER_OFFENSIVE_SPELL_E_STA";
			msgAgressor = "MAGIC_U_CAST_OFFENSIVE_SPELL_E_STA";
			break;
		case SCORES::hit_points:
			msgVictim	= "MAGIC_U_SUFFER_OFFENSIVE_SPELL_E_HP";
			msgAgressor = "MAGIC_U_CAST_OFFENSIVE_SPELL_E_HP";
			break;
		default:
			nlwarning("<PHRASE_UTILITIES sendScoreModifierSpellMessage> invalid scrore %s", SCORES::toString(score).c_str() );
		}
		break;
		case ACTNATURE::DEFENSIVE:
		switch(score)
		{
		case SCORES::sap:
			msgVictim	= "MAGIC_U_SUFFER_HEAL_SPELL_E_SAP";
			msgAgressor = "MAGIC_U_CAST_HEAL_SPELL_E_SAP";
			break;
			case SCORES::stamina:
			msgVictim	= "MAGIC_U_SUFFER_HEAL_SPELL_E_STA";
			msgAgressor = "MAGIC_U_CAST_HEAL_SPELL_E_STA";
			break;
		case SCORES::hit_points:
			msgVictim	= "MAGIC_U_SUFFER_HEAL_SPELL_E_HP";
			msgAgressor = "MAGIC_U_CAST_HEAL_SPELL_E_HP";
			break;
		default:
			nlwarning("<PHRASE_UTILITIES sendScoreModifierSpellMessage> invalid scrore %s", SCORES::toString(score).c_str() );
		}
		break;
	};

	if ( aggressor->getId().getType() == RYZOMID::player )
	{
		CCharacter::sendMessageToClient(aggressor->getId(),msgAgressor,victim->getId(),(uint32)value );
	}
	if ( victim->getId().getType() == RYZOMID::player )
	{
		CCharacter::sendMessageToClient(victim->getId(),msgVictim,aggressor->getId(),(uint32)value );
	}
} // sendScoreModifierSpellMessage //




//--------------------------------------------------------------
//					sendSpellResistMessages()  
//--------------------------------------------------------------
void sendSpellResistMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId  )
{
	if ( !aggressorRowId.isValid() || !TheDataset.isDataSetRowStillValid(aggressorRowId) || !victimRowId.isValid() || !TheDataset.isDataSetRowStillValid(victimRowId) )
		return;

	CEntityId victimId = TheDataset.getEntityId(victimRowId);
	CEntityId aggressorId = TheDataset.getEntityId(aggressorRowId);

	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	string msgName;

	if ( victimId.getType() == RYZOMID::player)
	{
		msgName ="EGS_TARGET_CASTING_RESIST_E";
		CMessage msgVictim("STATIC_STRING");

		msgVictim.serial( const_cast<CEntityId&> (victimId) );
		set<CEntityId> excluded;
		msgVictim.serialCont( excluded );
		msgVictim.serial( msgName );
		msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
		sendMessageViaMirror ("IOS", msgVictim);
	}

	if ( aggressorId.getType() == RYZOMID::player)
	{
		msgName ="EGS_ACTOR_CASTING_RESIST_E";
		CMessage msgAggressor("STATIC_STRING");

		msgAggressor.serial( const_cast<CEntityId&> (aggressorId) );
		set<CEntityId> excluded;
		msgAggressor.serialCont( excluded );
		msgAggressor.serial( msgName );
		msgAggressor.serial( const_cast<CEntityId&> (victimId) );		
		sendMessageViaMirror ("IOS", msgAggressor);
	}
	
// send message to spectators
	// Send to 'speech' group
	msgName ="EGS_SPECT_CASTING_RESIST_EE";

	CEntityId speechGroupId;
	if ( victimId.getType() == RYZOMID::player )
	{
		speechGroupId = victimId;
	}
	else
	{
		speechGroupId = aggressorId;
	}
	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msg("STATIC_STRING");

	msg.serial( speechGroupId );
	
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	
	msg.serialCont( excluded );
	msg.serial( msgName );

	msg.serial( const_cast<CEntityId&> (victimId) );
	msg.serial( const_cast<CEntityId&> (aggressorId) );

	sendMessageViaMirror ("IOS", msg);
} // sendSpellResistMessages //


//--------------------------------------------------------------
//					sendSpellBeginCastMessages()  
//--------------------------------------------------------------
void sendSpellBeginCastMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId, ACTNATURE::EActionNature nature )
{
	if ( !aggressorRowId.isValid() || !TheDataset.isDataSetRowStillValid(aggressorRowId) || !victimRowId.isValid() || !TheDataset.isDataSetRowStillValid(victimRowId) )
		return;

	CEntityId victimId = TheDataset.getEntityId(victimRowId);
	CEntityId aggressorId = TheDataset.getEntityId(aggressorRowId);

	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	string msgName;
	bool self = (aggressorId == victimId);

	if ( aggressorId.getType() == RYZOMID::player)
	{
		switch( nature )
		{
		case ACTNATURE::NEUTRAL:
			msgName = self ? "EGS_ACTOR_BEGIN_SELFCAST_NEUTRAL" : "EGS_ACTOR_BEGIN_CASTING_NEUTRAL_E";
			break;
		case ACTNATURE::DEFENSIVE:
			msgName = self ? "EGS_ACTOR_BEGIN_SELFCAST_GOOD" : "EGS_ACTOR_BEGIN_CASTING_GOOD_E";
			break;
		case ACTNATURE::OFFENSIVE:
		default:
			msgName ="EGS_ACTOR_BEGIN_CASTING_BAD_E";
		};

		CMessage msgVictim("STATIC_STRING");

		msgVictim.serial( const_cast<CEntityId&> (aggressorId) );
		set<CEntityId> excluded;
		msgVictim.serialCont( excluded );
		msgVictim.serial( msgName );
		
		if (!self)
			msgVictim.serial( const_cast<CEntityId&> (victimId) );

		sendMessageViaMirror ("IOS", msgVictim);
	}

	if (self)
		return;

	if ( victimId.getType() == RYZOMID::player)
	{
		switch( nature )
		{
		case ACTNATURE::NEUTRAL:
			msgName = "EGS_TARGET_BEGIN_CASTING_NEUTRAL_E";
			break;
		case ACTNATURE::DEFENSIVE:
			msgName = "EGS_TARGET_BEGIN_CASTING_GOOD_E";
			break;
		case ACTNATURE::OFFENSIVE:
		default:
			msgName ="EGS_TARGET_BEGIN_CASTING_BAD_E";
		};

		CMessage msgAggressor("STATIC_STRING");

		msgAggressor.serial( const_cast<CEntityId&> (victimId) );
		set<CEntityId> excluded;
		msgAggressor.serialCont( excluded );
		msgAggressor.serial( msgName );
		msgAggressor.serial( const_cast<CEntityId&> (aggressorId) );		
		sendMessageViaMirror ("IOS", msgAggressor);
	}
	
// send message to spectators
	// Send to 'speech' group
	switch( nature )
	{
	case ACTNATURE::NEUTRAL:
		msgName = "EGS_SPECT_BEGIN_CASTING_NEUTRAL_EE";
		break;
	case ACTNATURE::DEFENSIVE:
		msgName = "EGS_SPECT_BEGIN_CASTING_GOOD_EE";
		break;
	case ACTNATURE::OFFENSIVE:
	default:
		msgName ="EGS_SPECT_BEGIN_CASTING_BAD_EE";
	};

	CEntityId speechGroupId;
	if ( victimId.getType() == RYZOMID::player )
	{
		speechGroupId = victimId;
	}
	else
	{
		speechGroupId = aggressorId;
	}
	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msg("STATIC_STRING");

	msg.serial( speechGroupId );
	
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	
	msg.serialCont( excluded );
	msg.serial( msgName );
	
	msg.serial( const_cast<CEntityId&> (aggressorId) );
	msg.serial( const_cast<CEntityId&> (victimId) );

	sendMessageViaMirror ("IOS", msg);
} // sendSpellBeginCastMessages //


//--------------------------------------------------------------
//					sendSpellSuccessMessages()  
//--------------------------------------------------------------
void sendSpellSuccessMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId  )
{
	if ( !aggressorRowId.isValid() || !TheDataset.isDataSetRowStillValid(aggressorRowId) || !victimRowId.isValid() || !TheDataset.isDataSetRowStillValid(victimRowId) )
		return;

	CEntityId victimId = TheDataset.getEntityId(victimRowId);
	CEntityId aggressorId = TheDataset.getEntityId(aggressorRowId);

	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	sendSimpleMessage(aggressorRowId, "EGS_ACTOR_CAST_END_SUCCESS");
	
	if ( aggressorRowId == victimRowId)
		return;

	sendMessage(victimRowId, "EGS_TARGET_CAST_END_SUCCESS_E", aggressorRowId);

	
// send message to spectators
	// Send to 'speech' group
	string msgName ="EGS_SPECT_CAST_END_SUCCESS_EE";
	CEntityId speechGroupId;
	if ( victimId.getType() == RYZOMID::player )
		speechGroupId = victimId;
	else
		speechGroupId = aggressorId;

	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msg("STATIC_STRING");
	msg.serial( speechGroupId );
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	msg.serialCont( excluded );
	msg.serial( msgName );
	msg.serial( const_cast<CEntityId&> (victimId) );
	msg.serial( const_cast<CEntityId&> (aggressorId) );
	sendMessageViaMirror ("IOS", msg);
} // sendSpellSuccessMessages //


//--------------------------------------------------------------
//					sendSpellFailedMessages()  
//--------------------------------------------------------------
void sendSpellFailedMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId  )
{
	if ( !aggressorRowId.isValid() || !TheDataset.isDataSetRowStillValid(aggressorRowId) || !victimRowId.isValid() || !TheDataset.isDataSetRowStillValid(victimRowId) )
		return;

	CEntityId victimId = TheDataset.getEntityId(victimRowId);
	CEntityId aggressorId = TheDataset.getEntityId(aggressorRowId);

	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	sendSimpleMessage(aggressorRowId, "EGS_ACTOR_CAST_END_FAILED");
	
	if ( aggressorRowId == victimRowId)
		return;

	sendMessage(victimRowId, "EGS_TARGET_CAST_END_FAILED_E", aggressorRowId);

	
// send message to spectators
	// Send to 'speech' group
	string msgName ="EGS_SPECT_CAST_END_FAILED_E";
	CEntityId speechGroupId;
	if ( victimId.getType() == RYZOMID::player )
		speechGroupId = victimId;
	else
		speechGroupId = aggressorId;

	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msg("STATIC_STRING");
	msg.serial( speechGroupId );
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	msg.serialCont( excluded );
	msg.serial( msgName );
	msg.serial( const_cast<CEntityId&> (aggressorId) );
	sendMessageViaMirror ("IOS", msg);
} // sendSpellFailedMessages //

//--------------------------------------------------------------
//					sendSpellFumbleMessages()  
//--------------------------------------------------------------
void sendSpellFumbleMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId  )
{
	if ( !aggressorRowId.isValid() || !TheDataset.isDataSetRowStillValid(aggressorRowId) || !victimRowId.isValid() || !TheDataset.isDataSetRowStillValid(victimRowId) )
		return;

	CEntityId victimId = TheDataset.getEntityId(victimRowId);
	CEntityId aggressorId = TheDataset.getEntityId(aggressorRowId);

	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	sendSimpleMessage(aggressorRowId, "EGS_ACTOR_CAST_END_FUMBLE");
	
	if ( aggressorRowId == victimRowId)
		return;

	sendMessage(victimRowId, "EGS_TARGET_CAST_END_FUMBLE_E", aggressorRowId);

	
// send message to spectators
	// Send to 'speech' group
	string msgName ="EGS_SPECT_CAST_END_FUMBLE_E";
	CEntityId speechGroupId;
	if ( victimId.getType() == RYZOMID::player )
		speechGroupId = victimId;
	else
		speechGroupId = aggressorId;

	speechGroupId.setType( RYZOMID::dynChatGroup );

	CMessage msg("STATIC_STRING");
	msg.serial( speechGroupId );
	set<CEntityId> excluded;
	excluded.insert( aggressorId );
	excluded.insert( victimId );
	msg.serialCont( excluded );
	msg.serial( msgName );
	msg.serial( const_cast<CEntityId&> (aggressorId) );
	sendMessageViaMirror ("IOS", msg);
} // sendSpellFumbleMessages //

}; // namespace PHRASE_UTILITIES

