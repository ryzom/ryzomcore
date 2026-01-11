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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef CHARACTER_SYNC_ITF
#define CHARACTER_SYNC_ITF
#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "nel/misc/entity_id.h"

#include "game_share/r2_basic_types.h"

#include "continent.h"

namespace CHARSYNC
{

	class TCharInfo;

	class TCharBestLevelInfo;

	class CValidateNameResult;

	class CGuildInfo;

	class TNameEntry;

	class TCharSyncResultEntry;



	struct TRace
	{
		enum TValues
		{
			r_fyros,
			r_matis,
			r_tryker,
			r_zorai,
			/// the highest valid value in the enum
			last_enum_item = r_zorai,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 4
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(r_fyros, 0));
				indexTable.insert(std::make_pair(r_matis, 1));
				indexTable.insert(std::make_pair(r_tryker, 2));
				indexTable.insert(std::make_pair(r_zorai, 3));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(r_fyros)
				NL_STRING_CONVERSION_TABLE_ENTRY(r_matis)
				NL_STRING_CONVERSION_TABLE_ENTRY(r_tryker)
				NL_STRING_CONVERSION_TABLE_ENTRY(r_zorai)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TRace()
			: _Value(invalid_val)
		{
		}
		TRace(TValues value)
			: _Value(value)
		{
		}

