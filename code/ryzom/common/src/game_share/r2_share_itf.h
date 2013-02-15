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

#ifndef R2_SHARE_ITF
#define R2_SHARE_ITF
#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "game_share/object.h"

#include "nel/misc/md5.h"

#include "game_share/r2_types.h"

namespace R2
{

	class TRunningScenarioInfo;

	class TCharMappedInfo;



	struct TSessionLevel
	{
		enum TValues
		{
			sl_a = 1,
			sl_b,
			sl_c,
			sl_d,
			sl_e,
			sl_f,
			/// the highest valid value in the enum
			last_enum_item = sl_f,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 6
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(sl_a, 0));
				indexTable.insert(std::make_pair(sl_b, 1));
				indexTable.insert(std::make_pair(sl_c, 2));
				indexTable.insert(std::make_pair(sl_d, 3));
				indexTable.insert(std::make_pair(sl_e, 4));
				indexTable.insert(std::make_pair(sl_f, 5));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(sl_a)
				NL_STRING_CONVERSION_TABLE_ENTRY(sl_b)
				NL_STRING_CONVERSION_TABLE_ENTRY(sl_c)
				NL_STRING_CONVERSION_TABLE_ENTRY(sl_d)
				NL_STRING_CONVERSION_TABLE_ENTRY(sl_e)
				NL_STRING_CONVERSION_TABLE_ENTRY(sl_f)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TSessionLevel()
			: _Value(invalid_val)
		{
		}
		TSessionLevel(TValues value)
			: _Value(value)
		{
		}

		TSessionLevel(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TSessionLevel &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TSessionLevel &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TSessionLevel &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TSessionLevel &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TSessionLevel &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TSessionLevel &other) const
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

	public:
		void fromCharacterLevel(uint32 level)
		{
			if (level < 21)
				_Value = sl_a;	// 0..20
			else if (level < 51)
				_Value = sl_b;	// 21..50
			else if (level < 101)
				_Value = sl_c;	// 51..100
			else if (level < 151)
				_Value = sl_d;	// 101..150
			else if (level < 201)
				_Value = sl_e;	// 151..200
			else
				_Value = sl_f;	// 201..oo
		}

		void fromSkillName(const std::string &skillName)
		{
			// we considere the length of the skill name to be proportional to the level
			if (skillName.size() < 2)
			{
				// skill name too short
				_Value = invalid_val;
			}
			uint32 size = (uint32)skillName.size()-1;
			if (size < end_of_enum)
				_Value = TValues(size);
			else
				// max the level
				_Value = sl_f;
		}

