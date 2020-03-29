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

#include "phrase_manager/phrase_utilities_functions.h"
#include "egs_mirror.h"


// game share
#include "game_share/mode_and_behaviour.h"
#include "game_share/characteristics.h"
#include "server_share/creature_size.h"
#include "game_share/combat_flying_text.h"
// georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"

#include "server_share/r2_vision.h"

//
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "phrase_manager/phrase_manager.h"
#include "creature_manager/creature_manager.h"
#include "egs_globals.h"
#include "phrase_manager/s_effect.h"
#include "pvp_manager/pvp_manager.h"
#include "pvp_manager/pvp_manager_2.h"
#include "pvp_manager/pvp_faction_reward_manager/pvp_faction_reward_manager.h"
#include "entity_manager/entity_base.h"

#include "egs_sheets/egs_sheets.h"

extern uint8					CMSIsUp;
extern CPlayerManager			PlayerManager;
extern CCreatureManager			CreatureManager;
extern CGameItemManager			GameItemManager;


extern uint8					EntityForcedDefaultLevel; // 0 by default, it's the level of an entity when it has a level 0 (for tests purposes only)

extern CGenericXmlMsgHeaderManager	GenericMsgManager;

using namespace std;
using namespace NLGEORGES;
using namespace NLMISC;
using namespace NLNET;


bool VerboseBrickManagerInfo = false;
bool VerboseBrickManagerDbg = false;
bool AggroLog = false;
bool AiActionLog = false;


NLMISC_COMMAND(VerboseBrickManagerInfo,"Turn on or off or check the state of verbose brick manager logging (for info)","")
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
NLMISC_COMMAND(VerboseBrickManagerDbg,"Turn on or off or check the state of verbose brick manager logging (for debug)","")
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
NLMISC_COMMAND(AggroLog,"Turn on or off aggro log display or check the state of the flag","0/1")
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

// ai actions msg
NLMISC_COMMAND(AiActionLog,"Turn on or off ai actionAiActionLog log display or check the state of the flag","0/1")
{
	if(args.size()>1)
		return false;
	if(args.size()==1)
	{
		if(args[0]==string("on")||args[0]==string("ON")||args[0]==string("true")||args[0]==string("TRUE")||args[0]==string("1"))
			AiActionLog=true;
		
		if(args[0]==string("off")||args[0]==string("OFF")||args[0]==string("false")||args[0]==string("FALSE")||args[0]==string("0"))
			AiActionLog=false;
	}
	
	log.displayNL("AiActionLog is %s",AiActionLog?"ON":"OFF");
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
	UFormLoader::releaseLoader(loader);
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

				if ( attackerSize >= CREATURE_SIZE::NB_SIZE || defenderSize >= CREATURE_SIZE::NB_SIZE || attackerSize < 0 || defenderSize < 0)
				{
					nlwarning("<loadLocalisationSizeAdjusmentTable> Invalid line, attacker size = %s, defender size = %s", attacker.c_str(), defender.c_str() );
					continue;
				}

				///
				LocalisationAdjustments[attackerSize][defenderSize] = modifier;
			}
		}
	}
	UFormLoader::releaseLoader(loader);
	
} // loadLocalisationSizeAdjusmentTable //


//--------------------------------------------------------------
//					getLocalisation()  
//--------------------------------------------------------------
TPairSlotShield  getLocalisation( SHIELDTYPE::EShieldType shield, sint8 adjustement, SLOT_EQUIPMENT::TSlotEquipment forcedSlot)
{
	TPairSlotShield slotShield;
	SLocalisation local;

	const uint nbLoc = (uint)LocalisationTable.size() / 3;

	if ( forcedSlot == SLOT_EQUIPMENT::UNDEFINED)
	{
		sint32 randVal = RandomGenerator.rand() + adjustement;
		if (randVal < 0)
			randVal = 0;

		local.LocalisationNumber = randVal%nbLoc + 1;
		local.ShieldType = shield;

		TLocalisationSet::iterator it = LocalisationTable.find( local );
		if (it != LocalisationTable.end() )
		{
			slotShield.first = (*it).Slot;
			slotShield.second = (*it).ShieldIsEffective;
		}
		else
		{
			nlwarning("Cannot find entry in loc table for shield %s and loc part %u", SHIELDTYPE::toString(shield).c_str(), local.LocalisationNumber);
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
			nlwarning("Cannot find entry in loc table for shield %s and forced slot %s", SHIELDTYPE::toString(shield).c_str(), SLOT_EQUIPMENT::toString(forcedSlot).c_str());
			slotShield.first = SLOT_EQUIPMENT::UNDEFINED;
			slotShield.second = false;
		}
		else
		{
			local.LocalisationNumber = 1 + (uint8)RandomGenerator.rand((uint16)subTable.size()-1);
			local.ShieldType = shield;

			it = subTable.find( local );
			if (it != subTable.end() )
			{			
				slotShield.first = (*it).Slot;
				slotShield.second = (*it).ShieldIsEffective;
			}
			else
			{
				nlwarning("Cannot find entry in loc table for shield %s and loc part %u, forced slot %s", 
					SHIELDTYPE::toString(shield).c_str(), local.LocalisationNumber, SLOT_EQUIPMENT::toString(forcedSlot).c_str());
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

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( attacker );
	if (entity == NULL)
	{
		nlwarning("<getLocalisationSizeAdjustement> Invalid entity Id %s", attacker.toString().c_str() );
		return 0;
	}

	attackerSize = entity->getSize();

	entity = CEntityBaseManager::getEntityBasePtr( defender );
	if (entity == NULL)
	{
		nlwarning("<getLocalisationSizeAdjustement> Invalid entity Id %s", defender.toString().c_str() );
		return 0;
	}

	defenderSize = entity->getSize();

	if ( attackerSize >= CREATURE_SIZE::NB_SIZE || defenderSize >= CREATURE_SIZE::NB_SIZE || attackerSize < 0 || defenderSize < 0)
	{
		return 0;
	}

	return LocalisationAdjustments[attackerSize][defenderSize];
} // getLocalisationSizeAdjustement //


//--------------------------------------------------------------
//					getDefenseLocalisationModifier()  
//--------------------------------------------------------------
sint32 getDefenseLocalisationModifier( SLOT_EQUIPMENT::TSlotEquipment hitSlot, SLOT_EQUIPMENT::TSlotEquipment protectedSlot )
{
	if (protectedSlot == SLOT_EQUIPMENT::UNDEFINED)
		return 0;

	/// TODO : REAL MANAGEMENT

	if (hitSlot == protectedSlot)
		return 20;
	else
		return -10;
} // getDefenseLocalisationModifier //

//--------------------------------------------------------------
//					getDefenseLocalisationModifiers()  
//--------------------------------------------------------------
const vector<sint8> &getDefenseLocalisationModifiers( uint8 indexProtection )
{
	/// TODO : real management

	static vector<sint8> modifiers; 
	modifiers.resize(6,0);

	for (uint i = 0 ; i < 6 ; ++i)
	{
		modifiers[i] = (indexProtection > 5) ? 0 : (i==indexProtection)?20:-10;
	}

	return modifiers;

} // getDefenseLocalisationModifiers //


//--------------------------------------------------------------
//					getDistance()  
//--------------------------------------------------------------
double getDistance( const CEntityId &entity1, const CEntityId &entity2 )
{
	TDataSetRow entityIndex1 = TheDataset.getDataSetRow( entity1 );
	if ( !TheDataset.isAccessible(entityIndex1) )
		return -1;
	CMirrorPropValueRO<TYPE_POSX> posX( TheDataset, entityIndex1, DSPropertyPOSX );
	const double playerX = (double)posX / 1000.0f;
	CMirrorPropValueRO<TYPE_POSY> posY( TheDataset, entityIndex1, DSPropertyPOSY );
	const double playerY = (double)posY / 1000.0f;

	double targetX, targetY;//, targetZ;

	TDataSetRow entityIndex2 = TheDataset.getDataSetRow( entity2 );
	if ( !TheDataset.isAccessible(entityIndex2) )
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
//					testRange()  
//--------------------------------------------------------------
bool testRange( CEntityBase &source, CEntityBase &target, uint32 maxRange )
{
	// test range
	const double dx = source.getState().X - target.getState().X;
	const double dy = source.getState().Y - target.getState().Y;

	// Get target collision size.
	double radius = 0.5;	// Player Radius(User)
	switch(target.getId().getType())
	{
	// Target is Creature
	case RYZOMID::creature:
		{
			const CStaticCreatures * sheet = target.getForm();
			if(sheet)
				radius += sheet->getColRadius() * sheet->getScale();
		}
		break;
		// Target is Player
	case RYZOMID::player:
		radius += 0.5;
		break;
	}
	// Convert in mm
	radius *= 1000.0;
	
	// Check the Range
	const double distance = sqr(dx) + sqr(dy);
	const double range = sqr(maxRange + radius);

	return (distance <= range);
} // testRange //


//--------------------------------------------------------------
//					sendUpdateBehaviour()  
//--------------------------------------------------------------
void sendUpdateBehaviour( const CEntityId &entityId, const MBEHAV::CBehaviour &behaviour, bool forceUpdate )
{
	MBEHAV::CBehaviour b = behaviour;

	// Warning: the first 4 bits of every member of the union must be Time!
	b.Combat.Time = uint16(CTickEventHandler::getGameCycle() >>3);

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( entityId );
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
	if ( entityId.getType() != RYZOMID::player)
		return;

	CCharacter::sendDynamicSystemMessage(entityId, msgName);

	INFOLOG("<sendSimpleMessage>send %s for entity %s",msgName.c_str(), entityId.toString().c_str());
} // sendSimpleMessage //


//--------------------------------------------------------------
//					sendMessage()  
//--------------------------------------------------------------
void sendMessage( const NLMISC::CEntityId &entityId, const std::string &msgName, const NLMISC::CEntityId &entityIdForText )
{
	if ( entityId.getType() != RYZOMID::player)
		return;

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
	params[0].setEIdAIAlias( entityIdForText, CAIAliasTranslator::getInstance()->getAIAlias( entityIdForText ) );
	CCharacter::sendDynamicSystemMessage(entityId, msgName, params);

	INFOLOG("<sendMessage>send %s (param entity %s) for entity %s",msgName.c_str(), entityIdForText.toString().c_str(), entityId.toString().c_str());
} // sendMessage //


//--------------------------------------------------------------
//					sendCombatFailedMessages()  
//--------------------------------------------------------------
void sendCombatFailedMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId, ECombatFailDisplay failDisplay )
{
	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	if ( aggressorId.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);		
		params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias( victimId ) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_ACTOR_MISS", params);

		// In case of melee, send a "Evade" flying text msg onto the victim
		if(failDisplay==FailMelee)
			PlayerManager.sendImpulseToClient(aggressorId, std::string("COMBAT:FLYING_TEXT"), TheDataset.getDataSetRow(victimId).getCompressedIndex(), (uint8)COMBAT_FLYING_TEXT::TargetEvade);
		// In case of Range, It is better if we display a "Fail" onto the agressor (because the missile isn't even launched!)
		else if(failDisplay==FailRange)
			PlayerManager.sendImpulseToClient(aggressorId, std::string("COMBAT:FLYING_TEXT"), TheDataset.getDataSetRow(aggressorId).getCompressedIndex(), (uint8)COMBAT_FLYING_TEXT::SelfFailure);
	}

	if ( victimId.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);		
		params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias( aggressorId ) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "COMBAT_DEFENDER_MISS", params);

		// send flying text msg to victim
		if(failDisplay==FailMelee)
			PlayerManager.sendImpulseToClient(victimId, std::string("COMBAT:FLYING_TEXT"), TheDataset.getDataSetRow(victimId).getCompressedIndex(), (uint8)COMBAT_FLYING_TEXT::SelfEvade);
		// In case of range do nothing (the agrressor actually doesn't send any missile/bullet)
	}

// spectators
	// Send to 'speech' group
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	//speechGroupId.setType( RYZOMID::dynChatGroup );
//	SM_STATIC_PARAMS_2(params2, STRING_MANAGER::entity, STRING_MANAGER::entity);
//	params2[0].EId = aggressorId;
//	params2[1].EId = victimId;
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//	excluded.push_back(victimId);
//
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "COMBAT_SPECTATOR_MISS", params2);
//
} // sendCombatFailedMessages //


