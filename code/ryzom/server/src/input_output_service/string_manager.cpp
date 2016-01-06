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
#include "string_manager.h"
#include "input_output_service.h"
#include "nel/misc/i18n.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/net/unified_network.h"
#include "nel/net/service.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/diff_tool.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/backup_service_interface.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"
#include "server_share/r2_variables.h"
#include <time.h>

//---------------------------------------------------------------------------------------
// Stuff used for management of log messages

//static bool VerboseLog=false;


//bool DebugReplacementParameter = false;
//NLMISC_VARIABLE(bool, DebugReplacementParameter, "Debug missing replacement parameter.");
NLMISC::CVariable<bool> DebugReplacementParameter("ios","DebugReplacementParameter", "Insert error debugging information in generated text", false, 0, true);
NLMISC::CVariable<bool> VerboseStringManager("ios","VerboseStringManager", "Turn on or off or check the state of verbose string manager logging", false, 0, true);
NLMISC::CVariable<bool> VerboseStringManagerParser("ios","VerboseStringManagerParser", "Turn on or off or check the state of verbose string manager logging when parsing files", false, 0, true);
NLMISC::CVariable<std::string>	StringManagerCacheDirectory("ios","StringManagerCacheDirectory", "Directory to read/write string cache file (default (empty) is service SaveFilesDirectory)", "", 0, true);

#define LOG if (!VerboseStringManager) {} else nlinfo
#define LOGPARSE if (!VerboseStringManagerParser) {} else nlinfo


// Instantiate the string manager.
CStringManager Instance;
CStringManager *SM = &Instance;
CIosLocalSender	IosLocalSender;

using namespace STRING_MANAGER;
using namespace NLMISC;
using namespace NLNET;
using namespace std;

const ucstring		nl("\r\n");


std::string CStringManager::_LanguageCode[NB_LANGUAGES] = 
{
	"wk",		// Work
	"en",		// english
	"de",
	"fr",
	"ru",
	"es",

/* ace: currently, we only want english, i remove other language to remove warning during IOS launch
	"fr",		// french
	"zh",		// traditionnal chinese
	"zh-CN"		// simplified chinese
*/
};

/*
ucstring	CStringManager::getEntityDisplayName(const NLMISC::CEntityId &eid)
{
	ucstring ret;
	NLMISC::CSheetId sid = SM->getSheetId(eid);
	if (sid != NLMISC::CSheetId::Unknown)
	{
//		TSheetInfoContainer::iterator it(_SheetInfo.find(sid));

		if (it != _SheetInfo.end())
			ret =  it->second.DisplayName;
	}

	// not found.
	if (ret.empty())
		ret = ucstring(eid.toString());

	return ret;
}
*/





uint32	CStringManager::CEntityWords::getStringId(const std::string &rowName, const std::string columnName) const
{
	std::map<std::string, uint32>::const_iterator colIt(_ColumnInfo.find(columnName));
	if (colIt != _ColumnInfo.end())
	{
		std::map<std::string, uint32>::const_iterator rowIt(_RowInfo.find(rowName));
		if (rowIt != _RowInfo.end())
		{
			return _Data[rowIt->second*_NbColums + colIt->second];
		}
	}
	// not found, return rowName.columnName.
	if (DebugReplacementParameter)
		return SM->storeString(ucstring(std::string("<")+rowName+"."+columnName+">"));
	else
	{
		ucstring s;
		s += ucchar(8);
		return SM->storeString(s);
	}
}



CStringManager::CStringManager()
{
	_TestOnly = false;
	_CacheLoaded = false;
	_Mapper = NLMISC::CStringMapper::createLocalMapper();
	_DefaultSetPhraseLanguage = NB_LANGUAGES;
	// init the game share string manager pointer.
//	GameShareSM = this;
}


void CStringManager::loadCache()
{
	if (_CacheLoaded)
	{
		return;
	}
	nlassert(_StringIdx.empty());
	nlassert(_StringBase.empty());

//	std::string filename("data_shard/ios.string_cache");
//	_CacheFilename = NLMISC::CPath::lookup(filename, false, false);
//	if (!_CacheFilename.empty())

	std::string	cacheDirectory = ((StringManagerCacheDirectory.get() == "") ? Bsi.getLocalPath() : StringManagerCacheDirectory.get());
	_CacheFilename = CPath::standardizePath(cacheDirectory) + "ios.string_cache";
	if (CFile::fileExists(_CacheFilename))
	{
		nlinfo("Reading string cache...");
		// ok, load the cache data
		NLMISC::CIFile file(_CacheFilename);
		file.serial(_CacheTimestamp);

		uint32 start = CTime::getSecondsSince1970();
		uint32 t1 = start;

		while (!file.eof())
		{
			uint32 id;
			ucstring str;

			file.serial(id);
			file.serial(str);

//			nldebug("Loaded from cache [%u][%s]", id, str.toString().c_str());
			// create a new entry
			std::pair<TMappedUStringContainer::iterator, bool> ret;
			ret = _StringIdx.insert(std::make_pair(str, id));
//			nlassert(ret.second);
			if (!ret.second)
			{
				TMappedUStringContainer::iterator it = _StringIdx.find(str);
				nlassert(it != _StringIdx.end());
				nlwarning("String cache : string [%s][%u] already in the string map with id [%u] !", str.c_str(), id, it->second);
				if (id != it->second)
				{
					nlwarning(" !!! ID diff : string cache invalide.");
				}
			}
//			if (_StringBase.size() <= id)
//			{
//				_StringBase.reserve(_StringBase.size()*2);
//			}
//			_StringBase.resize(id+1);
//			_StringBase[id] = str;
			while (_StringBase.size() <= id)
				_StringBase.push_back(string());
			_StringBase[id] = str;

			// some logging
			uint32 now = CTime::getSecondsSince1970();
			if ((now - t1) > 10)
			{
				// more than 10 s... log where we are
				double progress = 100*(file.getPos()/double(file.getFileSize()));

				nlinfo(" %6.2f%% read", progress);

				t1 = now;
			}
		}
		uint32 now = CTime::getSecondsSince1970();
		nlinfo("Done, %u strings read in %u seconds.", _StringBase.size(), now-start);
	}
	else
	{
		// create a new cache
		if (!CFile::isExists(cacheDirectory))
			CFile::createDirectoryTree(cacheDirectory);

		_CacheTimestamp = CTime::getSecondsSince1970();
		NLMISC::COFile file(_CacheFilename);
		file.serial(_CacheTimestamp);
	}

	_CacheLoaded = true;
}

