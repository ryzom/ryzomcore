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
#include "nel/misc/algo.h"
#include "mission_step_template.h"
#include "mission_manager/mission_template.h"
#include "mission_log.h"
#include "mission_manager/mission_parser.h"
#include "mission_manager/ai_alias_translator.h"
#include "egs_mirror.h"
#include "primitives_parser.h"
#include "egs_sheets/egs_sheets.h"


using namespace std;
using namespace NLMISC;

/***************************************************************************************************
Steps linked with items
	-forage
	-craft
	-buy
	-sell
***************************************************************************************************/

bool	IMissionStepItem::buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
{	
	_SourceLine = line;
	bool ret = true;
	if ( script.size() < 2 )
	{
		MISLOGSYNTAXERROR("<item> [<quantity>][<quality>] *[; <item> [<quantity>][<quality>]]");
		return false;
	}
	else
	{
		std::vector< std::string > subs;
		NLMISC::splitString( script[1],";", subs );
		bool qualityFound = false;
		bool qualityNotFound = false;
		for ( uint i = 0; i < subs.size(); i++ )
		{
			std::vector< std::string > args;
			CMissionParser::tokenizeString( subs[i]," \t", args );
			if ( args.size() < 1 || args.size() > 3 )
			{
				ret = false;
				MISLOGSYNTAXERROR("<item> [<quantity>][<quality>] *[; <item> [<quantity>][<quality>]]");
				return false;
			}
			CSubStep subStep;
			subStep.Sheet = CSheetId( args[0] + ".sitem" );
			missionData.ChatParams.push_back( make_pair( args[0], STRING_MANAGER::item ) );
			if ( args.size() > 1 )
				NLMISC::fromString(args[1], subStep.Quantity);
			else
				subStep.Quantity = 1;

			if ( args.size() == 3 )
			{
				NLMISC::fromString(args[2], subStep.Quality);
				qualityFound = true;
			}
			else
			{
				subStep.Quality = 0;
				qualityNotFound = true;
			}
			if ( subStep.Sheet == CSheetId::Unknown )
			{
				ret = false;
				MISLOGERROR1("invalid sheet %s", args[0].c_str());
			}
			else
			{
				_SubSteps.push_back( subStep );
			}
		}
		if ( qualityFound && qualityNotFound )
		{
			ret = false;
			MISLOGERROR("step mixing steps mixing quality and quantity");
		}
		return ret;
	}
}
	
void IMissionStepItem::getInitState( std::vector<uint32>& ret )
{
	ret.clear();
	ret.resize( _SubSteps.size() );
	for ( uint i = 0; i < _SubSteps.size(); i++ )
	{
		ret[i] = _SubSteps[i].Quantity;
	}
}
	
inline void IMissionStepItem::getTextParams(uint & nbSubSteps, TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
{
	nlassert( _SubSteps.size() == subStepStates.size() );
	for ( uint i  = 0; i < subStepStates.size();i++ )
	{
		if ( subStepStates[i] != 0 )
		{
			nbSubSteps++;
			retParams.push_back(STRING_MANAGER::TParam());
			retParams.back().Type = STRING_MANAGER::item;
			retParams.back().SheetId = _SubSteps[i].Sheet;
		
			retParams.push_back(STRING_MANAGER::TParam());
			retParams.back().Type = STRING_MANAGER::integer;
			retParams.back().Int = subStepStates[i];
		
			if ( _SubSteps[i].Quality )
			{
				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::integer;
				retParams.back().Int = _SubSteps[i].Quality;
			}
		}
	}
}
	

// ----------------------------------------------------------------------------
class CMissionStepForage : public IMissionStepItem
{

	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::Forage )
		{
			CMissionEventForage & eventSpe = (CMissionEventForage&)event;
			if ( eventSpe.Sheet == _SubSteps[subStepIndex].Sheet && eventSpe.Quality >= _SubSteps[subStepIndex].Quality )
			{
				LOGMISSIONSTEPSUCCESS("forage");
				return eventSpe.Quantity;
			}
		}
		return 0;
	}

	virtual void getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		IMissionStepItem::getTextParams(nbSubSteps, retParams, subStepStates);
		static const std::string stepText = "MIS_FORAGE_";
		textPtr = &stepText;
	}

	MISSION_STEP_GETNEWPTR(CMissionStepForage)
};
MISSION_REGISTER_STEP(CMissionStepForage,"forage")