//--------------------------------------------------------------
//					sendDeathMessages()  
//--------------------------------------------------------------
void sendDeathMessages( const NLMISC::CEntityId &killerId, const NLMISC::CEntityId &deadId )
{
	bool self = (killerId == deadId);

	if (self)
	{
		sendDynamicSystemMessage(TheDataset.getDataSetRow(killerId), "DEATH_SELF_KILL");
	}
	else
	{
		if ( killerId.getType() == RYZOMID::player )
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);			
			params[0].setEIdAIAlias( deadId, CAIAliasTranslator::getInstance()->getAIAlias( deadId ) );
			sendDynamicSystemMessage(TheDataset.getDataSetRow(killerId), "DEATH_KILLER", params);
		}

		if ( deadId.getType() == RYZOMID::player )
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);			
			params[0].setEIdAIAlias( killerId, CAIAliasTranslator::getInstance()->getAIAlias( killerId) );
			sendDynamicSystemMessage(TheDataset.getDataSetRow(deadId), "DEATH_VICTIM", params);
		}
	}

// spectators
//	// Send to 'speech' group
//	CEntityId senderId;
//	if ( deadId.getType() == RYZOMID::player || deadId.getType() == RYZOMID::npc )
//	{
//		senderId = deadId;
//	}
//	else if ( killerId.getType() == RYZOMID::player || killerId.getType() == RYZOMID::npc )
//	{
//		senderId = killerId;
//	}
//	else
//		return;
//
//	if (self)
//	{
//		string msgName = "DEATH_SELF_KILL_SPECTATORS";
//		params.resize(1);
//		switch(killerId.getType())
//		{
//		case RYZOMID::player:
//			msgName += "_PLAYER";
//			params[0].Type = STRING_MANAGER::player;
//			break;
//		case RYZOMID::npc:
//			msgName += "_NPC";
//			params[0].Type = STRING_MANAGER::bot;
//			break;
//		case RYZOMID::creature:
//			msgName += "_CREATURE";
//			params[0].Type = STRING_MANAGER::creature;
//			break;
//		default:
//			return;
//		};		
//		params[0].EId = killerId;
//
//		vector<CEntityId> excluded;
//		excluded.push_back(killerId);
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(killerId), excluded, msgName, params);
//	}
//	else
//	{
//		params.resize(2);
//		params[0].Type = STRING_MANAGER::entity;
//		params[0].EId = killerId;
//		params[1].Type = STRING_MANAGER::entity;
//		params[1].EId = deadId;
//
//		vector<CEntityId> excluded;
//		excluded.push_back(killerId);
//		excluded.push_back(deadId);
//
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "DEATH_SPECTATORS", params);
//	}

} // sendDeathMessages //

//--------------------------------------------------------------
//					sendHitMessages()  
//--------------------------------------------------------------
void sendHitMessages( const CEntityId &aggressorId, const CEntityId &victimId, bool self, sint32 amount, sint32 maxDamage, sint32 lostStamina, sint32 lostSap, BODY::TBodyPart bodyPart)
{
	if (!self)
		self = (aggressorId == victimId);

	amount = abs(amount);
	lostStamina = abs(lostStamina);
	lostSap = abs(lostSap);

	if (maxDamage == amount)
		maxDamage = 0;

//	TVectorParamCheck params;

	if ( aggressorId.getType() == RYZOMID::player )
	{
		if (! self)
		{
			if (bodyPart != BODY::UnknownBodyPart)
			{
				SM_STATIC_PARAMS_4(params, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::body_part);
				params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias( victimId) );
				params[1].Int = amount;
				params[2].Int = maxDamage;
				params[3].Enum = bodyPart;
				sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_ACTOR_HIT_LOC", params);
			}
			else
			{
				SM_STATIC_PARAMS_3(params, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::integer);				
				params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias( victimId ) );
				params[1].Int = amount;
				params[2].Int = maxDamage;
				sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_ACTOR_HIT", params);
			}

			// lost stamina
			if (lostStamina>0)
			{
				SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::integer);				
				params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias( victimId ) );
				params[1].Int = lostStamina;
				sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_LOSE_STA_ACTOR", params);
			}
			// lost sap
			if (lostSap>0)
			{	
				SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::integer);				
				params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias( victimId) );
				params[1].Int = lostSap;
				sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_LOSE_SAP_ACTOR", params);
			}
		}
		else
		{
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params[0].Int = amount;
			params[1].Int = maxDamage;
			sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_ACTOR_HIT_SELF", params);

			// lost stamina
			if (lostStamina>0)
			{				
				params[0].Int = lostStamina;
				sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_LOSE_STA_SELF", params);
			}
			// lost sap
			if (lostSap>0)
			{				
				params[0].Int = lostSap;
				sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_LOSE_SAP_SELF", params);
			}
		}
	}

	if ( !self && victimId.getType() == RYZOMID::player )
	{
		if (bodyPart != BODY::UnknownBodyPart)
		{
			SM_STATIC_PARAMS_4(params, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::body_part);			
			params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias( aggressorId) );
			params[1].Int = amount;
			params[2].Int = maxDamage;
			params[3].Enum = bodyPart;
			sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "COMBAT_DEFENDER_HIT_LOC", params);
		}
		else
		{
			SM_STATIC_PARAMS_3(params, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::integer);			
			params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias( aggressorId ) );
			params[1].Int = amount;
			params[2].Int = maxDamage;
			sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "COMBAT_DEFENDER_HIT", params);
		}

		// lost stamina
		if (lostStamina>0)
		{				
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::integer);			
			params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias( aggressorId ) );
			params[1].Int = lostStamina;
			sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "COMBAT_LOSE_STA_TARGET", params);
		}
		// lost sap
		if (lostSap>0)
		{
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::integer);			
			params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias( aggressorId) );
			params[1].Int = lostSap;
			sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "COMBAT_LOSE_SAP_TARGET", params);
		}
	}
	
