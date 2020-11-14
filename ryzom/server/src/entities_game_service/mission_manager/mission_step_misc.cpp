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
#include "mission_step_misc.h"
#include "mission_manager/mission_template.h"
#include "mission_log.h"
#include "mission_manager/ai_alias_translator.h"
#include "creature_manager/creature.h"
#include "creature_manager/creature_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "zone_manager.h"
#include "mission_manager/mission_parser.h"
#include "mission_manager/mission_manager.h"
#include "nel/misc/algo.h"


using namespace std;
using namespace NLMISC;


/***************************************************************************************************
Miscellaneous Steps
	-target_fauna
	-target_race
	-target_npc
	-skill
	-visit
	-cast
	-mission
***************************************************************************************************/

// ----------------------------------------------------------------------------
// This class is the base class for target steps (fauna, npc, race)
// Composed with a vector of substep (all of the same nature) and a place where the targeting must be done
class IMissionStepTarget : public IMissionStepTemplate
{
public:
	// This class is the base class for a sub step. Derived substep must have data
	class ISubStep
	{
	public:
		// called when we build the sub step (construct the data from the string : target_xxx str0 ; str1 ; str2)
		virtual bool assign(const std::string &str) = 0;
		// called when we process the sub step (check if the substep is validated)
		virtual bool check(const CMissionEventTarget &event, const TDataSetRow & giverRow) = 0;
		// called when we construct the text representing the sub step
		virtual void fillParam(STRING_MANAGER::TParam &outParam) = 0;
	};

	std::vector<ISubStep*>	_SubSteps;
	uint16					_PlaceId;

	// Implements to provide the creation of your own kind of substep
	virtual ISubStep *createNewSubStep() = 0;
	// Called when a substep is not well constructed
	virtual void logSyntaxError(uint32 line, const vector<string> & script) = 0;
	// Called when a substep is well processed
	virtual void logSuccessString(const TDataSetRow & userRow, uint subStepIndex) = 0;
	// Called when we constructs the text for all substeps
	virtual string getStepDefaultText() = 0;
	// Provide the type of the substep for the string manager to be able to know it
	virtual STRING_MANAGER::TParamType getStringManagerType() = 0;

	
	//*****************************************************
	// Destructor : remove all substeps
	virtual ~IMissionStepTarget()
	{
		for (uint32 i = 0; i < _SubSteps.size(); ++i)
			delete _SubSteps[i];
	}

	//*****************************************************
	// Constructs all substep from the script and if there is a place needed, get it
	virtual bool buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		bool ret = true;
		if (( script.size() == 2 ) || ( script.size() == 3 ))
		{
			vector<string>  vSubs;
			NLMISC::splitString( script[1], ";", vSubs );

			// Assign all substeps
			_SubSteps.resize( vSubs.size() );
			for (uint32 i = 0; i < vSubs.size(); ++i)
			{
				ISubStep *pointerSubStep = createNewSubStep();
				CMissionParser::removeBlanks( vSubs[i] );
				if (!pointerSubStep->assign(vSubs[i]))
				{
					ret = false;
					MISLOGERROR("bad assignation of the substep");
				}
				missionData.ChatParams.push_back( make_pair(vSubs[i], getStringManagerType()) );
				_SubSteps[i] = pointerSubStep;
			}
			
			// Get the place if required
			_PlaceId = InvalidPlaceId;
			if (script.size() == 3)
			{
				CPlace *place = CZoneManager::getInstance().getPlaceFromName( CMissionParser::getNoBlankString( script[2] ) );
				if ( !place )
				{
					MISLOGERROR1("invalid place '%s'", CMissionParser::getNoBlankString(script[2]).c_str());
					ret = false;
				}
				else
				{
					missionData.ChatParams.push_back( make_pair(script[2], STRING_MANAGER::place) );
					_PlaceId = place->getId();
				}
			}
			
			return ret;
		}
		else
		{
			logSyntaxError(line,script);
			return false;
		}
	}

	//*****************************************************
	// Process the step events with place check if needed
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex, const TDataSetRow & giverRow )
	{
		uint32 ret = 0;
		if ( event.Type == CMissionEvent::Target )
		{
			CMissionEventTarget &eventSpe = (CMissionEventTarget&)event;

			if (_SubSteps[subStepIndex]->check(eventSpe, giverRow))
			{
				// If a place is required
				if (_PlaceId != InvalidPlaceId)
				{
					// Check that the creature is in the right place
					CCreature * c = CreatureManager.getCreature( event.TargetEntity );
					float gooDistance;
					const CPlace * stable = NULL;
					std::vector<const CPlace *> places;
					const CRegion * region = NULL;
					const CContinent * continent = NULL;
					if ( CZoneManager::getInstance().getPlace( c->getState().X, c->getState().Y, gooDistance, &stable, places, &region , &continent ) )
					{
						if ( region && region->getId() == _PlaceId )
							ret = 1;

						for ( uint i = 0; i < places.size(); i++ )
						{
							if ( places[i] && places[i]->getId() == _PlaceId )
								ret = 1;
						}
					}
				}
				else
				{
					ret = 1; // We just need to target the specified fauna
				}
			}
		}
		if (ret == 1)
			logSuccessString(userRow, subStepIndex);
			
		return ret;
	}
	
	//*****************************************************
	virtual void getInitState( std::vector<uint32>& ret )
	{
		ret.clear();
		ret.resize( _SubSteps.size() );
		for ( uint i = 0; i < _SubSteps.size(); i++ )
		{
			ret[i] = 1;
		}
	}

	//*****************************************************
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		static const std::string stepText = getStepDefaultText();
		textPtr = &stepText;
		nlassert( _SubSteps.size() == subStepStates.size() );
		for ( uint i  = 0; i < _SubSteps.size(); i++ )
		{
			if( subStepStates[i] != 0 )
			{
				nbSubSteps++;
				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = getStringManagerType();
				_SubSteps[i]->fillParam(retParams.back());
			}
		}
	}
};