		// return a value corresponding to the top of the game level range for the given value
		uint32 asLevel()
		{
			switch (_Value)
			{
				case sl_a: return  20;
				case sl_b: return  50;
				case sl_c: return 100;
				case sl_d: return 150;
				case sl_e: return 200;
				case sl_f: return 250;
				default: BOMB("Invalid value being converted to level - returning 1",return 1);
			}
		}

	};
		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TRunningScenarioInfo
	{
	protected:
		//
		NLMISC::CHashKeyMD5	_ScenarioKey;
		//
		std::string	_ScenarioTitle;
		//
		std::string	_ScenarioDesc;
		//
		TSessionLevel	_SessionLevel;
		//
		std::string	_ScenarioAuthorName;
		//
		uint32	_SessionAnimatorCharId;
		//
		bool	_DMLess;
		//
		std::string	_MissionTag;
	public:
		//
		const NLMISC::CHashKeyMD5 &getScenarioKey() const
		{
			return _ScenarioKey;
		}

		NLMISC::CHashKeyMD5 &getScenarioKey()
		{
			return _ScenarioKey;
		}


		void setScenarioKey(const NLMISC::CHashKeyMD5 &value)
		{


				_ScenarioKey = value;


		}
			//
		const std::string &getScenarioTitle() const
		{
			return _ScenarioTitle;
		}

		std::string &getScenarioTitle()
		{
			return _ScenarioTitle;
		}


		void setScenarioTitle(const std::string &value)
		{


				_ScenarioTitle = value;


		}
			//
		const std::string &getScenarioDesc() const
		{
			return _ScenarioDesc;
		}

		std::string &getScenarioDesc()
		{
			return _ScenarioDesc;
		}


		void setScenarioDesc(const std::string &value)
		{


				_ScenarioDesc = value;


		}
			//
		TSessionLevel getSessionLevel() const
		{
			return _SessionLevel;
		}

		void setSessionLevel(TSessionLevel value)
		{

				_SessionLevel = value;

		}
			//
		const std::string &getScenarioAuthorName() const
		{
			return _ScenarioAuthorName;
		}

		std::string &getScenarioAuthorName()
		{
			return _ScenarioAuthorName;
		}


		void setScenarioAuthorName(const std::string &value)
		{


				_ScenarioAuthorName = value;


		}
			//
		uint32 getSessionAnimatorCharId() const
		{
			return _SessionAnimatorCharId;
		}

		void setSessionAnimatorCharId(uint32 value)
		{

				_SessionAnimatorCharId = value;

		}
			//
		bool getDMLess() const
		{
			return _DMLess;
		}

		void setDMLess(bool value)
		{

				_DMLess = value;

		}
			//
		const std::string &getMissionTag() const
		{
			return _MissionTag;
		}

		std::string &getMissionTag()
		{
			return _MissionTag;
		}


		void setMissionTag(const std::string &value)
		{


				_MissionTag = value;


		}

		bool operator == (const TRunningScenarioInfo &other) const
		{
			return _ScenarioKey == other._ScenarioKey
				&& _ScenarioTitle == other._ScenarioTitle
				&& _ScenarioDesc == other._ScenarioDesc
				&& _SessionLevel == other._SessionLevel
				&& _ScenarioAuthorName == other._ScenarioAuthorName
				&& _SessionAnimatorCharId == other._SessionAnimatorCharId
				&& _DMLess == other._DMLess
				&& _MissionTag == other._MissionTag;
		}


		// constructor
		TRunningScenarioInfo()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_ScenarioKey);
			s.serial(_ScenarioTitle);
			s.serial(_ScenarioDesc);
			s.serial(_SessionLevel);
			s.serial(_ScenarioAuthorName);
			s.serial(_SessionAnimatorCharId);
			s.serial(_DMLess);
			s.serial(_MissionTag);

		}


	private:


	};




	struct TPioneerRight
	{
		enum TValues
		{
			Tester = 0,
			DM,
			/// the highest valid value in the enum
			last_enum_item = DM,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 2
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(Tester, 0));
				indexTable.insert(std::make_pair(DM, 1));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(Tester)
				NL_STRING_CONVERSION_TABLE_ENTRY(DM)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TPioneerRight()
			: _Value(invalid_val)
		{
		}
		TPioneerRight(TValues value)
			: _Value(value)
		{
		}

		TPioneerRight(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TPioneerRight &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TPioneerRight &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TPioneerRight &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TPioneerRight &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TPioneerRight &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TPioneerRight &other) const
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
	class CShareServerAnimationItfSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CShareServerAnimationItfSkel>	TInterceptor;
	protected:
		CShareServerAnimationItfSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CShareServerAnimationItfSkel()
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

		typedef void (CShareServerAnimationItfSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void connectAnimationModePlay_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void askMissionItemsDescription_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void askActPositionDescriptions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void askUserTriggerDescriptions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onUserTriggerTriggered_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onDssTarget_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CShareServerAnimationItfSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// request the connection to play mode in an animation session
		virtual void connectAnimationModePlay(NLNET::IModuleProxy *sender) =0;
		// A client Message to register mission item of a scenario
		virtual void askMissionItemsDescription(NLNET::IModuleProxy *sender) =0;
		// A client Message to update client Act Position Description
		virtual void askActPositionDescriptions(NLNET::IModuleProxy *sender) =0;
		// A client Message to update client User Trigger Description
		virtual void askUserTriggerDescriptions(NLNET::IModuleProxy *sender) =0;
		// client wants to trigger an user trigger
		virtual void onUserTriggerTriggered(NLNET::IModuleProxy *sender, uint32 actId, uint32 triggerId) =0;
		// client wants to execute a dm action on its target
		virtual void onDssTarget(NLNET::IModuleProxy *sender, const std::vector<std::string> &args) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CShareServerAnimationItfProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CShareServerAnimationItfSkel	*_LocalModuleSkel;


	public:
		CShareServerAnimationItfProxy(NLNET::IModuleProxy *proxy)
		{
			nlassert(proxy->getModuleClassName() == "ServerAnimationModule");
			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CShareServerAnimationItfSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CShareServerAnimationItfProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// request the connection to play mode in an animation session
		void connectAnimationModePlay(NLNET::IModule *sender);
		// A client Message to register mission item of a scenario
		void askMissionItemsDescription(NLNET::IModule *sender);
		// A client Message to update client Act Position Description
		void askActPositionDescriptions(NLNET::IModule *sender);
		// A client Message to update client User Trigger Description
		void askUserTriggerDescriptions(NLNET::IModule *sender);
		// client wants to trigger an user trigger
		void onUserTriggerTriggered(NLNET::IModule *sender, uint32 actId, uint32 triggerId);
		// client wants to execute a dm action on its target
		void onDssTarget(NLNET::IModule *sender, const std::vector<std::string> &args);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_connectAnimationModePlay(NLNET::CMessage &__message);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_askMissionItemsDescription(NLNET::CMessage &__message);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_askActPositionDescriptions(NLNET::CMessage &__message);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_askUserTriggerDescriptions(NLNET::CMessage &__message);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onUserTriggerTriggered(NLNET::CMessage &__message, uint32 actId, uint32 triggerId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onDssTarget(NLNET::CMessage &__message, const std::vector<std::string> &args);




	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CShareServerEditionItfSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CShareServerEditionItfSkel>	TInterceptor;
	protected:
		CShareServerEditionItfSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CShareServerEditionItfSkel()
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

		typedef void (CShareServerEditionItfSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void startingScenario_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void startScenario_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void advConnACK_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onUserComponentRegistered_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onUserComponentDownloading_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onScenarioUploadAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onNodeSetAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onNodeInsertAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onNodeEraseAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onNodeMoveAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onMapConnectionAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onCharModeUpdateAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onTpPositionAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void tpToEntryPoint_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setStartingAct_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onScenarioRingAccessUpdated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void saveScenarioFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void loadScenarioFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void saveUserComponentFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void loadUserComponentFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void teleportOneCharacterToAnother_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void teleportWhileUploadingScenario_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void multiPartMsgHead_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void multiPartMsgBody_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void multiPartMsgFoot_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void forwardToDss_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CShareServerEditionItfSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		virtual void startingScenario(NLNET::IModuleProxy *sender) =0;
		// Request the start of a test scenario
		virtual void startScenario(NLNET::IModuleProxy *sender, bool ok, const TScenarioHeaderSerializer &header, const CObjectSerializerServer &data, uint32 startingAct) =0;
		// Client has received the ADV_CONN message
		virtual void advConnACK(NLNET::IModuleProxy *sender) =0;
		// The client announce to the server that he has registered a component.
		virtual void onUserComponentRegistered(NLNET::IModuleProxy *sender, const NLMISC::CHashKeyMD5 &md5) =0;
		// The client announce to the server that he need a componennt so the server must uploading it.
		virtual void onUserComponentDownloading(NLNET::IModuleProxy *sender, const NLMISC::CHashKeyMD5 &md5) =0;
		// Upload the high level scenario.
		virtual void onScenarioUploadAsked(NLNET::IModuleProxy *sender, uint32 msgId, const CObjectSerializerServer &hlScenario, bool mustBrodcast) =0;
		// The client request to set a node on a hl scenario.
		virtual void onNodeSetAsked(NLNET::IModuleProxy *sender, uint32 msgId, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerServer &value) =0;
		// The client request to insert a node on a hl scenario.
		virtual void onNodeInsertAsked(NLNET::IModuleProxy *sender, uint32 msgId, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerServer &value) =0;
		// The client request to erase a node on a hl scenario.
		virtual void onNodeEraseAsked(NLNET::IModuleProxy *sender, uint32 msgId, const std::string &instanceId, const std::string &attrName, sint32 position) =0;
		// The client request to move a node on a hl scenario.
		virtual void onNodeMoveAsked(NLNET::IModuleProxy *sender, uint32 msgId, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2) =0;
		// Call by the client in order to download its current scenario (and tp).
		virtual void onMapConnectionAsked(NLNET::IModuleProxy *sender, TSessionId scenarioId, bool updateHighLevel, bool mustTp, R2::TUserRole role) =0;
		// Call by the client when he change its mode (Dm, Tester, Player)
		virtual void onCharModeUpdateAsked(NLNET::IModuleProxy *sender, R2::TCharMode mode) =0;
		// client wants to tp at a specific position (clicking in map)
		virtual void onTpPositionAsked(NLNET::IModuleProxy *sender, float x, float y, float z) =0;
		// Update the mode of the pioneer (DM/TEST).
		virtual void tpToEntryPoint(NLNET::IModuleProxy *sender, uint32 actIndex) =0;
		// Set the starting act of the scenario
		virtual void setStartingAct(NLNET::IModuleProxy *sender, uint32 actIndex) =0;
		// Update the ring access of a scenario.
		virtual void onScenarioRingAccessUpdated(NLNET::IModuleProxy *sender, bool ok, const std::string &ringAccess, const std::string &errMsg) =0;
		// a message to validate a file waiting to be saved
		virtual void saveScenarioFile(NLNET::IModuleProxy *sender, const std::string &md5, const R2::TScenarioHeaderSerializer &header) =0;
		// a message to validate a file waiting to be loaded
		virtual void loadScenarioFile(NLNET::IModuleProxy *sender, const std::string &md5, const std::string &signature) =0;
		// a message to validate a user component file waiting to be saved
		virtual void saveUserComponentFile(NLNET::IModuleProxy *sender, const std::string &md5, const R2::TScenarioHeaderSerializer &header) =0;
		// a message to validate a user component file waiting to be loaded
		virtual void loadUserComponentFile(NLNET::IModuleProxy *sender, const std::string &md5, const std::string &signature) =0;
		// a message to ask the dss to teleport a character to another character
		virtual void teleportOneCharacterToAnother(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 sourceId, uint32 destId) =0;
		// teleport the player while uploading the scenario
		virtual void teleportWhileUploadingScenario(NLNET::IModuleProxy *sender, const std::string &island, const std::string &entryPoint, const std::string &season) =0;
		// send the header of a multi-part message
		virtual void multiPartMsgHead(NLNET::IModuleProxy *sender, uint32 charId, const std::string &msgName, uint32 nbPacket, uint32 size) =0;
		// send a part of a multi-part message
		virtual void multiPartMsgBody(NLNET::IModuleProxy *sender, uint32 charId, uint32 partId, const std::vector<uint8> &data) =0;
		// send the footer of a multi-part message
		virtual void multiPartMsgFoot(NLNET::IModuleProxy *sender, uint32 charId) =0;
		// simulate the forward of a message (to dss)
		virtual void forwardToDss(NLNET::IModuleProxy *sender, uint32 charId, const NLNET::CMessage &msg) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CShareServerEditionItfProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CShareServerEditionItfSkel	*_LocalModuleSkel;


	public:
		CShareServerEditionItfProxy(NLNET::IModuleProxy *proxy)
		{
			nlassert(proxy->getModuleClassName() == "ServerEditionModule");
			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CShareServerEditionItfSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CShareServerEditionItfProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		void startingScenario(NLNET::IModule *sender);
		// Request the start of a test scenario
		void startScenario(NLNET::IModule *sender, bool ok, const TScenarioHeaderSerializer &header, const CObjectSerializerServer &data, uint32 startingAct);
		// Client has received the ADV_CONN message
		void advConnACK(NLNET::IModule *sender);
		// The client announce to the server that he has registered a component.
		void onUserComponentRegistered(NLNET::IModule *sender, const NLMISC::CHashKeyMD5 &md5);
		// The client announce to the server that he need a componennt so the server must uploading it.
		void onUserComponentDownloading(NLNET::IModule *sender, const NLMISC::CHashKeyMD5 &md5);
		// Upload the high level scenario.
		void onScenarioUploadAsked(NLNET::IModule *sender, uint32 msgId, const CObjectSerializerServer &hlScenario, bool mustBrodcast);
		// The client request to set a node on a hl scenario.
		void onNodeSetAsked(NLNET::IModule *sender, uint32 msgId, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerServer &value);
		// The client request to insert a node on a hl scenario.
		void onNodeInsertAsked(NLNET::IModule *sender, uint32 msgId, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerServer &value);
		// The client request to erase a node on a hl scenario.
		void onNodeEraseAsked(NLNET::IModule *sender, uint32 msgId, const std::string &instanceId, const std::string &attrName, sint32 position);
		// The client request to move a node on a hl scenario.
		void onNodeMoveAsked(NLNET::IModule *sender, uint32 msgId, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2);
		// Call by the client in order to download its current scenario (and tp).
		void onMapConnectionAsked(NLNET::IModule *sender, TSessionId scenarioId, bool updateHighLevel, bool mustTp, R2::TUserRole role);
		// Call by the client when he change its mode (Dm, Tester, Player)
		void onCharModeUpdateAsked(NLNET::IModule *sender, R2::TCharMode mode);
		// client wants to tp at a specific position (clicking in map)
		void onTpPositionAsked(NLNET::IModule *sender, float x, float y, float z);
		// Update the mode of the pioneer (DM/TEST).
		void tpToEntryPoint(NLNET::IModule *sender, uint32 actIndex);
		// Set the starting act of the scenario
		void setStartingAct(NLNET::IModule *sender, uint32 actIndex);
		// Update the ring access of a scenario.
		void onScenarioRingAccessUpdated(NLNET::IModule *sender, bool ok, const std::string &ringAccess, const std::string &errMsg);
		// a message to validate a file waiting to be saved
		void saveScenarioFile(NLNET::IModule *sender, const std::string &md5, const R2::TScenarioHeaderSerializer &header);
		// a message to validate a file waiting to be loaded
		void loadScenarioFile(NLNET::IModule *sender, const std::string &md5, const std::string &signature);
		// a message to validate a user component file waiting to be saved
		void saveUserComponentFile(NLNET::IModule *sender, const std::string &md5, const R2::TScenarioHeaderSerializer &header);
		// a message to validate a user component file waiting to be loaded
		void loadUserComponentFile(NLNET::IModule *sender, const std::string &md5, const std::string &signature);
		// a message to ask the dss to teleport a character to another character
		void teleportOneCharacterToAnother(NLNET::IModule *sender, TSessionId sessionId, uint32 sourceId, uint32 destId);
		// teleport the player while uploading the scenario
		void teleportWhileUploadingScenario(NLNET::IModule *sender, const std::string &island, const std::string &entryPoint, const std::string &season);
		// send the header of a multi-part message
		void multiPartMsgHead(NLNET::IModule *sender, uint32 charId, const std::string &msgName, uint32 nbPacket, uint32 size);
		// send a part of a multi-part message
		void multiPartMsgBody(NLNET::IModule *sender, uint32 charId, uint32 partId, const std::vector<uint8> &data);
		// send the footer of a multi-part message
		void multiPartMsgFoot(NLNET::IModule *sender, uint32 charId);
		// simulate the forward of a message (to dss)
		void forwardToDss(NLNET::IModule *sender, uint32 charId, const NLNET::CMessage &msg);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_startingScenario(NLNET::CMessage &__message);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_startScenario(NLNET::CMessage &__message, bool ok, const TScenarioHeaderSerializer &header, const CObjectSerializerServer &data, uint32 startingAct);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_advConnACK(NLNET::CMessage &__message);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onUserComponentRegistered(NLNET::CMessage &__message, const NLMISC::CHashKeyMD5 &md5);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onUserComponentDownloading(NLNET::CMessage &__message, const NLMISC::CHashKeyMD5 &md5);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onScenarioUploadAsked(NLNET::CMessage &__message, uint32 msgId, const CObjectSerializerServer &hlScenario, bool mustBrodcast);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onNodeSetAsked(NLNET::CMessage &__message, uint32 msgId, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerServer &value);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onNodeInsertAsked(NLNET::CMessage &__message, uint32 msgId, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerServer &value);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onNodeEraseAsked(NLNET::CMessage &__message, uint32 msgId, const std::string &instanceId, const std::string &attrName, sint32 position);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onNodeMoveAsked(NLNET::CMessage &__message, uint32 msgId, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onMapConnectionAsked(NLNET::CMessage &__message, TSessionId scenarioId, bool updateHighLevel, bool mustTp, R2::TUserRole role);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onCharModeUpdateAsked(NLNET::CMessage &__message, R2::TCharMode mode);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onTpPositionAsked(NLNET::CMessage &__message, float x, float y, float z);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_tpToEntryPoint(NLNET::CMessage &__message, uint32 actIndex);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setStartingAct(NLNET::CMessage &__message, uint32 actIndex);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onScenarioRingAccessUpdated(NLNET::CMessage &__message, bool ok, const std::string &ringAccess, const std::string &errMsg);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_saveScenarioFile(NLNET::CMessage &__message, const std::string &md5, const R2::TScenarioHeaderSerializer &header);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_loadScenarioFile(NLNET::CMessage &__message, const std::string &md5, const std::string &signature);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_saveUserComponentFile(NLNET::CMessage &__message, const std::string &md5, const R2::TScenarioHeaderSerializer &header);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_loadUserComponentFile(NLNET::CMessage &__message, const std::string &md5, const std::string &signature);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_teleportOneCharacterToAnother(NLNET::CMessage &__message, TSessionId sessionId, uint32 sourceId, uint32 destId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_teleportWhileUploadingScenario(NLNET::CMessage &__message, const std::string &island, const std::string &entryPoint, const std::string &season);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_multiPartMsgHead(NLNET::CMessage &__message, uint32 charId, const std::string &msgName, uint32 nbPacket, uint32 size);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_multiPartMsgBody(NLNET::CMessage &__message, uint32 charId, uint32 partId, const std::vector<uint8> &data);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_multiPartMsgFoot(NLNET::CMessage &__message, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_forwardToDss(NLNET::CMessage &__message, uint32 charId, const NLNET::CMessage &msg);




	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CShareClientEditionItfSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CShareClientEditionItfSkel>	TInterceptor;
	protected:
		CShareClientEditionItfSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CShareClientEditionItfSkel()
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

		typedef void (CShareClientEditionItfSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void startingScenario_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void startScenario_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onUserComponentRegistered_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onUserComponentUploading_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onScenarioUploaded_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onNodeSet_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onNodeInserted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onNodeErased_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onNodeMoved_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onQuotaUpdated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onCharModeUpdated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onTestModeDisconnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onTpPositionSimulated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onKicked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onDisconnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void scheduleStartAct_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onAnimationModePlayConnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateScenarioHeader_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateMissionItemsDescription_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateActPositionDescriptions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateUserTriggerDescriptions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onCurrentActIndexUpdated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateTalkingAsList_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateIncarningList_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void systemMsg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onRingAccessUpdated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void saveScenarioFileAccepted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void loadScenarioFileAccepted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void saveUserComponentFileAccepted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void loadUserComponentFileAccepted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void multiPartMsgHead_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void multiPartMsgBody_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void multiPartMsgFoot_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void ackMsg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CShareClientEditionItfSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// The start of a test has been requested
		virtual void startingScenario(NLNET::IModuleProxy *sender, uint32 charId) =0;
		// A Scenario has started
		virtual void startScenario(NLNET::IModuleProxy *sender, bool ok, uint32 startingAct, const std::string &errorMsg) =0;
		// A User component has been registered
		virtual void onUserComponentRegistered(NLNET::IModuleProxy *sender, const NLMISC::CHashKeyMD5 &md5) =0;
		// Request the upload of a component
		virtual void onUserComponentUploading(NLNET::IModuleProxy *sender, const NLMISC::CHashKeyMD5 &md5) =0;
		// The client request to upload an hl ata.
		virtual void onScenarioUploaded(NLNET::IModuleProxy *sender, const R2::CObjectSerializerClient &hlScenario) =0;
		// The client request to set a node on a hl scenario.
		virtual void onNodeSet(NLNET::IModuleProxy *sender, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerClient &value) =0;
		// The ServerEditionMode inserts a node on a hl scenario.
		virtual void onNodeInserted(NLNET::IModuleProxy *sender, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerClient &value) =0;
		// The ServerEditionMode erases a node on a hl scenario.
		virtual void onNodeErased(NLNET::IModuleProxy *sender, const std::string &instanceId, const std::string &attrName, sint32 position) =0;
		// The ServerEditionMode a move node on a hl scenario.
		virtual void onNodeMoved(NLNET::IModuleProxy *sender, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2) =0;
		// Updates the client quota
		virtual void onQuotaUpdated(NLNET::IModuleProxy *sender, uint32 maxNpcs, uint32 maxStaticObjects) =0;
		// Updates the client Mode (tester, dm, editor, player) be the speed
		virtual void onCharModeUpdated(NLNET::IModuleProxy *sender, R2::TCharMode mode) =0;
		// Indicates to the client that an animation has stop (animation, play, test)
		virtual void onTestModeDisconnected(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 lastActIndex, R2::TScenarioSessionType animationType) =0;
		// A DSS Message to make a local client tp (because egs can not do it)/
		virtual void onTpPositionSimulated(NLNET::IModuleProxy *sender, TSessionId sessionId, uint64 characterId64, sint32 x, sint32 y, sint32 z, uint8 scenarioSeason) =0;
		// A DSS Message to indicates that the client will be disconnect in secondes./
		virtual void onKicked(NLNET::IModuleProxy *sender, uint32 timeBeforeDisconnection, bool mustKick) =0;
		// A DSS Message to make to disconnect the client./
		virtual void onDisconnected(NLNET::IModuleProxy *sender) =0;
		// Tell to the client that an act begin in nbSeconds
		virtual void scheduleStartAct(NLNET::IModuleProxy *sender, uint32 errorId, uint32 actId, uint32 nbSeconds) =0;
		// Tell to the client that he is connected in play mode in an animation session
		virtual void onAnimationModePlayConnected(NLNET::IModuleProxy *sender) =0;
		// A DSS Message to update the scenario Header
		virtual void updateScenarioHeader(NLNET::IModuleProxy *sender, const R2::TScenarioHeaderSerializer &scenarioHeader) =0;
		// A DSS Message to update discription mission item of a scenario
		virtual void updateMissionItemsDescription(NLNET::IModuleProxy *sender, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem) =0;
		// A DSS Message to update the discription of acts (name and positions)
		virtual void updateActPositionDescriptions(NLNET::IModuleProxy *sender, const R2::TActPositionDescriptions &actPositionDescriptions) =0;
		// A DSS Message to update the discription of acts (name and positions)
		virtual void updateUserTriggerDescriptions(NLNET::IModuleProxy *sender, const R2::TUserTriggerDescriptions &userTriggerDescriptions) =0;
		// A DSS Message to update the discription of acts (name and positions)
		virtual void onCurrentActIndexUpdated(NLNET::IModuleProxy *sender, uint32 actIndex) =0;
		// Update the Talking as list.
		virtual void updateTalkingAsList(NLNET::IModuleProxy *sender, const std::vector<uint32> &botsId) =0;
		// Update the Incarning list.
		virtual void updateIncarningList(NLNET::IModuleProxy *sender, const std::vector<uint32> &botsId) =0;
		// A message that will be printed an client
		virtual void systemMsg(NLNET::IModuleProxy *sender, const std::string &msgType, const std::string &who, const std::string &msg) =0;
		// Update the ring access of the client
		virtual void onRingAccessUpdated(NLNET::IModuleProxy *sender, const std::string &ringAccess) =0;
		// a message to validate a file waiting to be saved
		virtual void saveScenarioFileAccepted(NLNET::IModuleProxy *sender, const std::string &md5, const std::string &signature, bool isAccepted) =0;
		// a message to validate a file waiting to be loaded
		virtual void loadScenarioFileAccepted(NLNET::IModuleProxy *sender, const std::string &md5, bool ok) =0;
		// a message to validate a user component file waiting to be saved
		virtual void saveUserComponentFileAccepted(NLNET::IModuleProxy *sender, const std::string &md5, const std::string &signature, bool isAccepted) =0;
		// a message to validate a user component file waiting to be loaded
		virtual void loadUserComponentFileAccepted(NLNET::IModuleProxy *sender, const std::string &md5, bool ok) =0;
		// send the header of a multi-part message
		virtual void multiPartMsgHead(NLNET::IModuleProxy *sender, const std::string &msgName, uint32 nbPacket, uint32 size) =0;
		// send a part of a multi-part message
		virtual void multiPartMsgBody(NLNET::IModuleProxy *sender, uint32 partId, uint32 packetSize) =0;
		// send the footer of a multi-part message
		virtual void multiPartMsgFoot(NLNET::IModuleProxy *sender) =0;
		// send an ack messag to the client
		virtual void ackMsg(NLNET::IModuleProxy *sender, uint32 msgId, bool ok) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CShareClientEditionItfProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CShareClientEditionItfSkel	*_LocalModuleSkel;


	public:
		CShareClientEditionItfProxy(NLNET::IModuleProxy *proxy)
		{
			nlassert(proxy->getModuleClassName() == "ClientEditionModule");
			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CShareClientEditionItfSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CShareClientEditionItfProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// The start of a test has been requested
		void startingScenario(NLNET::IModule *sender, uint32 charId);
		// A Scenario has started
		void startScenario(NLNET::IModule *sender, bool ok, uint32 startingAct, const std::string &errorMsg);
		// A User component has been registered
		void onUserComponentRegistered(NLNET::IModule *sender, const NLMISC::CHashKeyMD5 &md5);
		// Request the upload of a component
		void onUserComponentUploading(NLNET::IModule *sender, const NLMISC::CHashKeyMD5 &md5);
		// The client request to upload an hl ata.
		void onScenarioUploaded(NLNET::IModule *sender, const R2::CObjectSerializerClient &hlScenario);
		// The client request to set a node on a hl scenario.
		void onNodeSet(NLNET::IModule *sender, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerClient &value);
		// The ServerEditionMode inserts a node on a hl scenario.
		void onNodeInserted(NLNET::IModule *sender, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerClient &value);
		// The ServerEditionMode erases a node on a hl scenario.
		void onNodeErased(NLNET::IModule *sender, const std::string &instanceId, const std::string &attrName, sint32 position);
		// The ServerEditionMode a move node on a hl scenario.
		void onNodeMoved(NLNET::IModule *sender, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2);
		// Updates the client quota
		void onQuotaUpdated(NLNET::IModule *sender, uint32 maxNpcs, uint32 maxStaticObjects);
		// Updates the client Mode (tester, dm, editor, player) be the speed
		void onCharModeUpdated(NLNET::IModule *sender, R2::TCharMode mode);
		// Indicates to the client that an animation has stop (animation, play, test)
		void onTestModeDisconnected(NLNET::IModule *sender, TSessionId sessionId, uint32 lastActIndex, R2::TScenarioSessionType animationType);
		// A DSS Message to make a local client tp (because egs can not do it)/
		void onTpPositionSimulated(NLNET::IModule *sender, TSessionId sessionId, uint64 characterId64, sint32 x, sint32 y, sint32 z, uint8 scenarioSeason);
		// A DSS Message to indicates that the client will be disconnect in secondes./
		void onKicked(NLNET::IModule *sender, uint32 timeBeforeDisconnection, bool mustKick);
		// A DSS Message to make to disconnect the client./
		void onDisconnected(NLNET::IModule *sender);
		// Tell to the client that an act begin in nbSeconds
		void scheduleStartAct(NLNET::IModule *sender, uint32 errorId, uint32 actId, uint32 nbSeconds);
		// Tell to the client that he is connected in play mode in an animation session
		void onAnimationModePlayConnected(NLNET::IModule *sender);
		// A DSS Message to update the scenario Header
		void updateScenarioHeader(NLNET::IModule *sender, const R2::TScenarioHeaderSerializer &scenarioHeader);
		// A DSS Message to update discription mission item of a scenario
		void updateMissionItemsDescription(NLNET::IModule *sender, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem);
		// A DSS Message to update the discription of acts (name and positions)
		void updateActPositionDescriptions(NLNET::IModule *sender, const R2::TActPositionDescriptions &actPositionDescriptions);
		// A DSS Message to update the discription of acts (name and positions)
		void updateUserTriggerDescriptions(NLNET::IModule *sender, const R2::TUserTriggerDescriptions &userTriggerDescriptions);
		// A DSS Message to update the discription of acts (name and positions)
		void onCurrentActIndexUpdated(NLNET::IModule *sender, uint32 actIndex);
		// Update the Talking as list.
		void updateTalkingAsList(NLNET::IModule *sender, const std::vector<uint32> &botsId);
		// Update the Incarning list.
		void updateIncarningList(NLNET::IModule *sender, const std::vector<uint32> &botsId);
		// A message that will be printed an client
		void systemMsg(NLNET::IModule *sender, const std::string &msgType, const std::string &who, const std::string &msg);
		// Update the ring access of the client
		void onRingAccessUpdated(NLNET::IModule *sender, const std::string &ringAccess);
		// a message to validate a file waiting to be saved
		void saveScenarioFileAccepted(NLNET::IModule *sender, const std::string &md5, const std::string &signature, bool isAccepted);
		// a message to validate a file waiting to be loaded
		void loadScenarioFileAccepted(NLNET::IModule *sender, const std::string &md5, bool ok);
		// a message to validate a user component file waiting to be saved
		void saveUserComponentFileAccepted(NLNET::IModule *sender, const std::string &md5, const std::string &signature, bool isAccepted);
		// a message to validate a user component file waiting to be loaded
		void loadUserComponentFileAccepted(NLNET::IModule *sender, const std::string &md5, bool ok);
		// send the header of a multi-part message
		void multiPartMsgHead(NLNET::IModule *sender, const std::string &msgName, uint32 nbPacket, uint32 size);
		// send a part of a multi-part message
		void multiPartMsgBody(NLNET::IModule *sender, uint32 partId, uint32 packetSize);
		// send the footer of a multi-part message
		void multiPartMsgFoot(NLNET::IModule *sender);
		// send an ack messag to the client
		void ackMsg(NLNET::IModule *sender, uint32 msgId, bool ok);
		// The start of a test has been requested

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_startingScenario(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, uint32 charId)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_startingScenario(message , charId);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// A Scenario has started

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_startScenario(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, bool ok, uint32 startingAct, const std::string &errorMsg)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_startScenario(message , ok, startingAct, errorMsg);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// A User component has been registered

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_onUserComponentRegistered(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const NLMISC::CHashKeyMD5 &md5)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_onUserComponentRegistered(message , md5);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// The client request to upload an hl ata.

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_onScenarioUploaded(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const R2::CObjectSerializerClient &hlScenario)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_onScenarioUploaded(message , hlScenario);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// The client request to set a node on a hl scenario.

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_onNodeSet(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerClient &value)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_onNodeSet(message , instanceId, attrName, value);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// The ServerEditionMode inserts a node on a hl scenario.

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_onNodeInserted(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerClient &value)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_onNodeInserted(message , instanceId, attrName, position, key, value);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// The ServerEditionMode erases a node on a hl scenario.

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_onNodeErased(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const std::string &instanceId, const std::string &attrName, sint32 position)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_onNodeErased(message , instanceId, attrName, position);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// The ServerEditionMode a move node on a hl scenario.

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_onNodeMoved(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_onNodeMoved(message , instanceId1, attrName1, position1, instanceId2, attrName2, position2);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// Indicates to the client that an animation has stop (animation, play, test)

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_onTestModeDisconnected(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, TSessionId sessionId, uint32 lastActIndex, R2::TScenarioSessionType animationType)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_onTestModeDisconnected(message , sessionId, lastActIndex, animationType);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// Tell to the client that an act begin in nbSeconds

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_scheduleStartAct(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, uint32 errorId, uint32 actId, uint32 nbSeconds)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_scheduleStartAct(message , errorId, actId, nbSeconds);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// Tell to the client that he is connected in play mode in an animation session

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_onAnimationModePlayConnected(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_onAnimationModePlayConnected(message );

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// A message that will be printed an client

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_systemMsg(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const std::string &msgType, const std::string &who, const std::string &msg)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_systemMsg(message , msgType, who, msg);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_startingScenario(NLNET::CMessage &__message, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_startScenario(NLNET::CMessage &__message, bool ok, uint32 startingAct, const std::string &errorMsg);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onUserComponentRegistered(NLNET::CMessage &__message, const NLMISC::CHashKeyMD5 &md5);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onUserComponentUploading(NLNET::CMessage &__message, const NLMISC::CHashKeyMD5 &md5);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onScenarioUploaded(NLNET::CMessage &__message, const R2::CObjectSerializerClient &hlScenario);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onNodeSet(NLNET::CMessage &__message, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerClient &value);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onNodeInserted(NLNET::CMessage &__message, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerClient &value);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onNodeErased(NLNET::CMessage &__message, const std::string &instanceId, const std::string &attrName, sint32 position);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onNodeMoved(NLNET::CMessage &__message, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onQuotaUpdated(NLNET::CMessage &__message, uint32 maxNpcs, uint32 maxStaticObjects);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onCharModeUpdated(NLNET::CMessage &__message, R2::TCharMode mode);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onTestModeDisconnected(NLNET::CMessage &__message, TSessionId sessionId, uint32 lastActIndex, R2::TScenarioSessionType animationType);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onTpPositionSimulated(NLNET::CMessage &__message, TSessionId sessionId, uint64 characterId64, sint32 x, sint32 y, sint32 z, uint8 scenarioSeason);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onKicked(NLNET::CMessage &__message, uint32 timeBeforeDisconnection, bool mustKick);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onDisconnected(NLNET::CMessage &__message);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_scheduleStartAct(NLNET::CMessage &__message, uint32 errorId, uint32 actId, uint32 nbSeconds);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onAnimationModePlayConnected(NLNET::CMessage &__message);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateScenarioHeader(NLNET::CMessage &__message, const R2::TScenarioHeaderSerializer &scenarioHeader);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateMissionItemsDescription(NLNET::CMessage &__message, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateActPositionDescriptions(NLNET::CMessage &__message, const R2::TActPositionDescriptions &actPositionDescriptions);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateUserTriggerDescriptions(NLNET::CMessage &__message, const R2::TUserTriggerDescriptions &userTriggerDescriptions);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onCurrentActIndexUpdated(NLNET::CMessage &__message, uint32 actIndex);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateTalkingAsList(NLNET::CMessage &__message, const std::vector<uint32> &botsId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateIncarningList(NLNET::CMessage &__message, const std::vector<uint32> &botsId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_systemMsg(NLNET::CMessage &__message, const std::string &msgType, const std::string &who, const std::string &msg);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onRingAccessUpdated(NLNET::CMessage &__message, const std::string &ringAccess);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_saveScenarioFileAccepted(NLNET::CMessage &__message, const std::string &md5, const std::string &signature, bool isAccepted);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_loadScenarioFileAccepted(NLNET::CMessage &__message, const std::string &md5, bool ok);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_saveUserComponentFileAccepted(NLNET::CMessage &__message, const std::string &md5, const std::string &signature, bool isAccepted);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_loadUserComponentFileAccepted(NLNET::CMessage &__message, const std::string &md5, bool ok);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_multiPartMsgHead(NLNET::CMessage &__message, const std::string &msgName, uint32 nbPacket, uint32 size);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_multiPartMsgBody(NLNET::CMessage &__message, uint32 partId, uint32 packetSize);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_multiPartMsgFoot(NLNET::CMessage &__message);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_ackMsg(NLNET::CMessage &__message, uint32 msgId, bool ok);




	};
	// Describe an user item in a ring session
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TCharMappedInfo
	{
	protected:
		//
		NLMISC::CSheetId	_ItemSheet;
		//
		ucstring	_Name;
	public:
		//
		const NLMISC::CSheetId &getItemSheet() const
		{
			return _ItemSheet;
		}

		NLMISC::CSheetId &getItemSheet()
		{
			return _ItemSheet;
		}


		void setItemSheet(const NLMISC::CSheetId &value)
		{


				_ItemSheet = value;


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

		bool operator == (const TCharMappedInfo &other) const
		{
			return _ItemSheet == other._ItemSheet
				&& _Name == other._Name;
		}


		// constructor
		TCharMappedInfo()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_ItemSheet);
			s.serial(_Name);

		}


	private:


	};



	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CIOSRingItfSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CIOSRingItfSkel>	TInterceptor;
	protected:
		CIOSRingItfSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CIOSRingItfSkel()
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

		typedef void (CIOSRingItfSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void storeItemNamesForAIInstance_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CIOSRingItfSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// DSS send a list of ring names user item with a AI instance
		virtual void storeItemNamesForAIInstance(NLNET::IModuleProxy *sender, uint32 aiInstance, const std::vector < TCharMappedInfo > &itemInfos) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CIOSRingItfProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CIOSRingItfSkel	*_LocalModuleSkel;


	public:
		CIOSRingItfProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CIOSRingItfSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CIOSRingItfProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// DSS send a list of ring names user item with a AI instance
		void storeItemNamesForAIInstance(NLNET::IModule *sender, uint32 aiInstance, const std::vector < TCharMappedInfo > &itemInfos);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_storeItemNamesForAIInstance(NLNET::CMessage &__message, uint32 aiInstance, const std::vector < TCharMappedInfo > &itemInfos);




	};

}

#endif
