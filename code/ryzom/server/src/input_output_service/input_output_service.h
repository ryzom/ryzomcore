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



#ifndef INPUT_OUTPUT_SERVICE_H
#define INPUT_OUTPUT_SERVICE_H

#include "nel/misc/types_nl.h"
#include "nel/net/service.h"
#include "nel/misc/sstring.h"

//#include "game_share/generic_msg_mngr.h"
#include "game_share/generic_xml_msg_mngr.h"

#include "chat_manager.h"
#include "game_share/ryzom_entity_id.h"
#include "string_manager.h"

#include "game_share/mirror_prop_value.h"
#include "game_share/mirror.h"
#include "game_share/player_visual_properties.h"
#include "game_share/shard_names.h"

#include "game_share/r2_basic_types.h"


extern bool							ShowChat;

extern uint8 MaxDistSay; 
extern uint8 MaxDistShout;

extern TPropertyIndex DSPropertyAI_INSTANCE;
extern TPropertyIndex DSPropertyCELL;
extern TPropertyIndex DSPropertySHEET;
extern TPropertyIndex DSPropertySHEET_SERVER;
extern TPropertyIndex DSPropertyVPA;
extern TPropertyIndex DSPropertyCONTEXTUAL;

extern CGenericXmlMsgHeaderManager GenericXmlMsgHeaderMngr;


/*#ifndef TRACE_SHARD_MESSAGES
#define TRACE_SHARD_MESSAGES
#endif*/


// typedef uint32 TSessionId;

 
/**
 * CCharacterInfos
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CCharacterInfos
{
public:

	/// character id
	NLMISC::CEntityId	EntityId;
	TDataSetRow			DataSetIndex;

	/// Name of the character
	ucstring						Name;
	/// index of the name in the string manager
	CMirrorPropValue< uint32, CPropLocationPacked<2> >		NameIndex;
	/// Short name (ie name without the $title$ spec)
	ucstring						ShortName;
	/// Short name index
	uint32							ShortNameIndex;
	/// The home mainland session id
	TSessionId						HomeSessionId;
	/// The home mainland session name id (in string mapper)
	NLMISC::TStringId				HomeSessionNameId;
	/// The title of the bot (if any), ie the '$' delimited part of the name
	std::string						Title;
	/// The title string id
	uint32							TitleIndex;
	/// The untranslated bot name index.
	uint32							UntranslatedNameIndex;
	/// The untranslated bot short name index.
	uint32							UntranslatedShortNameIndex;
	/// The untranslated event faction index.
	uint32							UntranslatedEventFactionId;
	
	/// Gender of the entity
	CMirrorPropValueRO< SPropVisualA >	VisualPropertyA;

	/// Keep the AIInstance
	CMirrorPropValueRO<uint32>			AIInstance;


	/// TODO : REMOVE when old string DB removed : index of the name in the dynamic string database
//	uint32	OldNameIndex;

	/// id of the front end
//	uint16 FrontendId;

	/// Language code used by the player.
	CStringManager::TLanguages	Language;

	// character with privilege (true if have privilege)
	bool HavePrivilege;

		// custom afk text
	ucstring	AfkCustomTxt;

	/**
	 * Default constructor
	 */
	CCharacterInfos() 
		: /*FrontendId(0),*/
			HomeSessionId(0),
			HomeSessionNameId(0),
			Language(CStringManager::work),
			UntranslatedEventFactionId(0),
			HavePrivilege( false )
	{}

	GSGENDER::EGender getGender() const
	{
		if (VisualPropertyA.isReadable())
			return GSGENDER::EGender(VisualPropertyA().PropertySubData.Sex ? GSGENDER::female : GSGENDER::male);
		return GSGENDER::male;
	}
};



class CAIAliasManager
{

