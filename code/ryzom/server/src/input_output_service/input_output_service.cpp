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
#include "input_output_service.h"

#include "game_share/tick_event_handler.h"
#include "game_share/msg_client_server.h"
#include "game_share/mode_and_behaviour.h" //TEMP!!!
#include "game_share/news_types.h"
#include "game_share/bot_chat_types.h"
#include "game_share/brick_types.h"
#include "game_share/loot_harvest_state.h"
#include "game_share/ryzom_version.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/singleton_registry.h"
#include "server_share/logger_service_client.h"

#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/net/message.h"

#include "news_manager.h"
#include "string_manager.h"
#include "messages.h"
//#include "ios_pd.h"

/*
#if defined(NL_DEBUG) && defined(NL_OS_WINDOWS)
#include <crtdbg.h>
#endif
*/

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}


using namespace NLNET;
using namespace NLMISC;
using namespace std;

CInputOutputService * IOS = NULL;

uint8 MaxDistSay = 1; 
uint8 MaxDistShout = 3;

// true if we display all chat received
bool ShowChat = false;

CVariable<bool>	VerboseNameTranslation("ios","VerboseNameTranslation", "Set verbosity for bot name trnaslation", false, 0, true);
extern CVariable<bool>	VerboseChatManagement;


CGenericXmlMsgHeaderManager GenericXmlMsgHeaderMngr;


void CAIAliasManager::clear()
{ 
	_Translation.clear(); 
}

void CAIAliasManager::add(uint32 alias, const std::string &name)
{
//	_Translation[alias].first = name;
	//_Translation[alias].second = 0; //undef
	CCharacterInfos cInfo;
	

	std::string botName = name;
	ucstring ucname  = botName;

		//addCharacterName
	
	ucstring title;

	// remove any $title$ specification in the short name
	ucstring::size_type pos = ucname.find('$');
	if (pos != ucstring::npos)
	{
		cInfo.ShortName = ucname.substr(0, pos);
		// extract $title$ spec in the title
		ucstring::size_type pos2 = ucname.find('$', pos+1);
		title = ucname.substr(pos+1, pos2-pos-1);
		cInfo.Title = title.toString();
		cInfo.TitleIndex = SM->storeString(title);
	}
	else
		cInfo.ShortName = ucname;


	cInfo.ShortNameIndex = SM->storeString(cInfo.ShortName);

	// try to map a translated bot name on the short name
	cInfo.UntranslatedNameIndex = SM->storeString(ucname);
	cInfo.UntranslatedShortNameIndex = SM->storeString(cInfo.ShortName);
	

	cInfo.ShortNameIndex = SM->translateShortName(cInfo.UntranslatedShortNameIndex);
	
	cInfo.ShortName = SM->getString(cInfo.ShortNameIndex);

	// extract title from the translated bot name (if needed)
	// this allow translated name to overload the title and
	// to support generic name
	{
		ucstring::size_type pos = cInfo.ShortName.find('$');
		if (pos != ucstring::npos)
		{
			ucstring sn = cInfo.ShortName;
			cInfo.ShortName = sn.substr(0, pos);
			// extract $title$ spec in the title
			ucstring::size_type pos2 = sn.find('$', pos+1);
			title = sn.substr(pos+1, pos2-pos-1);
			cInfo.Title = title.toString();
			cInfo.TitleIndex = SM->storeString(title);
		}
	}



	

//	_Translation[alias].UntranslatedShortNameIndex = cInfo.ShortNameIndex;
	if (!cInfo.ShortName.empty())
	{
		uint32 index = SM->translateShortName(cInfo.ShortNameIndex);
		if (index != cInfo.UntranslatedShortNameIndex) // not translated
		{
			_Translation[alias].ShortNameIndex = index;
		}
		else
		{
			_Translation[alias].ShortNameIndex = 0;
		}
	}
	else 
	{
		_Translation[alias].ShortNameIndex = 0;
	}

	
	if (!cInfo.Title.empty())
	{
	//	_Translation[alias].UntranslatedTitleIndex = cInfo.TitleIndex;
		unsigned int first = 0;
		unsigned int last = static_cast<unsigned int>( CStringManager::NB_LANGUAGES );
		for  ( ;first != last; ++first)
		{	
			uint32 index = SM->translateTitle(cInfo.Title, static_cast<CStringManager::TLanguages>(first));;
			if (index == cInfo.TitleIndex)
			{
				_Translation[alias].TitleIndex[first] = 0;
				//not translated
			}
			else
			{
				_Translation[alias].TitleIndex[first] = index;
			}
		}
	}
	else
	{
	//	_Translation[alias].UntranslatedTitleIndex = 0;
		unsigned int first = 0;
		unsigned int last = static_cast<unsigned int>( CStringManager::NB_LANGUAGES );
		for  ( ;first != last; ++first)
		{	
			_Translation[alias].TitleIndex[first] =  0;
		}
	}

}