// ----------------------------------------------------------------------------
class CMissionStepTargetFauna : public IMissionStepTarget
{
	class CSubStepFauna : public IMissionStepTarget::ISubStep
	{
		CSheetId	Sheet;

		virtual bool assign(const std::string &str)
		{
			Sheet = CSheetId(str + ".creature" );
			if ( Sheet == CSheetId::Unknown )
				return false;
			return true;
		}

		virtual bool check(const CMissionEventTarget &event, const TDataSetRow & giverRow)
		{
			CCreature * c = CreatureManager.getCreature( event.TargetEntity );
			if ( c && Sheet == c->getType() && !c->isDead() )
				return true;
			return false;
		}

		virtual void fillParam(STRING_MANAGER::TParam &outParam)
		{
			outParam.SheetId = Sheet;
		}
	};

	virtual IMissionStepTarget::ISubStep *createNewSubStep()
	{
		return new CSubStepFauna;
	}
	
	virtual void logSyntaxError(uint32 line, const vector<string> &script)
	{
		MISLOGSYNTAXERROR("<sheet> *[; <sheet>] [: <place>]");
	}

	virtual void logSuccessString(const TDataSetRow & userRow, uint subStepIndex)
	{
		LOGMISSIONSTEPSUCCESS("target_fauna");
	}

	virtual STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::creature_model;
	}

	virtual string getStepDefaultText()
	{
		return "MIS_TARGET_FAUNA_";
	}

	MISSION_STEP_GETNEWPTR(CMissionStepTargetFauna)
};
MISSION_REGISTER_STEP(CMissionStepTargetFauna, "target_fauna")

// ----------------------------------------------------------------------------
class CMissionStepTargetRace : public IMissionStepTarget
{
	struct CSubStepRace : public IMissionStepTarget::ISubStep
	{
		EGSPD::CPeople::TPeople	Race;
		
		virtual bool assign(const std::string &str)
		{
			Race = EGSPD::CPeople::fromString(str);
			if ( Race == EGSPD::CPeople::EndPeople )
				return false;
			return true;
		}
		
		virtual bool check(const CMissionEventTarget &event, const TDataSetRow & giverRow)
		{
			CCreature * c = CreatureManager.getCreature( event.TargetEntity );
			if ( c && Race == c->getRace() && !c->isDead() )
				return true;
			return false;
		}
		
		virtual void fillParam(STRING_MANAGER::TParam &outParam)
		{
			outParam.Enum = Race;
		}
	};
	
	virtual IMissionStepTarget::ISubStep *createNewSubStep()
	{
		return new CSubStepRace;
	}
	
	virtual void logSyntaxError(uint32 line, const vector<string> &script)
	{
		MISLOGSYNTAXERROR("<race> *[; <race>] [: <place>]");
	}
	
	virtual void logSuccessString(const TDataSetRow & userRow, uint subStepIndex)
	{
		LOGMISSIONSTEPSUCCESS("target_race");
	}
	
	virtual STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::race;
	}
	
	virtual string getStepDefaultText()
	{
		return "MIS_TARGET_SPECIES_";
	}
	
	MISSION_STEP_GETNEWPTR(CMissionStepTargetRace)
};
MISSION_REGISTER_STEP(CMissionStepTargetRace,"target_race")

// ----------------------------------------------------------------------------
class CMissionStepTargetNpc : public IMissionStepTarget
{
	struct CSubStepNpc : public IMissionStepTarget::ISubStep
	{
		TAIAlias	Alias;

		virtual bool assign(const std::string &str)
		{
			Alias = CAIAliasTranslator::Invalid;
			string name = CMissionParser::getNoBlankString( str );
			if ( name == "giver" )
				return true;
			
			// get the first matching bot
			vector<TAIAlias> aliases;
			CAIAliasTranslator::getInstance()->getNPCAliasesFromName( name , aliases );
			if ( aliases.empty() )
				return false;

			Alias = aliases[0];
			return true;
		}
		
		virtual bool check(const CMissionEventTarget &event, const TDataSetRow & giverRow)
		{
			CCreature * c = CreatureManager.getCreature( event.TargetEntity );
			if ( c && !c->isDead() )
			{
				if ( Alias != CAIAliasTranslator::Invalid )
				{
					if ( Alias == c->getAlias() )
						return true;
				}
				else if ( event.TargetEntity == giverRow )
				{					
					return true;
				}
			}
			return false;
		}
		
		virtual void fillParam(STRING_MANAGER::TParam &outParam)
		{
			if ( Alias != CAIAliasTranslator::Invalid )
				outParam.Int = Alias ;
			else
				outParam.Identifier = "giver";
		}
	};

	virtual IMissionStepTarget::ISubStep *createNewSubStep()
	{
		return new CSubStepNpc;
	}
	