void CStringManager::clearCache(NLMISC::CLog *log)
{
	if (_CacheFilename.empty())
	{
		log->displayNL("Clear cache requested but no cache file name available!");
		return;
	}

	log->displayNL("Clearing cache file and reloading string files...");
	CFile::deleteFile(_CacheFilename);
	_StringIdx.clear();
	_StringBase.clear();
	_CacheLoaded = false;
	// this will clear all loaded string, then reinit the string manager
	reload(log);

	// warn all the client that they must dropout there cache file
}




const CStringManager::CEntityWords &CStringManager::getEntityWords(TLanguages lang, STRING_MANAGER::TParamType type) const
{
	nlassert(lang < NB_LANGUAGES);
	nlassert(type < STRING_MANAGER::NB_PARAM_TYPES ||type == STRING_MANAGER::self);

	return _AllEntityWords[lang][type];
}


bool CStringManager::CClause::eval(CStringManager::TLanguages lang, const CCharacterInfos *charInfo, const CPhrase *phrase)
{
	bool ret = false;

	// if no condition, it's true !
	if (Conditions.empty())
		return true;

	for (uint i=0; !ret && i<Conditions.size(); ++i)
	{
		
		std::vector<TCondition> &andedCond = Conditions[i];
		bool temp = true;

		for (uint j=0; temp && j<andedCond.size(); ++j)
		{
			const CParameterTraits *param = phrase->Params[andedCond[j].ParamIndex];
			temp &= param->eval(lang,charInfo ,andedCond[j]);
		}
		
		ret |= temp;
	}

	return ret;
}

NLMISC::CSheetId CStringManager::getSheetId(const NLMISC::CEntityId &entityId)
{
	TDataSetRow entityIndex = TheDataset.getDataSetRow( entityId );
	
	if (!entityIndex.isValid())
		return NLMISC::CSheetId::Unknown;

	CMirrorPropValueBase<TYPE_SHEET> sheetId( TheDataset, entityIndex, DSPropertySHEET ); 
	return NLMISC::CSheetId( sheetId );
}

NLMISC::CSheetId CStringManager::getSheetServerId(const NLMISC::CEntityId &entityId)
{
	TDataSetRow entityIndex = TheDataset.getDataSetRow( entityId );

	if (!entityIndex.isValid())
		return NLMISC::CSheetId::Unknown;

	CMirrorPropValueBase<uint32> sheetServerId( TheDataset, entityIndex, DSPropertySHEET_SERVER ); 
	return NLMISC::CSheetId( sheetServerId );
}


const CStringManager::TSheetInfo &CStringManager::getSheetInfo(const NLMISC::CSheetId &sheetId)
{
	TSheetInfoContainer::iterator	it(_SheetInfo.find(sheetId));
	if (it != _SheetInfo.end())
		return it->second;

	static TSheetInfo unknown;
//	nlwarning("Unknown sheetId : %s", sheetId.toString().c_str());
	return unknown;
}



void CStringManager::buildMissingPhraseStream(CCharacterInfos * charInfo, uint32 seqNum, NLMISC::CBitMemStream & bmsOut, const std::string &phraseName)
{
	// store a string for this error message.
	uint32 id = storeString(ucstring("<missing:")+phraseName+">");
	// now, build the message for the client.
	GenericXmlMsgHeaderMngr.pushNameToStream( "STRING_MANAGER:PHRASE_SEND", bmsOut);
	bmsOut.serial(seqNum);
	bmsOut.serial(id);
	
	LOG("Sending phrase [%s] content", phraseName.c_str());
}