bool CAIAliasManager::is(uint32 alias) const
{
	return _Translation.find(alias) != _Translation.end(); 
}
/*
std::string CAIAliasManager::getShortName(uint32 alias) const 
{ 
	return _Translation.find(alias)->second.ShortName; 
}*/

uint32 CAIAliasManager::getShortNameIndex(uint32 alias) const 
{ 
	return _Translation.find(alias)->second.ShortNameIndex; 
}

uint32 CAIAliasManager::getTitleIndex(uint32 alias, CStringManager::TLanguages lang) const 
{
	nlassert( lang < CStringManager::NB_LANGUAGES );
	return _Translation.find(alias)->second.TitleIndex[lang]; 
}


//-----------------------------------------------
//	cbConnection :
//
//-----------------------------------------------
static void cbConnection( const string &serviceName, TServiceId serviceId, void *arg )
{

} // cbConnection //



//-----------------------------------------------
//	cbDisconnection :
//
//-----------------------------------------------
static void cbDisconnection( const string &serviceName, TServiceId serviceId, void *arg )
{
	//IOS->releaseEntitiesManagedByService( serviceId ); // obsolete

	// warn the chat manager
	IOS->getChatManager().onServiceDown(serviceName);
	
} // cbDisconnection //




//---------------------------------------------------
// iosUpdate :
// 
//---------------------------------------------------
void iosUpdate()
{
/*
#if defined(NL_DEBUG) && defined(NL_OS_WINDOWS)
	_CrtCheckMemory();
#endif
*/

//	IOSPD::update();

} // iosUpdate //



//---------------------------------------------------
// iosSync :
// 
//---------------------------------------------------
void iosSync()
{
} // iosSync //



//-----------------------------------------------
//	update :
//
//-----------------------------------------------

bool CInputOutputService::update()
{
	// clear old charactere names
	NLMISC::TGameCycle now = CTickEventHandler::getGameCycle();

	TTempCharInfoCont::iterator first(_RemovedCharInfos.begin());
	for (; first != _RemovedCharInfos.end(); )
	{
		if (first->second.first < now)
		{
			// need to erase this one
			delete first->second.second;
		
			CEntityId	id = first->first;
			_RemovedCharInfos.erase(first);


			first = _RemovedCharInfos.lower_bound(id);
		}
		else
			++first;
	}

	return true;
} // update //


/*
 * Initialisation 2
 */
void	cbMirrorIsReady( CMirror *mirror )
{
	IOS->initMirror();

	// activate logs
	LGS::ILoggerServiceClient::startLoggerComm();
}


