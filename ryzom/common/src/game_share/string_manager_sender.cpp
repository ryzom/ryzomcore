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
#include "string_manager_sender.h"
#include "nel/net/unified_network.h"
#include "nel/net/message.h"
#include "nel/net/service.h"
#include "nel/net/service.h"
#include "nel/misc/string_conversion.h"
#include "nel/misc/variable.h"
#include "utils.h"
#include "synchronised_message.h"

using namespace std;
using namespace NLMISC;

NLMISC::CVariable<bool>	DebugPhraseSender("debug", "DebugPhraseSender", "Send debug phrase message to the IOS", false, 0, true);

namespace STRING_MANAGER
{

	void TParam::serial(NLMISC::IStream &f)
	{
		serialParam(true, f, invalid_value);
	}

	bool TParam::serialParam(bool debug, NLMISC::IStream & f,TParamType	type)
	{
		bool ret = true;
		if (debug)
		{
			TParamType	checkType = NB_PARAM_TYPES;


			if (!f.isReading() && type != invalid_value)
			{
				nlassert(Type == type);
				checkType = Type;
			}

			f.serialEnum(checkType);

			if (f.isReading() && type != invalid_value)
			{
				if (type != checkType)
				{
					nlwarning("Serializing a string TParam : need a '%s', receive a '%s' !",
						paramTypeToString(type).c_str(),
						paramTypeToString(checkType).c_str());
					ret = false;
				}
				Type = checkType;
			}
		}
		else
		{
			if ( f.isReading() )
				Type = type;
		}

		switch (Type)
		{
		case item:
		case creature_model:
		case sphrase:
		case sbrick:
		case outpost:
			// sheetId
			f.serial(const_cast<NLMISC::CSheetId&>(SheetId));
			break;
		case creature:
		case guild:
		case player:
		case bot:
		case entity:
			// entity
			f.serial(const_cast<NLMISC::CEntityId&>(EId));
  			f.serial(_AIAlias);
			break;

		case place:
		case bot_name:
		case title:
		case event_faction:
			// identifier (string)
			f.serial(const_cast<std::string &>(Identifier));
			break;

		case skill:
		case role:
//		case career:
//		case job:
		case ecosystem:
		case race:
		case faction:
		case compass:
		case body_part:
		case score:
		case characteristic:
		case damage_type:
		case power_type:
		case classification_type:
		case clan:
			// enum
			f.serial(const_cast<uint32&>(Enum));
			break;
		case integer:
			// integer.
			f.serial(const_cast<sint32&>(Int));
			break;
		case time:
			// time
			f.serial(const_cast<uint32&>(Time));
			break;
		case money:
			f.serial(const_cast<uint64&>(Money));
			break;
		case string_id:
		case dyn_string_id:
			f.serial(const_cast<uint32&>(StringId));
			break;
		case literal:
			f.serial(Literal);
			break;
		case self:
			nlwarning("Invalid parameter type : can't be self");
			return false;
		default:
			nlwarning("Invalid parameter type : unknown (%d)", Type);
			return false;
		}
		return ret;
	}

	static uint32 SequenceCounter = 0;

	uint32 pickStringSerialNumber()
	{
		SequenceCounter = (SequenceCounter+1) & 0xffffff ;
		if (SequenceCounter == 0)
			SequenceCounter++;

		uint32 stringSeq = SequenceCounter++;

		stringSeq = (NLNET::IService::getInstance()->getServiceId().get() << 24) | (stringSeq & 0xffffff);

		return stringSeq;
	}


	/*
	 * Generic method used to send localised texts to clients ( whether they are ingame or in the character creation process )
	 * this method is inline and declared there so that it cannot be accessed from elsewhere.
	 * Use sendStringToClient or sendStringToUser
	 */
	inline uint32	sendString(NLNET::CMessage & msg, const std::string &stringName, const std::vector<TParam> &params, ISender *sender)
	{
		uint32 stringSeq = pickStringSerialNumber();

		msg.serial(stringSeq);
		msg.serial(const_cast<std::string&>(stringName));

		for (uint i=0; i<params.size(); ++i)
		{
			const TParam &param = params[i];
			if (!((TParam&)param).serialParam(DebugPhraseSender, msg, param.Type) )
				return 0xFFFFFFFF;
		}

		if (sender == NULL)
		{
			// send the message through network.
//			NLNET::CUnifiedNetwork::getInstance()->send("IOS", msg);
			sendMessageViaMirror("IOS", msg);
		}
		else
		{
			// send message with sender interface
			sender->send(msg, DebugPhraseSender);
		}

		return stringSeq;
	}