// spectators
//	// Send to 'speech' group
//	CEntityId speechGroupId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		speechGroupId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		speechGroupId = victimId;
//	}
//	else
//		return;
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//	excluded.push_back(victimId);
//	if (!self)
//	{		
//		params.resize(4);
//		params[0].Type = STRING_MANAGER::entity;
//		params[0].EId = aggressorId;
//		params[1].Type = STRING_MANAGER::entity;
//		params[1].EId = victimId;
//		params[2].Type = STRING_MANAGER::integer;
//		params[2].Int = amount;
//		params[3].Type = STRING_MANAGER::integer;
//		params[3].Int = maxDamage;
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(speechGroupId), excluded, "COMBAT_SPECTATOR_HIT", params);
//
//		// lost stamina
//		if (lostStamina>0)
//		{				
//			params[2].Int = lostStamina;
//			sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(speechGroupId), excluded, "COMBAT_LOSE_STA_SPECTATORS", params);
//		}
//		// lost sap
//		if (lostSap>0)
//		{				
//			params[2].Int = lostSap;
//			sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(speechGroupId), excluded, "COMBAT_LOSE_SAP_SPECTATORS", params);
//		}
//	}
//	else
//	{
//		params.resize(2);
//		string type;
//		switch( aggressorId.getType() )
//		{
//		case RYZOMID::player:
//			type ="_PLAYER";
//			break;
//		case RYZOMID::npc:
//			type ="_NPC";
//			break;
//		case RYZOMID::creature:
//			type ="_CREATURE";
//			break;
//		default:
//			return;
//		};
//
//		params[0].Type = STRING_MANAGER::player;
//		params[0].EId = aggressorId;
//		params[1].Type = STRING_MANAGER::integer;
//		params[1].Int = amount;
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(speechGroupId), excluded, "COMBAT_SPECTATOR_HIT_SELF" + type, params);
//		// lost stamina
//		if (lostStamina>0)
//		{				
//			params[1].Int = lostStamina;
//			sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(speechGroupId), excluded, "COMBAT_LOSE_STA_SELF_SPECTATORS" + type, params);
//		}
//		// lost sap
//		if (lostSap>0)
//		{				
//			params[1].Int = lostSap;
//			sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(speechGroupId), excluded, "COMBAT_LOSE_SAP_SELF_SPECTATORS" + type, params);
//		}
//	}	
} // sendHitMessages //

/**
 * an entity is wounded by a damage shield
 */
void sendDamageShieldDamageMessages(const NLMISC::CEntityId &woundedEntity, const NLMISC::CEntityId &defender, uint16 damage, uint16 hpDrain)
{
	if (!damage && !hpDrain)
		return;
	
//	TVectorParamCheck params;
	if (woundedEntity.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_3(params, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].setEIdAIAlias( defender, CAIAliasTranslator::getInstance()->getAIAlias( defender) );
		params[1].Int = damage;
		params[2].Int = hpDrain;
		sendDynamicSystemMessage(TheDataset.getDataSetRow(woundedEntity), "COMBAT_DMG_SHIELD_ATTACKER", params);
	}

	if (defender.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_3(params, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::integer);		
		params[0].setEIdAIAlias( woundedEntity, CAIAliasTranslator::getInstance()->getAIAlias( woundedEntity) );
		params[1].Int = damage;
		params[2].Int = hpDrain;
		sendDynamicSystemMessage(TheDataset.getDataSetRow(defender), "COMBAT_DMG_SHIELD_DEFENDER", params);
	}
	
	// spectators
	// Send to 'speech' group
	CEntityId speechGroupId;
	if ( woundedEntity.getType() == RYZOMID::player || woundedEntity.getType() == RYZOMID::npc )
	{
		speechGroupId = woundedEntity;
	}
	else if ( defender.getType() == RYZOMID::player || defender.getType() == RYZOMID::npc )
	{
		speechGroupId = defender;
	}
	else
		return;
	
	vector<CEntityId> excluded;
	excluded.push_back(woundedEntity);
	excluded.push_back(defender);

	SM_STATIC_PARAMS_4(params, STRING_MANAGER::entity, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::integer);	
	params[0].setEIdAIAlias( woundedEntity, CAIAliasTranslator::getInstance()->getAIAlias( woundedEntity) );
	params[1].setEIdAIAlias( defender, CAIAliasTranslator::getInstance()->getAIAlias( defender) );
	params[2].Int = damage;
	params[3].Int = hpDrain;
	sendDynamicSystemMessage(TheDataset.getDataSetRow(defender), "COMBAT_DMG_SHIELD_SPECTATORS", params);
	
} // sendDamageShieldDamageMessages //

/**
 * an vampirism proc has been triggered
 */
void sendVampirismProcMessages(const NLMISC::CEntityId &actingEntity, const NLMISC::CEntityId &defender, sint32 hpDrain)
{
	if (!hpDrain)
		return;
	
	if (actingEntity.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::integer);
		params[0].setEIdAIAlias(defender, CAIAliasTranslator::getInstance()->getAIAlias(defender));
		params[1].Int = hpDrain;
		sendDynamicSystemMessage(TheDataset.getDataSetRow(actingEntity), "COMBAT_PROC_VAMPIRISM_ATTACKER", params);
	}
	if (defender.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::integer);
		params[0].setEIdAIAlias(actingEntity, CAIAliasTranslator::getInstance()->getAIAlias(actingEntity));
		params[1].Int = hpDrain;
		sendDynamicSystemMessage(TheDataset.getDataSetRow(defender), "COMBAT_PROC_VAMPIRISM_DEFENDER", params);
	}
	// spectators
	// :TODO: See if this is needed and activate it if that's the case.
	// :NOTE: This worked when this line was first commited (see CVS annotate).
	bool const sendMsgToSpectators = false;
	if (sendMsgToSpectators)
	{
		// Send to 'speech' group
		CEntityId speechGroupId;
		if ( actingEntity.getType() == RYZOMID::player || actingEntity.getType() == RYZOMID::npc )
		{
			speechGroupId = actingEntity;
		}
		else if ( defender.getType() == RYZOMID::player || defender.getType() == RYZOMID::npc )
		{
			speechGroupId = defender;
		}
		else
			return;
		
		vector<CEntityId> excluded;
		excluded.push_back(actingEntity);
		excluded.push_back(defender);
		
		SM_STATIC_PARAMS_3(params, STRING_MANAGER::entity, STRING_MANAGER::entity, STRING_MANAGER::integer);
		params[0].setEIdAIAlias(actingEntity, CAIAliasTranslator::getInstance()->getAIAlias(actingEntity));
		params[1].setEIdAIAlias(defender, CAIAliasTranslator::getInstance()->getAIAlias(defender));
		params[2].Int = hpDrain;
		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(speechGroupId), excluded, "COMBAT_PROC_VAMPIRISM_SPECTATORS", params);
	}
}

static void sendItemSpecialEffectProcMessageHelper(ITEM_SPECIAL_EFFECT::TItemSpecialEffect type, CEntityBase* actor, CEntityBase* target, sint32 param, sint32 param2)
{
	switch (type)
	{
	case ITEM_SPECIAL_EFFECT::ISE_FIGHT_ADD_CRITICAL:
		// Not a proc
		break;
	case ITEM_SPECIAL_EFFECT::ISE_FIGHT_VAMPIRISM:
		PHRASE_UTILITIES::sendVampirismProcMessages(actor->getId(), target->getId(), param);
		break;
	case ITEM_SPECIAL_EFFECT::ISE_MAGIC_DIVINE_INTERVENTION:
		sendDynamicSystemMessage(actor->getEntityRowId(), "ISE_MAGIC_DIVINE_INTERVENTION", STRING_MANAGER::CVectorParamCheck());
		break;
	case ITEM_SPECIAL_EFFECT::ISE_MAGIC_SHOOT_AGAIN:
		sendDynamicSystemMessage(actor->getEntityRowId(), "ISE_MAGIC_SHOOT_AGAIN", STRING_MANAGER::CVectorParamCheck());
		break;
	case ITEM_SPECIAL_EFFECT::ISE_CRAFT_ADD_STAT_BONUS:
		{
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::score, STRING_MANAGER::integer);
			params[0].Enum = (SCORES::TScores)param;
			params[1].Int = param2;
			sendDynamicSystemMessage(actor->getEntityRowId(), "ISE_CRAFT_ADD_STAT_BONUS", params);
		}
		break;
	case ITEM_SPECIAL_EFFECT::ISE_CRAFT_ADD_LIMIT:
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = param;
			sendDynamicSystemMessage(actor->getEntityRowId(), "ISE_CRAFT_ADD_LIMIT", params);
		}
		break;
	case ITEM_SPECIAL_EFFECT::ISE_FORAGE_ADD_RM:
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = param;
			sendDynamicSystemMessage(actor->getEntityRowId(), "ISE_FORAGE_ADD_RM", params);
		}
		break;
	case ITEM_SPECIAL_EFFECT::ISE_FORAGE_NO_RISK:
		sendDynamicSystemMessage(actor->getEntityRowId(), "ISE_FORAGE_NO_RISK", STRING_MANAGER::CVectorParamCheck());
		break;
	}
}