//-----------------------------------------------
//	init :
//
//-----------------------------------------------
void CInputOutputService::init()
{
//	CPath::addSearchPath("data_leveldesign/leveldesign", true, false) ;

	CSingletonRegistry::getInstance()->init();

	// init string manager parameter traits
	CStringManager::CParameterTraits::init();

	setVersion (RYZOM_VERSION);
	
	IOS = this;
	setUpdateTimeout(100);

	// load config infos
//	string staticDBFileName = "chat_static.cdb";
//	string dynamicDBFileName = "chat_dynamic.occ";
//	try
//	{
//		CConfigFile::CVar& cvStaticDB = ConfigFile.getVar("StaticDB");
//		staticDBFileName = cvStaticDB.asString();
//
//		CConfigFile::CVar& cvDynamicDB = ConfigFile.getVar("DynamicDB");
//		dynamicDBFileName = cvDynamicDB.asString();
//	}
//	catch(const EUnknownVar &) 
//	{
//		nlwarning("<CInputOutputService::init> using default chat files");
//	}
	try
	{
		CConfigFile::CVar& cvMaxDistSay = ConfigFile.getVar("MaxDistSay");
		MaxDistSay = cvMaxDistSay.asInt();

		CConfigFile::CVar& cvMaxDistShout = ConfigFile.getVar("MaxDistShout");
		MaxDistShout = cvMaxDistShout.asInt();
	}
	catch(const EUnknownVar &) 
	{
		nlinfo("<CInputOutputService::init> using default chat max distance values");
	}

	if (IService::getInstance()->haveArg('Q'))
		SM->setTestOnly();

	// init mission string manager.
	SM->init(); 

	if (IService::getInstance()->haveArg('Q'))
	{
		// only for testing phrase, no more to do
		return;
	}

	// init IOSPD
//	IOSPD::init(1);

	// init the chat manager
	_ChatManager.init( /*CPath::lookup(staticDBFileName, false), CPath::lookup(dynamicDBFileName, false) */);

	// Init the XML message manager
	const string pathXmlMsg = CPath::lookup( "msg.xml" );
	GenericXmlMsgHeaderMngr.init( pathXmlMsg );
	CMessages::init();

	// Init the mirror system
	vector<string> datasetNames;
	datasetNames.push_back( "fe_temp" );
	Mirror.init( datasetNames, cbMirrorIsReady, iosUpdate, iosSync );

	CUnifiedNetwork::getInstance()->setServiceUpCallback( string("*"), cbConnection, 0);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( string("*"), cbDisconnection, 0);		

	CShardNames::getInstance().init(ConfigFile);

//	// read the mainland session name table
//	CConfigFile::CVar *sessionNames = ConfigFile.getVarPtr("HomeMainlandNames");
//	if (sessionNames == NULL)
//	{
//		nlwarning("No variable 'HomeMainlandNames', domain unified character names will not work correctly !");
//	}
//	else
//	{
//		for (uint i=0; i<sessionNames->size()/3; ++i)
//		{
//			TSessionName sn;
//			sn.SessionId = uint32(sessionNames->asInt(i*3));
//			sn.DisplayName = sessionNames->asString(i*3+1);
//			sn.ShortName = sessionNames->asString(i*3+2);
//			sn.DisplayNameId = CStringMapper::map(sn.DisplayName);
//			
//
//			_SessionNames.push_back(sn);
//		}
//
//		nlinfo("Read %u home session names from config files", _SessionNames.size());
//	}

} // init //


void cbScanMirrorChanges()
{
	IOS->scanMirrorChanges();
}


/*
 * Init after the mirror init
 */
