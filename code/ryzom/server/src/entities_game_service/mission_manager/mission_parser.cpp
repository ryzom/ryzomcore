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
#include "mission_manager/mission_parser.h"
#include "mission_log.h"

#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "shop_type/shop_type_manager.h"

using namespace std;
using namespace NLMISC;

bool CMissionParser::solveTextsParams(uint32 lineNum, TVectorParamCheck & txtParams, CMissionSpecificParsingData & missionData  )
{
	if (  txtParams.empty() )
		return true;
	std::vector< std::pair< std::string, STRING_MANAGER::TParamType> > & paramTypes = missionData.ChatParams;
	bool ret = true;
	for ( uint i = 0; i < txtParams.size(); i++ )
	{
		std::map< std::string, uint >::iterator itInt = missionData.Integers.find( txtParams[i].Identifier );
		if ( itInt != missionData.Integers.end() )
		{
			txtParams[i].Identifier.clear();
			txtParams[i].Type = STRING_MANAGER::integer;
			txtParams[i].Int = (*itInt).second;
		}
		// if it is a bot name param
		else if ( txtParams[i].Identifier.size() > 2 && txtParams[i].Identifier[0] =='\"' &&  txtParams[i].Identifier[ txtParams[i].Identifier.size() - 1] =='\"' )
		{
			// remove quotes
			txtParams[i].Identifier.erase(txtParams[i].Identifier.begin());
			//txtParams[i].Identifier.pop_back();
			txtParams[i].Identifier.resize(txtParams[i].Identifier.size()-1);
			txtParams[i].Type = STRING_MANAGER::bot_name;
		}
		else
		{
			uint j = 0;
			for (; j < paramTypes.size(); j++ )
			{
				if ( txtParams[i].Identifier == paramTypes[j].first )
				{
					txtParams[i].Identifier.clear();
					if ( !fillTextParam( txtParams[i],paramTypes[j].first,paramTypes[j].second ) )
					{
						MISLOG("<MISSIONS> at line %u: param '%s' could not be parsed", lineNum, paramTypes[j].first.c_str());
						ret = false;
					}
					break;
				}
			}
			if ( j == paramTypes.size() )
			{
				txtParams[i].Type = STRING_MANAGER::integer;
				NLMISC::fromString( txtParams[i].Identifier, txtParams[i].Int);
				if ( txtParams[i].Int == 0 && txtParams[i].Identifier != "0" )
				{
					MISLOG("<MISSIONS> at line %u: param '%s' has an unknown type", lineNum, txtParams[i].Identifier.c_str());
					ret = false;
				}
				txtParams[i].Identifier.clear();
			}
		}
	}
	return ret;
}// CMissionTemplate::solveTextsParams

void CMissionParser::solvePlayerName( TVectorParamCheck & params , const TDataSetRow & playerRow )
{
	CCharacter * user = PlayerManager.getChar(playerRow);
	if( !user )
	{
		nlwarning("<CMissionParser solvePlayerName>Invalid user %u",playerRow.getIndex());
		return;
	}
	for ( uint i = 0; i< params.size(); i++)
	{
		if ( params[i].Type == STRING_MANAGER::player  )
		{
			params[i].setEIdAIAlias( user->getId(), CAIAliasTranslator::getInstance()->getAIAlias(user->getId()) );
		}
	}
}

void CMissionParser::solveEntitiesNames( TVectorParamCheck & params , const TDataSetRow & playerRow,const NLMISC::CEntityId& giver )
{
	CCharacter * user = PlayerManager.getChar(playerRow);
	if( !user )
	{
		nlwarning("<CMissionParser solveEntitiesNames>Invalid user %u",playerRow.getIndex());
		return;
	}
	for ( uint i = 0; i< params.size(); i++)
	{
		if ( params[i].Type == STRING_MANAGER::player  )
		{
			params[i].setEIdAIAlias( user->getId(), CAIAliasTranslator::getInstance()->getAIAlias(user->getId()) );
		}
		else if ( params[i].Type == STRING_MANAGER::bot  )
		{
			if ( params[i].getEId() == NLMISC::CEntityId::Unknown )
			{
				if ( params[i].Identifier == "giver" )
				{
					params[i].setEIdAIAlias( giver, CAIAliasTranslator::getInstance()->getAIAlias(giver) );
				}
				else
				{
					params[i].setEIdAIAlias( CAIAliasTranslator::getInstance()->getEntityId( params[i].Int ), params[i].Int );
				}
			}
		}
		else if ( params[i].Type == STRING_MANAGER::string_id && params[i].Identifier == "$guild_name$" )
		{
			// resolve the player guild name
			/// todo guild mission
			/*
			CGuild *g = user->getGuild();
			if (g == NULL)
			{
				nlwarning("<CMissionParser solveEntitiesNames>No guild for user %u",playerRow.getIndex());
				return;
			}
			params[i].StringId = g->getNameId();
			*/
		}
	}
}