void sendItemSpecialEffectProcMessage(ITEM_SPECIAL_EFFECT::TItemSpecialEffect type, CEntityBase* actor, CEntityBase* target, sint32 param, sint32 param2)
{
	switch (type)
	{
	case ITEM_SPECIAL_EFFECT::ISE_FIGHT_ADD_CRITICAL:
		// Not a proc
		break;
	case ITEM_SPECIAL_EFFECT::ISE_CRAFT_ADD_STAT_BONUS:
	case ITEM_SPECIAL_EFFECT::ISE_CRAFT_ADD_LIMIT:
		sendItemSpecialEffectProcMessageHelper(type, actor, target, param, param2);
		break;
	case ITEM_SPECIAL_EFFECT::ISE_FIGHT_VAMPIRISM:
	case ITEM_SPECIAL_EFFECT::ISE_MAGIC_DIVINE_INTERVENTION:
	case ITEM_SPECIAL_EFFECT::ISE_MAGIC_SHOOT_AGAIN:
	case ITEM_SPECIAL_EFFECT::ISE_FORAGE_ADD_RM:
	case ITEM_SPECIAL_EFFECT::ISE_FORAGE_NO_RISK:
		sendItemSpecialEffectProcMessageHelper(type, actor, target, param, param2);
		PlayerManager.sendImpulseToClient(actor->getId(), std::string("COMBAT:FLYING_TEXT_ISE"), (uint32)(actor->getEntityRowId().getIndex()), (uint32)0xC0D0FF, (uint8)type, (sint32)param);
		break;
	}
}

/**
 * A natural event (forage source explosion, toxic cloud...) hits an entity, send all relevant messages to the entities around.
 */
void sendNaturalEventHitMessages( RYZOMID::TTypeId aggressorType, const NLMISC::CEntityId &victimId, sint32 amount, sint32 amountWithoutArmor, sint32 avoided )
{
	amount = abs(amount);

	// Select msg
	const char *msgD; //, *msgS;
	switch ( aggressorType )
	{
	case RYZOMID::forageSource:
		{
			msgD = "SOURCE_EXPLOSION_DEFENDER_HIT";
			if ( victimId.getType() == RYZOMID::player )
			{
				SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
				params[0].Int = amount;
				params[1].Int = amountWithoutArmor;
				params[2].Int = avoided;
				sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), msgD, params);
			}
			//msgS = "SOURCE_EXPLOSION_SPECTATOR_HIT";
			return;
			break;
		}
	case RYZOMID::fx_entity:
		msgD = "TOXIC_CLOUD_DEFENDER_HIT";
		//msgS = "TOXIC_CLOUD_SPECTATOR_HIT";
		break;
	case RYZOMID::creature: // Kami!
		msgD = "KAMI_ANGER_DEFENDER_HIT";
		//msgS = "KAMI_ANGER_SPECTATOR_HIT";
		break;
	default:
		nlwarning( "sendNaturalEventHitMessages: unknown aggressor type %u", aggressorType );
		return;
	}

	// Send msg to hit player
	if ( victimId.getType() == RYZOMID::player )
	{
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].Int = amount;
		params[1].Int = avoided;
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), msgD, params);
	}

//	// Send msg to spectators
//	vector<CEntityId> excluded;
//	excluded.push_back(victimId);
//	params.resize(2);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = victimId;
//	params[1].Type = STRING_MANAGER::integer;
//	params[1].Int = amount;
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(victimId), excluded, msgS, params);
}



//--------------------------------------------------------------
//					sendCriticalHitMessage()  
//--------------------------------------------------------------
void sendCriticalHitMessage( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId )
{
//	TVectorParamCheck params;

	if ( aggressorId.getType() == RYZOMID::player )
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);		
		params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias( victimId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_ACTOR_CRITICAL_HIT", params);
	}

	if ( victimId.getType() == RYZOMID::player )
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);		
		params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias( aggressorId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "COMBAT_DEFENDER_CRITICAL_HIT", params);
	}

// spectators
//	// Send to 'speech' group
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	params.resize(2);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = aggressorId;
//	params[1].Type = STRING_MANAGER::entity;
//	params[1].EId = victimId;
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//	excluded.push_back(victimId);
//
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "COMBAT_SPECTATOR_CRITICAL_HIT", params);
//
} // sendCriticalHitMessage //

//--------------------------------------------------------------
//					sendFumbleMessage()  
//--------------------------------------------------------------
void sendFumbleMessage( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId )
{
//	TVectorParamCheck params;

	if ( aggressorId.getType() == RYZOMID::player )
	{
		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_ACTOR_FUMBLE");
	}

	if ( victimId.getType() == RYZOMID::player )
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);		
	params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias( aggressorId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "COMBAT_DEFENDER_FUMBLE", params);
	}

// spectators
//	// Send to 'speech' group
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	params.resize(1);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = aggressorId;
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//	excluded.push_back(victimId);
//
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "COMBAT_SPECTATOR_FUMBLE", params);

} // sendFumbleMessage //

//--------------------------------------------------------------
//					sendDodgeMessages()  
//--------------------------------------------------------------
void sendDodgeMessages( const CEntityId &aggressorId, const CEntityId &victimId, bool sendFlyingText )
{
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);

	if ( aggressorId.getType() == RYZOMID::player )
	{		
		params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias( victimId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_DODGE_ATTACKER", params);

		// send flying text msg to agressor
		if(sendFlyingText)
			PlayerManager.sendImpulseToClient(aggressorId, std::string("COMBAT:FLYING_TEXT"), TheDataset.getDataSetRow(victimId).getCompressedIndex(), (uint8)COMBAT_FLYING_TEXT::TargetDodge);
	}

	if ( victimId.getType() == RYZOMID::player )
	{
		params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias( aggressorId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "COMBAT_DODGE_DEFENDER", params);
	}

// spectators
//	// Send to 'speech' group
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	params.resize(2);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = aggressorId;
//	params[1].Type = STRING_MANAGER::entity;
//	params[1].EId = victimId;
//	
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//	excluded.push_back(victimId);
//
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "COMBAT_DODGE_SPECTATOR", params);
} // sendDodgeMessages //

//--------------------------------------------------------------
//					sendParryMessages()  
//--------------------------------------------------------------
void sendParryMessages( const CEntityId &aggressorId, const CEntityId &victimId, bool sendFlyingText )
{
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);

	if ( aggressorId.getType() == RYZOMID::player )
	{
		params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias( victimId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_PARRY_ATTACKER", params);

		// send flying text msg to agressor
		if(sendFlyingText)
			PlayerManager.sendImpulseToClient(aggressorId, std::string("COMBAT:FLYING_TEXT"), TheDataset.getDataSetRow(victimId).getCompressedIndex(), (uint8)COMBAT_FLYING_TEXT::TargetParry);
	}

	if ( victimId.getType() == RYZOMID::player )
	{		
		params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias( aggressorId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "COMBAT_PARRY_DEFENDER", params);
	}

// spectators
//	// Send to 'speech' group
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	params.resize(2);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = aggressorId;
//	params[1].Type = STRING_MANAGER::entity;
//	params[1].EId = victimId;
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//	excluded.push_back(victimId);
//
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "COMBAT_PARRY_SPECTATOR", params);
} // sendParryMessages //

//--------------------------------------------------------------
//					sendEngageMessages()  
//--------------------------------------------------------------
void sendEngageMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId )
{
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
	
	if ( aggressorId.getType() == RYZOMID::player )
	{
		params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias(victimId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_ATTACK_ACTOR", params);
	}

	if ( victimId.getType() == RYZOMID::player )
	{
		params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias(aggressorId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "COMBAT_ATTACK_DEFENDER", params);
	}

// spectators
//	// Send to 'speech' group
//	CEntityId speechGroupId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		speechGroupId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		speechGroupId = victimId;
//	}
//	else
//		return;
//
//	params.resize(2);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = aggressorId;
//	params[1].Type = STRING_MANAGER::entity;
//	params[1].EId = victimId;
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//	excluded.push_back(victimId);
//
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(speechGroupId), excluded, "COMBAT_ATTACK_SPECTATOR", params);
//
} // sendEngageMessages //