void CInputOutputService::initMirror()
{
	DataSet = &(Mirror.getDataSet("fe_temp"));
	DataSet->declareProperty( "Sheet", PSOReadOnly );
	DataSet->declareProperty( "SheetServer", PSOReadOnly );
	DataSet->declareProperty( "NameIndex", PSOReadWrite );
	//DataSet->declareProperty( "Target", PSOReadOnly );
	DataSet->declareProperty( "Cell", PSOReadOnly );
	DataSet->declareProperty( "Stunned", PSOReadOnly );
	DataSet->declareProperty( "VisualPropertyA", PSOReadOnly );
	DataSet->declareProperty( "ContextualProperty", PSOReadOnly );
	DataSet->declareProperty( "EventFactionId", PSOReadWrite );
	DataSet->declareProperty( "AIInstance", PSOReadOnly );


	// No need for all the ones of ryzom_mirror_properties.h
	DSPropertyNAME_STRING_ID = DataSet->getPropertyIndex( "NameIndex" );
	DSPropertyCELL = DataSet->getPropertyIndex( "Cell" );
	DSPropertySHEET = DataSet->getPropertyIndex("Sheet");
	DSPropertySHEET_SERVER = DataSet->getPropertyIndex("SheetServer");
	DSPropertyVPA = DataSet->getPropertyIndex("VisualPropertyA");
	DSPropertyCONTEXTUAL = DataSet->getPropertyIndex("ContextualProperty");
	DSPropertyEVENT_FACTION_ID = DataSet->getPropertyIndex("EventFactionId" );
	DSPropertyAI_INSTANCE = DataSet->getPropertyIndex("AIInstance" );

	Mirror.setNotificationCallback( cbScanMirrorChanges );
}


/*
 * React to the mirror changes
 */
void CInputOutputService::scanMirrorChanges()
{
	// Scan additions
	DataSet->beginAddedEntities();
	TDataSetRow entityIndex = DataSet->getNextAddedEntity();
	while ( entityIndex != LAST_CHANGED )
	{
		const CEntityId & entityId = DataSet->getEntityId( entityIndex );

		// Mirrorize the name index
		CCharacterInfos * charInfos = IOS->getCharInfos( entityId, false );
		if ( charInfos != NULL )
		{
			nlwarning( "ERROR: New entity E%hu %s has already charInfo (by CHARACTER_NAME!)", entityIndex.getIndex(), entityId.toString().c_str() );
			// addCharacterName() has already been called before
			//charInfos->NameIndex.tempMirrorize( *DataSet, entityIndex, DSPropertyNAME_STRING_ID );
			//charInfos->VisualPropertyA.init( *DataSet, entityIndex, DSPropertyVPA );
		}
		else
		{
			// init the nameID property to 'no name' (except for forage sources, done by EGS)
			if ( entityId.getType() != RYZOMID::forageSource )
			{
				CMirrorPropValue< uint32, CPropLocationPacked<2> > nameId(*DataSet, entityIndex, DSPropertyNAME_STRING_ID);
				nameId = ~0; // force emitting the value, as a workaround to RT 12501
				nameId = 0;  // (potential bug of CDataSetMS::syncEntityToDataSet())
			}
		}

		if (entityId.getType() == RYZOMID::player)
		{
			// init event faction ID in mirror
			//CMirrorPropValue<TYPE_EVENT_FACTION_ID> propEventFactionId( TheDataset, entityIndex, DSPropertyEVENT_FACTION_ID );
			//propEventFactionId = 0;
		}

		entityIndex = DataSet->getNextAddedEntity();
	}
	DataSet->endAddedEntities();

	// Scan removals
	CEntityId *entityId;
	DataSet->beginRemovedEntities();
	entityIndex = DataSet->getNextRemovedEntity( &entityId );
	while ( entityIndex != LAST_CHANGED )
	{
		removeEntity( entityIndex );
		entityIndex = DataSet->getNextRemovedEntity( &entityId );
	}
	DataSet->endRemovedEntities();
}


//const CShardNames::TSessionNames &CInputOutputService::getSessionNames() const
//{
//	return _SessionNames;
//}