	virtual void logSyntaxError(uint32 line, const vector<string> &script)
	{
		MISLOGSYNTAXERROR("<npc_name> *[; <npc_name>] [: <place>]");
	}
	
	virtual void logSuccessString(const TDataSetRow & userRow, uint subStepIndex)
	{
		LOGMISSIONSTEPSUCCESS("target_npc");
	}
	
	virtual STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::bot;
	}
	
	virtual string getStepDefaultText()
	{
		return "MIS_TARGET_NPC_";
	}

	virtual TAIAlias getInvolvedBot(bool& invalidIsGiver) const
	{
		if (_SubSteps.empty())
		{
			invalidIsGiver = false; 
			return CAIAliasTranslator::Invalid;
		}
		
		invalidIsGiver = true;
		STRING_MANAGER::TParam aliasParam;
		_SubSteps.front()->fillParam(aliasParam);
		return aliasParam.Int;
	}
	
	MISSION_STEP_GETNEWPTR(CMissionStepTargetNpc)
};
MISSION_REGISTER_STEP(CMissionStepTargetNpc,"target_npc")


// ----------------------------------------------------------------------------
//class CMissionStepTargetFauna : public IMissionStepTemplate
//{
//	struct CSubStep
//	{
//		CSheetId	Sheet;
//	};
//	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
//	{
//		_SourceLine = line;
//		bool ret = true;
//		if (( script.size() == 2 ) || ( script.size() == 3 ))
//		{
//			std::vector< std::string > subs;
//			NLMISC::splitString( script[1],";", subs );
//			_SubSteps.resize( subs.size() );
//			for ( uint i = 0; i < subs.size(); i++ )
//			{
//				CMissionParser::removeBlanks( subs[i] );
//				missionData.ChatParams.push_back( make_pair(subs[i],STRING_MANAGER::creature_model) );
//				_SubSteps[i].Sheet = CSheetId(subs[i] + ".creature" );
//				if ( _SubSteps[i].Sheet == CSheetId::Unknown )
//				{
//					ret = false;
//					MISLOGERROR1("invalid sheet '%s'", subs[i].c_str());
//				}
//			}
//
//			_PlaceId = InvalidPlaceId;
//			if (script.size() == 3)
//			{
//				CPlace *place = CZoneManager::getInstance().getPlaceFromName( CMissionParser::getNoBlankString( script[2] ) );
//				if ( !place )
//				{
//					MISLOGERROR1("invalid place '%s'", CMissionParser::getNoBlankString(script[2]).c_str());
//					ret = false;
//				}
//				else
//				{
//					_PlaceId = place->getId();
//				}
//			}
//
//			return ret;
//		}
//		else
//		{
//			MISLOGSYNTAXERROR("<sheet> *[; <sheet>] [: <place>]");
//			return false;
//		}
//	}
//	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex, const TDataSetRow & giverRow )
//	{
//		uint32 ret = 0;
//		if ( _IsActive && event.Type == CMissionEvent::Target )
//		{
//			CMissionEventTarget & eventSpe = (CMissionEventTarget&)event;
//			CCreature * c = CreatureManager.getCreature( event.TargetEntity );
//			if ( c && _SubSteps[subStepIndex].Sheet == c->getType() && !c->isDead() )
//			{
//				// If a place is required
//				if (_PlaceId != InvalidPlaceId)
//				{
//					// Check that the player is in the right place
//					CCharacter *pChar = PlayerManager.getChar(userRow);
//					if (pChar != NULL)
//					{
//						if (pChar->isInPlace(_PlaceId))
//							ret = 1;
//					}
//				}
//				else
//				{
//					ret = 1; // We just need to target the specified fauna
//				}
//			}
//		}
//		if (ret == 1)
//			LOGMISSIONSTEPSUCCESS("target_fauna")
//			
//		return ret;
//	}
//	
//	void getInitState( std::vector<uint32>& ret )
//	{
//		ret.clear();
//		ret.resize( _SubSteps.size() );
//		for ( uint i = 0; i < _SubSteps.size(); i++ )
//		{
//			ret[i] = 1;
//		}
//	}
//	
//	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
//	{
//		static const std::string stepText = "MIS_TARGET_FAUNA_";
//		textPtr = &stepText;
//		nlassert( _SubSteps.size() == subStepStates.size() );
//		for ( uint i  = 0; i < subStepStates.size(); i++ )
//		{
//			if( subStepStates[i] != 0 )
//			{
//				nbSubSteps++;
//				retParams.push_back(STRING_MANAGER::TParam());
//				retParams.back().Type = STRING_MANAGER::creature_model;
//				retParams.back().SheetId = _SubSteps[i].Sheet;
//			}
//		}
//	}
//
//	std::vector< CSubStep > _SubSteps;
//	uint16					_PlaceId;
//	
//	MISSION_STEP_GETNEWPTR(CMissionStepTargetFauna)
//};
//MISSION_REGISTER_STEP(CMissionStepTargetFauna,"target_fauna")