// ----------------------------------------------------------------------------
class CMissionStepLootItem : public IMissionStepItem
{
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::LootItem )
		{
			CMissionEventLootItem & eventSpe = (CMissionEventLootItem&)event;
			if ( eventSpe.Sheet == _SubSteps[subStepIndex].Sheet && eventSpe.Quality >= _SubSteps[subStepIndex].Quality )
			{
				LOGMISSIONSTEPSUCCESS("loot_item");
				return eventSpe.Quantity;
			}
		}
		return 0;
	}
	
	virtual void getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		IMissionStepItem::getTextParams(nbSubSteps, retParams, subStepStates);
		static const std::string stepText = "MIS_LOOT_ITEM_";
		textPtr = &stepText;
	}

	MISSION_STEP_GETNEWPTR(CMissionStepLootItem)
};
MISSION_REGISTER_STEP(CMissionStepLootItem,"loot_item")


// ----------------------------------------------------------------------------
uint CMissionStepLootRm::processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
{
	if ( event.Type == CMissionEvent::LootRm )
	{
		CMissionEventLootRm & eventSpe = (CMissionEventLootRm&)event;
		if ( eventSpe.Sheet == _SubSteps[subStepIndex].Sheet && eventSpe.Quality >= _SubSteps[subStepIndex].Quality )
		{
			LOGMISSIONSTEPSUCCESS("loot_mp");
			return eventSpe.Quantity;
		}
	}
	return 0;
}

void CMissionStepLootRm::getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
{
	IMissionStepItem::getTextParams(nbSubSteps, retParams, subStepStates);
	static const std::string stepText = "MIS_LOOT_MP_";
	textPtr = &stepText;
}
MISSION_REGISTER_STEP(CMissionStepLootRm,"loot_mp")


// ----------------------------------------------------------------------------
class CMissionStepCraft : public IMissionStepItem
{
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::Craft )
		{
			CMissionEventCraft & eventSpe = (CMissionEventCraft&)event;
			if ( eventSpe.Sheet == _SubSteps[subStepIndex].Sheet && 
				eventSpe.Quality >= _SubSteps[subStepIndex].Quality )
			{
				LOGMISSIONSTEPSUCCESS("craft");
				return eventSpe.Quantity;
			}
		}
		return 0;
	}
	
	virtual void getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		IMissionStepItem::getTextParams(nbSubSteps, retParams, subStepStates);
		static const std::string stepText = "MIS_CRAFT_";
		textPtr = &stepText;
	}

	MISSION_STEP_GETNEWPTR(CMissionStepCraft)
};
MISSION_REGISTER_STEP(CMissionStepCraft,"craft")