		TRace(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TRace &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TRace &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TRace &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TRace &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TRace &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TRace &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TCivilisation
	{
		enum TValues
		{
			c_neutral,
			c_fyros,
			c_matis,
			c_tryker,
			c_zorai,
			/// the highest valid value in the enum
			last_enum_item = c_zorai,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 5
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(c_neutral, 0));
				indexTable.insert(std::make_pair(c_fyros, 1));
				indexTable.insert(std::make_pair(c_matis, 2));
				indexTable.insert(std::make_pair(c_tryker, 3));
				indexTable.insert(std::make_pair(c_zorai, 4));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(c_neutral)
				NL_STRING_CONVERSION_TABLE_ENTRY(c_fyros)
				NL_STRING_CONVERSION_TABLE_ENTRY(c_matis)
				NL_STRING_CONVERSION_TABLE_ENTRY(c_tryker)
				NL_STRING_CONVERSION_TABLE_ENTRY(c_zorai)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TCivilisation()
			: _Value(invalid_val)
		{
		}
		TCivilisation(TValues value)
			: _Value(value)
		{
		}

		TCivilisation(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TCivilisation &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TCivilisation &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TCivilisation &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TCivilisation &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TCivilisation &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TCivilisation &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TCult
	{
		enum TValues
		{
			c_neutral,
			c_kami,
			c_karavan,
			/// the highest valid value in the enum
			last_enum_item = c_karavan,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 3
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(c_neutral, 0));
				indexTable.insert(std::make_pair(c_kami, 1));
				indexTable.insert(std::make_pair(c_karavan, 2));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(c_neutral)
				NL_STRING_CONVERSION_TABLE_ENTRY(c_kami)
				NL_STRING_CONVERSION_TABLE_ENTRY(c_karavan)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TCult()
			: _Value(invalid_val)
		{
		}
		TCult(TValues value)
			: _Value(value)
		{
		}

		TCult(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TCult &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TCult &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TCult &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TCult &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TCult &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TCult &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};
		// Info about a character, used for block tranfert
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TCharInfo
	{
	protected:
		// The entity id of the character
		NLMISC::CEntityId	_CharEId;
		// The name of the character
		std::string	_CharName;
		// The session id of the character home mainland
		uint32	_HomeSessionId;
		// The best combat level of the character
		sint32	_BestCombatLevel;
		// The guild id of this player
		uint32	_GuildId;
		// the list of respawn points validated by the character
		CONTINENT::TRespawnPointCounters	_RespawnPoints;
		//
		TRace	_Race;
		//
		TCivilisation	_Civilisation;
		//
		TCult	_Cult;
		//
		bool	_Newcomer;
	public:
		// The entity id of the character
		const NLMISC::CEntityId &getCharEId() const
		{
			return _CharEId;
		}

		NLMISC::CEntityId &getCharEId()
		{
			return _CharEId;
		}


		void setCharEId(const NLMISC::CEntityId &value)
		{


				_CharEId = value;


		}
			// The name of the character
		const std::string &getCharName() const
		{
			return _CharName;
		}

		std::string &getCharName()
		{
			return _CharName;
		}


		void setCharName(const std::string &value)
		{


				_CharName = value;


		}
			// The session id of the character home mainland
		uint32 getHomeSessionId() const
		{
			return _HomeSessionId;
		}

		void setHomeSessionId(uint32 value)
		{

				_HomeSessionId = value;

		}
			// The best combat level of the character
		sint32 getBestCombatLevel() const
		{
			return _BestCombatLevel;
		}

		void setBestCombatLevel(sint32 value)
		{

				_BestCombatLevel = value;

		}
			// The guild id of this player
		uint32 getGuildId() const
		{
			return _GuildId;
		}

		void setGuildId(uint32 value)
		{

				_GuildId = value;

		}
			// the list of respawn points validated by the character
		const CONTINENT::TRespawnPointCounters &getRespawnPoints() const
		{
			return _RespawnPoints;
		}

		CONTINENT::TRespawnPointCounters &getRespawnPoints()
		{
			return _RespawnPoints;
		}


		void setRespawnPoints(const CONTINENT::TRespawnPointCounters &value)
		{


				_RespawnPoints = value;


		}
			//
		TRace getRace() const
		{
			return _Race;
		}

		void setRace(TRace value)
		{

				_Race = value;

		}
			//
		TCivilisation getCivilisation() const
		{
			return _Civilisation;
		}

		void setCivilisation(TCivilisation value)
		{

				_Civilisation = value;

		}
			//
		TCult getCult() const
		{
			return _Cult;
		}

		void setCult(TCult value)
		{

				_Cult = value;

		}
			//
		bool getNewcomer() const
		{
			return _Newcomer;
		}

		void setNewcomer(bool value)
		{

				_Newcomer = value;

		}

		bool operator == (const TCharInfo &other) const
		{
			return _CharEId == other._CharEId
				&& _CharName == other._CharName
				&& _HomeSessionId == other._HomeSessionId
				&& _BestCombatLevel == other._BestCombatLevel
				&& _GuildId == other._GuildId
				&& _RespawnPoints == other._RespawnPoints
				&& _Race == other._Race
				&& _Civilisation == other._Civilisation
				&& _Cult == other._Cult
				&& _Newcomer == other._Newcomer;
		}


		// constructor
		TCharInfo()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_CharEId);
			s.serial(_CharName);
			s.serial(_HomeSessionId);
			s.serial(_BestCombatLevel);
			s.serial(_GuildId);
			s.serialCont(_RespawnPoints);
			s.serial(_Race);
			s.serial(_Civilisation);
			s.serial(_Cult);
			s.serial(_Newcomer);

		}


	private:


	};


		// Info about a the best level of a character, used for block tranfert
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TCharBestLevelInfo
	{
	protected:
		// The entity id of the character
		NLMISC::CEntityId	_CharEId;
		// The best combat level of the character
		sint32	_BestCombatLevel;
	public:
		// The entity id of the character
		const NLMISC::CEntityId &getCharEId() const
		{
			return _CharEId;
		}

		NLMISC::CEntityId &getCharEId()
		{
			return _CharEId;
		}


		void setCharEId(const NLMISC::CEntityId &value)
		{


				_CharEId = value;


		}
			// The best combat level of the character
		sint32 getBestCombatLevel() const
		{
			return _BestCombatLevel;
		}

		void setBestCombatLevel(sint32 value)
		{

				_BestCombatLevel = value;

		}

		bool operator == (const TCharBestLevelInfo &other) const
		{
			return _CharEId == other._CharEId
				&& _BestCombatLevel == other._BestCombatLevel;
		}


		// constructor
		TCharBestLevelInfo()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_CharEId);
			s.serial(_BestCombatLevel);

		}


	private:


	};



	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CCharacterSyncSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CCharacterSyncSkel>	TInterceptor;
	protected:
		CCharacterSyncSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CCharacterSyncSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CCharacterSyncSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void addCharacter_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void deleteCharacter_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateCharGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateCharRespawnPoints_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateCharsBestLevel_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateCharNewbieFlag_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateCharAllegiance_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateCharHomeMainlandSessionId_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void syncUserChars_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CCharacterSyncSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// A new character have been create by a client
		virtual void addCharacter(NLNET::IModuleProxy *sender, const TCharInfo &charInfo) =0;
		// A character have been deleted
		virtual void deleteCharacter(NLNET::IModuleProxy *sender, uint32 charId) =0;
		// A character guild have changed
		virtual void updateCharGuild(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, uint32 guildId) =0;
		// Update the respawn points count of a character
		virtual void updateCharRespawnPoints(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, const CONTINENT::TRespawnPointCounters &respawnPoints) =0;
		// Update the best level for a set of characters
		virtual void updateCharsBestLevel(NLNET::IModuleProxy *sender, const std::vector < TCharBestLevelInfo > &charLevelInfos) =0;
		// Update the newbie flag of a characters
		virtual void updateCharNewbieFlag(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, bool newbie) =0;
		// Update the allegiance of a characters
		virtual void updateCharAllegiance(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, TCivilisation civilisation, TCult cult) =0;
		// The home mainland has changed (used when converting a character file from an old version)
		virtual void updateCharHomeMainlandSessionId(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, TSessionId homeMainlandSessionId) =0;
		// The characters for a player have been loaded
		// EGS send the full list to SU to make
		// sure any divergence in the database is cleared
		// SU send back the list of character with there
		// unified names and home session ID
		virtual void syncUserChars(NLNET::IModuleProxy *sender, uint32 userId, const std::vector < TCharInfo > &charInfos) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CCharacterSyncProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CCharacterSyncSkel	*_LocalModuleSkel;