bool CMissionParser::fillTextParam( STRING_MANAGER::TParam & param ,const std::string & value, STRING_MANAGER::TParamType type )
{
	param.Type = type;
	switch ( type )
	{
	case STRING_MANAGER::item:
		param.SheetId = CSheetId( value +".sitem" );
		if ( param.SheetId == CSheetId::Unknown )
		{
			MISLOG( "<MISSIONS> Invalid item '%s'", value.c_str() );
			return false;
		}
		return true;
	case STRING_MANAGER::place:
		param.Identifier = value;
		return true;
	case STRING_MANAGER::creature_model:
		param.SheetId = CSheetId( value +".creature" );
		if ( param.SheetId == CSheetId::Unknown )
		{
			MISLOG( "<MISSIONS> Invalid creature '%s'", value.c_str() );
			return false;
		}
		return true;
		
	case STRING_MANAGER::skill:
		param.Enum = (uint)SKILLS::toSkill( value );
		if ( param.Enum == (uint) SKILLS::unknown )
		{
			MISLOG( "<MISSIONS> Invalid skill '%s'", value.c_str() );
			return false;
		}
		return true;
	case STRING_MANAGER::race:
		param.Enum = (uint) EGSPD::CPeople::fromString( value );
		if ( param.Enum == (uint)EGSPD::CPeople::EndPeople )
		{
			MISLOG( "<MISSIONS> Invalid race '%s'", value.c_str() );
			return false;
		}
		return true;
	case STRING_MANAGER::bot:
	case STRING_MANAGER::creature:
		if ( value == "giver" )
		{
			param.Identifier = value;
		}
		else
		{
			// get the first matching bot
			vector<TAIAlias> aliases;
			CAIAliasTranslator::getInstance()->getNPCAliasesFromName( value , aliases );
			if ( aliases.empty() )
			{
				MISLOG( "<MISSIONS> Invalid bot '%s'", value.c_str() );
				return false;
			}
			param.Int = aliases[0];
		}
		return true;
	case STRING_MANAGER::sbrick: 
		param.SheetId = CSheetId(value + ".sbrick"); 
		if ( param.SheetId == CSheetId::Unknown ) 
		{ 
			MISLOG( "<MISSIONS> Invalid sbrick '%s'", value.c_str() ); 
			return false; 
		} 
		return true; 
	case STRING_MANAGER::sphrase: 
		param.SheetId = CSheetId(value + ".sphrase"); 
		if ( param.SheetId == CSheetId::Unknown ) 
		{ 
			MISLOG( "<MISSIONS> Invalid sphrase '%s'", value.c_str() ); 
			return false; 
		} 
		return true; 
	case STRING_MANAGER::player:
		return true;//nothing to do here
	case STRING_MANAGER::faction:
		param.Enum = CStaticFames::getInstance().getFactionIndex(value);
		if (param.Enum == CStaticFames::INVALID_FACTION_INDEX)
		{
			MISLOG( "<MISSIONS> Invalid faction '%s'", value.c_str() );
			return false;
		}
		return true;
	case STRING_MANAGER::string_id:
		if (value == "guild_name")
		{
			// tag the identifier for later use (when instantiating mission)
			param.Identifier = "$guild_name$";
			return true;
		}
		else
		{
			MISLOG( "<MISSIONS> Unsupported string_id parameter '%s'", value.c_str() );
			return false;
		}
	}
	MISLOG( "<MISSIONS> unsupported param type '%s'", STRING_MANAGER::paramTypeToString(type).c_str() );
	return false;
} // CMissionTemplate::fillTextParam


void CMissionParser::removeBlanks(std::string & str)
{
	if ( str.empty() )
		return;
	sint pos = -1;
	for ( uint i = 0; i < str.size() && (str[i] == ' ' || str[i] == '\t'); i++ )
		pos = (sint)i;
	
	if ( pos >= 0)
		str.erase(0,pos + 1);
	
	if ( str.empty() )
		return;
	
	pos = -1;
	for ( sint i = (sint) str.size() - 1 ; i >= 0  && (str[i] == ' ' || str[i] == '\t'); i-- )
		pos = i;
	
	if ( pos == -1 )
		return;
	str.erase(pos);
}// CMissionTemplate removeBlanks

