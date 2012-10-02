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
#include "mission_step_template.h"
#include "mission_log.h"
#include "mission_manager/mission_parser.h"
#include "mission_manager/ai_alias_translator.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "creature_manager/creature_manager.h"
#include "mission_manager/mission_manager.h"
#include "game_item_manager/player_inv_xchg.h"
#include "egs_pd.h"

#include "nel/misc/algo.h"

using namespace std;
using namespace NLMISC;
using namespace EGSPD;



/***************************************************************************************************
Steps linked with bot chat
	-talk_to
	-give_money
	-give_item
	-dyn_chat
***************************************************************************************************/

// ----------------------------------------------------------------------------
class CMissionStepTalk : public IMissionStepTemplate
{
	bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		if ( script.size() < 2 )
		{
			MISLOGSYNTAXERROR("<npc_name> [: <phrase_id>*[;<param>]]");
			return false;
		}
		bool ret = true;
		//// Dynamic Mission Args : #dynamic#
		if (trim(script[1]) == "#dynamic#")
		{
			_Dynamic = missionData.Name;
			_PhraseId = _Dynamic+"_ACTION";
			_IsDynamic = true;
		}
		else
		{
			_IsDynamic = false;
			// parse bot
			if ( !CMissionParser::parseBotName(script[1], _Bot, missionData) )
			{
				MISLOGERROR1("invalid npc '%s'", script[1].c_str());
				return false;
			}

			// parse phrase and params
			if (script.size() > 2)
			{
				// parse a specific phrase
				if (!CMissionParser::parseParamText(line, script[2], _PhraseId, _Params ))
				{
					MISLOGERROR1("invalid text '%s'", script[2].c_str());
					return false;
				}
			}
			else
			{
				// use the default phrase
				_PhraseId = "MIS_TALK_TO_MENU";
			}

			// add a first default param (the name of the bot we talk to)
			_Params.insert(_Params.begin(), STRING_MANAGER::TParam());
			_Params[0].Identifier = CMissionParser::getNoBlankString(script[1]);
		}
		return true;
	}

	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		string webAppUrl;

		_User = PlayerManager.getChar(getEntityIdFromRow(userRow));

		if (_IsDynamic && _User != NULL)
		{
			vector<string> params = _User->getCustomMissionParams(_Dynamic);
			if (params.size() < 2)
			{
				LOGMISSIONSTEPERROR("talk_to : invalid npc name");
				return 0;
			}
			else
			{
				webAppUrl = params[0];
			}
		}
		
		// not check here : they are done befor. If a talk event comes here, the step is complete
		if( event.Type == CMissionEvent::Talk )		
		{
			if (!webAppUrl.empty() && _User != NULL)
				_User->validateDynamicMissionStep(webAppUrl);
			LOGMISSIONSTEPSUCCESS("talk_to");
			return 1;
		}
		return 0;
	}

	void getInitState( std::vector<uint32>& ret )
	{
		ret.clear();
		ret.resize( 1 );
		ret[0] = 1;
	}

	bool getDynamicBot(TAIAlias & aliasRet)
	{
		if (_User != NULL)
		{
			vector<string> params = _User->getCustomMissionParams(_Dynamic);
			if (params.size() < 2)
			{
				MISLOG("sline:%u ERROR : talk_to (sendContextText) : invalid bot", _SourceLine);
				return false;
			}
			else
			{
				vector<TAIAlias> aliases;
				CAIAliasTranslator::getInstance()->getNPCAliasesFromName(params[1], aliases);
				if ( aliases.empty() )
				{
					MISLOG("sline:%u ERROR : talk_to (sendContextText) : invalid bot", _SourceLine);
					return false;
				}

				aliasRet = aliases[0];
				return true;
			}
		}
		return false;
	}

	virtual uint32 sendContextText(const TDataSetRow& user, const TDataSetRow& interlocutor, CMission * instance, bool & gift, const NLMISC::CEntityId & giver )
	{

		if (_IsDynamic)
		{
			if (!getDynamicBot(_Bot) || _User == NULL)
			{
				MISLOG("sline:%u ERROR : talk_to (sendContextText) : invalid bot", _SourceLine);
				return 0;
			}

			TVectorParamCheck params;
			return STRING_MANAGER::sendStringToClient( user, _PhraseId, params );
		}

		CCreature * bot = CreatureManager.getCreature( interlocutor );

		if ( bot )
		{
			if ( ( _Bot != CAIAliasTranslator::Invalid && _Bot == bot->getAlias() ) ||
				 ( _Bot == CAIAliasTranslator::Invalid && bot->getAlias() == instance->getGiver() ) )
			{
				TVectorParamCheck params = _Params;
				CMissionParser::solveEntitiesNames(params, user, giver);
				return STRING_MANAGER::sendStringToClient( user, _PhraseId, params );	
			}
		}
		else
		{
			MISLOG("sline:%u ERROR : talk_to (sendContextText) : invalid bot %u", _SourceLine, interlocutor.getIndex());
		}
		return 0;
	}
	
	virtual bool hasBotChatOption(const TDataSetRow & interlocutor, CMission * instance, bool & gift)
	{
		if (_IsDynamic && !getDynamicBot(_Bot))
		{
			MISLOG("sline:%u ERROR : talk_to (sendContextText) : invalid bot", _SourceLine);
			return 0;	
		}

		CCreature * bot = CreatureManager.getCreature( interlocutor );
		if ( bot )
		{
			if ( ( _Bot != CAIAliasTranslator::Invalid && _Bot == bot->getAlias() ) ||
				( _Bot == CAIAliasTranslator::Invalid && bot->getAlias() == instance->getGiver() ) )
				return true;	
		}
		else
		{
			MISLOG("sline:%u ERROR : talk_to (hasBotChatOption) : invalid bot %u", _SourceLine, interlocutor.getIndex());
		}
		return false;
	}

	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		if (_IsDynamic && !getDynamicBot(_Bot))
		{
			MISLOG("sline:%u ERROR : talk_to (sendContextText) : invalid bot", _SourceLine);
			static const std::string stepText = "DEBUG_CRASH_P_SMG_CRASH2";
			textPtr = &stepText;
			return;	
		}

		if (_IsDynamic &&  _User != NULL)
		{

			vector<string> params = _User->getCustomMissionParams(_Dynamic);
			if (params.size() < 2)
			{
				MISLOG("sline:%u ERROR : talk_to (sendContextText) : invalid bot", _SourceLine);
				return;
			}
			_Params.insert(_Params.begin(), STRING_MANAGER::TParam());
			_Params[0].Identifier = params[1];
		}

		nbSubSteps = 1;
		static const std::string stepText = "MIS_TALK_TO";
		textPtr = &stepText;
		
		retParams.resize( 1 );
		retParams[0].Type = STRING_MANAGER::bot;
		if ( _Bot != CAIAliasTranslator::Invalid )
			retParams[0].Int = _Bot;
		else
			retParams[0].Identifier = "giver";		
	}

	bool solveTextsParams( CMissionSpecificParsingData & missionData,CMissionTemplate * templ  )
	{
		if (!_IsDynamic)
		{
			bool ret = IMissionStepTemplate::solveTextsParams(missionData,templ);
			if ( !CMissionParser::solveTextsParams(_SourceLine, _Params,missionData ) )
				ret = false;
			return ret;
		}
		return true;
	}

	virtual TAIAlias getInvolvedBot(bool& invalidIsGiver) const { invalidIsGiver=true; return _Bot; }

	bool _IsDynamic;
	std::string	_Dynamic;
	std::string _PhraseId;
	TVectorParamCheck _Params;
	TAIAlias	_Bot;

	MISSION_STEP_GETNEWPTR(CMissionStepTalk)
};
MISSION_REGISTER_STEP(CMissionStepTalk,"talk_to")