bool CStringManager::buildPhraseStream( CCharacterInfos * charInfo, uint32 seqNum, NLNET::CMessage & message, bool debug, CStringManager::TLanguages lang, NLMISC::CBitMemStream & bmsOut)
{
//	uint32		seqNum;
	std::string phraseName;
	message.serial(phraseName);
	LOG("Receiving phrase %u as '%s'", seqNum, phraseName.c_str() );

	if (phraseName.empty())
	{
		nlwarning("buildPhraseStream: phrase name is empty !");
		return false;
	}
	
	// ok, try to find this phrase
	bool found = false;
	TPhrasesContainer::iterator it(_AllPhrases[lang].find(phraseName));
	if (it != _AllPhrases[lang].end())
		found = true;

	// if not found try to get the working text
	if (!found && lang != english)
	{
		it = _AllPhrases[english].find(phraseName);
		if (it != _AllPhrases[english].end())
			found = true;
	}

	// if still not found try to get the work text
	if (!found && lang != work)
	{
		it = _AllPhrases[work].find(phraseName);
		if (it != _AllPhrases[work].end())
		{
			found = true;

			// every phrases should be at least translated in english
			nlwarning("CStringManager::buildPhraseStream the phrase [%s] has not been translated in english!", phraseName.c_str());
		}
	}

	if (!found)
	{
		nlwarning("CStringManager::buildPhraseStream the phrase [%s] is unknown in %s, in english and in work", phraseName.c_str(), _LanguageCode[lang].c_str());

		buildMissingPhraseStream(charInfo, seqNum, bmsOut, phraseName);
		return true;
	}
	
	// ok, we have the phrase, we can parse the parameter from the message
	CPhrase &phrase = it->second;
	//	std::vector<TStringParam>	params;
	//	params.resize(phrase.Params.size());
	uint i;
	
	try
	{
		bool result = true;
		for (i=1; i<phrase.Params.size(); ++i)
		{
			result &= phrase.Params[i]->extractFromMessage(message, debug);
		}

		if (!result)
			nlwarning("Format error extracting parameters in phrase %s, result string could be erroneous !",phrase.Name.c_str() );
	}
	catch(...)
	{
		nlwarning("Exception while extracting parameters in phrase %s, result string could be erroneous !",phrase.Name.c_str() );
		
		// init the rest with default values
		for (; i<phrase.Params.size(); ++i)
		{
			phrase.Params[i]->setDefaultValue();
		}
		//		return;
	}
	
	// update the self parameter with dest eid.
	if ( charInfo )
		phrase.Params[0]->EId = charInfo->EntityId;
	else
		phrase.Params[0]->EId = CEntityId::Unknown;
	
	
	// ok, we have the phrase and a list of typed param, we can search for the good clause.
	found = false;
	for (i=0; i<phrase.Clauses.size(); ++i)
	{
		// if the first clause as no condition, consider it as a fallback clause.
		if (i==0 && phrase.Clauses[i].Conditions.empty())
		{
			// skip it, it will be used only for fallback when no clause match
			continue;
		}
		if (phrase.Clauses[i].eval(lang,charInfo, &phrase))
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		// force the use of the first clause.
		// NB : this is a 'best effort' fallback. Either the first clause has 
		// no condition, so it is designed to be the fallback one, or
		// the first clause has some condition not valid, but we use it.
		i=0;
	}
	
	CClause &clause = phrase.Clauses[i];
	
	// now, build the message for the client.
	GenericXmlMsgHeaderMngr.pushNameToStream( "STRING_MANAGER:PHRASE_SEND", bmsOut);
	bmsOut.serial(seqNum);
	bmsOut.serial(clause.ClientStringId);
	
	// for each replacement parameter in order...
	for (i=0; i<clause.Replacements.size(); ++i)
	{
		TReplacement &rep = clause.Replacements[i];
		CParameterTraits *param = phrase.Params[clause.Replacements[i].ParamIndex];
		
		param->fillBitMemStream(charInfo,lang, rep, bmsOut);
	}
	LOG("Sending phrase [%s] content", phraseName.c_str());
	return true;

}

void CStringManager::receiveUserPhrase(NLNET::CMessage &message, bool debug)
{
	// extract the parameters
	uint32 userId;
	uint32 seqNum;
	message.serial( userId );
	message.serial(seqNum);
	LOG("Receiving a phrase for user id %d", userId);

	// get the user language
	TUserLanguagesContainer::const_iterator itUser = _UsersLanguages.find( userId );
	if ( itUser == _UsersLanguages.end() )
	{
		// just extract the phrase name for more info in the warning
		std::string phraseName;
		message.serial(phraseName);

		nlwarning("CStringManager::receiveUserPhrase '%s', unknown user %u", phraseName.c_str(), userId);
		return;
	}
	// built the stream to be sent to the client
	NLMISC::CBitMemStream bmsOut;
	if ( buildPhraseStream( NULL, seqNum, message, debug, (*itUser).second.Language, bmsOut ) )
	{
		CMessage msgout( "IMPULSION_UID" );
		msgout.serial( userId );
		msgout.serialBufferWithSize((uint8*)bmsOut.buffer(), bmsOut.length());
		NLNET::CUnifiedNetwork::getInstance()->send( (*itUser).second.FrontEndId, msgout );
		nldebug( "IOSSM: Sent IMPULSION_UID to %hu (PHRASE_SEND)", (*itUser).second.FrontEndId.get() );
	}
}

void CStringManager::receivePhrase(NLNET::CMessage &message, bool debug, const std::string &serviceName)
{
	// extract the parameters
//	NLMISC::CEntityId	dest;
	TDataSetRow		dest;
	uint32			seqNum;
	message.serial( dest );
	LOG("Receiving a phrase for char %s:%x", 
		TheDataset.getEntityId(dest).toString().c_str(),
		dest.getIndex() );

	message.serial(seqNum);

	CEntityId	destId = TheDataset.getEntityId(dest);
	// retrieve the dest player infos.
	CCharacterInfos *charInfo = IOS->getCharInfos(destId);
	if (charInfo == 0)
	{
		nlwarning("CStringManager::receivePhrase unknown eid %s:%x", 
			TheDataset.getEntityId(dest).toString().c_str(),
			dest.getIndex());
		return;
	}

	if (destId.getType() == RYZOMID::npc || destId.getType() == RYZOMID::creature)
	{
		// read the phrase name
		std::string phraseName;
		message.serial(phraseName);

		nlwarning("Service '%s' is trying to send phrase '%s' to a AIS entity !",
			serviceName.c_str(),
			phraseName.c_str());
		return;

	}
	
	// built the stream to be sent to the client
	NLMISC::CBitMemStream bmsOut;
	if ( buildPhraseStream( charInfo, seqNum, message, debug, charInfo->Language, bmsOut ) )
	{
		NLNET::CMessage msgout( "IMPULS_CH_ID" );
		NLMISC::CEntityId destId = charInfo->EntityId;
		uint8 channel = 1;
		msgout.serial( destId );
		msgout.serial( channel );
		
		msgout.serialBufferWithSize((uint8*)bmsOut.buffer(), bmsOut.length());
		NLNET::CUnifiedNetwork::getInstance()->send(TServiceId(charInfo->EntityId.getDynamicId()), msgout);
	}
}