std::string CMissionParser::getNoBlankString( const std::string & str)
{
	string ret = str;
	removeBlanks(ret);
	return ret;
}// CMissionTemplate removeBlanks

bool CMissionParser::parseParamText(uint32 line, const std::string & script,std::string & textId, TVectorParamCheck & textParams )
{
	vector< string > args;
	splitString(script,";",args);
	textId = CMissionParser::getNoBlankString(args[0]);
	if( textId.find_first_of(";:\t ,") != string::npos )
	{
		MISLOG("<MISSIONS> line %u: invalid text %s", line, textId.c_str() );
		return false;
	}
	textParams.resize( args.size() - 1 );
	for ( uint i = 1; i < args.size(); i++ )
	{
		CMissionParser::removeBlanks(args[i]);
		textParams[i-1].Identifier = args[i];
	}
	return true;
}


bool CMissionParser::parseStringList(uint32 line, const std::string & separator, const std::vector< std::string > & preparsedParams, std::vector< std::string > & ret )
{
	if ( preparsedParams.size() != 2 )
	{
		if ( !preparsedParams.empty())
			MISLOG("<MISSIONS> line %u: syntax error usage : '%s:<string>*[%s<string>]'",line, preparsedParams[0].c_str(),separator.c_str());
		else
			MISLOG("<MISSIONS> line %u: syntax error usage : ':<string>*[%s<string>]'",line, separator.c_str());
		return false;
	}
	else
	{
		NLMISC::splitString(preparsedParams[1],separator,ret);
		for ( uint i = 0; i < ret.size(); i++ )
		{
			CMissionParser::removeBlanks( ret[i] );
		}
		return true;
	}
}// CMissionParser parseStringList

void CMissionParser::tokenizeString(const std::string &str, const std::string &separators, std::vector<std::string> &retList)
{
	retList.clear();
	if ( str.empty() )
		return;
	std::string::size_type	pos=0;
	std::string::size_type	newPos=0;
	while( (newPos= str.find_first_of(separators,pos)) != string::npos)
	{
		// if not empty sub str. (skip repetition of separator )
		if(newPos-pos>0)
		{
			string ret = str.substr(pos, newPos-pos);
			retList.push_back(ret);
		}
		// skip token
		pos= newPos+1;
	}
	// copy the last substr
	if( pos<(uint)str.size() )
		retList.push_back(str.substr(pos, str.size()-pos));	
}

bool CMissionParser::parseBotName(const std::string & botName,TAIAlias & aliasRet,CMissionSpecificParsingData & data)
{
	aliasRet = CAIAliasTranslator::Invalid;
	string name = CMissionParser::getNoBlankString( botName );
	if ( name == "giver" )
		return true;

	// get the first matching bot
	vector<TAIAlias> aliases;
	CAIAliasTranslator::getInstance()->getNPCAliasesFromName( name , aliases );
	if ( aliases.empty() )
	{
		MISLOG( "<parseBotName> Invalid bot %s",name.c_str() );
		return false;
	}
	aliasRet = aliases[0];
	data.ChatParams.push_back( std::make_pair( name,STRING_MANAGER::bot ) );
	return true;
}

bool CMissionParser::addItemPrice(uint32 line, const vector<string>& args, uint & amount )
{
	if ( args.size() != 3 )
	{
		MISLOG("<MISSIONS> line %u: syntax error : recv_money:<money> OR %s:<item><quality><factor> *[;<item><quality><factor>]", line);
		return false;
	}
	bool ret = true;
	string sheetName = CMissionParser::getNoBlankString( args[0] ) + ".sitem";
	CSheetId sheet( sheetName );
	
	if ( sheet == CSheetId::Unknown )
	{
		MISLOG("<MISSIONS> line %u: syntax error sheetId '%s' is unknon", line, sheetName.c_str() ) ;
		ret = false;
	}
	uint16 quality;
	NLMISC::fromString(args[1], quality);
	if ( quality == 0 )
	{
		MISLOG("<MISSIONS> line %u: syntax error quality is 0 ('%s')", line, args[1].c_str() ) ;
		ret = false;
	}
	float factor = (float) atof( args[2].c_str() );
	amount += uint32 ( factor * CShopTypeManager::computeBasePrice( sheet, quality ) );
	return ret;
}