// ----------------------------------------------------------------------------
class CMissionGiveMoney : public IMissionStepTemplate
{
	uint		_Amount;
	TAIAlias	_Bot;
		
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		if ( script.size() != 3 )
		{
			MISLOGSYNTAXERROR("<amount><npc_name>");
			return false;
		}
		NLMISC::fromString(script[1], _Amount);
		
		if ( !CMissionParser::parseBotName(script[2],_Bot,missionData) )
		{
			MISLOGERROR1("invalid npc '%s'", script[2].c_str());
			return false;
		}
		return true;	
	}
	
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		// not check here : they are done before. If a give event comes here, the step advances
		if( event.Type == CMissionEvent::GiveMoney )
		{
			LOGMISSIONSTEPSUCCESS("give_money");
			return ((CMissionEventGiveMoney&)event).Amount;
		}
		return 0;
	}
	
	void getInitState( std::vector<uint32>& ret )
	{
		ret.resize( 1,_Amount );
	}
	
	virtual uint32 sendContextText(const TDataSetRow& user, const TDataSetRow& interlocutor, CMission * instance, bool & gift, const NLMISC::CEntityId & giver )
	{
		CCreature * bot = CreatureManager.getCreature( interlocutor );
		if ( bot )
		{
			if ( ( _Bot != CAIAliasTranslator::Invalid && _Bot == bot->getAlias() ) ||
				( _Bot == CAIAliasTranslator::Invalid && bot->getAlias() == instance->getGiver() ) )
			{
				gift = true;
				return STRING_MANAGER::sendStringToClient( user, "MIS_GIVE_MONEY_MENU" , TVectorParamCheck() );
			}
		}
		else
		{
			MISLOG("sline:%u ERROR : give_money (sendContextText) : invalid bot %u", _SourceLine, interlocutor.getIndex());
		}
		return 0;
	}
	
	virtual bool hasBotChatOption(const TDataSetRow & interlocutor, CMission * instance, bool & gift)
	{
		CCreature * bot = CreatureManager.getCreature( interlocutor );
		if ( ( _Bot != CAIAliasTranslator::Invalid && _Bot == bot->getAlias() ) ||
			( _Bot == CAIAliasTranslator::Invalid && bot->getAlias() == instance->getGiver() ) )
		{
			gift = true;
			return true;
		}
		else
		{
			MISLOG("sline:%u ERROR : give_money (hasBotChatOption) : invalid bot %u", _SourceLine, interlocutor.getIndex());
		}
		return false;
	}
	
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		nlassert( subStepStates.size() == 1);
		static const std::string stepText = "MIS_GIVE_MONEY";
		textPtr = &stepText;
			
		if ( subStepStates[0] != 0 )
		{
			nbSubSteps++;
			retParams.push_back(STRING_MANAGER::TParam());
			retParams.back().Type = STRING_MANAGER::integer;
			retParams.back().Int = subStepStates[0];
		}

		retParams.push_back(STRING_MANAGER::TParam());
		retParams.back().Type = STRING_MANAGER::bot;
		if ( _Bot != CAIAliasTranslator::Invalid )
			retParams.back().Int = _Bot;
		else
			retParams.back().Identifier = "giver";
	}

	virtual TAIAlias getInvolvedBot(bool& invalidIsGiver) const { invalidIsGiver=true; return _Bot; }

	bool checkPlayerGift( CMission * instance, CCharacter * user )
	{
		// check that player gives money and no item
		if (user->getExchangeMoney() == 0)
			return false;
		// clamp give money to amount needed
		user->setExchangeMoney( (uint32)min( (uint32)_Amount, (uint32)user->getExchangeMoney() ) );
		nlassert(user->getExchangeView() != NULL);
		return user->getExchangeView()->isEmpty();
	}

	MISSION_STEP_GETNEWPTR(CMissionGiveMoney)
};
MISSION_REGISTER_STEP(CMissionGiveMoney,"give_money")


