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



#ifndef STRING_MANAGER_SENDER_H
#define STRING_MANAGER_SENDER_H

#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/ucstring.h"
#include "nel/net/message.h"
#include "base_types.h"
#include "chat_group.h"
#include "misc_const.h"

namespace STRING_MANAGER
{

	/// All the parameters types.
	enum TParamType
	{
		item,
		place,
		creature,
		skill,
		role,
		ecosystem,
		race,
		sbrick,
		faction,
		guild,
		player,
		bot,
		integer,
		time,
		money,
		compass,
		string_id,
		dyn_string_id,
		self,
		creature_model,
		entity,
		body_part,
		score,
		sphrase,
		characteristic,
		damage_type,
		bot_name,
		power_type,
		literal,
		title,
		event_faction,
		classification_type,
		outpost,
		clan,

		invalid_value,
		NB_PARAM_TYPES = invalid_value
	};

	struct TParam
	{
		TParamType			Type;


	private:
		/// All this properties should have been in an unamed union
		/// but CEntityId has a constructor, so it can't be
		/// in an union.
		/// So, I removed the union, leaving just the same basic types as union. :(
		NLMISC::CEntityId	EId;
		TAIAlias			_AIAlias;
	public:


		sint32				Int;
		union
		{
			uint32				Time;
			uint32				Enum;
			uint32				StringId;
		};
		uint64				Money;
		NLMISC::CSheetId	SheetId;
		std::string			Identifier;
		ucstring			Literal;

		/// Serial with format control embedded
		void serial(NLMISC::IStream &f);
		/// Serial with optional format control
		bool serialParam(bool debug, NLMISC::IStream & f,TParamType	type  = NB_PARAM_TYPES);

		TParam()
			: Type(invalid_value){}

		TParam(TParamType type)
			: Type(type){}
	private:
		TParam(TParamType type, const NLMISC::CEntityId &eid)
			: Type(type), EId(eid), _AIAlias(0)		{}
	public:
		TParam(TParamType type, sint32 i)
			: Type(type), Int(i)		{}
		TParam(TParamType type, uint32 i)
			: Type(type), Time(i)		{}
		TParam(TParamType type, uint64 money)
			: Type(type), Money(money)	{}
		TParam(TParamType type, NLMISC::CSheetId sheetId)
			: Type(type), SheetId(sheetId){}
		TParam(TParamType type, const std::string &ident)
			: Type(type), Identifier(ident)	{}
		TParam(TParamType type, const ucstring &literal)
			: Type(type), Literal(literal)	{}

		NLMISC::CEntityId getEId() const;

		void setEId(const NLMISC::CEntityId&	eId);

		TAIAlias getAIAlias() const;

		void setAIAlias(TAIAlias aiAlias);

		void setEIdAIAlias(const NLMISC::CEntityId&	eId, TAIAlias aiAlias);



	};

	/** Interface for user provided transport of the string message.
	 *	Used in IOS to bipass the network send of the message and
	 *	pass it directly to the string manager.
	 */
	class ISender
	{
	public:
		virtual  void send(NLNET::CMessage &message, bool debug) =0;
	};

	/** Send a string to a client thrue IOS.
	 *	This method create a message and send it to the IOS
	 *	service.
	 *	The return value is the string sequence number.
	 *	This number can then be send to the client via the
	 *	database mecanisme.
	 */
	uint32	sendStringToClient(const TDataSetRow &clientIndex, const std::string &stringName, const std::vector<TParam> &params, ISender *sender = NULL);

	/** Send a string to a client through IOS.
	 *	This method create a message and send it to the IOS
	 *	service.
	 *	The return value is the string sequence number.
	 *	This number can then be send to the client via the
	 *	database mecanisme.
	 */
	inline uint32	sendStringToClient(const TDataSetRow &clientIndex, const char * stringName, const std::vector<TParam> &params, ISender *sender = NULL)
	{
		std::string str(stringName);
		return sendStringToClient(clientIndex, str, params, sender);
	}

	/** Return (and 'consume') a dynamic string serial number. */
	uint32 pickStringSerialNumber();

	/** Send a emote text to the audience of a client/bot
	 *	This method create a message and send it to the IOS
	 *	service.
	 *	The IOS then broacast it to all the charactere in the audience of the
	 *	originator.
	 */
	void	sendCustomEmoteTextToClientAudience(const TDataSetRow &clientIndex, const std::vector<NLMISC::CEntityId> &excluded, const char * stringName, const std::vector<STRING_MANAGER::TParam> &params, ISender *sender = NULL);

		/** Send a string to the system chat of audience of a client
	 *	This method create a message and send it to the IOS
	 *	service.
	 *	The IOS then broacast it to all the charactere in the audience of the
	 *	originator.
	 */
	void	sendSystemStringToClientAudience(const TDataSetRow &clientIndex, const std::vector<NLMISC::CEntityId> &excluded, CChatGroup::TGroupType audience, const char * stringName, const std::vector<TParam> &params, ISender *sender = NULL);


	/** Send a string to a user (i.e.: a player that has not chosen a character yet) through IOS.
	 *	This method create a message and send it to the IOS
	 *	service.
	 *	The return value is the string sequence number.
	 *	This number can then be send to the client via the
	 *	database mecanisme.
	 */
	uint32	sendStringToUser(uint32 userId, const std::string &stringName, const std::vector<TParam> &params, ISender *sender = NULL);