// ----------------------------------------------------------------------------
class CMissionStepBuyItem : public IMissionStepItem
{
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{	
		_SourceLine = line;
		_HasBot = false;
		_Bot = CAIAliasTranslator::Invalid;
		if ( !IMissionStepItem::buildStep(line, script,globalData,missionData) )
			return false;
		if ( script.size() == 3 )
		{
			_HasBot = true;;
			if ( !CMissionParser::parseBotName(script[2],_Bot,missionData) )
				return false;
		}
		return true;
	}
	
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::BuyItem )
		{
			CMissionEventBuyItem & eventSpe = (CMissionEventBuyItem&)event;
			if ( eventSpe.Sheet == _SubSteps[subStepIndex].Sheet && eventSpe.Quality >= _SubSteps[subStepIndex].Quality )
			{
				if ( _HasBot )
				{
					if ( _Bot != CAIAliasTranslator::Invalid )
					{
						const CEntityId & eId = CAIAliasTranslator::getInstance()->getEntityId( _Bot );
						if ( eId == CEntityId::Unknown )
						{
							LOGMISSIONSTEPERROR("buy : invalid npc alias '" + CPrimitivesParser::aliasToString(_Bot) + "'");
							return 0;
						}
						if ( getEntityIdFromRow(eventSpe.TargetEntity) != eId )
							return 0;
					}
					else
					{
						if ( eventSpe.TargetEntity != giverRow )
							return 0;
					}
				}
				LOGMISSIONSTEPSUCCESS("buy");
				return eventSpe.Quantity;
			}
		}
		return 0;
	}
	
	virtual void getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		IMissionStepItem::getTextParams(nbSubSteps, retParams, subStepStates);
		static const std::string stepText = "MIS_BUY_";
		static const std::string stepTextNpc = "MIS_BUY_NPC_";
		if ( _HasBot )
		{
			STRING_MANAGER::TParam param;
			param.Type = STRING_MANAGER::bot;

			if ( _Bot != CAIAliasTranslator::Invalid )
				param.Int = _Bot;
			else
				param.Identifier = "giver";
			retParams.push_back(param);
			textPtr = &stepTextNpc;
		}
		else
			textPtr = &stepText;
	}
	
	virtual TAIAlias getInvolvedBot(bool& invalidIsGiver) const { invalidIsGiver=true; return _Bot; }

	bool _HasBot;
	TAIAlias _Bot;

	MISSION_STEP_GETNEWPTR(CMissionStepBuyItem)
};
MISSION_REGISTER_STEP(CMissionStepBuyItem,"buy")


// ----------------------------------------------------------------------------
class CMissionStepSellItem : public IMissionStepItem
{
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{	
		_SourceLine = line;
		_HasBot = false;
		_Bot = CAIAliasTranslator::Invalid;
		if ( !IMissionStepItem::buildStep(line, script,globalData,missionData) )
			return false;
		if ( script.size() == 3 )
		{
			_HasBot = true;;
			if ( !CMissionParser::parseBotName(script[2],_Bot,missionData) )
				return false;
		}
		return true;
	}
	
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::SellItem )
		{
			CMissionEventSellItem & eventSpe = (CMissionEventSellItem&)event;
			if ( eventSpe.Sheet == _SubSteps[subStepIndex].Sheet && eventSpe.Quality >= _SubSteps[subStepIndex].Quality )
			{
				if ( _HasBot )
				{
					if ( _Bot != CAIAliasTranslator::Invalid )
					{
						const CEntityId & eId = CAIAliasTranslator::getInstance()->getEntityId( _Bot );
						if ( eId == CEntityId::Unknown )
						{
							LOGMISSIONSTEPERROR("sell : invalid bot alias '" + CPrimitivesParser::aliasToString(_Bot) + "'");
							return 0;
						}
						if ( getEntityIdFromRow(eventSpe.TargetEntity) != eId )
							return 0;
					}
					else
					{
						if ( eventSpe.TargetEntity != giverRow )
							return 0;
					}
				}
				LOGMISSIONSTEPSUCCESS("sell");				
				return eventSpe.Quantity;
			}
		}
		return 0;
	}
	
	virtual void getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		IMissionStepItem::getTextParams(nbSubSteps,retParams, subStepStates);
		static const std::string stepText = "MIS_SELL_";
		static const std::string stepTextNpc = "MIS_SELL_NPC_";
		if ( _HasBot )
		{
			STRING_MANAGER::TParam param;
			param.Type = STRING_MANAGER::bot;

			if ( _Bot != CAIAliasTranslator::Invalid )
				param.Int = _Bot;
			else
				param.Identifier = "giver";
			
			retParams.push_back(param);
			textPtr = &stepTextNpc;
		}
		else
			textPtr = &stepText;
	}
	
	virtual TAIAlias getInvolvedBot(bool& invalidIsGiver) const { invalidIsGiver=true; return _Bot; }

	bool _HasBot;
	TAIAlias _Bot;

	MISSION_STEP_GETNEWPTR(CMissionStepSellItem)
};
MISSION_REGISTER_STEP(CMissionStepSellItem,"sell")