	public:
		CCharacterSyncProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CCharacterSyncSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CCharacterSyncProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// A new character have been create by a client
		void addCharacter(NLNET::IModule *sender, const TCharInfo &charInfo);
		// A character have been deleted
		void deleteCharacter(NLNET::IModule *sender, uint32 charId);
		// A character guild have changed
		void updateCharGuild(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, uint32 guildId);
		// Update the respawn points count of a character
		void updateCharRespawnPoints(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, const CONTINENT::TRespawnPointCounters &respawnPoints);
		// Update the best level for a set of characters
		void updateCharsBestLevel(NLNET::IModule *sender, const std::vector < TCharBestLevelInfo > &charLevelInfos);
		// Update the newbie flag of a characters
		void updateCharNewbieFlag(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, bool newbie);
		// Update the allegiance of a characters
		void updateCharAllegiance(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, TCivilisation civilisation, TCult cult);
		// The home mainland has changed (used when converting a character file from an old version)
		void updateCharHomeMainlandSessionId(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, TSessionId homeMainlandSessionId);
		// The characters for a player have been loaded
		// EGS send the full list to SU to make
		// sure any divergence in the database is cleared
		// SU send back the list of character with there
		// unified names and home session ID
		void syncUserChars(NLNET::IModule *sender, uint32 userId, const std::vector < TCharInfo > &charInfos);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_addCharacter(NLNET::CMessage &__message, const TCharInfo &charInfo);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_deleteCharacter(NLNET::CMessage &__message, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateCharGuild(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, uint32 guildId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateCharRespawnPoints(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, const CONTINENT::TRespawnPointCounters &respawnPoints);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateCharsBestLevel(NLNET::CMessage &__message, const std::vector < TCharBestLevelInfo > &charLevelInfos);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateCharNewbieFlag(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, bool newbie);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateCharAllegiance(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, TCivilisation civilisation, TCult cult);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateCharHomeMainlandSessionId(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, TSessionId homeMainlandSessionId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_syncUserChars(NLNET::CMessage &__message, uint32 userId, const std::vector < TCharInfo > &charInfos);




	};


	struct TCharacterNameResult
	{
		enum TValues
		{
			cnr_ok,
			cnr_invalid_name,
			cnr_already_exist,
			/// the highest valid value in the enum
			last_enum_item = cnr_already_exist,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 3
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(cnr_ok, 0));
				indexTable.insert(std::make_pair(cnr_invalid_name, 1));
				indexTable.insert(std::make_pair(cnr_already_exist, 2));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(cnr_ok)
				NL_STRING_CONVERSION_TABLE_ENTRY(cnr_invalid_name)
				NL_STRING_CONVERSION_TABLE_ENTRY(cnr_already_exist)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TCharacterNameResult()
			: _Value(invalid_val)
		{
		}
		TCharacterNameResult(TValues value)
			: _Value(value)
		{
		}

		TCharacterNameResult(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TCharacterNameResult &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TCharacterNameResult &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TCharacterNameResult &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TCharacterNameResult &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TCharacterNameResult &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TCharacterNameResult &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};
		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CValidateNameResult
	{
	protected:
		//
		TCharacterNameResult	_Result;
		//
		uint32	_UserId;
		//
		uint8	_CharIndex;
		//
		ucstring	_FullName;
	public:
		//
		TCharacterNameResult getResult() const
		{
			return _Result;
		}

		void setResult(TCharacterNameResult value)
		{
				_Result = value;
		}
			//
		uint32 getUserId() const
		{
			return _UserId;
		}

		void setUserId(uint32 value)
		{
				_UserId = value;
		}
			//
		uint8 getCharIndex() const
		{
			return _CharIndex;
		}

		void setCharIndex(uint8 value)
		{
				_CharIndex = value;
		}
			//
		const ucstring& getFullName() const
		{
			return _FullName;
		}

		void setFullName(const ucstring &value)
		{
				_FullName = value;
		}

		bool operator == (const CValidateNameResult &other) const
		{
			return _Result == other._Result
				&& _UserId == other._UserId
				&& _CharIndex == other._CharIndex
				&& _FullName == other._FullName;
		}


		// constructor
		CValidateNameResult()
		{
		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_Result);
			s.serial(_UserId);
			s.serial(_CharIndex);
			s.serial(_FullName);
		}


	private:


	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CGuildInfo
	{
	protected:
		//
		ucstring	_GuildName;
		//
		uint32	_GuildId;
	public:
		//
		const ucstring &getGuildName() const
		{
			return _GuildName;
		}

		ucstring &getGuildName()
		{
			return _GuildName;
		}


		void setGuildName(const ucstring &value)
		{


				_GuildName = value;


		}
			//
		uint32 getGuildId() const
		{
			return _GuildId;
		}

		void setGuildId(uint32 value)
		{

				_GuildId = value;

		}

		bool operator == (const CGuildInfo &other) const
		{
			return _GuildName == other._GuildName
				&& _GuildId == other._GuildId;
		}


		// constructor
		CGuildInfo()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_GuildName);
			s.serial(_GuildId);

		}


	private:


	};



	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CNameUnifierSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CNameUnifierSkel>	TInterceptor;
	protected:
		CNameUnifierSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CNameUnifierSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CNameUnifierSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void registerNameUnifierClient_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void validateCharacterName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void assignNameToCharacter_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void renameCharacter_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void registerLoadedGuildNames_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void validateGuildName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void addGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void removeGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CNameUnifierSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// EGS register it's name unifier in order to receive
		// an updated eid to name translation table
		virtual void registerNameUnifierClient(NLNET::IModuleProxy *sender) =0;
		// EGS ask to validate a character name
		// If the NU valide the name, it temporary
		// lock it to the associated player.
		// This function is called before character creation.
		virtual void validateCharacterName(NLNET::IModuleProxy *sender, uint32 userId, uint8 charIndex, const std::string &name, uint32 homeMainlandSessionId) =0;
		// EGS ask to assign a name to a character
		// This function is called during character creation
		virtual void assignNameToCharacter(NLNET::IModuleProxy *sender, uint32 charId, const std::string &name, uint32 homeSessionId) =0;
		// EGS ask to rename a character.
		// Renaming consist of assigning a default ramdomly generated name to the character
		virtual void renameCharacter(NLNET::IModuleProxy *sender, uint32 charId) =0;
		// EGS send info about the list of loaded guild.
		// The name unifier will update is internal name table if needed
		// and rename any guild having a conflicting name.
		// If any guild is renamed, then the name unifier send back
		// a guildRenamed message to EGS.
		virtual void registerLoadedGuildNames(NLNET::IModuleProxy *sender, uint32 chardId, const std::vector < CGuildInfo > &guildInfos) =0;
		// EGS ask to the name unifier to validate a new guild name
		virtual void validateGuildName(NLNET::IModuleProxy *sender, uint32 guildId, const ucstring &guildName) =0;
		// EGS add newly created guild info
		virtual void addGuild(NLNET::IModuleProxy *sender, uint32 shardId, uint32 guildId, const ucstring &guildName) =0;
		// EGS remove deleted guild info
		virtual void removeGuild(NLNET::IModuleProxy *sender, uint32 shardId, uint32 guildId) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CNameUnifierProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CNameUnifierSkel	*_LocalModuleSkel;


	public:
		CNameUnifierProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CNameUnifierSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CNameUnifierProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// EGS register it's name unifier in order to receive
		// an updated eid to name translation table
		void registerNameUnifierClient(NLNET::IModule *sender);
		// EGS ask to validate a character name
		// If the NU valide the name, it temporary
		// lock it to the associated player.
		// This function is called before character creation.
		void validateCharacterName(NLNET::IModule *sender, uint32 userId, uint8 charIndex, const std::string &name, uint32 homeMainlandSessionId);
		// EGS ask to assign a name to a character
		// This function is called during character creation
		void assignNameToCharacter(NLNET::IModule *sender, uint32 charId, const std::string &name, uint32 homeSessionId);
		// EGS ask to rename a character.
		// Renaming consist of assigning a default ramdomly generated name to the character
		void renameCharacter(NLNET::IModule *sender, uint32 charId);
		// EGS send info about the list of loaded guild.
		// The name unifier will update is internal name table if needed
		// and rename any guild having a conflicting name.
		// If any guild is renamed, then the name unifier send back
		// a guildRenamed message to EGS.
		void registerLoadedGuildNames(NLNET::IModule *sender, uint32 chardId, const std::vector < CGuildInfo > &guildInfos);
		// EGS ask to the name unifier to validate a new guild name
		void validateGuildName(NLNET::IModule *sender, uint32 guildId, const ucstring &guildName);
		// EGS add newly created guild info
		void addGuild(NLNET::IModule *sender, uint32 shardId, uint32 guildId, const ucstring &guildName);
		// EGS remove deleted guild info
		void removeGuild(NLNET::IModule *sender, uint32 shardId, uint32 guildId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_registerNameUnifierClient(NLNET::CMessage &__message);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_validateCharacterName(NLNET::CMessage &__message, uint32 userId, uint8 charIndex, const std::string &name, uint32 homeMainlandSessionId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_assignNameToCharacter(NLNET::CMessage &__message, uint32 charId, const std::string &name, uint32 homeSessionId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_renameCharacter(NLNET::CMessage &__message, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_registerLoadedGuildNames(NLNET::CMessage &__message, uint32 chardId, const std::vector < CGuildInfo > &guildInfos);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_validateGuildName(NLNET::CMessage &__message, uint32 guildId, const ucstring &guildName);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_addGuild(NLNET::CMessage &__message, uint32 shardId, uint32 guildId, const ucstring &guildName);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_removeGuild(NLNET::CMessage &__message, uint32 shardId, uint32 guildId);




	};
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TNameEntry
	{
	protected:
		//
		uint32	_UserId;
		//
		uint8	_CharIndex;
		//
		ucstring	_Name;
		//
		std::string	_UserName;
		//
		uint32	_ShardId;
	public:
		//
		uint32 getUserId() const
		{
			return _UserId;
		}

		void setUserId(uint32 value)
		{

				_UserId = value;

		}
			//
		uint8 getCharIndex() const
		{
			return _CharIndex;
		}

		void setCharIndex(uint8 value)
		{

				_CharIndex = value;

		}
			//
		const ucstring &getName() const
		{
			return _Name;
		}

		ucstring &getName()
		{
			return _Name;
		}


		void setName(const ucstring &value)
		{


				_Name = value;


		}
			//
		const std::string &getUserName() const
		{
			return _UserName;
		}

		std::string &getUserName()
		{
			return _UserName;
		}


		void setUserName(const std::string &value)
		{


				_UserName = value;


		}
			//
		const uint32 &getShardId() const
		{
			return _ShardId;
		}

		uint32 &getShardId()
		{
			return _ShardId;
		}


		void setShardId(const uint32 &value)
		{


				_ShardId = value;


		}

		bool operator == (const TNameEntry &other) const
		{
			return _UserId == other._UserId
				&& _CharIndex == other._CharIndex
				&& _Name == other._Name
				&& _UserName == other._UserName
				&& _ShardId == other._ShardId;
		}


		// constructor
		TNameEntry()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_UserId);
			s.serial(_CharIndex);
			s.serial(_Name);
			s.serial(_UserName);
			s.serial(_ShardId);

		}


	private:


	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TCharSyncResultEntry
	{
	protected:
		//
		uint32	_CharId;
		//
		ucstring	_CharName;
		//
		uint32	_HomeSessionId;
		//
		bool	_IsOwnerOfActiveAnimSession;
		//
		uint32	_ActiveAnimSessionId;
		//
		uint32	_EditionSessionId;
	public:
		//
		uint32 getCharId() const
		{
			return _CharId;
		}

		void setCharId(uint32 value)
		{

				_CharId = value;

		}
			//
		const ucstring &getCharName() const
		{
			return _CharName;
		}

		ucstring &getCharName()
		{
			return _CharName;
		}


		void setCharName(const ucstring &value)
		{


				_CharName = value;


		}
			//
		uint32 getHomeSessionId() const
		{
			return _HomeSessionId;
		}

		void setHomeSessionId(uint32 value)
		{

				_HomeSessionId = value;

		}
			//
		bool getIsOwnerOfActiveAnimSession() const
		{
			return _IsOwnerOfActiveAnimSession;
		}

		void setIsOwnerOfActiveAnimSession(bool value)
		{

				_IsOwnerOfActiveAnimSession = value;

		}
			//
		uint32 getActiveAnimSessionId() const
		{
			return _ActiveAnimSessionId;
		}

		void setActiveAnimSessionId(uint32 value)
		{

				_ActiveAnimSessionId = value;

		}
			//
		uint32 getEditionSessionId() const
		{
			return _EditionSessionId;
		}

		void setEditionSessionId(uint32 value)
		{

				_EditionSessionId = value;

		}

		bool operator == (const TCharSyncResultEntry &other) const
		{
			return _CharId == other._CharId
				&& _CharName == other._CharName
				&& _HomeSessionId == other._HomeSessionId
				&& _IsOwnerOfActiveAnimSession == other._IsOwnerOfActiveAnimSession
				&& _ActiveAnimSessionId == other._ActiveAnimSessionId
				&& _EditionSessionId == other._EditionSessionId;
		}


		// constructor
		TCharSyncResultEntry()
		{
			// Default initialisation
			_ActiveAnimSessionId = 0;

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_CharId);
			s.serial(_CharName);
			s.serial(_HomeSessionId);
			s.serial(_IsOwnerOfActiveAnimSession);
			s.serial(_ActiveAnimSessionId);
			s.serial(_EditionSessionId);

		}


	private:


	};



	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CNameUnifierClientSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CNameUnifierClientSkel>	TInterceptor;
	protected:
		CNameUnifierClientSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CNameUnifierClientSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CNameUnifierClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void initEIdTranslator_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateEIdTranslator_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void validateCharacterNameResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void assignCharacterNameResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void characterRenamed_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void userCharUpdatedAndValidated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void userCharSyncFailed_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void guildRenamed_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void validateGuildNameResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void removeCharFromGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CNameUnifierClientSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// The name unifier send the initial content for the Eid translator.
		// EGS need to wait until it receive this message before continuing
		// it's startup sequence in order to have coherent name in guild.
		virtual void initEIdTranslator(NLNET::IModuleProxy *sender, bool firstPacket, bool lastPacket, const std::vector < TNameEntry > &nameEntries) =0;
		// The name unifier send an update for the EID translator.
		// releasedNames contains a list of charId whose names have been released
		// changedNames contains a list of add or update entries
		virtual void updateEIdTranslator(NLNET::IModuleProxy *sender, const std::vector < uint32 > &releasedNames, const std::vector < TNameEntry > &changedNames) =0;
		// The name unifier send the result for validation
		// of a character name before creation.
		virtual void validateCharacterNameResult(NLNET::IModuleProxy *sender, const CValidateNameResult &nameResult) =0;
		// The name unifier send the result for name assignment
		// of a new character name during creation.
		virtual void assignCharacterNameResult(NLNET::IModuleProxy *sender, const CValidateNameResult &nameResult) =0;
		// The name unifier has renamed a character
		// EGS must do what it need to take the new name into account
		virtual void characterRenamed(NLNET::IModuleProxy *sender, uint32 charId, const std::string &newName, bool sendSummary) =0;
		// The name unifier has updated/validated/eventualy renamed
		// all the characters send by EGS for a user.
		// EGS can proceed to send the characters summary to client
		// The result message contains the list of all characters
		// with their unified name and home session id from the
		// ring database
		virtual void userCharUpdatedAndValidated(NLNET::IModuleProxy *sender, uint32 userId, const std::vector < TCharSyncResultEntry > &charInfos) =0;
		// The name unifier has failed tp updated/validated/eventualy renamed
		// all the characters send by EGS for a user.
		// EGS can proceed to send the characters summary to client
		// but the character names are perhaps not good ?
		virtual void userCharSyncFailed(NLNET::IModuleProxy *sender, uint32 userId) =0;
		// The name unifier has renamed a guild to resolve a name conflict
		virtual void guildRenamed(NLNET::IModuleProxy *sender, uint32 guildId, const ucstring &newName) =0;
		// The name unifier respond to EGS about guild name validation request
		virtual void validateGuildNameResult(NLNET::IModuleProxy *sender, uint32 guildId, const ucstring &guildName, TCharacterNameResult result) =0;
		// The unifier has detected an invalid guild/character association
		// and ask to the EGS to remove the character from the guild
		virtual void removeCharFromGuild(NLNET::IModuleProxy *sender, uint32 charId, uint32 guildId) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CNameUnifierClientProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CNameUnifierClientSkel	*_LocalModuleSkel;


	public:
		CNameUnifierClientProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CNameUnifierClientSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CNameUnifierClientProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// The name unifier send the initial content for the Eid translator.
		// EGS need to wait until it receive this message before continuing
		// it's startup sequence in order to have coherent name in guild.
		void initEIdTranslator(NLNET::IModule *sender, bool firstPacket, bool lastPacket, const std::vector < TNameEntry > &nameEntries);
		// The name unifier send an update for the EID translator.
		// releasedNames contains a list of charId whose names have been released
		// changedNames contains a list of add or update entries
		void updateEIdTranslator(NLNET::IModule *sender, const std::vector < uint32 > &releasedNames, const std::vector < TNameEntry > &changedNames);
		// The name unifier send the result for validation
		// of a character name before creation.
		void validateCharacterNameResult(NLNET::IModule *sender, const CValidateNameResult &nameResult);
		// The name unifier send the result for name assignment
		// of a new character name during creation.
		void assignCharacterNameResult(NLNET::IModule *sender, const CValidateNameResult &nameResult);
		// The name unifier has renamed a character
		// EGS must do what it need to take the new name into account
		void characterRenamed(NLNET::IModule *sender, uint32 charId, const std::string &newName, bool sendSummary);
		// The name unifier has updated/validated/eventualy renamed
		// all the characters send by EGS for a user.
		// EGS can proceed to send the characters summary to client
		// The result message contains the list of all characters
		// with their unified name and home session id from the
		// ring database
		void userCharUpdatedAndValidated(NLNET::IModule *sender, uint32 userId, const std::vector < TCharSyncResultEntry > &charInfos);
		// The name unifier has failed tp updated/validated/eventualy renamed
		// all the characters send by EGS for a user.
		// EGS can proceed to send the characters summary to client
		// but the character names are perhaps not good ?
		void userCharSyncFailed(NLNET::IModule *sender, uint32 userId);
		// The name unifier has renamed a guild to resolve a name conflict
		void guildRenamed(NLNET::IModule *sender, uint32 guildId, const ucstring &newName);
		// The name unifier respond to EGS about guild name validation request
		void validateGuildNameResult(NLNET::IModule *sender, uint32 guildId, const ucstring &guildName, TCharacterNameResult result);
		// The unifier has detected an invalid guild/character association
		// and ask to the EGS to remove the character from the guild
		void removeCharFromGuild(NLNET::IModule *sender, uint32 charId, uint32 guildId);
		// The name unifier send an update for the EID translator.
		// releasedNames contains a list of charId whose names have been released
		// changedNames contains a list of add or update entries

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_updateEIdTranslator(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const std::vector < uint32 > &releasedNames, const std::vector < TNameEntry > &changedNames)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_updateEIdTranslator(message , releasedNames, changedNames);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// The name unifier has renamed a character
		// EGS must do what it need to take the new name into account

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_characterRenamed(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, uint32 charId, const std::string &newName, bool sendSummary)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_characterRenamed(message , charId, newName, sendSummary);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_initEIdTranslator(NLNET::CMessage &__message, bool firstPacket, bool lastPacket, const std::vector < TNameEntry > &nameEntries);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateEIdTranslator(NLNET::CMessage &__message, const std::vector < uint32 > &releasedNames, const std::vector < TNameEntry > &changedNames);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_validateCharacterNameResult(NLNET::CMessage &__message, const CValidateNameResult &nameResult);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_assignCharacterNameResult(NLNET::CMessage &__message, const CValidateNameResult &nameResult);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_characterRenamed(NLNET::CMessage &__message, uint32 charId, const std::string &newName, bool sendSummary);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_userCharUpdatedAndValidated(NLNET::CMessage &__message, uint32 userId, const std::vector < TCharSyncResultEntry > &charInfos);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_userCharSyncFailed(NLNET::CMessage &__message, uint32 userId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_guildRenamed(NLNET::CMessage &__message, uint32 guildId, const ucstring &newName);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_validateGuildNameResult(NLNET::CMessage &__message, uint32 guildId, const ucstring &guildName, TCharacterNameResult result);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_removeCharFromGuild(NLNET::CMessage &__message, uint32 charId, uint32 guildId);




	};

}

#endif