//--------------------------------------------------------------
//					sendDisengageMessages()  
//--------------------------------------------------------------
void sendDisengageMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId )
{
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
//	TVectorParamCheck params;

	if ( aggressorId.getType() == RYZOMID::player )
	{
		params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias(victimId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "COMBAT_LEAVE_ACTOR", params);
	}

	if ( victimId.getType() == RYZOMID::player )
	{
		params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias(aggressorId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "COMBAT_LEAVE_DEFENDER", params);
	}

// spectators
//	// Send to 'speech' group
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	params.resize(2);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = aggressorId;
//	params[1].Type = STRING_MANAGER::entity;
//	params[1].EId = victimId;
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//	excluded.push_back(victimId);
//
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "COMBAT_LEAVE_SPECTATOR", params);
//
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
	CPhraseManager::getInstance().engageMelee( TheDataset.getDataSetRow(entityId), TheDataset.getDataSetRow(targetId) );
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
	CPhraseManager::getInstance().engageRange( TheDataset.getDataSetRow(entityId), TheDataset.getDataSetRow(targetId) );

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
//						testOffensiveActionAllowed
//--------------------------------------------------------------
bool testOffensiveActionAllowed( const NLMISC::CEntityId &actorId, const NLMISC::CEntityId &targetId, string &errorCode, bool mainTarget )
{
	CEntityBase *target = CEntityBaseManager::getEntityBasePtr(targetId);
	if (!target)
	{
		nlwarning("<testOffensiveActionAllowed> Invalid entity Id %s", targetId.toString().c_str() );
		errorCode = "INVALID_TARGET";
		return false;
	}

	// test target is alive
	if (target->isDead())
	{
		errorCode = "COMBAT_TARGET_DEAD";
		return false;
	}

	// AI 
	if (actorId.getType() != RYZOMID::player)
	{
		// test target isn't invulnerable
		if (target->getContextualProperty().directAccessForStructMembers().invulnerable())
		{
			// check target Faction attackable flags
			CCreature *creature = dynamic_cast<CCreature *> (target);
			if (!creature || !creature->checkFactionAttackable(actorId))
			{
				errorCode = "BS_TARGET_NOT_ATTACKABLE";
				return false;
			}
		}

		if (mainTarget == true)
			return true;
		else
		{
			CCreature* actor = CreatureManager.getCreature( actorId );
			if (!actor)
				return false;

			if (!actor->getContextualProperty().directAccessForStructMembers().attackable())
			{
				// actor is a guard or similar so cannot harm a player or a non attackable creature, unless faction attackable
				if (targetId.getType() == RYZOMID::player || !target->getContextualProperty().directAccessForStructMembers().attackable())
				{
					if ( actor->checkFactionAttackable( targetId ) )
					{
						return true;
					}
					return false;
				}
				else
					return true;
			}
			else
			{
				// actor is a creature or similar so can harm a player or a non attackable creature
				if (targetId.getType() == RYZOMID::player || !target->getContextualProperty().directAccessForStructMembers().attackable())
					return true;
				else
					return false;
			}
		}
	}


	RYZOMID::TTypeId targetType = (RYZOMID::TTypeId)targetId.getType();
	switch ( targetType )
	{
	case RYZOMID::player:
		{
			if (actorId.getType() == RYZOMID::player)
			{
				CCharacter* actor = PlayerManager.getChar( actorId );
				if ( ! actor )
					return false;
				if ( AllowPVP ) // free PVP everywhere?
					return true;
				if ( CPVPManager2::getInstance()->isOffensiveActionValid( actor, target ) )
					return true;
				errorCode = "EGS_PVP_NOT_ALLOWED";
				return false;
			}
		}
		break;

	case RYZOMID::creature:	
	case RYZOMID::mount:
	case RYZOMID::npc:
		{
			RYZOMID::TTypeId type = (RYZOMID::TTypeId)actorId.getType();
			if (type == RYZOMID::player)
			{
				CCharacter* actor = PlayerManager.getChar( actorId );
				if ( ! actor )
					return false;
				if ( CPVPManager2::getInstance()->isOffensiveActionValid( actor, target ) )
					return true;
				if ( CPVPFactionRewardManager::getInstance().isAttackable( actor, target ) )
					return true;
				// check if ennemy outpost squad
				if( actor->getOutpostAlias() !=0 )
				{
					if( actor->getOutpostAlias() == target->getOutpostAlias() )
					{
						if( actor->getOutpostSide() != target->getOutpostSide() )
						{
							return true;
						}
					}
				}

			}
			if (!target->getContextualProperty().directAccessForStructMembers().attackable())
			{
				CCreature *creature = dynamic_cast<CCreature *> (target);
				// check target Faction attackable flags
				if (!creature || !creature->checkFactionAttackable(actorId))
				{
					errorCode = "BS_TARGET_NOT_ATTACKABLE";
					return false;
				}
			}
		}
		break;

	default:
		break;

	};
	return true;
} // testOffensiveActionAllowed //

bool validateSpellTarget( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId, ACTNATURE::TActionNature action, std::string &errorCode, bool mainTarget )
{
	if ( !( TheDataset.isAccessible(actorRowId) && TheDataset.isAccessible(targetRowId)) )
	{
		errorCode = "MAGIC_NEED_TARGET";
		return false;
	}
	
	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( targetRowId );
	if( !target )
	{
		//nlwarning("<validateSpellTarget> Cannot find target entity from rowId %s", targetRowId.toString().c_str());
		errorCode = "MAGIC_NEED_TARGET";
		nldebug("validateSpellTarget for entity %s returning false because MAGIC_NEED_TARGET",getEntityIdFromRow(actorRowId).toString().c_str());
		return false;
	}
	
	// an invisible player is not hit by the spell
	if( TheDataset.getEntityId(targetRowId).getType() == RYZOMID::player )
	{
		if( getEntityIdFromRow(actorRowId).getType() == RYZOMID::player )
		{
			if( !R2_VISION::isEntityVisibleToPlayers(target->getWhoSeesMe()) )
			{
//				nldebug("validateSpellTarget for PLAYER %s returning false because entity %s not visible to PLAYERS (%x)",getEntityIdFromRow(actorRowId).toString().c_str(),TheDataset.getEntityId(targetRowId).toString().c_str(),target->getWhoSeesMe());
				return false;
			}
		}
		else
		{
			if( !R2_VISION::isEntityVisibleToMobs(target->getWhoSeesMe()) )
			{
//				nldebug("validateSpellTarget for MOB %s returning false because entity %s not visible to MOBS (%x)",getEntityIdFromRow(actorRowId).toString().c_str(),TheDataset.getEntityId(targetRowId).toString().c_str(),target->getWhoSeesMe());
				return false;
			}
		}
	}
		
	// main target always valid for npcs
	//	if (mainTarget && TheDataset.getEntityId(actorRowId).getType() != RYZOMID::player)
	//		return true;
	
	if ( action == ACTNATURE::FIGHT || action == ACTNATURE::OFFENSIVE_MAGIC )
	{
		if ( target->isDead())
		{
			errorCode = "MAGIC_TARGET_DEAD";
//			nldebug("validateSpellTarget for entity %s returning false because %s MAGIC_TARGET_DEAD",getEntityIdFromRow(actorRowId).toString().c_str(),TheDataset.getEntityId(targetRowId).toString().c_str());
			return false;
		}

		// ok if the entity is mad and hits itself
		if (mainTarget && targetRowId == actorRowId)
			return true;

		if ( !PHRASE_UTILITIES::testOffensiveActionAllowed(actorRowId, targetRowId, errorCode, mainTarget) )
		{
			errorCode = "MAGIC_CAN_ONLY_CAST_ON_ENEMY";
//			nldebug("validateSpellTarget for entity %s returning false because %s MAGIC_CAN_ONLY_CAST_ON_ENEMY",getEntityIdFromRow(actorRowId).toString().c_str(),TheDataset.getEntityId(targetRowId).toString().c_str());
			return false;
		}
		return true;
	}
	else
	{
		// test entity isn't dead already (unless it's a player)
		if	(target->isDead())
		{
			if (target->getId().getType() == RYZOMID::player && mainTarget && target->currentHp() > (-target->maxHp()) )
			{
				// possible as a player in a 'coma' can still be healed (only main target)
			}
			else
			{
				errorCode = "MAGIC_TARGET_DEAD";
//				nldebug("validateSpellTarget for entity %s returning false because %s MAGIC_TARGET_DEAD",getEntityIdFromRow(actorRowId).toString().c_str(),TheDataset.getEntityId(targetRowId).toString().c_str());
				return false;
			}
		}

		// player can only heal other players
		if ( getEntityIdFromRow(actorRowId).getType() == RYZOMID::player &&	target->getId().getType() != RYZOMID::player )
		{
			CEntityBase * actor = CEntityBaseManager::getEntityBasePtr( actorRowId );
			CCreature * creature = CreatureManager.getCreature( target->getId() );
			bool bFactionAttackable = (actor && creature && creature->checkFactionAttackable( actor->getId()));

			if (bFactionAttackable || target->getContextualProperty().directAccessForStructMembers().attackable() )
			{
				errorCode = "MAGIC_CANNOT_CAST_ON_ENEMY";
//				nldebug("validateSpellTarget for entity %s returning false because %s MAGIC_CANNOT_CAST_ON_ENEMY",getEntityIdFromRow(actorRowId).toString().c_str(),TheDataset.getEntityId(targetRowId).toString().c_str());
				return false;
			}
			else
			{
				errorCode = "MAGIC_CAN_ONLY_CAST_ON_PLAYERS";
//				nldebug("validateSpellTarget for entity %s returning false because %s MAGIC_CAN_ONLY_CAST_ON_PLAYERS",getEntityIdFromRow(actorRowId).toString().c_str(),TheDataset.getEntityId(targetRowId).toString().c_str());
				return false;
			}
		}

		// player can heal players unless they are engaged in duels, and bots in outpost pvp
		if ( getEntityIdFromRow(actorRowId).getType() == RYZOMID::player )
		{
			CCharacter * actor = PlayerManager.getChar( actorRowId );
			if (!actor)
			{
//				nldebug("validateSpellTarget for entity %s returning false because !actor",getEntityIdFromRow(actorRowId).toString().c_str());
				return false;
			}
			if ( AllowPVP ) // free PVP everywhere?
				return true;
			if ( !CPVPManager2::getInstance()->isCurativeActionValid( actor, target ) )
			{
//				nldebug("validateSpellTarget for entity %s target %s returning false because !CPVPManager2::getInstance()->isCurativeActionValid( actor, target )",getEntityIdFromRow(actorRowId).toString().c_str(),TheDataset.getEntityId(targetRowId).toString().c_str());
				return false;
			}
		}

		// npcs can heal other npcs of same type (ie: attackable can heal attackables, non attackables can heal non attackables)
		if ( getEntityIdFromRow(actorRowId).getType() != RYZOMID::player )
		{
			CEntityBase * actor = CEntityBaseManager::getEntityBasePtr( actorRowId );
			if (!actor)
			{
//				nldebug("validateSpellTarget for entity %s returning false because !actor",getEntityIdFromRow(actorRowId).toString().c_str());
				return false;
			}

			if (!actor->getContextualProperty().directAccessForStructMembers().attackable())
			{
				// actor is a guard or similar so can heal a player or a non attackable creature
				if (target->getId().getType() == RYZOMID::player || !target->getContextualProperty().directAccessForStructMembers().attackable())
					return true;
				else
				{
//					nldebug("validateSpellTarget for entity %s returning false because mob not allowed to heal player %s",getEntityIdFromRow(actorRowId).toString().c_str(),TheDataset.getEntityId(targetRowId).toString().c_str());
					return false;
				}
			}
			else
			{
				// actor is an offensive creature or similar so cannot heal a player or a non attackable creature
				if (target->getId().getType() == RYZOMID::player || !target->getContextualProperty().directAccessForStructMembers().attackable())
				{
//					nldebug("validateSpellTarget for entity %s returning false because mob not allowed to heal player %s",getEntityIdFromRow(actorRowId).toString().c_str(),TheDataset.getEntityId(targetRowId).toString().c_str());
					return false;
				}
				else
					return true;
			}
		}
		
		return true;
	}
}