	//-----------------------------------------------
	// setPhrase
	//-----------------------------------------------
	void	setPhrase(const std::string &phraseName, const ucstring &phraseContent)
	{
		NLNET::CMessage msg("SET_PHRASE");
		std::string mutablePhraseName = phraseName;
		ucstring    mutablePhraseContent = phraseContent;
		msg.serial(mutablePhraseName);
		msg.serial(mutablePhraseContent);
		sendMessageViaMirror("IOS", msg);
	}

	//-----------------------------------------------
	// sendStringToClient
	//-----------------------------------------------
	uint32	sendStringToClient(const TDataSetRow &clientRowId, const std::string &stringName, const std::vector<TParam> &params, ISender *sender)
	{
		BOMB_IF(stringName.empty(), "Trying to send a phrase to IOS with an empty phraseName", return 0;);
		if (DebugPhraseSender)
		{
			NLNET::CMessage msg("PHRASE_DEBUG");
			msg.serial(const_cast<TDataSetRow &>(clientRowId));
			return sendString( msg,stringName,params,sender );
		}
		else
		{
			NLNET::CMessage msg("PHRASE");
			msg.serial(const_cast<TDataSetRow &>(clientRowId));
			return sendString( msg,stringName,params,sender );
		}
	}

	void	sendSystemStringToClientAudience(const TDataSetRow &clientRowId, const std::vector<NLMISC::CEntityId> &excluded, CChatGroup::TGroupType audience, const char * stringName, const std::vector<TParam> &params, ISender *sender)
	{
		BOMB_IF(strlen(stringName) == 0, "Trying to send a phrase to IOS with an empty stringName", return;);
		if (DebugPhraseSender)
		{
			NLNET::CMessage msg("BROADCAST_SYSTEM_PHRASE_DEBUG");
			msg.serial(const_cast<TDataSetRow &>(clientRowId));
			msg.serialCont(const_cast<vector<CEntityId> &>(excluded));
			msg.serialEnum(audience);
			sendString( msg,stringName,params,sender );
		}
		else
		{
			NLNET::CMessage msg("BROADCAST_SYSTEM_PHRASE");
			msg.serial(const_cast<TDataSetRow &>(clientRowId));
			msg.serialCont(const_cast<vector<CEntityId> &>(excluded));
			msg.serialEnum(audience);
			sendString( msg,stringName,params,sender );
		}
	}

	void	sendCustomEmoteTextToClientAudience(const TDataSetRow &clientRowId, const std::vector<NLMISC::CEntityId> &excluded,  const char * stringName, const std::vector<STRING_MANAGER::TParam> &params, ISender *sender)
	{
		BOMB_IF(strlen(stringName) == 0, "Trying to send a custom emot phrase to IOS with an empty stringName", return;);

		// send crowd emote message to IOS
		NLNET::CMessage	msgout2("EMOTE_CROWD");
		std::string stringNameStr(stringName);
		msgout2.serial( const_cast<TDataSetRow&>(clientRowId) );
		msgout2.serial(const_cast<string&>(stringNameStr));

		uint32 size = (uint32)params.size();
		msgout2.serial(size);
		for ( uint i = 0; i < size; i++ )
		{
			uint8 type8 = (uint8)params[i].Type;
			msgout2.serial(type8);
			const_cast<STRING_MANAGER::TParam&>(params[i]).serialParam( false, msgout2, params[i].Type );
		}
		msgout2.serialCont(const_cast<std::vector<NLMISC::CEntityId> &>(excluded));
		sendMessageViaMirror("IOS", msgout2);
	}



	//-----------------------------------------------
	// sendStringToUser
	//-----------------------------------------------
	uint32	sendStringToUser(uint32 userId, const std::string &stringName, const std::vector<TParam> &params, ISender *sender)
	{
		BOMB_IF(stringName.empty(), "Trying to send a phrase to IOS with an empty phraseName", return 0;);
		if (DebugPhraseSender)
		{
			NLNET::CMessage msg("PHRASE_USER_DEBUG");
			msg.serial(userId);
			return sendString( msg,stringName,params,sender );
		}
		else
		{
			NLNET::CMessage msg("PHRASE_USER");
			msg.serial(userId);
			return sendString( msg,stringName,params,sender );
		}
	}