// ----------------------------------------------------------------------------
class CMissionStepGiveItem : public IMissionStepTemplate
{
	
//	struct CSubStep
//	{
//		CSheetId	Sheet;
//		uint16		Quality;
//		uint32		Quantity;
//	};
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		bool ret = true;
		if ( script.size() != 3 && script.size() != 4)
		{
			MISLOGSYNTAXERROR("<item> <quantity> [<quality>]*[; <item>  <quantity> [<quality>]] : <npc_name> [:<context_phrase_id>]*[;<params>]");
			return false;
		}
		
		bool noQuality = false;
		bool Quality = false;
		std::vector<std::string> subs;
		splitString(script[1],";",subs);
		for ( uint i = 0;i < subs.size(); i++)
		{
			std::vector<std::string> args;
			CMissionParser::tokenizeString(subs[i]," \t",args);
			if ( args.size() != 2 && args.size() != 3 )
			{
				MISLOGSYNTAXERROR("<item> <quantity> [<quality>]*[; <item>  <quantity> [<quality>]] : <npc_name> [:<context_phrase_id>]*[;<params>]");
				return false;
			}
			
			CSubStep subStep;
			subStep.Sheet = CSheetId( CMissionParser::getNoBlankString(args[0]) + ".sitem" );
			missionData.ChatParams.push_back( make_pair( args[0], STRING_MANAGER::item ) );
			NLMISC::fromString(args[1], subStep.Quantity);
			if ( subStep.Sheet == CSheetId::Unknown )
			{
				ret = false;
				MISLOGERROR1("invalid sheet '%s'", ( CMissionParser::getNoBlankString(args[0]) + ".sitem" ).c_str());
			}
			else
			{
				if( args.size() == 3 )
				{
					NLMISC::fromString(args[2], subStep.Quality);
					Quality = true;
				}
				else
				{
					noQuality = true;
					subStep.Quality = 0;
				}
				_SubSteps.push_back( subStep );
			}
		}
		// check coherence of all parameters
		if  ( Quality &&  noQuality )
		{
			MISLOGERROR("all items must have a quality OR they must all have NO quality");
			ret = false;
		}
		// sort by quality
		for ( uint i = 0; i < _SubSteps.size(); i++ )
		{
			for ( uint j = i+1; j < _SubSteps.size(); j++ )
			{
				if ( _SubSteps[i].Quality < _SubSteps[j].Quality )
				{
					CSubStep buf = _SubSteps[i];
					_SubSteps[i] = _SubSteps[j];
					_SubSteps[j] = buf;
				}
			}
		}
		