//-----------------------------------------------
//	addCharacterName :
//
//-----------------------------------------------
void CInputOutputService::addCharacterName( const TDataSetRow& chId, const ucstring& ucname, TSessionId homeSessionId )
{
	if (!chId.isValid())
	{
		nlwarning("addCharacterName: receiveing char info with an invalid dataset row (datasetRow = %u, name = '%s'). IGNORING.",
			chId.getIndex(),
			ucname.toString().c_str());
		return;
	}
	// Add player in the chat manager (note: not done when chId added in the mirror, because callback called earlier)
	if (!IOS->getChatManager().checkClient(chId))
		IOS->getChatManager().addClient( chId ); // may has been already added if this is a player (cf cbAddToGroup)

	CEntityId eid = TheDataset.getEntityId(chId);

	ucstring oldname;
	CCharacterInfos * charInfos = IOS->getCharInfos( eid, false );
	if( charInfos == NULL )
	{
		charInfos = new CCharacterInfos();
		// store association
		_IdToInfos.insert( make_pair(eid,charInfos) );
	}
	else
	{
		// remove previous name association
//		_NameToInfos.erase(ucname);
		_NameToInfos.erase(charInfos->ShortName.toUtf8());
		oldname = charInfos->ShortName;

		if (charInfos->EntityId != eid)
		{
			nlwarning("Bad id (%s) in charInfo associated to entity %s", charInfos->EntityId.toString().c_str(), eid.toString().c_str());
			charInfos->EntityId = eid;
		}
		if (charInfos->DataSetIndex != chId)
		{
			nlwarning("Bad id (%x) in charInfo associated to entity %x", charInfos->DataSetIndex.getIndex(), chId.getIndex());
			charInfos->DataSetIndex = chId;
		}
	}

	ucstring title;
	charInfos->EntityId = eid;
	charInfos->DataSetIndex = chId;
		
	// remove any $title$ specification in the short name
	ucstring::size_type pos = ucname.find('$');
	if (pos != ucstring::npos)
	{
		charInfos->ShortName = ucname.substr(0, pos);
		// extract $title$ spec in the title
		ucstring::size_type pos2 = ucname.find('$', pos+1);
		title = ucname.substr(pos+1, pos2-pos-1);
		charInfos->Title = title.toString();
		charInfos->TitleIndex = SM->storeString(title);
	}
	else
	{
		charInfos->ShortName = ucname;
	}

	if (eid.getType() == RYZOMID::player)
	{
		// store the premapped session display name
		for (uint i=0; i<CShardNames::getInstance().getSessionNames().size(); ++i)
		{
			if (CShardNames::getInstance().getSessionNames()[i].SessionId == homeSessionId)
			{
				charInfos->HomeSessionId = CShardNames::getInstance().getSessionNames()[i].SessionId;
				charInfos->HomeSessionNameId = CShardNames::getInstance().getSessionNames()[i].DisplayNameId;

				break;
			}
		}

		// See if an existing player was renamed
		if (!oldname.empty())
		{
			TSessionId sessionid;
			string name = charInfos->ShortName.toUtf8();
			// Make sure that the short name contains the home session name, but only if the new name does not exist yet
			// otherwise we will have problems. If the new name contains opening parentheses then don't add it because
			// it will try to match it as a homeland name
			string::size_type pos = name.find('(');
			if (pos == string::npos)
			{
				name = CShardNames::getInstance().makeFullName(name, charInfos->HomeSessionId);
			}

			TCharInfoCont::iterator itInfos = _NameToInfos.find( name );
			if( itInfos == _NameToInfos.end() )
			{
				// New name does not exist
				charInfos->ShortName.fromUtf8(name);
			}

			// Save the old name only if new name is not found (and the player is getting original name back)
			itInfos = _RenamedCharInfos.find( charInfos->ShortName.toUtf8() );
			if( itInfos != _RenamedCharInfos.end() )
			{
				// New name was in the saved list; player is getting original name back. 
				// Remove the new name
				_RenamedCharInfos.erase(charInfos->ShortName.toUtf8());
			}
			else
			{
				// New name was not in the list, save old name
				_RenamedCharInfos.insert( make_pair(oldname.toUtf8(), charInfos) );
			}
		}
	}

	// try to map a translated bot name on the short name
	charInfos->UntranslatedNameIndex = SM->storeString(ucname);
	charInfos->UntranslatedShortNameIndex = SM->storeString(charInfos->ShortName);
	
	// don't translate players names
	if (eid.getType() != RYZOMID::player)
	{
		charInfos->ShortNameIndex = SM->translateShortName(charInfos->UntranslatedShortNameIndex);
	}
	else
	{
		charInfos->ShortNameIndex = charInfos->UntranslatedShortNameIndex; 
	}

	charInfos->ShortName = SM->getString(charInfos->ShortNameIndex);

	// extract title from the translated bot name (if needed)
	// this allow translated name to overload the title and
	// to support generic name
	{
		ucstring::size_type pos = charInfos->ShortName.find('$');
		if (pos != ucstring::npos)
		{
			ucstring sn = charInfos->ShortName;
			charInfos->ShortName = sn.substr(0, pos);
			// extract $title$ spec in the title
			ucstring::size_type pos2 = sn.find('$', pos+1);
			title = sn.substr(pos+1, pos2-pos-1);
			charInfos->Title = title.toString();
			charInfos->TitleIndex = SM->storeString(title);
		}
	}
	
	// build the translated name
	if (!charInfos->Title.empty())
		charInfos->Name = charInfos->ShortName + "$" + charInfos->Title+"$";
	else
		charInfos->Name = charInfos->ShortName;

	if (VerboseNameTranslation && charInfos->Name != charInfos->ShortName)
	{
		if (charInfos->ShortNameIndex != charInfos->UntranslatedShortNameIndex)
		{
			string sn = charInfos->ShortName.toString();
			string usn = SM->getString(charInfos->UntranslatedShortNameIndex).toString();
			nlinfo(" Translated short name for this character : '%s' (index %u) (untranslated : '%s')", sn.c_str(), charInfos->ShortNameIndex, usn.c_str());
		}
		else
		{
			string sn = charInfos->ShortName.toString();
			nlinfo(" Short name for this character : '%s' (index %u)", sn.c_str(), charInfos->ShortNameIndex);
		}
	}

	// store (or pre-store) the full translated name in the mirror
//	TDataSetRow entityIndex = DataSet->getDataSetRow( chId );
	if ( chId.isValid())
	{
		// Already in the mirror
		if ( ! charInfos->NameIndex.isInitialized() )
		{
			if ( charInfos->NameIndex.isReadable() )
			{
				nlwarning( "ERROR: Received CHARACTER_NAME for a NameIndex already set" ); // TEMP
				charInfos->NameIndex.tempDelete();
			}
			charInfos->NameIndex.init( TheDataset, chId, DSPropertyNAME_STRING_ID );
		}
		charInfos->NameIndex = SM->storeString(charInfos->Name);
		charInfos->VisualPropertyA.init( TheDataset, chId, DSPropertyVPA );
		charInfos->AIInstance.init( TheDataset, chId, DSPropertyAI_INSTANCE );
	}
	else
	{
		nlwarning( "ERROR: Received CHARACTER_NAME with null row!" );
		// Previously: not yet in the mirror
		/*if ( ! charInfos->NameIndex.isReadable() )
			charInfos->NameIndex.tempAllocate();
		charInfos->NameIndex.tempReassign(SM->storeString(charInfos->Name));*/
	}
	if (VerboseNameTranslation)
	{
		if (charInfos->NameIndex == charInfos->UntranslatedNameIndex)
			nldebug("IOS: addCharacterName Adding name '%s' for entity %s:%x",
				ucname.toString().c_str(),
				TheDataset.getEntityId(chId).toString().c_str(),
				chId.getIndex());
		else
			nldebug("IOS: addCharacterName Adding name '%s' translated as '%s' for entity %s:%x",
				ucname.toString().c_str(), 
				charInfos->Name.toString().c_str(), 
				TheDataset.getEntityId(chId).toString().c_str(),
				chId.getIndex());
	}

	// store name assoc
//	_NameToInfos.insert( make_pair(ucname,charInfos) );
	// store the short name assoc.
	_NameToInfos.insert( make_pair(charInfos->ShortName.toUtf8(), charInfos) );
	// TODO : remove when dynDB removed
//	charInfos->OldNameIndex = IOS->getChatManager().getDynamicDB().add(charInfos->ShortName, false);

} // addCharacterName //



