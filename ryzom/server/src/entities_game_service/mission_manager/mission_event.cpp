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
#include "mission_manager/mission_event.h"
#include "zone_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "creature_manager/creature_manager.h"
#include "player_manager/character.h"

#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CMissionEvent);

bool CMissionEvent::simMissionEvent(const std::vector< std::string > & script, CLog & log)
{

	if ( script.size() < 2 )
	{
		log.displayNL( "at least <player> <event type>");
		return false;
	}
	NL_BEGIN_STRING_CONVERSION_TABLE (TMissionEventType)
		NL_STRING_CONVERSION_TABLE_ENTRY(MissionDone)
		NL_STRING_CONVERSION_TABLE_ENTRY(GiveItem)
		NL_STRING_CONVERSION_TABLE_ENTRY(GiveMoney)
		NL_STRING_CONVERSION_TABLE_ENTRY(EnterZone)
		NL_STRING_CONVERSION_TABLE_ENTRY(Cast)
		NL_STRING_CONVERSION_TABLE_ENTRY(Kill)
		NL_STRING_CONVERSION_TABLE_ENTRY(KillPlayer)
		NL_STRING_CONVERSION_TABLE_ENTRY(BuyItem)
		NL_STRING_CONVERSION_TABLE_ENTRY(SellItem)
		NL_STRING_CONVERSION_TABLE_ENTRY(Forage)
		NL_STRING_CONVERSION_TABLE_ENTRY(Talk)
		NL_STRING_CONVERSION_TABLE_ENTRY(SkillProgress)
		NL_STRING_CONVERSION_TABLE_ENTRY(Target)
		NL_STRING_CONVERSION_TABLE_ENTRY(Craft)
		NL_STRING_CONVERSION_TABLE_ENTRY(Escort)
		NL_STRING_CONVERSION_TABLE_ENTRY(AIMsg)
		NL_STRING_CONVERSION_TABLE_ENTRY(LootRm)
		NL_STRING_CONVERSION_TABLE_ENTRY(LootItem)
		NL_STRING_CONVERSION_TABLE_ENTRY(KillGroup)
		NL_STRING_CONVERSION_TABLE_ENTRY(EndDynChat)
		NL_STRING_CONVERSION_TABLE_ENTRY(Debug)
		NL_STRING_CONVERSION_TABLE_ENTRY(ChargePoints)
		NL_STRING_CONVERSION_TABLE_ENTRY(OutpostGain)
		NL_STRING_CONVERSION_TABLE_ENTRY(QueueEntryOk)
		NL_STRING_CONVERSION_TABLE_ENTRY(QueueExit)
	NL_END_STRING_CONVERSION_TABLE(TMissionEventType, CMissionEventConversionType, Unknown)

	CMissionEvent::TMissionEventType type = CMissionEventConversionType.fromString( script[1] );
	if ( type == CMissionEvent::Unknown )
	{
		log.displayNL( "Invalid event %s",script[1].c_str() );
		return false;
	}

	CMissionEvent * event = NULL;
	switch(type) 
	{
	case MissionDone:
		event = new CMissionEventMissionDone;
		break;
	case GiveItem:
		event = new CMissionEventGiveItem;
		break;
	case GiveMoney:
		event = new CMissionEventGiveMoney;
		break;
	case EnterZone:
		event = new CMissionEventVisitPlace;
		break;
	case Cast:
		event = new CMissionEventCast;
		break;
	case Kill:
		event = new CMissionEventKill;
		break;
	case KillPlayer:
		event = new CMissionEventKillPlayer;
		break;
	case BuyItem:
		event = new CMissionEventBuyItem;
		break;
	case SellItem:
		event = new CMissionEventSellItem;
		break;
	case Forage:
		event = new CMissionEventForage;
		break;
	case Talk:
		event = new CMissionEventTalk;
		break;
	case SkillProgress:
		event = new CMissionEventSkillProgress;
		break;
	case Target:
		event = new CMissionEventTarget;
		break;
	case Craft:
		event = new CMissionEventCraft;
		break;
	case Escort:
		event = new CMissionEventEscort;
		break;
	case LootRm:
		event = new CMissionEventLootRm;
		break;
	case LootItem:
		event = new CMissionEventLootItem;
		break;
	case KillGroup:
		event = new CMissionEventKillGroup;
		break;
	case QueueEntryOk:
		event = new CMissionEventQueueEntryOk;
		break;
	case QueueExit:
		event = new CMissionEventQueueExit;
		break;
	case TaggedRingScenario:
		event = new CMissionEventTaggedRingScenarioDone;
		break;
	}
	if ( event == NULL )
	{
		log.displayNL( "event %s is either not implemented, or this command is not up to date");
		return false;
	}
	event->Type = type;

	std::vector< std::string > script2 = script;
	script2.erase( script2.begin() );
	script2.erase( script2.begin() );
	if ( !event->buildFromScript(script2, log) )
	{
		log.displayNL( "Failed to build event %s : reason on the previous log lines",script[1].c_str());
		delete event;
		return false;
	}
	log.displayNL( "build event %s successfully built",script[1].c_str());

	CEntityId id;
	id.fromString( script[0].c_str() );
	CCharacter  * c = PlayerManager.getChar( id );
	if ( !c )
	{
		log.displayNL("invalid player %s. Cant launch event",script[0].c_str() );
		return false;
	}
	c->processMissionEvent(*event);
	return true;
}