// ----------------------------------------------------------------------------
//class CMissionStepTargetRace : public IMissionStepTemplate
//{
//	struct CSubStep
//	{
//		EGSPD::CPeople::TPeople	Race;
//	};
//	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
//	{
//		_SourceLine = line;
//		bool ret = true;
//		if ( script.size() !=2 )
//		{
//			MISLOGSYNTAXERROR("<race>*[; <race>]");
//			return false;
//		}
//		else
//		{
//			std::vector< std::string > subs;
//			NLMISC::splitString( script[1],";", subs );
//			_SubSteps.resize( subs.size() );
//			for ( uint i = 0; i < subs.size(); i++ )
//			{
//				CMissionParser::removeBlanks( subs[i] );
//				missionData.ChatParams.push_back( make_pair(subs[i], STRING_MANAGER::race) );
//				_SubSteps[i].Race = EGSPD::CPeople::fromString( subs[i] );
//				if ( _SubSteps[i].Race == EGSPD::CPeople::EndPeople)
//				{
//					ret = false;
//					MISLOGERROR1("invalid race '%s'", subs[i].c_str());
//				}
//			}
//			return ret;
//		}
//	}
//	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex, const TDataSetRow & giverRow )
//	{
//		if ( _IsActive && event.Type == CMissionEvent::Target )
//		{
//			CMissionEventTarget & eventSpe = (CMissionEventTarget&)event;
//			CCreature * c = CreatureManager.getCreature( event.TargetEntity );
//			if ( c && _SubSteps[subStepIndex].Race == c->getRace() && !c->isDead() )
//			{
//				LOGMISSIONSTEPSUCCESS("target_race");
//				return 1;
//			}
//		}
//		return 0;
//	}
//	
//	void getInitState( std::vector<uint32>& ret )
//	{
//		ret.clear();
//		ret.resize( _SubSteps.size() );
//		for ( uint i = 0; i < _SubSteps.size(); i++ )
//		{
//			ret[i] = 1;
//		}
//	}
//	
//	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
//	{
//		static const std::string stepText = "MIS_TARGET_SPECIES_";
//		textPtr = &stepText;
//		nlassert( _SubSteps.size() == subStepStates.size() );
//		for ( uint i  = 0; i < subStepStates.size(); i++ )
//		{
//			if( subStepStates[i] != 0 )
//			{
//				nbSubSteps++;
//				retParams.push_back(STRING_MANAGER::TParam());
//				retParams.back().Type = STRING_MANAGER::race;
//				retParams.back().Enum = _SubSteps[i].Race;
//			}
//		}
//	}
//	std::vector< CSubStep > _SubSteps;
//
//	MISSION_STEP_GETNEWPTR(CMissionStepTargetRace)
//};
//MISSION_REGISTER_STEP(CMissionStepTargetRace,"target_race")

// ----------------------------------------------------------------------------
//class CMissionStepTargetNpc : public IMissionStepTemplate
//{
//	struct CSubStep
//	{
//		TAIAlias	Alias;
//	};
//	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
//	{
//		_SourceLine = line;
//		bool ret = true;
//		if ( script.size() < 2)
//		{
//			MISLOGSYNTAXERROR("<npc_name>*[; <npc_name>]");
//			return false;
//		}
//		else
//		{
//			std::vector< std::string > subs;
//			NLMISC::splitString( script[1],";", subs );
//			_SubSteps.resize( subs.size() );
//			for ( uint i = 0; i < subs.size(); i++ )
//			{
//				CMissionParser::removeBlanks( subs[i] );
//				CSubStep subStep;
//				subStep.Alias = CAIAliasTranslator::Invalid;
//				if ( !CMissionParser::parseBotName(subs[i],subStep.Alias,missionData) )
//					ret = false;
//				_SubSteps[i] = subStep;
//			}
//			return ret;
//		}
//	}
//	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
//	{
//		if ( _IsActive && event.Type == CMissionEvent::Target )
//		{
//			CMissionEventTarget & eventSpe = (CMissionEventTarget&)event;
//			CCreature * c = CreatureManager.getCreature( event.TargetEntity );
//			if ( c && !c->isDead() )
//			{
//				if ( _SubSteps[subStepIndex].Alias != CAIAliasTranslator::Invalid )
//				{
//					if ( _SubSteps[subStepIndex].Alias == c->getAlias() )
//					{
//						LOGMISSIONSTEPSUCCESS("target_npc");
//						return 1;
//					}
//				}
//				else if ( event.TargetEntity == giverRow )
//				{
//					LOGMISSIONSTEPSUCCESS("target_npc");
//					return 1;
//				}
//			}
//		}
//		return 0;
//	}
//	
//	void getInitState( std::vector<uint32>& ret )
//	{
//		ret.clear();
//		ret.resize( _SubSteps.size() );
//		for ( uint i = 0; i < _SubSteps.size(); i++ )
//		{
//			ret[i] = 1;
//		}
//	}
//	
//	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
//	{
//		static const std::string stepText = "MIS_TARGET_NPC_";
//		textPtr = &stepText;
//		nlassert( _SubSteps.size() == subStepStates.size() );
//		for ( uint i  = 0; i < _SubSteps.size(); i++ )
//		{
//			if( subStepStates[i] != 0 )
//			{
//				nbSubSteps++;
//				retParams.push_back(STRING_MANAGER::TParam());
//				retParams.back().Type = STRING_MANAGER::bot;
//				
//				if ( _SubSteps[i].Alias != CAIAliasTranslator::Invalid )
//					retParams.back().Int = _SubSteps[i].Alias ;
//				else
//					retParams.back().Identifier = "giver";
//				
//			}
//		}
//	}
//	std::vector< CSubStep > _SubSteps;
//
//	MISSION_STEP_GETNEWPTR(CMissionStepTargetNpc)
//};
//MISSION_REGISTER_STEP(CMissionStepTargetNpc,"target_npc")