//-----------------------------------------------
//	getCharInfos :
//
//-----------------------------------------------
CCharacterInfos * CInputOutputService::getCharInfos( const CEntityId& chId, bool lookInTemp )
{
	TIdToInfos::iterator itInfos = _IdToInfos.find( chId );
	if( itInfos != _IdToInfos.end() )
	{
		return 	itInfos->second;
	}
	else if (lookInTemp)
	{
		// look inside temp storage
		TTempCharInfoCont::iterator it (_RemovedCharInfos.find(chId));
		if (it != _RemovedCharInfos.end())
		{
			return it->second.second;
		}
	}

	return NULL;

} // getCharInfos //



//-----------------------------------------------
//	getCharInfos :
//
//-----------------------------------------------
CCharacterInfos * CInputOutputService::getCharInfos( const ucstring& chName )
{
	TCharInfoCont::iterator itInfos = _NameToInfos.find( chName.toUtf8() );
	if( itInfos != _NameToInfos.end() )
	{
		return 	itInfos->second;
	}
	
	// Not found so check any renamed players
	itInfos = _RenamedCharInfos.find( chName.toUtf8() );
	if( itInfos != _NameToInfos.end() )
	{
		return 	itInfos->second;
	}
	else
	{
		return NULL;
	}
} // getCharInfos //