//--------------------------------------------------------------
//					sendScoreModifierSpellMessage
//--------------------------------------------------------------
void sendScoreModifierSpellMessage( const CEntityId &aggressorId, const CEntityId &victimId, sint32 damage, sint32 maxDamage, SCORES::TScores score, ACTNATURE::TActionNature nature )
{
	// at least the victim or the agressor must be a player, otherwise do not send any messages
	if ( (aggressorId.getType() != RYZOMID::player) && ( victimId.getType() != RYZOMID::player ))
		return;

	TVectorParamCheck params;

	string msgName;
	bool self = (aggressorId == victimId);

	if ( aggressorId.getType() == RYZOMID::player)
	{
		switch( score )
		{
		case SCORES::hit_points:
			if ( damage > 0)
				msgName = self ? "MAGIC_SELF_HEAL_HP" : "MAGIC_HEAL_HP_CASTER";
			else
				msgName = self ? "MAGIC_SELF_DAMAGE_HP" : "MAGIC_DAMAGE_HP_CASTER";
			break;
		case SCORES::stamina:
			if (damage > 0)
				msgName = self ? "MAGIC_SELF_HEAL_STA" : "MAGIC_HEAL_STA_CASTER";
			else
				msgName = self ? "MAGIC_SELF_DAMAGE_STA" : "MAGIC_DAMAGE_STA_CASTER";
			break;
		case SCORES::sap:
			if (damage > 0)
				msgName = self ? "MAGIC_SELF_HEAL_SAP" : "MAGIC_HEAL_SAP_CASTER";
			else
				msgName = self ? "MAGIC_SELF_DAMAGE_SAP" : "MAGIC_DAMAGE_SAP_CASTER";
			break;
		case SCORES::focus:
			if (damage > 0)
				msgName = self ? "MAGIC_SELF_HEAL_FOCUS" : "MAGIC_HEAL_FOCUS_CASTER";
			else
				msgName = self ? "MAGIC_SELF_DAMAGE_FOCUS" : "MAGIC_DAMAGE_FOCUS_CASTER";
			break;
		default:
			return;
		};
		
		if (self)
		{
//			SM_STATIC_PARAMS_2(params, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params.resize(2);
			params[0].Type = STRING_MANAGER::integer;
			params[0].Int = abs(damage);
			params[1].Type = STRING_MANAGER::integer;
			params[1].Int = abs(maxDamage);
		}
		else
		{
//			SM_STATIC_PARAMS_3(params, STRING_MANAGER::entity, STRING_MANAGER::integer);
			params.resize(3);
			params[0].Type = STRING_MANAGER::entity;
			params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias(victimId) );
			params[1].Type = STRING_MANAGER::integer;
			params[1].Int = abs(damage);
			params[2].Type = STRING_MANAGER::integer;
			params[2].Int = abs(maxDamage);
		}
		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), msgName, params);
	}

	if ( victimId.getType() == RYZOMID::player && !self)
	{
		switch( score )
		{
		case SCORES::hit_points:
			if ( damage > 0)
				msgName = "MAGIC_HEAL_HP_TARGET";
			else
				msgName = "MAGIC_DAMAGE_HP_TARGET";
			break;
		case SCORES::stamina:
			if (damage > 0)
				msgName = "MAGIC_HEAL_STA_TARGET";
			else
				msgName = "MAGIC_DAMAGE_STA_TARGET";
			break;
		case SCORES::sap:
			if (damage > 0)
				msgName = "MAGIC_HEAL_SAP_TARGET";
			else
				msgName = "MAGIC_DAMAGE_SAP_TARGET";
			break;
		case SCORES::focus:
			if (damage > 0)
				msgName = "MAGIC_HEAL_FOCUS_TARGET";
			else
				msgName = "MAGIC_DAMAGE_FOCUS_TARGET";
			break;
		default:
			return;
		};

		if( damage > 0 )
		{
//			SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params.resize(2);
			params[0].Type = STRING_MANAGER::entity;
			params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias(aggressorId) );
			params[1].Type = STRING_MANAGER::integer;
			params[1].Int = abs(damage);
		}
		else
		{
//			SM_STATIC_PARAMS_3(params, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params.resize(3);
			params[0].Type = STRING_MANAGER::entity;
			params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias(aggressorId) );
			params[1].Type = STRING_MANAGER::integer;
			params[1].Int = abs(damage);
			params[2].Type = STRING_MANAGER::integer;
			params[2].Int = abs(maxDamage);
		}
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), msgName, params);
	}
	
// send message to spectators
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	// Send to 'speech' group
//	switch( score )
//	{
//	case SCORES::hit_points:
//		if ( value > 0)
//			msgName = self ? "MAGIC_SELF_HEAL_HP_SPECTATORS" : "MAGIC_HEAL_HP_SPECTATORS";
//		else
//			msgName = self ? "MAGIC_SELF_DAMAGE_HP_SPECTATORS" : "MAGIC_DAMAGE_HP_SPECTATORS";
//		break;
//	case SCORES::stamina:
//		if (value > 0)
//			msgName = self ? "MAGIC_SELF_HEAL_STA_SPECTATORS" : "MAGIC_HEAL_STA_SPECTATORS";
//		else
//			msgName = self ? "MAGIC_SELF_DAMAGE_STA_SPECTATORS" : "MAGIC_DAMAGE_STA_SPECTATORS";
//		break;
//	case SCORES::sap:
//		if (value > 0)
//			msgName = self ? "MAGIC_SELF_HEAL_SAP_SPECTATORS" : "MAGIC_HEAL_SAP_SPECTATORS";
//		else
//			msgName = self ? "MAGIC_SELF_DAMAGE_SAP_SPECTATORS" : "MAGIC_DAMAGE_SAP_SPECTATORS";
//		break;
//	default:
//		return;
//	};
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//	excluded.push_back(victimId);
//
//	if (self)
//	{
//		params.resize(2);
//		switch(aggressorId.getType())
//		{
//		case RYZOMID::player:
//			msgName += "_PLAYER";
//			params[0].Type = STRING_MANAGER::player;
//			break;
//		case RYZOMID::npc:
//			msgName += "_NPC";
//			params[0].Type = STRING_MANAGER::bot;
//			break;
//		case RYZOMID::creature:
//			msgName += "_CREATURE";
//			params[0].Type = STRING_MANAGER::creature;
//			break;
//		default:
//			return;
//		};
//		
//		params[0].EId = aggressorId;
//		params[1].Type = STRING_MANAGER::integer;
//		params[1].Int = abs(value);
//
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, msgName, params);
//	}
//	else
//	{
//		params.resize(3);
//		params[0].Type = STRING_MANAGER::entity;
//		params[0].EId = aggressorId;
//		params[1].Type = STRING_MANAGER::entity;
//		params[1].EId = victimId;
//		params[2].Type = STRING_MANAGER::integer;
//		params[2].Int = abs(value);
//
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, msgName, params);
//	}
} // sendScoreModifierSpellMessage //