void CStringManager::broadcastSystemMessage(NLNET::CMessage &message, bool debug)
{
	TDataSetRow client;
	vector<CEntityId> temp;
	set<CEntityId>	excluded;
	CChatGroup::TGroupType audience = CChatGroup::nbChatMode;
	uint32 seqNum;

	message.serial(client);
	message.serialCont(temp);
	excluded.insert(temp.begin(), temp.end());
	message.serialEnum(audience);
	message.serial(seqNum);

	if (!IOS->getChatManager().checkClient(client))
	{
		nlwarning("broadcastSystemMessage : can't find client %s:%x in chat client", 
			TheDataset.getEntityId(client).toString().c_str(),
			client.getIndex());
		return;
	}

	try
	{
		// can thow CChatManager::EChatClient exception
		CChatClient &chatClient = IOS->getChatManager().getClient(client);

		TGroupId	groupId;

		switch (audience)
		{
		case CChatGroup::say:
			// force an update of the say audience
			chatClient.getSayAudience(true);
			groupId = chatClient.getSayAudienceId();
			break;
		case CChatGroup::team:
			groupId = chatClient.getTeamChatGroup();
			break;
		case CChatGroup::guild:
			groupId = chatClient.getGuildChatGroup();
			break;
		case CChatGroup::shout:
			// force an update of the shout audience
			chatClient.getShoutAudience(true);
			groupId = chatClient.getShoutAudienceId();
			break;
		default:
			nlwarning("broadcastSystemMessage : unsupported chat mode '%s' for broadcast", CChatGroup::groupTypeToString(audience).c_str());
			return;
		}

		// can throw EChatGroup
		CChatGroup &chatGroup = IOS->getChatManager().getGroup(groupId);

		// generate a unique string seq number
		seqNum = STRING_MANAGER::pickStringSerialNumber();

		// and send to all the client in audience.
		CChatGroup::TMemberCont::iterator first(chatGroup.Members.begin()), last(chatGroup.Members.end());
		for (; first != last; ++first)
		{
			TDataSetRow dsr = *first;

			// send to all client except the sender
			if (dsr != client)
			{
				CEntityId eid = TheDataset.getEntityId(dsr);

				if (excluded.find(eid) != excluded.end())
					continue;

				CCharacterInfos *charInfo = IOS->getCharInfos(eid);
				if (charInfo == NULL)
					continue;

				// copy the message
				CMessage msg;
				msg.assignFromSubMessage(message);
				// built the stream to be sent to the client
				NLMISC::CBitMemStream bmsOut;
				if ( buildPhraseStream( charInfo, seqNum, msg, debug, charInfo->Language, bmsOut ) )
				{
					NLNET::CMessage msgout( "IMPULS_CH_ID" );
					NLMISC::CEntityId destId = charInfo->EntityId;
					uint8 channel = 1;
					msgout.serial( destId );
					msgout.serial( channel );
					
					msgout.serialBufferWithSize((uint8*)bmsOut.buffer(), bmsOut.length());
					NLNET::CUnifiedNetwork::getInstance()->send(TServiceId(charInfo->EntityId.getDynamicId()), msgout);


					// inform the client to display the dyn string in system info
					{
						CMessage msgout( "IMPULSION_ID" );
						msgout.serial( const_cast<CEntityId&> (eid) );
						CBitMemStream bms;
						if ( ! GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:DYN_STRING", bms) )
						{
							nlwarning("<sendDynamicSystemMessage> Msg name CHAT:DYN_STRING not found");
						}
						else
						{
							bms.serial( seqNum );
							msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
							CUnifiedNetwork::getInstance()->send( TServiceId(eid.getDynamicId()), msgout );
						}
					}
				}
			}
		}
	}
	catch(const CChatManager::EChatClient &e)
	{
		nlwarning("%s", e.what());
	}
	catch(...)
	{
	}
}

//void CStringManager::requestString(const NLMISC::CEntityId &client, uint32 stringId)
//{
//	ucstring str = getString(stringId);
//
//	CCharacterInfos *charInfo = IOS->getCharInfos(client);
//	if (charInfo == 0)
//	{
//		if (IsRingShard.get())
//		{
//			// In ring shard, it is possible that the client autologin and 
//			// autochoose the character rapidly, and if a string need to be resolved 
//			// to display the char summary, it is possible client receive the 
//			// dynamic string from IOS after the EGS has passed the frontend in 
//			// entityId mode.
//			// So, the IOS receive a stringId request with a Eid not registered yet.
//			// Look in the user table, if we have the user, use the UID version
//			uint32 userId = client.getShortId()>>4;
//			TUserLanguagesContainer::const_iterator itUser = _UsersLanguages.find( userId );
//			if ( itUser != _UsersLanguages.end() )
//			{
//				requestString(userId, stringId);
//				return;
//			}
//		}
//		nlwarning("requestString : Client with eid %s is unknown (requesting string %u as [%s])", 
//			client.toString().c_str(),
//			stringId,
//			str.toString().c_str());
//		return;
//	}
//
//
//	LOG("Sending string %u as [%s] to client %s", stringId, str.toString().c_str(), client.toString().c_str());
//
//	// build the response message
//	NLMISC::CBitMemStream bmsOut;
//	GenericXmlMsgHeaderMngr.pushNameToStream( "STRING_MANAGER:STRING_RESP", bmsOut);
//	bmsOut.serial(stringId);
//	// Send in utf8 format to save bandwith
//	string	strUtf8= str.toUtf8();
//	bmsOut.serial(strUtf8);
//	
//	// send the message to Front End
//	NLNET::CMessage msgout( "IMPULS_CH_ID" );
//	NLMISC::CEntityId destId = client;
//	uint8 channel = 1;
//	msgout.serial( destId );
//	msgout.serial( channel );
//
//	msgout.serialBufferWithSize((uint8*)bmsOut.buffer(), bmsOut.length());
//	NLNET::CUnifiedNetwork::getInstance()->send(TServiceId(charInfo->EntityId.getDynamicId()), msgout);
//}

void CStringManager::requestString(uint32 userId, uint32 stringId)
{
	TServiceId frontendId;
	TUserLanguagesContainer::const_iterator itUser = _UsersLanguages.find( userId );
	if ( itUser == _UsersLanguages.end() )
	{
		// no user language, try to find a character
		breakable
		{
			const CInputOutputService::TIdToInfos &chars = IOS->getCharInfosCont();
			CInputOutputService::TIdToInfos::const_iterator it(chars.lower_bound(CEntityId(RYZOMID::player, userId<<4)));
			if (it != chars.end())
			{
				CEntityId eid = it->first;
				
				if (eid.getShortId()>>4 == userId)
				{
					// we found a character for this user, use it
					frontendId = TServiceId(eid.getDynamicId());

					break;
				}
			}

			// if we are here, we did not found a character for this user
			nlwarning("<CStringManager requestString> Invalid user id %u",userId);
			return;
		}
	}
	else
	{
		// we know the user
		frontendId = itUser->second.FrontEndId;
	}

	ucstring str = getString(stringId);
	LOG("Sending string %u as [%s] to user %u", stringId, str.toString().c_str(), userId);
	// build the response message
	NLMISC::CBitMemStream bmsOut;
	GenericXmlMsgHeaderMngr.pushNameToStream( "STRING_MANAGER:STRING_RESP", bmsOut);
	bmsOut.serial(stringId);
	// Send in utf8 format to save bandwidth
	string	strUtf8= str.toUtf8();
	bmsOut.serial(strUtf8);
	
	// send the message to Front End
	CMessage msgout( "IMPULSION_UID" );
	msgout.serial( userId );
	msgout.serialBufferWithSize((uint8*)bmsOut.buffer(), bmsOut.length());
	NLNET::CUnifiedNetwork::getInstance()->send( frontendId, msgout );
	nldebug( "IOSSM: Sent IMPULSION_UID to %hu (STRING_RESP)", frontendId.get() );
}


void CStringManager::reload(NLMISC::CLog *log)
{
	uint i;
	for(i=0; i<NB_LANGUAGES; ++i)
	{
		uint j;

		// need to manualy delete param object.
		while (!_AllPhrases[i].empty())
		{
			CPhrase &phrase = _AllPhrases[i]. begin()->second;

			while (!phrase.Params.empty())
			{
				delete phrase.Params.back();
				phrase.Params.pop_back();
			}

			_AllPhrases[i].erase(_AllPhrases[i].begin());
		}
		_AllPhrases[i].clear();

		for (j=0; j<STRING_MANAGER::NB_PARAM_TYPES; ++j)
		{
			CEntityWords	&ew = _AllEntityWords[i][j];

			ew._ColumnInfo.clear();
			ew._NbColums = 0;
			ew._RowInfo.clear();
			delete [] ew._Data;
		}
	}

	init(log);
}




CStringManager::TLanguages		CStringManager::checkLanguageCode(const std::string &languageCode)
{
	for (uint i=0; i<NB_LANGUAGES; ++i)
	{
		if (_LanguageCode[i] == languageCode)
			return TLanguages(i);
	}

	nlwarning("Unrecognized language code %s, default to english", languageCode.c_str());
	// default to english.
	return english;
}

const std::string		&CStringManager::getLanguageCodeString(TLanguages language)
{
	if (language < NB_LANGUAGES)
		return _LanguageCode[language];

	nlwarning("Language number %u is out of range, returning english", language);
	nlassert(english < NB_LANGUAGES);	// just to avoid oopsie
	return _LanguageCode[english];
}



uint32	CStringManager::storeString(const ucstring &str)
{
//	TMappedUStringContainer				_StringIdx;
//	TUStringContainer					_StringBase;

	TMappedUStringContainer::iterator it(_StringIdx.find(str));
	if (it != _StringIdx.end())
	{
		// the string already in base, just return the index.
		return it->second;
	}
	else
	{
		// create a new entry
		std::pair<TMappedUStringContainer::iterator, bool> ret;
		ret = _StringIdx.insert(std::make_pair(str, (uint32)_StringBase.size()));
		nlassert(ret.second);
		_StringBase.push_back(str);

		if (!_TestOnly)
		{
			// add the string in the cache file
			NLMISC::COFile file(_CacheFilename, true);
			LOGPARSE("Writing to cache [%u][%s]", ret.first->second, ret.first->first.toString().c_str());
			file.serial(ret.first->second);
			ucstring temp = ret.first->first;
			file.serial(temp);
		}

		return ret.first->second;
	}
}

const ucstring &CStringManager::getString(uint32 stringId)
{
	if (stringId < _StringBase.size())
		return _StringBase[stringId];
	else
		return _StringBase.front();
}



uint32	CStringManager::translateShortName(uint32 shortNameIndex)
{
	// No bot name translation on ring shards
	if (IsRingShard)
		return shortNameIndex;

	std::map<uint32, uint32>::iterator it(_BotNameTranslation.find(shortNameIndex));

	if (it != _BotNameTranslation.end())
	{
		if (it->second != 0)
			// yeaa, we found a translation with a non empty short name
			return it->second;
		else
			// the translated name is empty, ignore the translated name
			return shortNameIndex;
	}
	else
		// no translation, return the same index.
		return shortNameIndex;

	return 0;
}

uint32	CStringManager::translateShortName(const ucstring &shortName)
{
	//
	return translateShortName(storeString(shortName));
}

uint32	CStringManager::translateTitle(const std::string  &title, TLanguages language)
{
	const std::string colName("name");
	const CStringManager::CEntityWords &ew = getEntityWords(language, STRING_MANAGER::title);
	std::string rowName = NLMISC::toLower(title);
	uint32 stringId;
	stringId = ew.getStringId(rowName, colName);

	return stringId;
}

uint32	CStringManager::translateEventFaction(uint32 eventFactionId)
{
	if (VerboseStringManager)
		nlinfo("Event faction translation asked for : '%s' (%u)", getString(eventFactionId).toString().c_str(), eventFactionId);

	if (eventFactionId == 0)
		return 0;

	std::map<uint32, uint32>::iterator it = _EventFactionTranslation.find(eventFactionId);
	if (it != _EventFactionTranslation.end())
	{
		if (VerboseStringManager)
			nlinfo("Found event faction translation : '%s' (%u)", getString(it->second).toString().c_str(), it->second);

		return it->second;
	}

	return 0;
}

uint32	CStringManager::translateEventFaction(const ucstring &eventFaction)
{
	if (eventFaction.empty())
		return 0;

	return translateEventFaction(storeString(eventFaction));
}

/*
 * Send the requested string
 */
void	CStringManager::sendString( uint32 nameIndex, TServiceId serviceId )
{
	CMessage msgout( "RECV_STRING" );
	msgout.serial( nameIndex );
	const ucstring& ucs = getString( nameIndex );
	msgout.serial( const_cast<ucstring&>(ucs) );
	CUnifiedNetwork::getInstance()->send( serviceId, msgout ); // reply => not via mirror
}


/*
 * Send the names of all online entities
 */
void	CStringManager::retrieveEntityNames( TServiceId serviceId )
{
	vector< pair<TDataSetRow,string> > names;
	TEntityIdToEntityIndexMap::const_iterator itEntityIndex;
	for( itEntityIndex = TheDataset.entityBegin(); itEntityIndex != TheDataset.entityEnd(); ++itEntityIndex )
	{
		TDataSetRow entityIndex = TheDataset.getCurrentDataSetRow( GET_ENTITY_INDEX(itEntityIndex) );
		if ( entityIndex.isValid() )
		{
			CMirrorPropValueRO<TYPE_NAME_STRING_ID> nameIndex( TheDataset, entityIndex, DSPropertyNAME_STRING_ID );
			if ( nameIndex() != 0 )
			{
				names.push_back( make_pair( entityIndex, getString(nameIndex).toString() ) );
			}
		}
	}
	NLNET::CMessage msgout( "ENTITY_NAMES" );
	uint32 len = (uint32)names.size();
	msgout.serial( len );
	vector< pair<TDataSetRow,string> >::const_iterator itn;
	for ( itn=names.begin(); itn!=names.end(); ++itn )
	{
		msgout.serial( const_cast<TDataSetRow&>((*itn).first) );
		msgout.serial( const_cast<string&>((*itn).second) );
	}
	NLNET::CUnifiedNetwork::getInstance()->send( serviceId, msgout );
	nldebug( "IOSSM: Sent %u names to service %hu", names.size(), serviceId.get() );
}

void CStringManager::updateUserLanguage( uint32 userId, TServiceId frontEndId, const std::string & lang )
{ 
	CStringManager::TLanguages language = checkLanguageCode( lang );
	SUserLanguageEntry entry( frontEndId, language );
	TUserLanguagesContainer::iterator it = _UsersLanguages.find(userId);
	if (it == _UsersLanguages.end())
	{
		_UsersLanguages.insert(make_pair(userId, entry));
	}
	else
	{
		it->second.FrontEndId = frontEndId;
		it->second.Language = language;
	}

	// TODO : send cache time stamp to client.
	nldebug ("IOSSM: updateUserLanguage : set userId %u to front end %u using language code '%s'", 
		userId, 
		frontEndId.get(), 
		SM->getLanguageCodeString(language).c_str());

	// send back the cache time stamp info
	uint32 timestamp = SM->getCacheTimestamp();

	// now, build the message for the client.
	NLMISC::CBitMemStream bmsOut;
	GenericXmlMsgHeaderMngr.pushNameToStream( "STRING_MANAGER:RELOAD_CACHE", bmsOut);
	bmsOut.serial(timestamp);

	// send the message to Front End
	NLNET::CMessage msgout( "IMPULSION_UID" );
	msgout.serial(userId);

	msgout.serialBufferWithSize((uint8*)bmsOut.buffer(), bmsOut.length());
	try
	{
		CUnifiedNetwork::getInstance()->send( frontEndId, msgout);
	}
	catch(const Exception& e)
	{
		nlwarning( "CStringManager::updateUserLanguage : Error : %s", e.what() );
	}
}

/*
 * Replace a phrase in default language(s) (message handler)
 */
void CStringManager::setPhrase(NLNET::CMessage &message)
{
	std::string phraseName;
	ucstring phraseContent;
	try
	{
		message.serial(phraseName);
		message.serial(phraseContent);
	}
	catch(const Exception& e)
	{
		nlwarning("<setPhrase> %s",e.what());
		return;
	}
	setPhrase(phraseName, phraseContent);
}

/*
 * Replace a phrase in specified language (message handler)
 */
void CStringManager::setPhraseLang(NLNET::CMessage &message)
{
	std::string phraseName;
	ucstring phraseContent;
	std::string langString;
	try
	{
		message.serial(phraseName);
		message.serial(phraseContent);
		message.serial(langString);
	}
	catch( Exception& e )
	{
		nlwarning("<setPhrase> %s",e.what());
		return;
	}

	TLanguages lang = checkLanguageCode(langString);
	setPhrase(phraseName, phraseContent, lang);
}

/*
 * Replace a phrase in default language(s)
 */
void CStringManager::setPhrase(std::string const& phraseName, ucstring const& phraseContent)
{
	if (_DefaultSetPhraseLanguage==NB_LANGUAGES)
		for (int i=0; i<NB_LANGUAGES; ++i)
			setPhrase(phraseName, phraseContent, (TLanguages)i);
	else
		setPhrase(phraseName, phraseContent, _DefaultSetPhraseLanguage);
}


/// Store a set of user named item associated with an AIInstance
void CStringManager::storeItemNamesForAIInstance(uint32 aiInstance, const std::vector < R2::TCharMappedInfo > &itemInfos)
{
	// first, parse all the container to remove any previously user item with this aiInstance
	TRingUserItemInfos::iterator first(_RingUserItemInfos.begin()), last(_RingUserItemInfos.end());
	// for each item having one or more translation...
	for (; first != last; ++first)
	{
		std::vector<TRingUserItemInfo> &items = first->second;
		// for each translation of this item...
		for (uint i=0; i<items.size(); ++i)
		{
			if (items[i].AIInstance == aiInstance)
			{
				// remove this one
				items.erase(items.begin() + i);
				--i;

				// NB : item with 0 translation are keept in the table because
				// the set of user item in the ring is closed and limited.
			}
		}

	}

	// insert the new items definition
	for (uint i=0; i<itemInfos.size(); ++i)
	{
		const R2::TCharMappedInfo &itemInfo = itemInfos[i];

		TRingUserItemInfo ruii;
		ruii.AIInstance = aiInstance;
		ruii.ItemNameId = storeString(itemInfo.getName());
		_RingUserItemInfos[itemInfo.getItemSheet()].push_back(ruii);
	}
}


/*
NLMISC_COMMAND(verboseStringManager,"Turn on or off or check the state of verbose string manager logging","")
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
	{
		if(args[0]==string("on")||args[0]==string("ON")||args[0]==string("true")||args[0]==string("TRUE")||args[0]==string("1"))
			VerboseLog=true;

		if(args[0]==string("off")||args[0]==string("OFF")||args[0]==string("false")||args[0]==string("FALSE")||args[0]==string("0"))
			VerboseLog=false;
	}

	nlinfo("VerboseLogging is %s",VerboseLog?"ON":"OFF");
	return true;
}
*/

NLMISC_CATEGORISED_COMMAND(stringmanager, loadPhraseFile, "Merge a phrase file into string manager","<language code> <directory>[/file]")
{
	if (args.size() != 2)
		return false;

	std::string	lang = args[0];
	std::string	file = args[1];

	CStringManager::TLanguages	language = SM->checkLanguageCode(lang);

	if (SM->getLanguageCodeString(language) != lang)
	{
		log.displayNL("Failed, language '%s' is not a valid language", lang.c_str());
		return false;
	}

	if (!CFile::fileExists(file))
	{
		if (!CFile::isDirectory(file))
		{
			log.displayNL("Failed, path '%s' is not a valid file nor directory", file.c_str());
			return false;
		}

		file = CPath::standardizePath(file)+"phrase_"+lang+".txt";

		if (!CFile::fileExists(file))
		{
			log.displayNL("Failed to locate default phrase file '%s'", file.c_str());
			return false;
		}
	}

	SM->loadPhraseFile(file, language, "", &log);

	return true;
}

NLMISC_CATEGORISED_COMMAND(stringmanager, mergeWordFile, "Merge a word file into string manager","<language code> <word type> <directory>[/file]")
{
	if (args.size() != 3)
		return false;

	std::string	lang = args[0];
	std::string	word = toLower(args[1]);
	std::string	file = args[2];

	// get language
	CStringManager::TLanguages	language = SM->checkLanguageCode(lang);
	if (SM->getLanguageCodeString(language) != lang)
	{
		log.displayNL("Failed, language '%s' is not a valid language", lang.c_str());
		return false;
	}

	// get word type
	CStringManager::TParameterTraitList	typeNames = CStringManager::CParameterTraits::getParameterTraitsNames();
	STRING_MANAGER::TParamType	wordType;
	uint	i;
	for (i=0; i<typeNames.size(); ++i)
	{
		if (toLower(typeNames[i].second) == word)
		{
			wordType = typeNames[i].first;
			break;
		}
	}
	if (i == typeNames.size())
	{
		log.displayNL("Failed, word type '%s' is not valid", word.c_str());
		return false;
	}

	//
	if (!CFile::fileExists(file))
	{
		if (!CFile::isDirectory(file))
		{
			log.displayNL("Failed, path '%s' is not a valid file nor directory", file.c_str());
			return false;
		}

		file = CPath::standardizePath(file)+word+"_words_"+lang+".txt";

		if (!CFile::fileExists(file))
		{
			log.displayNL("Failed to locate default word file '%s'", file.c_str());
			return false;
		}
	}

	bool oldMode = SM->ReadTranslationWork;
	// We don't want diff with work file now
	SM->ReadTranslationWork = false;
	SM->mergeEntityWordsFile(file, language, wordType);
	SM->ReadTranslationWork = oldMode;

	return true;
}

NLMISC_CATEGORISED_COMMAND(stringmanager, displayEntityWords, "display entity words for a language and type","<language code> <word type> [wordwildcard]")
{
	if (args.size() < 2 || args.size() > 3)
		return false;

	std::string	lang = args[0];
	std::string	word = toLower(args[1]);
	std::string	wc;

	if (args.size() == 3)
		wc = args[2];

	// get language
	CStringManager::TLanguages	language = SM->checkLanguageCode(lang);
	if (SM->getLanguageCodeString(language) != lang)
	{
		log.displayNL("Failed, language '%s' is not a valid language", lang.c_str());
		return false;
	}

	// get word type
	CStringManager::TParameterTraitList	typeNames = CStringManager::CParameterTraits::getParameterTraitsNames();
	STRING_MANAGER::TParamType	wordType;
	uint	i;
	for (i=0; i<typeNames.size(); ++i)
	{
		if (toLower(typeNames[i].second) == word)
		{
			wordType = typeNames[i].first;
			break;
		}
	}
	if (i == typeNames.size())
	{
		log.displayNL("Failed, word type '%s' is not valid", word.c_str());
		return false;
	}

	SM->displayEntityWords(language, wordType, wc, &log);
	return true;
}

NLMISC_CATEGORISED_COMMAND(stringmanager, setEntityWord, "set a word value","<language code>.<word type>.<word>.<determinant> <value>")
{
	if (args.size() < 2 || args.size() > 5)
		return false;

	std::string	path = args[0];
	uint	wi = 1;

	while (wi < args.size()-1)
		path += "."+args[wi++];

	ucstring	word(args[wi]);

	// get language
	SM->setEntityWord(path, word);
	return true;
}


NLMISC_CATEGORISED_COMMAND(stringmanager, loadBotNames, "load a bot names file","[bot names file] [reset bot names (0|1)]")
{
	if (args.size() > 2)
		return false;

	std::string	filename = "bot_names.txt";
	bool		resetBotnames = false;

	if (args.size() > 0)
		filename = args[0];

	if (args.size() > 1)
		NLMISC::fromString(args[1], resetBotnames);

	SM->loadBotNames(filename, resetBotnames, &log);
	return true;
}

NLMISC_CATEGORISED_COMMAND(stringmanager, setBotName, "set a bot name","<bot name (utf8)> <translation (utf8)>")
{
	if (args.size() != 2)
		return false;

	ucstring	botname, translation;

	botname.fromUtf8(args[0]);
	translation.fromUtf8(args[1]);

	SM->setBotName(botname, translation);
	return true;
}

NLMISC_CATEGORISED_COMMAND(stringmanager, readStringManagerRepository, "parse a whole repository with phrases and words (language is optional, none will load rep for all languages)","<directory> [language code]")
{
	if (args.size() < 1 || args.size() > 2)
		return false;

	string	path = args[0];

	if (args.size() == 2)
	{
		string	lang = args[1];

		CStringManager::TLanguages	language = SM->checkLanguageCode(lang);
		if (SM->getLanguageCodeString(language) != lang)
		{
			log.displayNL("Failed, language '%s' is not a valid language", lang.c_str());
			return false;
		}

		log.displayNL("Reading text repository '%s' for language '%s'",
			path.c_str(),
			lang.c_str());
		SM->readRepository(path, language, &log);
	}
	else
	{
		log.displayNL("Reading text repository '%s' for all language",
			path.c_str());
		uint	i;
		for (i=1; i<CStringManager::NB_LANGUAGES; ++i)
		{
			SM->readRepository(path, (CStringManager::TLanguages)i, &log);
		}
	}

	return true;
}


NLMISC_CATEGORISED_COMMAND(stringmanager, defaultSetPhraseLanguage, "Selects the language overriden by AIS messages","<language code>")
{
	if (args.size() < 0 || args.size() > 1)
		return false;

	if (args.size()!=0)
	{
		CStringManager::TLanguages language = CStringManager::english;
		if (args[0]=="all")
		{
			language = CStringManager::NB_LANGUAGES;
		}
		else
		{
			language = SM->checkLanguageCode(args[0]);
		}
		SM->setDefaultSetPhraseLanguage(language);
	}
	CStringManager::TLanguages language = SM->getDefaultSetPhraseLanguage();
	std::string	languageCode;
	if (language==CStringManager::NB_LANGUAGES)
		languageCode = "all";
	else
		languageCode = 	SM->getLanguageCodeString(language);
	log.displayNL("Language overriden by AIS messages is %s", languageCode.c_str());
	return true;
}