//-----------------------------------------------
//	removeEntity :
//
//-----------------------------------------------
void CInputOutputService::removeEntity( const TDataSetRow &chId )
{
	if (_ChatManager.checkClient(chId))
	{
		if (VerboseChatManagement)
			nldebug("IOSCU: removeEntity : removing the client %s:%x from chat manager !", 
				TheDataset.getEntityId(chId).toString().c_str(),
				chId.getIndex());
		_ChatManager.removeClient( chId );
	}

	CEntityId eid = TheDataset.getEntityId(chId);

	ucstring name;
//	uint32 index;
	TIdToInfos::iterator itInfos = _IdToInfos.find( eid );
	if( itInfos != _IdToInfos.end() )
	{
		CCharacterInfos *oldChar = itInfos->second;
		// create a temporary copy of the charactere info.
		CCharacterInfos *tempChar = new CCharacterInfos;

		tempChar->EntityId = oldChar->EntityId;
		tempChar->DataSetIndex = TDataSetRow();
		tempChar->Name = oldChar->Name;
		if (oldChar->NameIndex.isReadable())
			tempChar->NameIndex.tempStore(oldChar->NameIndex());
		tempChar->ShortName = oldChar->ShortName ;
		tempChar->ShortNameIndex = oldChar->ShortNameIndex ;
		tempChar->Title = oldChar->Title ;
		tempChar->TitleIndex = oldChar->TitleIndex;
		tempChar->UntranslatedNameIndex = oldChar->UntranslatedNameIndex ;
		tempChar->UntranslatedShortNameIndex = oldChar->UntranslatedShortNameIndex ;
		tempChar->UntranslatedEventFactionId = oldChar->UntranslatedEventFactionId;
		if (oldChar->VisualPropertyA.isReadable())
			tempChar->VisualPropertyA.tempStore(oldChar->VisualPropertyA()) ;
//		tempChar->OldNameIndex = oldChar->OldNameIndex ;
		tempChar->Language = oldChar->Language ;

		// store the temporary char info.
		_RemovedCharInfos.insert(make_pair(tempChar->EntityId, make_pair(CTickEventHandler::getGameCycle(), tempChar)));

//		index = itInfos->second->OldNameIndex;

		_NameToInfos.erase(itInfos->second->ShortName.toUtf8());

		// erase the entry in _IdToInfos
		delete itInfos->second;
		itInfos->second = NULL;
		_IdToInfos.erase( itInfos );

		// erase the entry in _NameToInfos		
/*		CDynamicStringInfos * infos = _ChatManager.getDynamicDB().getInfos( index );
		if( infos )
		{
			name = infos->Str;
			map<ucstring,CCharacterInfos *>::iterator itInfos2 = _NameToInfos.find( name );
			if( itInfos2 != _NameToInfos.end() )
			{
				_NameToInfos.erase( itInfos2 );
			}
*/
/*			else
			{
				nlwarning("<CInputOutputService::removeEntity> Unknown entity : %s",name.toString().c_str());
			}
*//*		}
		else
		{
			nlwarning("<CInputOutputService::removeEntity> Dynamic string %d unknown",index);
		}
*/
	}
	else
	{
		if (eid.getType() == RYZOMID::player)
			nlwarning("<CInputOutputService::removeEntity> Unknown entity : %s",eid.toString().c_str());
	}

} // removeEntity //