		if ( !CMissionParser::parseBotName(script[2],_Bot,missionData) )
		{
			MISLOGERROR1("invalid npc '%s'", script[2].c_str());
			return false;
		}
		
		if ( script.size() == 4 )
		{
			if ( !CMissionParser::parseParamText(line, script[3], _PhraseId, _Params ) )
			{
				MISLOGERROR1("invalid text '%s'", script[3].c_str());
				ret = false;
			}
		}
		
		return ret;
		
	}
	bool solveTextsParams( CMissionSpecificParsingData & missionData,CMissionTemplate * templ  )
	{
		bool ret = IMissionStepTemplate::solveTextsParams(missionData,templ);
		if ( !CMissionParser::solveTextsParams(_SourceLine, _Params,missionData ) )
			ret = false;
		return ret;
	}
	
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if( event.Type == CMissionEvent::GiveItem )
		{
			const CMissionEventGiveItem& eventSpe = (const CMissionEventGiveItem&)event;
			if ( subStepIndex == eventSpe.StepIndex )
			{
				LOGMISSIONSTEPSUCCESS("give_item");
				return eventSpe.Quantity;
			}
		}
		return 0;
	}

	
	void getInitState( std::vector<uint32>& ret )
	{
		ret.clear();
		ret.resize( _SubSteps.size() );
		for ( uint i = 0; i < _SubSteps.size(); i++ )
		{
			ret[i] = _SubSteps[i].Quantity;
		}
	}
	
	virtual uint32 sendContextText(const TDataSetRow& user, const TDataSetRow& interlocutor, CMission * instance, bool & gift, const NLMISC::CEntityId & giver )
	{
		CCreature * bot = CreatureManager.getCreature( interlocutor );
		if ( bot )
		{
			if ( ( _Bot != CAIAliasTranslator::Invalid && _Bot == bot->getAlias() ) ||
				( _Bot == CAIAliasTranslator::Invalid && bot->getAlias() == instance->getGiver() ) )
			{
				gift = true;
				TVectorParamCheck vect( 1 + _Params.size() );
				vect[0].Type = STRING_MANAGER::integer;
				vect[0].Int = (sint32)_SubSteps.size();
				
				if ( !_PhraseId.empty() )
				{
					std::copy( _Params.begin(),_Params.end(),vect.begin() + 1 );
					CMissionParser::solveEntitiesNames(vect,user,giver);
					return STRING_MANAGER::sendStringToClient( user, _PhraseId , vect );
				}
				else
				{
					CMissionParser::solveEntitiesNames(vect,user,giver);
					return STRING_MANAGER::sendStringToClient( user, "MIS_GIVE_ITEM_MENU" , vect );
				}
			}
		}
		else
		{
			MISLOG("sline:%u ERROR : give_item (sendContextText) : invalid bot %u", _SourceLine, interlocutor.getIndex());
		}
		return 0;
	}
	
	virtual bool hasBotChatOption(const TDataSetRow & interlocutor, CMission * instance, bool & gift)
	{
		CCreature * bot = CreatureManager.getCreature( interlocutor );
		if ( ( _Bot != CAIAliasTranslator::Invalid && _Bot == bot->getAlias() ) ||
			( _Bot == CAIAliasTranslator::Invalid && bot->getAlias() == instance->getGiver() ) )
		{
			gift = true;
			return true;
		}
		else
		{
			MISLOG("sline:%u ERROR : give_item (hasBotChatOption) : invalid bot %u", _SourceLine, interlocutor.getIndex());
		}
		return false;
	}
	
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		nlassert( _SubSteps.size() == subStepStates.size() );
		static const std::string stepText = "MIS_GIVE_ITEM_";
		static const std::string stepTextQly = "MIS_GIVE_ITEM_QUAL_";
		textPtr = &stepText;
		
		for ( uint i  = 0; i < subStepStates.size(); i++ )
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

				if ( _SubSteps[i].Quality != 0 )
				{
					retParams.push_back(STRING_MANAGER::TParam());
					retParams.back().Type = STRING_MANAGER::integer;
					retParams.back().Int = _SubSteps[i].Quality;
					textPtr = &stepTextQly;
				}
			}
		}
		retParams.push_back(STRING_MANAGER::TParam());
		retParams.back().Type = STRING_MANAGER::bot;
		if ( _Bot != CAIAliasTranslator::Invalid )
			retParams.back().Int = _Bot;
		else
			retParams.back().Identifier = "giver";
	}

	virtual TAIAlias getInvolvedBot(bool& invalidIsGiver) const { invalidIsGiver=true; return _Bot; }

	//***********************
	bool checkPlayerGift( CMission * instance, CCharacter * user )
	{
		if ( user->getExchangeMoney() != 0 )
			return false;

		std::vector<CSubStep> validateSteps = _SubSteps;

		// build the list of need item by substracting the template step with the current step state
		// TODO : this code is only aware of the first active step !
		if (instance->getStepsBegin() == instance->getStepsEnd())
			return false;

		const CActiveStepPD &activeStep = instance->getStepsBegin()->second;

		for (uint i=0; i<_SubSteps.size(); ++i)
		{
			const CActiveStepStatePD *activeStepState = activeStep.getStates(i+1);
			if (activeStepState != NULL)
				validateSteps[i].Quantity = activeStepState->getState();
		}
		bool notEmpty = false;

		nlassert(user->getExchangeView() != NULL);
		for (uint i = 0; i < CExchangeView::NbExchangeSlots; i++)
		{
			uint32 exchangeQuantity;
			CGameItemPtr item = user->getExchangeView()->getExchangeItem(i, &exchangeQuantity);
			if (item == NULL)
				continue;

			notEmpty = true;
			uint k = 0;
			for (; k < validateSteps.size(); k++)
			{
				if (item->getSheetId() == validateSteps[k].Sheet && item->recommended() >= validateSteps[k].Quality)
				{
					if (exchangeQuantity > validateSteps[k].Quantity)
					{
						// if user has put more that this sub step quantity, just decrease the quantity
						exchangeQuantity -= validateSteps[k].Quantity;
						validateSteps[k].Quantity = 0;
					}
					else
					{
						// This sub step consume all this stack in the exchange inventory
						validateSteps[k].Quantity -= exchangeQuantity;
						exchangeQuantity = 0;
					}
				}
			}
			// the give is invalid only if there are still some quantity in the exchange bag
			if (exchangeQuantity > 0)
				return false;
		}

		return notEmpty;
	}

	//***********************
	bool itemGiftDone( CCharacter & user , const std::vector< CGameItemPtr > & itemsGiven, const EGSPD::CActiveStepPD & step, std::vector<uint32>& result )
	{
		// init the result vector with the initial values of the sub steps
		for ( map<uint32,EGSPD::CActiveStepStatePD>::const_iterator it = step.getStatesBegin(); it != step.getStatesEnd(); ++it )
		{
			result.push_back( (*it).second.getState() );
		}
		
		// iterate through the exchange inventory items. For now Store in result the remianing quantity of each substep
		for ( uint i  =0; i < itemsGiven.size(); i++ )
		{
			if ( itemsGiven[i] != NULL)
			{
				// the slot is not empty : get quantity / quality /sheet. Special case if we have a stack...
				CSheetId sheet = itemsGiven[i]->getSheetId();
				uint32 quantity = itemsGiven[i]->getStackSize();
				uint32 recommended = itemsGiven[i]->recommended();
//				if ( sheet == CSheetId("stack.sitem") )
//				{
//					if( itemsGiven[i]->getChildren().empty() )
//					{
//						MISLOG("sline:%u user:%s ERROR : give_item (itemGiftDone) : player has an empty stack in exchange", _SourceLine, user.getId().toString().c_str());
//					}
//					else
//					{
//						sheet = itemsGiven[i]->getChildren()[0]->getSheetId();
//						recommended = itemsGiven[i]->getChildren()[0]->recommended();
//						quantity = itemsGiven[i]->getChildren().size();
//					}
//				}
				
				// check if a step matches the item? ( we are sure that the steps are sorted by quality, so the first step who matches will ve the good one )
				bool noMatch = true;
				for ( uint k = 0; k < _SubSteps.size(); k++ )
				{
					if ( _SubSteps[k].Sheet == sheet && _SubSteps[k].Quality <= recommended )
					{
						noMatch = false;
						if ( quantity > result[k] )
						{
							quantity -= result[k];
							result[k] = 0;
							if ( quantity == 0)
								break;
						}
						else
						{
							result[k] -= quantity;
							quantity = 0;
							break;
						}
					}
				}
				if( noMatch )
					return false;
			}
		}
		
		// we now store in result the actual given quantity
		uint i = 0;
		for ( map<uint32,EGSPD::CActiveStepStatePD>::const_iterator it = step.getStatesBegin(); it != step.getStatesEnd(); ++it )
		{
			result[i] = (*it).second.getState() - result[i];
			i++;
		}
		return true;
	}

	//***********************
	std::vector< CSubStep > getSubSteps()
	{
		return _SubSteps;
	}

	
	std::vector< CSubStep > _SubSteps;
	TAIAlias				_Bot;
	std::string				_PhraseId;
	TVectorParamCheck _Params;
	
	MISSION_STEP_GETNEWPTR(CMissionStepGiveItem)
};
MISSION_REGISTER_STEP(CMissionStepGiveItem,"give_item")