// ----------------------------------------------------------------------------
class CMissionStepSkill : public IMissionStepTemplate
{
	struct CSubStep
	{
		SKILLS::ESkills	Skill;
		uint			Level;
	};
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		bool ret = true;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<skill name> <level> *[;<skill name> <level>]");
			return false;
		}
		else
		{
			std::vector< std::string > subs;
			NLMISC::splitString( script[1],";", subs );
			for ( uint i = 0; i < subs.size(); i++ )
			{
				std::vector< std::string > args;
				CMissionParser::tokenizeString( subs[i]," \t", args );
				if ( args.size() != 2 )
				{
					ret = false;
					MISLOGSYNTAXERROR("<skill name> <level> *[;<skill name> <level>]");
				}
				else
				{
					CSubStep subStep;
					subStep.Skill = SKILLS::toSkill( args[0] );
					missionData.ChatParams.push_back( make_pair( args[0], STRING_MANAGER::skill ) );
					NLMISC::fromString(args[1], subStep.Level);
					if ( subStep.Skill == SKILLS::unknown )
					{
						ret = false;
						MISLOGERROR1("invalid skill '%s'", args[0].c_str());
					}
					else
					{
						_SubSteps.push_back( subStep );
					}
				}
			}
			return ret;
		}
	}
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::SkillProgress )
		{
			CMissionEventSkillProgress & eventSpe = (CMissionEventSkillProgress&)event;
			if ( eventSpe.Level >= _SubSteps[subStepIndex].Level )
			{
				LOGMISSIONSTEPSUCCESS("skill");
				return 1;
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
			ret[i] = 1;
		}
	}
	
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		static const std::string stepText = "MIS_SKILL_";
		textPtr = &stepText;
		nlassert( _SubSteps.size() == subStepStates.size() );
		for ( uint i  = 0; i < subStepStates.size(); i++ )
		{
			if( subStepStates[i] != 0 )
			{
				nbSubSteps++;
				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::skill;
				retParams.back().Enum = _SubSteps[i].Skill;
	
				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::integer;
				retParams.back().Int = _SubSteps[i].Level;
			}
		}
	}

	void onActivation(CMission* instance,uint32 stepIndex,std::list< CMissionEvent * > & eventList)
	{		

		IMissionStepTemplate::onActivation( instance, stepIndex, eventList );
		// check if user meets success condition when step is activated
		CCharacter * user = instance->getMainEntity();
		if ( user )
		{
			for ( uint i = 0; i < _SubSteps.size(); i++ )
			{
				SSkill * skill = user->getSkills().getSkillStruct( _SubSteps[i].Skill );
				if ( skill )
				{
					if ( skill->Base >= (sint32) _SubSteps[i].Level )
					{
						CMissionEventSkillProgress * event = new CMissionEventSkillProgress( _SubSteps[i].Skill,_SubSteps[i].Level );
						eventList.push_back( event );
					}
				}
			}
		}
	}

	std::vector< CSubStep > _SubSteps;

	MISSION_STEP_GETNEWPTR(CMissionStepSkill)
};
MISSION_REGISTER_STEP(CMissionStepSkill,"skill")


// ----------------------------------------------------------------------------

bool	CMissionStepVisit::buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
{
	_SourceLine = line;
	bool ret = true;
	if ( script.size() != 2 && script.size() != 3 )
	{
		MISLOGSYNTAXERROR("<place> [: <item> [; <item>]]");
		return false;
	}
	CPlace * place = CZoneManager::getInstance().getPlaceFromName( CMissionParser::getNoBlankString( script[1] ) );
	if ( !place )
	{
		MISLOGERROR1("invalid place '%s'", CMissionParser::getNoBlankString(script[1]).c_str());
		return false;
	}
	missionData.ChatParams.push_back( make_pair(script[1],STRING_MANAGER::place) );
	_PlaceId = place->getId();
	if ( script.size() == 3 )
	{
		std::vector< std::string > items;
		NLMISC::splitString( script[2],";", items);
		_WornItems.resize( items.size() );
		bool ok = true;
		for ( uint i = 0; i < items.size(); i++ )
		{
			CMissionParser::removeBlanks( items[i] );
			missionData.ChatParams.push_back( make_pair(items[i],STRING_MANAGER::item) );
			items[i] += ".sitem";
			_WornItems[i] = CSheetId(items[i]);
			if ( _WornItems[i] == CSheetId::Unknown )
			{
				MISLOGERROR1("invalid item '%s'", items[i].c_str());
				ret = false;
			}
		}
	}
	return ret;
}

/*
 * When the mission step is activated (after a character takes it or when he is back in game), 
 * insert the character id to the list of characters for which we have to check evenly
 * if they are in the place at the right time. stepIndex starts at 0 here.
 */
void	CMissionStepVisit::onActivation(CMission* inst, uint32 stepIndex, std::list< CMissionEvent * > & eventList)
{
	IMissionStepTemplate::onActivation( inst, stepIndex, eventList );

	// The character's place will need to be checked evenly.
	// We do even if the 'visit place' mission step as no time constraints (inst->getHourLowerBound()==0.0f),
	// because when entering a place, it's not easy to check if the characters has missions with time constraints
	// AND because there are other constraints, such as having a particular item.
	BOMB_IF( !inst, "No Inst", return );
	CCharacter *character = inst->getMainEntity();
	if ( character )
	{
		CMissionManager::getInstance()->insertMissionStepForPlaceCheck( character->getEntityRowId(),
			inst->getTemplateId(), stepIndex );
	}
}