	/** Send a string to a user (i.e.: a player that has not chosen a character yet) through IOS.
	 *	This method create a message and send it to the IOS
	 *	service.
	 *	The return value is the string sequence number.
	 *	This number can then be send to the client via the
	 *	database mecanisme.
	 */
	inline uint32	sendStringToUser(uint32 userId, const char * stringName, const std::vector<TParam> &params, ISender *sender = NULL)
	{
		std::string str(stringName);
		return sendStringToUser(userId, str, params, sender);
	}

	/**
	 * Convert a string to a TParamType enum
	 */
	TParamType stringToParamType( const std::string & str );

	/**
	 * Convert a TParamType enum to a string
	 */
	const std::string & paramTypeToString( TParamType type );

	// Set a phrase in IOS for all languages
	void	setPhrase(const std::string &phraseName, const ucstring &phraseContent);

	// A vector that check size
	class CVectorParamCheck : public std::vector<STRING_MANAGER::TParam>
	{
	public:
		explicit CVectorParamCheck() : std::vector<STRING_MANAGER::TParam>() {}
		explicit CVectorParamCheck(size_type s) : std::vector<STRING_MANAGER::TParam>(s) {}
		CVectorParamCheck(const std::vector<STRING_MANAGER::TParam> &o) : std::vector<STRING_MANAGER::TParam>(o) {}
		template <class InputIterator>
		CVectorParamCheck(InputIterator first, InputIterator last) : std::vector<STRING_MANAGER::TParam>(first, last) {}

		std::vector<STRING_MANAGER::TParam>::reference operator[](size_type pos)
		{
			nlassert(pos<size());
			return ((std::vector<STRING_MANAGER::TParam>&)(*this))[pos];
		}

		std::vector<STRING_MANAGER::TParam>::const_reference operator[](size_type pos) const
		{
			nlassert(pos<size());
			return ((const std::vector<STRING_MANAGER::TParam>&)(*this))[pos];
		}
	};

#define SM_STATIC_PARAMS_1(__vectorName, __paramType1)	\
	static STRING_MANAGER::TParam	__vectorName##Params[1] = \
	{\
		STRING_MANAGER::TParam(__paramType1)\
	};\
	static STRING_MANAGER::CVectorParamCheck	__vectorName(__vectorName##Params, __vectorName##Params+1)

#define SM_STATIC_PARAMS_2(__vectorName, __paramType1, __paramType2)	\
	static STRING_MANAGER::TParam	__vectorName##Params[2] = \
	{\
		STRING_MANAGER::TParam(__paramType1),\
		STRING_MANAGER::TParam(__paramType2)\
	};\
	static STRING_MANAGER::CVectorParamCheck	__vectorName(__vectorName##Params, __vectorName##Params+2)

#define SM_STATIC_PARAMS_3(__vectorName, __paramType1, __paramType2, __paramType3)	\
	static STRING_MANAGER::TParam	__vectorName##Params[3] = \
	{\
		STRING_MANAGER::TParam(__paramType1),\
		STRING_MANAGER::TParam(__paramType2),\
		STRING_MANAGER::TParam(__paramType3)\
	};\
	static STRING_MANAGER::CVectorParamCheck	__vectorName(__vectorName##Params, __vectorName##Params+3)

#define SM_STATIC_PARAMS_4(__vectorName, __paramType1, __paramType2, __paramType3, __paramType4)	\
	static STRING_MANAGER::TParam	__vectorName##Params[4] = \
	{\
		STRING_MANAGER::TParam(__paramType1),\
		STRING_MANAGER::TParam(__paramType2),\
		STRING_MANAGER::TParam(__paramType3),\
		STRING_MANAGER::TParam(__paramType4)\
	};\
	static STRING_MANAGER::CVectorParamCheck	__vectorName(__vectorName##Params, __vectorName##Params+4)

#define SM_STATIC_PARAMS_5(__vectorName, __paramType1, __paramType2, __paramType3, __paramType4, __paramType5)	\
	static STRING_MANAGER::TParam	__vectorName##Params[5] = \
	{\
		STRING_MANAGER::TParam(__paramType1),\
		STRING_MANAGER::TParam(__paramType2),\
		STRING_MANAGER::TParam(__paramType3),\
		STRING_MANAGER::TParam(__paramType4),\
		STRING_MANAGER::TParam(__paramType5)\
	};\
	static STRING_MANAGER::CVectorParamCheck	__vectorName(__vectorName##Params, __vectorName##Params+5)

#define SM_STATIC_PARAMS_6(__vectorName, __paramType1, __paramType2, __paramType3, __paramType4, __paramType5, __paramType6)	\
	static STRING_MANAGER::TParam	__vectorName##Params[6] = \
	{\
	STRING_MANAGER::TParam(__paramType1),\
	STRING_MANAGER::TParam(__paramType2),\
	STRING_MANAGER::TParam(__paramType3),\
	STRING_MANAGER::TParam(__paramType4),\
	STRING_MANAGER::TParam(__paramType5),\
	STRING_MANAGER::TParam(__paramType6)\
	};\
	static STRING_MANAGER::CVectorParamCheck	__vectorName(__vectorName##Params, __vectorName##Params+6)

}; // namespace STRING_MANAGER

typedef STRING_MANAGER::CVectorParamCheck	TVectorParamCheck;

#endif //STRING_MANAGER_SENDER_H