//--------------------------------------------------------------
//					sendSpellResistMessages()  
//--------------------------------------------------------------
void sendSpellResistMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId  )
{
	if ( !TheDataset.isAccessible(aggressorRowId) || !TheDataset.isAccessible(victimRowId) )
		return;

	CEntityId victimId = TheDataset.getEntityId(victimRowId);
	CEntityId aggressorId = TheDataset.getEntityId(aggressorRowId);

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
//	TVectorParamCheck params;

	if ( aggressorId.getType() == RYZOMID::player)
	{
		params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias(victimId) );

		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "MAGIC_RESIST_CASTER", params);

		// send flying text msg to agressor
		PlayerManager.sendImpulseToClient(aggressorId, std::string("COMBAT:FLYING_TEXT"), TheDataset.getDataSetRow(victimId).getCompressedIndex(), (uint8)COMBAT_FLYING_TEXT::TargetResist);
	}

	if ( victimId.getType() == RYZOMID::player)
	{
		params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias(aggressorId) );

		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "MAGIC_RESIST_TARGET", params);

		// send flying text msg to victim
		PlayerManager.sendImpulseToClient(victimId, std::string("COMBAT:FLYING_TEXT"), TheDataset.getDataSetRow(victimId).getCompressedIndex(), (uint8)COMBAT_FLYING_TEXT::SelfResist);
	}
	
// send message to spectators
//	// Send to 'speech' group
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	params.resize(2);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = aggressorId;
//	params[1].Type = STRING_MANAGER::entity;
//	params[1].EId = victimId;
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//	excluded.push_back(victimId);
//
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "MAGIC_RESIST_SPECTATORS", params);
//
} // sendSpellResistMessages //


//--------------------------------------------------------------
//					sendSpellBeginCastMessages()  
//--------------------------------------------------------------
void sendSpellBeginCastMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId, ACTNATURE::TActionNature nature )
{
	if ( !TheDataset.isAccessible(aggressorRowId) || !TheDataset.isAccessible(victimRowId) )
		return;

	CEntityId victimId = TheDataset.getEntityId(victimRowId);
	CEntityId aggressorId = TheDataset.getEntityId(aggressorRowId);

//	TVectorParamCheck params;
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);


	string msgName;
	bool self = (aggressorId == victimId);

	if ( aggressorId.getType() == RYZOMID::player)
	{
		switch( nature )
		{
		case ACTNATURE::CURATIVE_MAGIC:
			msgName = self ? "MAGIC_BEGIN_SELFCAST_GOOD_ACTOR" : "MAGIC_BEGIN_CAST_GOOD_ACTOR";
			break;
		case ACTNATURE::FIGHT:
		case ACTNATURE::OFFENSIVE_MAGIC:
			msgName = self ? "MAGIC_BEGIN_SELFCAST_BAD_ACTOR" : "MAGIC_BEGIN_CAST_BAD_ACTOR";
			break;
		default:
			msgName = self ? "MAGIC_BEGIN_SELFCAST_NEUTRAL_ACTOR" : "MAGIC_BEGIN_CAST_NEUTRAL_ACTOR";
		};

		if (self)
		{
			sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), msgName);
		}
		else
		{
			params[0].setEIdAIAlias( victimId, CAIAliasTranslator::getInstance()->getAIAlias(victimId) );
			sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), msgName, params);
		}
	}

	if ( !self && victimId.getType() == RYZOMID::player )
	{
		switch( nature )
		{
		case ACTNATURE::FIGHT:		
		case ACTNATURE::OFFENSIVE_MAGIC:		
			msgName ="MAGIC_BEGIN_CAST_BAD_TARGET";
			break;
		case ACTNATURE::CURATIVE_MAGIC:
			msgName = "MAGIC_BEGIN_CAST_GOOD_TARGET";
			break;
		default:
			msgName = "MAGIC_BEGIN_CAST_NEUTRAL_TARGET";			
		};

		params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias(aggressorId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), msgName, params);
	}
	
// send message to spectators
//	// Send to 'speech' group
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	switch( nature )
//	{
//	case ACTNATURE::FIGHT:
//	case ACTNATURE::OFFENSIVE_MAGIC:
//		msgName = self ? "MAGIC_BEGIN_SELFCAST_BAD_SPECTATOR" : "MAGIC_BEGIN_CAST_BAD_SPECTATOR";
//		break;
//	case ACTNATURE::CURATIVE_MAGIC:
//		msgName = self ? "MAGIC_BEGIN_SELFCAST_GOOD_SPECTATOR" : "MAGIC_BEGIN_CAST_GOOD_SPECTATOR";
//		break;
//	default:
//		msgName = self ? "MAGIC_BEGIN_SELFCAST_NEUTRAL_SPECTATOR" : "MAGIC_BEGIN_CAST_NEUTRAL_SPECTATOR";
//	};
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//
//	if (self)
//	{
//		params.resize(1);
//		switch(aggressorId.getType())
//		{
//		case RYZOMID::player:
//			msgName += "_PLAYER";
//			params[0].Type = STRING_MANAGER::player;
//			break;
//		case RYZOMID::npc:
//			msgName += "_NPC";
//			params[0].Type = STRING_MANAGER::bot;
//			break;
//		case RYZOMID::creature:
//			msgName += "_CREATURE";
//			params[0].Type = STRING_MANAGER::creature;
//			break;
//		default:
//			return;
//		};
//		
//		params[0].EId = aggressorId;
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, msgName, params);
//	}
//	else
//	{
//		excluded.push_back(victimId);
//
//		params.resize(2);
//		params[0].Type = STRING_MANAGER::entity;
//		params[0].EId = aggressorId;
//		params[1].Type = STRING_MANAGER::entity;
//		params[1].EId = victimId;
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, msgName, params);
//	}
} // sendSpellBeginCastMessages //


//--------------------------------------------------------------
//					sendSpellSuccessMessages()  
//--------------------------------------------------------------
void sendSpellSuccessMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId  )
{
	if ( !TheDataset.isAccessible(aggressorRowId) || !TheDataset.isAccessible(victimRowId) )
		return;

	CEntityId victimId = TheDataset.getEntityId(victimRowId);
	CEntityId aggressorId = TheDataset.getEntityId(aggressorRowId);

	bool self = (aggressorId == victimId);

	if (aggressorId.getType() == RYZOMID::player)
		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "MAGIC_END_CAST_SUCCESS_ACTOR");
	
	/*if (!self && victimId.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias(aggressorId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "MAGIC_END_CAST_SUCCESS_TARGET", params);
	}
*/
	
// send message to spectators
//	// Send to 'speech' group
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	string msgName;
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//
//	if (self)
//	{
//		params.resize(1);
//		switch(aggressorId.getType())
//		{
//		case RYZOMID::player:
//			msgName = "MAGIC_END_SELFCAST_SUCCESS_SPECTATORS_PLAYER";
//			params[0].Type = STRING_MANAGER::player;
//			break;
//		case RYZOMID::npc:
//			msgName = "MAGIC_END_SELFCAST_SUCCESS_SPECTATORS_NPC";
//			params[0].Type = STRING_MANAGER::bot;
//			break;
//		case RYZOMID::creature:
//			msgName = "MAGIC_END_SELFCAST_SUCCESS_SPECTATORS_CREATURE";
//			params[0].Type = STRING_MANAGER::creature;
//			break;
//		default:
//			return;
//		};
//		
//		params[0].EId = aggressorId;
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, msgName, params);
//	}
//	else
//	{
//		excluded.push_back(victimId);
//
//		params.resize(2);
//		params[0].Type = STRING_MANAGER::entity;
//		params[0].EId = aggressorId;
//		params[1].Type = STRING_MANAGER::entity;
//		params[1].EId = victimId;
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "MAGIC_END_CAST_SUCCESS_SPECTATORS", params);
//	}
	
} // sendSpellSuccessMessages //


//--------------------------------------------------------------
//					sendSpellFailedMessages()  
//--------------------------------------------------------------
void sendSpellFailedMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId  )
{
	if ( !TheDataset.isAccessible(aggressorRowId) || !TheDataset.isAccessible(victimRowId) )
		return;

	CEntityId victimId = TheDataset.getEntityId(victimRowId);
	CEntityId aggressorId = TheDataset.getEntityId(aggressorRowId);

	const bool self = (aggressorId == victimId);

//	TVectorParamCheck params;
	
	if (aggressorId.getType() == RYZOMID::player)
	{
		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "MAGIC_END_CAST_FAILED_ACTOR");
		// and send a flying text to user only (don't send to target, the agressor just failed to cast!)
		PlayerManager.sendImpulseToClient(aggressorId, std::string("COMBAT:FLYING_TEXT"), aggressorRowId.getCompressedIndex(), (uint8)COMBAT_FLYING_TEXT::SelfFailure);
	}

	if (!self && victimId.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias(aggressorId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "MAGIC_END_CAST_FAILED_TARGET", params);
	}

	
// send message to spectators
//	// Send to 'speech' group
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//
//	if (!self)
//	{
//		excluded.push_back(victimId);
//	}
//	
//	params.resize(1);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = aggressorId;
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "MAGIC_END_CAST_FAILED_SPECTATORS", params);
//	
} // sendSpellFailedMessages //