/*
 * React to the step deletion when not done by the visit place check in mission manager update
 */
void	CMissionStepVisit::onDeleteStepPrematurely(CMission *inst, uint32 stepIndex)
{
	BOMB_IF( !inst, "No Inst", return );
	CCharacter *character = inst->getMainEntity();
	if ( character )
	{
		CMissionManager::getInstance()->removeMissionStepForPlaceCheck( character->getEntityRowId(),
			inst->getTemplateId(), stepIndex );
	}
}


/*
 * Process an event.
 * See also testMatchEvent()
 */
uint CMissionStepVisit::processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
{
	if ( event.Type == CMissionEvent::EnterZone )
	{
		CMissionEventVisitPlace & eventSpe = (CMissionEventVisitPlace&)event;
		if ( eventSpe.PlaceId == _PlaceId )
		{
			CCharacter * user = PlayerManager.getChar(userRow);
			if ( !user )
			{
				LOGMISSIONSTEPERROR("visit : invalid user "+toString(userRow.getIndex()));
				return 0;
			}
			for ( uint i = 0; i < _WornItems.size(); i++ )
			{
				if ( !user->doesWear( _WornItems[i] ) )
					return 0;
			}

			LOGMISSIONSTEPSUCCESS("visit");
			return 1;
		}
	}
	return 0;
}


/*
 * Test if the event matches the step (including generic contraints from inst)
 * Preconditions: character and inst must be non-null
 */
bool CMissionStepVisit::testMatchEvent( const CCharacter *character, const CMission *inst, uint16 placeId ) const
{
	// Check place requirement
	if ( placeId != _PlaceId )
		return false;
	
	// Check generic constraints
	if ( ! inst->checkConstraints( false ) )
		return false;
	
	// Check wear item requirement
	for ( uint i = 0; i < _WornItems.size(); i++ )
	{
		if ( ! character->doesWear( _WornItems[i] ) )
			return false;
	}
	return true;
}


void CMissionStepVisit::getInitState( std::vector<uint32>& ret )
{
	ret.clear();
	ret.resize( 1 );
	ret[0] = 1;
}
	
void CMissionStepVisit::getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
{
	nbSubSteps = 1;
	static const std::string stepText = "MIS_VISIT";
	static std::vector<std::string> wornTexts;
	if ( _WornItems.size() > wornTexts.size() )
	{
		uint i = (uint)wornTexts.size();
		wornTexts.resize( _WornItems.size() );
		for (; i < wornTexts.size(); i++ )
		{
			wornTexts[i] = "MIS_VISIT_WEAR_" + toString(i+1);
		}
	}
	if ( _WornItems.empty() )
		textPtr = &stepText;
	else
		textPtr = &wornTexts[_WornItems.size()-1];

	retParams.push_back(STRING_MANAGER::TParam());
	retParams.back().Type = STRING_MANAGER::place;
	retParams.back().Identifier = CZoneManager::getInstance().getPlaceFromId( _PlaceId )->getName();

	for ( uint i  =0; i < _WornItems.size(); i++ )
	{
		retParams.push_back(STRING_MANAGER::TParam());
		retParams.back().Type = STRING_MANAGER::item;
		retParams.back().SheetId = _WornItems[i];
	}
}

MISSION_REGISTER_STEP(CMissionStepVisit,"visit")


// ----------------------------------------------------------------------------
class CMissionStepCast : public IMissionStepTemplate
{
	vector<CSheetId>	_Actions;
	uint16				_PlaceId;
	
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		_PlaceId = InvalidPlaceId; // it is optional (but cool) to require a location
		bool ret = true;
		if ( script.size() != 2 && script.size() != 3 )
		{
			MISLOGSYNTAXERROR("<sbrick> [; <sbrick>]* [:<place>]");
			return false;
		}

		std::vector< std::string > subs;
		NLMISC::splitString( script[1],";", subs );
		_Actions.resize(subs.size());
		for ( uint i = 0; i < subs.size(); i++ )
		{
			CMissionParser::removeBlanks( subs[i] );
			_Actions[i] = CSheetId( subs[i] + ".sbrick" );
			missionData.ChatParams.push_back( make_pair(  subs[i], STRING_MANAGER::sbrick ) );
			if ( _Actions[i] == CSheetId::Unknown )
			{
				ret = false;
				MISLOGERROR1("invalid brick '%s'", subs[i].c_str());
			}
		}
		if ( script.size() == 3 )
		{
			CPlace * place = CZoneManager::getInstance().getPlaceFromName(CMissionParser::getNoBlankString(script[ 2 ]));
			if ( !place )
			{
				MISLOGERROR1("invalid place '%s'", script[ 2 ].c_str());
				ret = false;
			}
			else
				_PlaceId = place->getId();
		}
		return ret;

	}
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::Cast )
		{
			CMissionEventCast & eventSpe = (CMissionEventCast&)event;
			uint i = 0;
			for (; i < eventSpe.Bricks.size(); i++)
			{
				if( eventSpe.Bricks[i] ==  _Actions[subStepIndex] )
					break;
			}
			if ( i == eventSpe.Bricks.size() )
				return 0;
			if ( _PlaceId == InvalidPlaceId ) // this is the value if a place was not specified, not if anything is actually "Invalid"
			{
				LOGMISSIONSTEPSUCCESS("cast");
				return 1;
			}
			CCharacter * user = PlayerManager.getChar(userRow);
			if ( user )
			{
				if ( user->isInPlace( _PlaceId ) )
				{
					LOGMISSIONSTEPSUCCESS("cast");
					return 1;
				}
			}
		}
		return 0;
	}
	
	void getInitState( std::vector<uint32>& ret )
	{
		ret.clear();
		ret.resize( _Actions.size(), 1 );
	}
	
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		static const std::string stepText = "MIS_CAST_";
		static const std::string stepTextPlace = "MIS_CAST_LOC_";

		if ( _PlaceId == InvalidPlaceId )
			textPtr = &stepText;
		else
			textPtr = &stepTextPlace;
			
		for ( uint i  =0; i < _Actions.size(); i++ )
		{
			if( subStepStates[i] != 0 )
			{
				nbSubSteps++;
				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::sbrick;
				retParams.back().SheetId = _Actions[i];
			}
		}
		if ( _PlaceId != InvalidPlaceId )
		{
			STRING_MANAGER::TParam param;
			param.Type = STRING_MANAGER::place;
			param.Identifier = CZoneManager::getInstance().getPlaceFromId( _PlaceId )->getName();
			retParams.push_back(param);
		}
	}

	MISSION_STEP_GETNEWPTR(CMissionStepCast)
};
MISSION_REGISTER_STEP(CMissionStepCast,"cast")