	struct TTranslation
	{
		uint32 ShortNameIndex;
		uint32 TitleIndex[CStringManager::NB_LANGUAGES];
		TTranslation()
		{
			ShortNameIndex = 0;
			unsigned int first(0), last(0);
			for ( ; first != last ; ++first)
			{
				TitleIndex[first] = 0;
			}
		}

	};
	std::map<uint32, TTranslation > _Translation;
public:
	void clear();

	void add(uint32 alias, const std::string &name);

	bool is(uint32 alias) const;

//	std::string getName(uint32 alias) const;
	
	uint32 getShortNameIndex(uint32 alias) const;

	uint32 getTitleIndex(uint32 alias, CStringManager::TLanguages lang) const;

};





/**
 * CInputOutputService
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CInputOutputService : public NLNET::IService
{
public:
	typedef std::map<NLMISC::CEntityId, CCharacterInfos *>	TIdToInfos;
private:
	/// chat manager
	CChatManager _ChatManager;

	/// infos on a character from his id
	TIdToInfos _IdToInfos;

	typedef std::map<NLMISC::CSString, CCharacterInfos *, CUnsensitiveSStringLessPred>	TCharInfoCont;
	/// infos on a character from his name
	TCharInfoCont	_NameToInfos;

	/// Original information about renamed characters
	TCharInfoCont	_RenamedCharInfos;

	typedef std::map<NLMISC::CEntityId, std::pair<NLMISC::TGameCycle, CCharacterInfos*> >	TTempCharInfoCont;
	/// Temporary storage for removed entities, will survive here for 3000 ticks.
	TTempCharInfoCont		_RemovedCharInfos;

	CAIAliasManager _AIAliasManager;

//	typedef uint32 TSessionId;

//public:
//	struct TSessionName
//	{
//		/// The home mainland session Id
//		TSessionId			SessionId;
//		/// Display name, as displayed in user interface and appended to player character names
//		std::string			DisplayName;
//		/// pre mapped name (in string mapper)
//		NLMISC::TStringId	DisplayNameId;
//		/// short name used in user commands like "/tell [<shortName>.]<userName>"
//		std::string			ShortName;
//	};
//
//	// This container is just a vector because is is very small and brute force parsing will be faster
//	typedef std::vector<TSessionName>	TSessionNames;
//private:
//	/// Table of home session names
//	TSessionNames	_SessionNames;

public:

	/// The list of named shard
//	CShardNames		ChardNames;

	/** 
	 * init the service
	 */
	void init();

	/// Init after the mirror init
	void initMirror();
	
	void release();

	/**
	 * main loop
	 */
	bool update();
	
	/**
	 *	get the alias manager
	 */
	CAIAliasManager& getAIAliasManager() { return _AIAliasManager; }

	/**
	 *	get the chat manager
	 */
	CChatManager& getChatManager() { return _ChatManager; }


//	const TSessionNames &getSessionNames() const;

	/**
	 *	Add the name of a character
	 */
	void addCharacterName( const TDataSetRow& chId, const ucstring& name, TSessionId homeSessionId );

	/**
	 * Get the infos of character from its id
	 */
	CCharacterInfos * getCharInfos( const NLMISC::CEntityId& chId, bool lookInTemp = true );

	/**
	 * Get the infos of character from its name
	 */
	CCharacterInfos * getCharInfos( const ucstring& name );

	/**
	 * Remove an entity
	 */
	void removeEntity( const TDataSetRow&chId );

	/**
	 *	Remove all the entities managed by a service
	 */
	void releaseEntitiesManagedByService( uint16 serviceId );

	/// debug display
	void display(NLMISC::CLog &log);

	/// React to the mirror changes
	void scanMirrorChanges();

	/// char info acces (for string manager)
	TIdToInfos &getCharInfosCont() { return _IdToInfos;}

	CMirror				Mirror;
	CMirroredDataSet	*DataSet;
};


extern CInputOutputService * IOS;


#define TheDataset (*(IOS->DataSet))


#endif // INPUT_OUTPUT_SERVICE_H

/* End of input_output_service.h */