// ----------------------------------------------------------------------------
// class CMissionStepDynChat
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
bool CMissionStepDynChat::buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
{
	_SourceLine = line;
	if ( script.size() < 3)
	{
		MISLOGSYNTAXERROR("<npc_name> : <phrase_id> *[ ;<parameter>] : *[ <jump_label> <answer_X_phrase_id> *[ ;< parameter>] :]");
		return false;
	}
	bool ret = true;
	if ( !CMissionParser::parseBotName(script[1],Bot,missionData) )
	{
		MISLOGERROR1("invalid npc '%s'", script[1].c_str());
		ret = false;
	}
	if ( ! CMissionParser::parseParamText(line, script[2], PhraseId, Params ) )
	{
		MISLOGERROR1("invalid text '%s'", script[2].c_str());
		ret = false;
	}
	for  (uint i = 3; i < script.size(); i++ )
	{
		string answerStr = CMissionParser::getNoBlankString( script[i] );
		string::size_type pos = answerStr.find( ' ' );
		if( pos == string::npos || answerStr.empty() )
		{
			MISLOGERROR1("invalid answer '%s'", answerStr.c_str());
			ret = false;
		}
		else
		{
			CAnswer answer;
			answer.Jump = answerStr.substr( 0,pos );
			missionData.Jumps.push_back( answer.Jump );
			
			string phrase = answerStr.substr( pos );
			vector< string > args;
			splitString(phrase,";",args);
			
			CMissionParser::removeBlanks( args[0] );
			if( args[0].find_first_of(";:\t ,") != string::npos || args.empty())
			{
				MISLOGERROR1("invalid text '%s'",  args[0].c_str() );
				return false;
			}
			answer.PhraseId = args[0];
			answer.Params.resize( args.size() - 1 );
			for ( uint i = 1; i < args.size(); i++ )
			{
				CMissionParser::removeBlanks(args[i]);
				answer.Params[i-1].Identifier = args[i];
			}
			Answers.push_back(answer);
		}
	}
	return ret;
}