// ----------------------------------------------------------------------------
class CMissionStepDoMissions : public IMissionStepTemplate
{
	struct MissionNb
	{
		std::string Mission;
		uint32 NbNeedCompletion;
	};

	std::vector< MissionNb > _Missions;
	
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{	
		_SourceLine = line;
		bool ret = true;
		if ( script.size() != 2 && script.size() != 3 )
		{
			MISLOGSYNTAXERROR("<mission> *[; <mission>]");
			return false;
		}

		std::vector< std::string > subs;
		NLMISC::splitString( script[1],";", subs );
		_Missions.resize(subs.size());
		for ( uint i = 0; i < subs.size(); i++ )
		{
			std::vector< std::string > params;
			//NLMISC::splitString( subs[i]," \t", params );
			subs[i] = CMissionParser::getNoBlankString(subs[i]);
			std::size_t pos = subs[i].find_first_of(" \t");
			std::string str = subs[i].substr(0, pos);
			params.push_back(str);
			if (pos != std::string::npos)
				str = subs[i].substr(pos + 1);
			else
				str = "";
			params.push_back(str);
			//std::size_t pos = _Missions[i].find_first_of(" \t");
			_Missions[i].Mission = CMissionParser::getNoBlankString( params[0] );
			if (params.size() > 1)
				NLMISC::fromString(params[1], _Missions[i].NbNeedCompletion);
			else
				_Missions[i].NbNeedCompletion = 1;
		}
		return true;
	}
	
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::MissionDone )
		{
			CMissionEventMissionDone & eventSpe = (CMissionEventMissionDone&)event;
			TAIAlias alias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( _Missions[subStepIndex].Mission );
			if ( eventSpe.Mission == alias )
			{
				LOGMISSIONSTEPSUCCESS("mission");
				return 1;
			}
		}
		return 0;
	}
	
	void getInitState( std::vector<uint32>& ret )
	{
		ret.resize( _Missions.size(), 1 );
		uint32 i = 0;
		for (std::vector<MissionNb>::const_iterator it = _Missions.begin(); it != _Missions.end(); ++it)
		{
			ret[i] = it->NbNeedCompletion;
			i++;
		}
	}
	
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		/*static const std::string stepText = "ERROR_UNSPECIFIED_MISSION_TEXT";
		textPtr = &stepText;*/

		// Because we can specify the number of times we want a mission to be completed, we specify the parameters
		static const std::string stepText = "MIS_DO_MISSION_";
		nlassert( _Missions.size() == subStepStates.size() );
		for ( uint i  = 0; i < subStepStates.size(); i++ )
		{
			if( subStepStates[i] != 0 )
			{
				nbSubSteps++;
				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::integer;
				retParams.back().Int = subStepStates[i];

				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::literal;
				retParams.back().Literal = _Missions[i].Mission;
			}
		}
		textPtr = &stepText;
	}
	bool checkTextConsistency()
	{
		if ( !_IsInOverridenOOO && isDisplayed() && _OverridenText.empty() )
		{
			MISLOG("sline:%u ERROR : mission : non overridden mission text",_SourceLine);
			return false;
		}
		return true;
	}

	MISSION_STEP_GETNEWPTR(CMissionStepDoMissions)
};
MISSION_REGISTER_STEP(CMissionStepDoMissions,"mission")

// ----------------------------------------------------------------------------
class CMissionStepRingScenario : public IMissionStepTemplate
{
	std::string _ScenarioTag;
	
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{	
		_SourceLine = line;
		bool ret = true;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<scenario_tag>");
			return false;
		}