//************** individual builders *******************

bool CMissionEventMissionDone::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	if ( script.size() != 1 )
	{
		log.displayNL("<mission>");
		return false;
	}
	Mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( script[0] );
	if ( Mission == CAIAliasTranslator::Invalid )
	{
		log.displayNL("param %s is not a valid mission",script[0].c_str());
		return false;
	}
	return true;
}

bool CMissionEventTaggedRingScenarioDone::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	if ( script.size() != 1 )
	{
		log.displayNL("<scenario_tag>");
		return false;
	}
	ScenarioTag = script[0];
	return true;
}

bool CMissionEventGiveItem::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	log.displayNL("Not the best way to test this event : bot chat needed");
	return true;
}
bool CMissionEventGiveMoney::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	if ( script.size() != 1 )
	{
		log.displayNL("<money>");
		return false;
	}
	NLMISC::fromString(script[0], Amount);
	log.displayNL("Not the best way to test this event : bot chat needed");
	return true;
}

bool CMissionEventVisitPlace::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	if ( script.size() != 1 )
	{
		log.displayNL("<zone name>");
		return false;
	}
	CPlace * place = CZoneManager::getInstance().getPlaceFromName( script[0] );
	if( place == NULL )
	{
		log.displayNL("Invalid place %s",script[0].c_str());
		return false;
	}
	PlaceId = place->getId();
	return true;
}
bool CMissionEventCast::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	if ( script.empty() )
	{
		log.displayNL("<brick>*[<brick>]");
		return false;
	}
	bool ret = true;
	for ( uint i = 0; i< script.size(); i++ )
	{
		CSheetId sheetId( script[i] + ".sbrick" );
		if ( sheetId == CSheetId::Unknown )
		{
			ret = false;
			log.displayNL("Invalid brick %s",script[i].c_str() );
		}
		Bricks.push_back( sheetId );
	}
	return ret;
}
bool CMissionEventKill::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	if ( script.size() != 1 )
	{
		log.displayNL("param entity expected");
		return false;
	}
	CEntityId id;
	id.fromString( script[0].c_str() );
	CCreature * c = CreatureManager.getCreature( id );
	if ( !c )
	{
		log.displayNL("invalid entity %s",script[0].c_str() );
		return false;
	}
	TargetEntity = c->getEntityRowId();
	return true;
}
bool CMissionEventKillPlayer::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	if ( script.size() != 1 )
	{
		log.displayNL("param entity expected");
		return false;
	}
	CEntityId id;
	id.fromString( script[0].c_str() );
	CCharacter *victim = PlayerManager.getChar( id );
	if ( !victim )
	{
		log.displayNL("invalid victim entity %s",script[0].c_str() );
		return false;
	}
	TargetEntity = victim->getEntityRowId();
	return true;
}
bool CMissionEventBuyItem::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	bool ret = true;
	if ( script.size() != 3 && script.size() != 4)
	{
		log.displayNL("<sheet><quantity><quality>[<bot>]");
		return false;
	}

	Sheet = CSheetId ( script[0] + ".sitem" );
	if ( Sheet == CSheetId::Unknown )
	{
		ret = false;
		log.displayNL("Invalid sheet %s",script[0].c_str() );
	}

	NLMISC::fromString(script[1], Quantity);
	NLMISC::fromString(script[2], Quality);
	if (script.size() == 4)
	{
		CEntityId id;
		id.fromString( script[3].c_str() );
		CCreature * c = CreatureManager.getCreature( id );
		if ( !c )
		{
			log.displayNL("invalid entity %s",script[3].c_str() );
			return false;
		}
		TargetEntity = c->getEntityRowId();		
	}
	return ret;
}
bool CMissionEventSellItem::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	bool ret = true;
	if ( script.size() != 3 && script.size() != 4)
	{
		log.displayNL("<sheet><quantity><quality>[<bot>]");
		return false;
	}
	
	Sheet = CSheetId ( script[0] + ".sitem" );
	if ( Sheet == CSheetId::Unknown )
	{
		ret = false;
		log.displayNL("Invalid sheet %s",script[0].c_str() );
	}
	
	NLMISC::fromString(script[1], Quantity);
	NLMISC::fromString(script[2], Quality);
	if (script.size() == 4)
	{
		CEntityId id;
		id.fromString( script[3].c_str() );
		CCreature * c = CreatureManager.getCreature( id );
		if ( !c )
		{
			log.displayNL("invalid entity %s",script[3].c_str() );
			return false;
		}
		TargetEntity = c->getEntityRowId();		
	}
	return ret;
}