bool CMissionStepDynChat::solveTextsParams( CMissionSpecificParsingData & missionData,CMissionTemplate * templ  )
{
		bool ret = IMissionStepTemplate::solveTextsParams(missionData,templ);
		if ( !CMissionParser::solveTextsParams(_SourceLine, Params,missionData ) )
			ret = false;
		for ( uint i =0; i < Answers.size(); i++ )
		{
			if ( !CMissionParser::solveTextsParams(_SourceLine, Answers[i].Params,missionData ) )
				ret = false;
		}
		return ret;
}

uint CMissionStepDynChat::processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
{	
	// 1 active dyn chat per mission, so if the event is triggered, it is for this step
	if ( event.Type == CMissionEvent::EndDynChat )
	{
		LOGMISSIONSTEPSUCCESS("dyn_chat");
		return 1;
	}
	return 0;
}

void CMissionStepDynChat::getInitState( std::vector<uint32>& ret )
{
	ret.clear();
	ret.resize( 1 );
	ret[0] = 1;
}

void CMissionStepDynChat::getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
{
	nbSubSteps = 1;
	static const std::string stepText = "MIS_TALK_TO";
	textPtr = &stepText;
	
	retParams.resize( 1 );
	retParams[0].Type = STRING_MANAGER::bot;
	if ( Bot != CAIAliasTranslator::Invalid )
		retParams[0].Int = Bot;
	else
		retParams[0].Identifier = "giver";		
}

void CMissionStepDynChat::onActivation(CMission* instance,uint32 stepIndex,std::list< CMissionEvent * > & eventList)
{
	logMissionStep(_SourceLine, TDataSetRow(), 0, "ACTIVE", "dyn_chat " + PhraseId);
	// register a new dyn chat session
	CMissionManager::getInstance()->addDynChat( instance,this, stepIndex);
}

MISSION_REGISTER_STEP(CMissionStepDynChat,"dyn_chat");