		_ScenarioTag = CMissionParser::getNoBlankString(script[1]);
		return true;
	}
	
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::TaggedRingScenario )
		{
			const CMissionEventTaggedRingScenarioDone & eventSpe = static_cast<const CMissionEventTaggedRingScenarioDone&>(event);
			if (_ScenarioTag == eventSpe.ScenarioTag)
			{
				LOGMISSIONSTEPSUCCESS("ring_scenario");
				return 1;
			}
		}
		return 0;
	}
	
	void getInitState( std::vector<uint32>& ret )
	{
		ret.resize( 1, 1 );
	}
	
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		static const std::string stepText = "ERROR_UNSPECIFIED_RING_SCENARIO_TEXT";
		textPtr = &stepText;
	}
	bool checkTextConsistency()
	{
		if ( !_IsInOverridenOOO && isDisplayed() && _OverridenText.empty() )
		{
			MISLOG("sline:%u ERROR : ring_scenario : non overridden scenario text",_SourceLine);
			return false;
		}
		return true;
	}

	MISSION_STEP_GETNEWPTR(CMissionStepRingScenario)
};
MISSION_REGISTER_STEP(CMissionStepRingScenario,"ring_scenario")


// ************************
// CMissionStepHandleCreate
// ************************

// ----------------------------------------------------------------------------
bool CMissionStepHandleCreate::buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
{	
	_SourceLine = line;
	if ( ( script.size() != 2 ) && ( script.size() != 3 ) )
	{
		MISLOGSYNTAXERROR("<group> [:<despawn_time>]");
		return false;
	}
	string sGroupName = CMissionParser::getNoBlankString( script[1] );
	vector<TAIAlias> aliases;
	CAIAliasTranslator::getInstance()->getGroupAliasesFromName(sGroupName, aliases);
	if (aliases.size() > 1)
	{
		MISLOGERROR1("group name '%s' do not have a unique alias", sGroupName.c_str());
		return false;
	}
	if (aliases.size() == 0)
	{
		MISLOGERROR1("group name '%s' alias doesn't exist", sGroupName.c_str());
		return false;
	}
	GroupAlias = aliases[0];
	DespawnTime = 0;
	if (script.size() == 3)
		NLMISC::fromString(CMissionParser::getNoBlankString( script[2] ), DespawnTime);
	
	return true;
}

// ----------------------------------------------------------------------------
void CMissionStepHandleCreate::onActivation(CMission* instance,uint32 stepIndex,std::list< CMissionEvent * > & eventList)
{
	vector<TDataSetRow> entities;
	instance->getEntities(entities);

	for (uint32 i = 0; i < entities.size(); ++i)
	{
		CCharacter *pChar = PlayerManager.getChar(entities[i]);
		if (pChar == NULL)
			continue;
		pChar->addHandledAIGroup(instance, GroupAlias, DespawnTime);
	}
}

// ----------------------------------------------------------------------------
uint CMissionStepHandleCreate::processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
{
	if ( event.Type == CMissionEvent::GroupSpawned )
	{
		CMissionEventGroupSpawned & eventSpe = (CMissionEventGroupSpawned&)event;
		if (eventSpe.Alias == GroupAlias)
			return 1;
	}
	return 0;
}
	
// ----------------------------------------------------------------------------
void CMissionStepHandleCreate::getInitState( std::vector<uint32>& ret )
{
	ret.clear();
	ret.resize(1);
	ret[0] = 1;
}

// ----------------------------------------------------------------------------
void CMissionStepHandleCreate::getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
{
}
	
MISSION_REGISTER_STEP(CMissionStepHandleCreate, "handle_create")

// *************************
// CMissionStepHandleRelease
// *************************

// ----------------------------------------------------------------------------
bool CMissionStepHandleRelease::buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
{	
	_SourceLine = line;
	if ( ( script.size() != 2 ) && ( script.size() != 3 ) )
	{
		MISLOGSYNTAXERROR("<group> [:<despawn_time>]");
		return false;
	}
	string sGroupName = CMissionParser::getNoBlankString( script[1] );
	vector<TAIAlias> aliases;
	CAIAliasTranslator::getInstance()->getGroupAliasesFromName(sGroupName, aliases);
	if (aliases.size() > 1)
	{
		MISLOGERROR1("group name '%s' do not have a unique alias", sGroupName.c_str());
		return false;
	}
	if (aliases.size() == 0)
	{
		MISLOGERROR1("group name '%s' alias doesn't exist", sGroupName.c_str());
		return false;
	}
	GroupAlias = aliases[0];
	
	return true;
}

// ----------------------------------------------------------------------------
void CMissionStepHandleRelease::onActivation(CMission* instance,uint32 stepIndex,std::list< CMissionEvent * > & eventList)
{
	vector<TDataSetRow> entities;
	instance->getEntities(entities);
	
	for (uint32 i = 0; i < entities.size(); ++i)
	{
		CCharacter *pChar = PlayerManager.getChar(entities[i]);
		if (pChar == NULL)
			continue;
		pChar->delHandledAIGroup(instance, GroupAlias);
	}
}

// ----------------------------------------------------------------------------
uint CMissionStepHandleRelease::processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
{
	if ( event.Type == CMissionEvent::GroupDespawned )
	{
		CMissionEventGroupDespawned & eventSpe = (CMissionEventGroupDespawned&)event;
		if (eventSpe.Alias == GroupAlias)
			return 1;
	}
	return 0;
}

// ----------------------------------------------------------------------------
void CMissionStepHandleRelease::getInitState( std::vector<uint32>& ret )
{
	ret.clear();
	ret.resize(1);
	ret[0] = 1;
}

// ----------------------------------------------------------------------------
void CMissionStepHandleRelease::getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
{
}

MISSION_REGISTER_STEP(CMissionStepHandleRelease, "handle_release")