//--------------------------------------------------------------
//					sendSpellFumbleMessages()  
//--------------------------------------------------------------
void sendSpellFumbleMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId  )
{
	if ( !TheDataset.isAccessible(aggressorRowId) || !TheDataset.isAccessible(victimRowId) )
		return;

	CEntityId victimId = TheDataset.getEntityId(victimRowId);
	CEntityId aggressorId = TheDataset.getEntityId(aggressorRowId);

	const bool self = (aggressorId == victimId);

//	TVectorParamCheck params;
	
	if (aggressorId.getType() == RYZOMID::player)
		sendDynamicSystemMessage(TheDataset.getDataSetRow(aggressorId), "MAGIC_END_CAST_FUMBLE_ACTOR");

	if (!self && victimId.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		params[0].setEIdAIAlias( aggressorId, CAIAliasTranslator::getInstance()->getAIAlias(aggressorId) );
		sendDynamicSystemMessage(TheDataset.getDataSetRow(victimId), "MAGIC_END_CAST_FUMBLE_TARGET", params);
	}

	
// send message to spectators
//	// Send to 'speech' group
//	CEntityId senderId;
//	if ( aggressorId.getType() == RYZOMID::player || aggressorId.getType() == RYZOMID::npc )
//	{
//		senderId = aggressorId;
//	}
//	else if ( victimId.getType() == RYZOMID::player || victimId.getType() == RYZOMID::npc )
//	{
//		senderId = victimId;
//	}
//	else
//		return;
//
//	vector<CEntityId> excluded;
//	excluded.push_back(aggressorId);
//
//	if (!self)
//	{
//		excluded.push_back(victimId);
//	}
//	
//	params.resize(1);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = aggressorId;
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "MAGIC_END_CAST_FUMBLE_SPECTATORS", params);
} // sendSpellFumbleMessages //


//--------------------------------------------------------------
//					sendDynamicSystemMessage()  
//--------------------------------------------------------------
uint32 sendDynamicSystemMessage(const TDataSetRow &playerRowId, const string &msgName, const TVectorParamCheck &params)
{
	if( TheDataset.isAccessible( playerRowId ) )
	{
		if (TheDataset.getEntityId(playerRowId).getType() != RYZOMID::player)
			return 0;
	}
	else
	{
		return 0;
	}

	const uint32 stringId = STRING_MANAGER::sendStringToClient(playerRowId, msgName, params );

	sendDynamicSystemMessage(playerRowId, stringId);

	return stringId;
} // sendDynamicSystemMessage //

//--------------------------------------------------------------
//					sendDynamicSystemMessage()  
//--------------------------------------------------------------
void sendDynamicSystemMessage(const TDataSetRow &playerRowId, uint32 stringId)
{
	const CEntityId eid = TheDataset.getEntityId(playerRowId);
	if (eid.getType() != RYZOMID::player)
		return;

	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( const_cast<CEntityId&> (eid) );
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "STRING:DYN_STRING", bms) )
	{
		nlwarning("<sendDynamicSystemMessage> Msg name CHAT:DYN_STRING not found");
	}
	else
	{
		bms.serial( stringId );
		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(eid.getDynamicId()), msgout );
	}
} // sendDynamicSystemMessage //

//--------------------------------------------------------------
//					sendDynamicGroupSystemMessage()  
//--------------------------------------------------------------
//uint32 sendDynamicGroupSystemMessage(const TDataSetRow &senderId, const vector<CEntityId> &excluded, const string &msgName, const TVectorParamCheck &params)
void sendDynamicGroupSystemMessage(const TDataSetRow &senderId, const vector<CEntityId> &excluded, const string &msgName, const TVectorParamCheck &params)
{
	STRING_MANAGER::sendSystemStringToClientAudience(senderId, excluded, CChatGroup::say, msgName.c_str(), params);
//	sendDynamicSystemMessage(senderId, msgName, params);
/*
	CMessage msg( "GROUP_DYN_STRING" );
	msg.serial( const_cast<TDataSetRow&> (senderId) );
	msg.serialCont( const_cast< vector<CEntityId> &> (excluded) );
	//msg.serial( stringId );
	msg.serial(msgName);
	msg.serialCont( const_cast< TVectorParamCheck &> (params) );

	sendMessageViaMirror ("IOS", msg);
*/
} // sendDynamicGroupSystemMessage //


//--------------------------------------------------------------
//					sendEffectStandardBeginMessages()  
//--------------------------------------------------------------
void sendEffectStandardBeginMessages(const NLMISC::CEntityId &creatorId, const NLMISC::CEntityId &targetId, const std::string &effectName)
{
//	TVectorParamCheck params;

	if (targetId != creatorId)
	{		
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);

		string msgName = "EFFECT_"+effectName+"_BEGIN";

		// effect creator
		if ( creatorId.getType() == RYZOMID::player)
		{
			params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias( targetId ) );

			const string str = NLMISC::toString("%s_CREATOR",msgName.c_str());
			sendDynamicSystemMessage(TheDataset.getDataSetRow(creatorId), str, params);
		}
		// effect target
		if (targetId.getType() == RYZOMID::player)
		{
			params[0].setEIdAIAlias( creatorId, CAIAliasTranslator::getInstance()->getAIAlias( creatorId ) );
			const string str = NLMISC::toString("%s_TARGET",msgName.c_str());
			PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(targetId), str, params);
		}

//		// spectators
//		CEntityId centerId;
//		if ( targetId.getType() == RYZOMID::player || targetId.getType() == RYZOMID::npc)
//			centerId = targetId;
//		else if ( creatorId.getType() == RYZOMID::player || creatorId.getType() == RYZOMID::npc)
//			centerId = creatorId;
//		else
//			return;
//
//		params.resize(2);
//		params[0].Type = STRING_MANAGER::entity;
//		params[0].EId = creatorId;
//		params[1].Type = STRING_MANAGER::entity;
//		params[1].EId = targetId;
//
//		vector<CEntityId> excluded;
//		excluded.push_back(creatorId);
//		excluded.push_back(targetId);
//		const string str = NLMISC::toString("%s_SPECTATORS",msgName.c_str());
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(centerId), excluded, str, params);
	}
	else
	{
		string msgName = "EFFECT_"+effectName+"_SELF_BEGIN";

		if ( creatorId.getType() == RYZOMID::player)
		{
			const string str = NLMISC::toString("%s_CREATOR",msgName.c_str());
			PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(creatorId), str);
		}
		else if( creatorId.getType() != RYZOMID::npc )
		{
			// cannot send spectator messages for creatures
			return;
		}

//		params.resize(1);
//		// send to spectators
//		switch(creatorId.getType())
//		{
//		case RYZOMID::player:
//			params[0].Type = STRING_MANAGER::player;
//			msgName = "EFFECT_"+effectName+"_SELF_BEGIN_SPECTATORS_PLAYER";
//			break;
//		case RYZOMID::npc:
//			params[0].Type = STRING_MANAGER::bot;
//			msgName = "EFFECT_"+effectName+"_SELF_BEGIN_SPECTATORS_NPC";
//			break;
//		default:
//			params[0].Type = STRING_MANAGER::creature;
//			msgName = "EFFECT_"+effectName+"_SELF_BEGIN_SPECTATORS_CREATURE";
//			break;
//		};
//		params[0].EId = creatorId;
//
//		vector<CEntityId> excluded;
//		excluded.push_back(creatorId);
//		sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(creatorId), excluded, msgName, params);
	}
} // sendEffectStandardBeginMessages //



//--------------------------------------------------------------
//					sendEffectStandardEndMessages()  
//--------------------------------------------------------------
void sendEffectStandardEndMessages(const NLMISC::CEntityId &creatorId, const NLMISC::CEntityId &targetId, const std::string &effectName)
{
//	TVectorParamCheck params;	

	string msgName = "EFFECT_"+effectName+"_END";

	// send chat message to target
	if (targetId.getType() == RYZOMID::player)
	{
		const string str = NLMISC::toString("%s_TARGET",msgName.c_str());
		sendDynamicSystemMessage(TheDataset.getDataSetRow(targetId), str);
	}
	// send chat message to creator if != target
	if (creatorId != targetId && creatorId.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		const string str = NLMISC::toString("%s_CREATOR",msgName.c_str());
		sendDynamicSystemMessage(TheDataset.getDataSetRow(creatorId), str, params);
	}
	
//	// spectators
//	CEntityId centerId;
//	if ( targetId.getType() == RYZOMID::player || targetId.getType() == RYZOMID::npc)
//		centerId = targetId;
//	else if ( creatorId.getType() == RYZOMID::player || creatorId.getType() == RYZOMID::npc)
//		centerId = creatorId;
//	else
//		return;
//
//	params.resize(1);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = targetId;
//	vector<CEntityId> excluded;
//	excluded.push_back(creatorId);
//	excluded.push_back(targetId);
//	const string str = NLMISC::toString("%s_SPECTATORS",msgName.c_str());
//	sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(centerId), excluded, str, params);
} // sendEffectStandardEndMessages //


//--------------------------------------------------------------
//					forceActionFailure()  
//						
// return true if the entity is FORCED to fail an action (cast, craft, combat...)
// (due to blindness for exemple)
//--------------------------------------------------------------
bool forceActionFailure(CEntityBase *entity)
{
#ifdef NL_DEBUG
	nlassert(entity);
#endif

	// do not use smart pointers for local use only
	const CSEffect *effect = entity->lookForActiveEffect(EFFECT_FAMILIES::Blind);
	if (effect)
	{
		sint32 randomFailure = RandomGenerator.rand(99);
		if (randomFailure<effect->getParamValue())
		{
			// Fail because of blindness
			return true;
		}
	}

	return false;
}



}; // namespace PHRASE_UTILITIES