	NL_BEGIN_STRING_CONVERSION_TABLE (TParamType)
		NL_STRING_CONVERSION_TABLE_ENTRY( item )
		NL_STRING_CONVERSION_TABLE_ENTRY( place )
		NL_STRING_CONVERSION_TABLE_ENTRY( creature )
		NL_STRING_CONVERSION_TABLE_ENTRY( skill )
		NL_STRING_CONVERSION_TABLE_ENTRY( role )
//		NL_STRING_CONVERSION_TABLE_ENTRY( career )
//		NL_STRING_CONVERSION_TABLE_ENTRY( job )
		NL_STRING_CONVERSION_TABLE_ENTRY( ecosystem )
		NL_STRING_CONVERSION_TABLE_ENTRY( race )
		NL_STRING_CONVERSION_TABLE_ENTRY( sbrick )
		NL_STRING_CONVERSION_TABLE_ENTRY( faction )
		NL_STRING_CONVERSION_TABLE_ENTRY( guild )
		NL_STRING_CONVERSION_TABLE_ENTRY( player )
		NL_STRING_CONVERSION_TABLE_ENTRY( bot )
		NL_STRING_CONVERSION_TABLE_ENTRY( integer )
		NL_STRING_CONVERSION_TABLE_ENTRY( time )
		NL_STRING_CONVERSION_TABLE_ENTRY( money )
		NL_STRING_CONVERSION_TABLE_ENTRY( compass )
		NL_STRING_CONVERSION_TABLE_ENTRY( string_id )
		NL_STRING_CONVERSION_TABLE_ENTRY( dyn_string_id )
		NL_STRING_CONVERSION_TABLE_ENTRY( self )
		NL_STRING_CONVERSION_TABLE_ENTRY( creature_model )
		NL_STRING_CONVERSION_TABLE_ENTRY( entity )
		NL_STRING_CONVERSION_TABLE_ENTRY( body_part )
		NL_STRING_CONVERSION_TABLE_ENTRY( score )
		NL_STRING_CONVERSION_TABLE_ENTRY( sphrase )
		NL_STRING_CONVERSION_TABLE_ENTRY( characteristic )
		NL_STRING_CONVERSION_TABLE_ENTRY( damage_type )
		NL_STRING_CONVERSION_TABLE_ENTRY( bot_name)
		NL_STRING_CONVERSION_TABLE_ENTRY( power_type )
		NL_STRING_CONVERSION_TABLE_ENTRY( literal )
		NL_STRING_CONVERSION_TABLE_ENTRY( title )
		NL_STRING_CONVERSION_TABLE_ENTRY( event_faction )
		NL_STRING_CONVERSION_TABLE_ENTRY( classification_type )
		NL_STRING_CONVERSION_TABLE_ENTRY( outpost )
		NL_STRING_CONVERSION_TABLE_ENTRY( clan )
	NL_END_STRING_CONVERSION_TABLE(TParamType, ParamTypeConversion, NB_PARAM_TYPES)

	//-----------------------------------------------
	// stringToParamType
	//-----------------------------------------------
	TParamType stringToParamType( const std::string & str )
	{
		return ParamTypeConversion.fromString( str );
	}

	//-----------------------------------------------
	// stringToParamType
	//-----------------------------------------------
	const std::string & paramTypeToString( TParamType type )
	{
		return ParamTypeConversion.toString( type );
	}


	void TParam::setEId(const NLMISC::CEntityId&	eId)
	{
		EId = eId;
		_AIAlias = 0;
	}

	NLMISC::CEntityId TParam::getEId() const
	{
		return EId;
	}

	TAIAlias TParam::getAIAlias() const
	{
		return _AIAlias;
	}

	void TParam::setAIAlias(TAIAlias aiAlias)
	{
		_AIAlias = aiAlias;
		EId = CEntityId::Unknown;
	}

	void TParam::setEIdAIAlias(const NLMISC::CEntityId&	eId, TAIAlias aiAlias)
	{
		_AIAlias = aiAlias;
		EId = eId;
	}


}