bool CMissionEventForage::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	bool ret = true;
	if ( script.size() != 3 )
	{
		log.displayNL("<sheet><quantity><quality>");
		return false;
	}
	
	Sheet = CSheetId ( script[0] + ".sitem" );
	if ( Sheet == CSheetId::Unknown )
	{
		ret = false;
		log.displayNL("Invalid sheet %s",script[0].c_str() );
	}
	
	NLMISC::fromString(script[1], Quantity);
	NLMISC::fromString(script[2], Quality);
	return ret;
}

bool CMissionEventTalk::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	if ( script.size()!=0 )
	{
		log.displayNL("no param expected expected");
		return false;
	}
	log.displayNL("Not a good way to test this event : bot chat needed");
	return true;
}


bool CMissionEventSkillProgress::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	bool ret = true;
	if ( script.size() != 2)
	{
		log.displayNL("<skill><level>");
		return false;
	}
	
	Skill = SKILLS::toSkill( script[0] );
	if ( Skill == SKILLS::unknown )
	{
		ret = false;
		log.displayNL("Invalid sheet %s",script[0].c_str() );
	}
	NLMISC::fromString(script[1], Level);
	return ret;
}

bool CMissionEventTarget::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	if ( script.size() != 2)
	{
		log.displayNL("<bot>");
		return false;
	}
	CEntityId id;
	id.fromString( script[0].c_str() );
	CCreature * c = CreatureManager.getCreature( id );
	if ( !c )
	{
		log.displayNL("invalid entity %s",script[0].c_str() );
		return false;
	}
	TargetEntity = c->getEntityRowId();	
	return true;
}

bool CMissionEventCraft::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	bool ret = true;
	if ( script.size() != 3 )
	{
		log.displayNL("<sheet><quantity><quality>");
		return false;
	}
	
	Sheet = CSheetId ( script[0] + ".sitem" );
	if ( Sheet == CSheetId::Unknown )
	{
		ret = false;
		log.displayNL("Invalid sheet %s",script[0].c_str() );
	}
	
	NLMISC::fromString(script[1], Quantity);
	NLMISC::fromString(script[2], Quality);
	return ret;
}

bool CMissionEventEscort::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	bool ret = true;
	if ( script.size() != 1 )
	{
		log.displayNL("no params");
		return false;
	}
	Mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName(script[0]);
	return ret;
}

bool CMissionEventAIMsg::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	bool ret = true;
	if ( script.size() != 2 )
	{
		log.displayNL("<msg_name>");
		return false;
	}
	Msg = script[1];
	return ret;
}

bool CMissionEventLootItem::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	bool ret = true;
	if ( script.size() != 3 )
	{
		log.displayNL("<sheet><quantity><quality>");
		return false;
	}
	
	Sheet = CSheetId ( script[0] + ".sitem" );
	if ( Sheet == CSheetId::Unknown )
	{
		ret = false;
		log.displayNL("Invalid sheet %s",script[0].c_str() );
	}
	
	NLMISC::fromString(script[1], Quantity);
	NLMISC::fromString(script[2], Quality);
	return ret;
}

bool CMissionEventLootRm::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	bool ret = true;
	if ( script.size() != 3 )
	{
		log.displayNL("<sheet><quantity><quality>");
		return false;
	}
	
	Sheet = CSheetId ( script[0] + ".sitem" );
	if ( Sheet == CSheetId::Unknown )
	{
		ret = false;
		log.displayNL("Invalid sheet %s",script[0].c_str() );
	}
	
	NLMISC::fromString(script[1], Quantity);
	NLMISC::fromString(script[2], Quality);
	return ret;
}

bool CMissionEventKillGroup::buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log)
{
	bool ret = true;
	if ( script.size() != 1 )
	{
		log.displayNL("<alias>");
		return false;
	}

	NLMISC::fromString(script[0], Alias);
	return ret;
}