//-----------------------------------------------
//	releaseEntitiesManagedByService :
//  OBSOLETE
//-----------------------------------------------
/*
void CInputOutputService::releaseEntitiesManagedByService( TServiceId serviceId )
{
	map<CEntityId,CCharacterInfos *>::iterator itInfos = _IdToInfos.begin();
	while(itInfos != _IdToInfos.end())
	{
		map<CEntityId,CCharacterInfos *>::iterator tmp = itInfos;
		++itInfos;
		if( tmp->first.getDynamicId() == serviceId )
			IOS->removeEntity( tmp->second->DataSetIndex );
	}
} // releaseEntitiesManagedByService //
*/



//-----------------------------------------------
//	display :
//
//-----------------------------------------------
void CInputOutputService::display(NLMISC::CLog &log)
{
//	log.displayNL("DYNAMIC DATABASE : ");
//	IOS->getChatManager().getDynamicDB().display(log);

	log.displayNL("ENTITIES : ");
	TCharInfoCont::iterator itInfos;
	for( itInfos = _NameToInfos.begin(); itInfos != _NameToInfos.end(); ++itInfos )
	{
//		uint32 nameIndex = itInfos->second->OldNameIndex;
//		CDynamicStringInfos * infos = IOS->getChatManager().getDynamicDB().getInfos( nameIndex );
		log.displayNL("Name: %s \tentity: %s" /* \tindex: %d \tstr: %s"*/, 
			itInfos->first.c_str(), 
			itInfos->second->EntityId.toString().c_str() 
			/*, infos->Index, 
			infos->Str.toString().c_str()*/);	
	}
	log.displayNL("CHAT GROUPS : ");
	IOS->getChatManager().displayChatGroups(log, false, false);

} // display //



//-----------------------------------------------
//	~CInputOutputService :
//
//-----------------------------------------------
void CInputOutputService::release()
{
	CMessages::release();

	TIdToInfos::iterator itInfos;
	for( itInfos = _IdToInfos.begin(); itInfos != _IdToInfos.end(); ++itInfos )
	{
		delete (*itInfos).second;
		(*itInfos).second = NULL;
	}

//	getChatManager().getStaticDB().saveStats( "data_common/chat/chat_static.occ" );

//	IOSPD::release();
}

TUnifiedCallbackItem CbArray[]=
{
	{"----",				NULL		}
};



/*-----------------------------------------------------------------*\
						NLNET_SERVICE_MAIN
\*-----------------------------------------------------------------*/
//#define LOCAL_TEST
#ifndef LOCAL_TEST
NLNET_SERVICE_MAIN( CInputOutputService, "IOS", "input_output_service", 0, CbArray, "", "" );
#else

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdline, int nCmdShow)
{
	IOS = new CInputOutputService();

	IOS->init();
	return 1;
	
}

#endif





 


