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

#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/msg_client_server.h"

#include "client_chat_manager.h"
#include "net_manager.h"
#include "nel/gui/group_list.h"
#include "interface_v3/interface_manager.h"
#include "interface_v3/people_interraction.h"
#include "string_manager_client.h"
#include "entity_cl.h"
#include "nel/gui/action_handler.h"
#include "entities.h"
#include "nel/gui/group_editbox.h"
#include "permanent_ban.h"
#include "global.h"
#include "nel/gui/ctrl_text_button.h"
#include "nel/gui/group_tab.h"
#include "string_manager_client.h"

#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/msg_client_server.h"
#include "game_share/chat_group.h"
#include "interface_v3/skill_manager.h"

#include "misc.h"

using namespace std;
using namespace NLMISC;






extern CGenericXmlMsgHeaderManager GenericMsgHeaderMngr;
extern CClientChatManager	ChatMngr;
extern CLog					g_log;
extern CEntityManager		EntitiesMngr;


//#ifdef OLD_STRING_SYSTEM
//
//bool CNetworkString::getString (ucstring &result, CClientChatManager *mng)
//{
//	result = StaticString + " / ";
//	for (uint i = 0; i < Args.size(); i++)
//	{
//		result += " ";
//		result += toString (Args[i]);
//
//		CDynamicStringInfos *res = mng->getDynamicDB().getDynamicStringInfos ((uint32)Args[i]);
//		if (res != NULL)
//		{
//			result += " ";
//			result += res->Str;
//		}
//	}
//
//	//nlinfo ("%s", result.c_str());
//
//	return mng->getString (result, Args, StaticString);
//}
//
//void CNetworkString::setString (const ucstring &staticStringId, CClientChatManager *mng)
//{
//	CBitMemStream bms;
//	mng->getStaticDB().getInfos(staticStringId, StaticString, bms);
//}
//
////-----------------------------------------------
////	add
////
////-----------------------------------------------
//uint32 CChatDynamicDatabase::add( uint32 index, ucstring& ucstr, vector<bool>& code )
//{
//	nlinfo ("receive dynamic string association '%d' '%s'", index, ucstr.toString().c_str());
//
//	map<ucstring, uint32>::iterator itIdx = _StringToIndex.find( ucstr );
//	if( itIdx == _StringToIndex.end() )
//	{
//		map<uint32,CDynamicStringInfos *>::iterator itStr = _Data.find( index );
//		if( itStr == _Data.end() )
//		{
//			CDynamicStringInfos * dynInfosTmp = new CDynamicStringInfos();
//			dynInfosTmp->Index = index;
//			dynInfosTmp->Associated = true;
//
//			// display the index number
//			//dynInfosTmp->Str = toString((uint32)index);
//			//dynInfosTmp->Str += " ";
//			dynInfosTmp->Str = ucstr;
//
//			if( !code.empty() )
//			{
//				_Huffman.add( ucstr, code );
//				dynInfosTmp->IsHuffman = true;
//			}
//			else
//			{
//				dynInfosTmp->IsHuffman = false;
//			}
//
//			_Data.insert( make_pair(index,dynInfosTmp) );
//			_StringToIndex.insert( make_pair(ucstr,index) );
//
//			return index;
//		}
//		else
//		{
//			// we already insert a fake entry usign getInfos(), now we set the good value
//
//			// display the index number
//			//(*itStr).second->Str = toString((uint32)index);
//			//(*itStr).second->Str += " ";
//			(*itStr).second->Str = ucstr;
//			(*itStr).second->Associated = true;
//
//			if( !code.empty() )
//			{
//				_Huffman.add( ucstr, code );
//			}
//			return index;
//		}
//	}
//	else
//	{
//		nlwarning("<CChatDynamicDatabase::add> the entry %s already exists",ucstr.toString().c_str());
//		return 0;
//	}
//
//} // add //
//
//
//
//
////-----------------------------------------------
////	decodeString
////
////-----------------------------------------------
//void CChatDynamicDatabase::decodeString( ucstring& str, CBitMemStream& bms )
//{
//	_Huffman.getId( str, bms );
//
//} // decodeString //
//
////-----------------------------------------------
////	getDynamicStringInfos
////
////-----------------------------------------------
//CDynamicStringInfos * CChatDynamicDatabase::getDynamicStringInfos( uint32 index )
//{
//	if( index == 0 )
//	{
//		nldebug("<CChatDynamicDatabase::getInfos> The index 0 is not a valid index");
//		return NULL;
//	}
//	map< uint32, CDynamicStringInfos *>::iterator itData = _Data.find( index );
//	if( itData != _Data.end() )
//	{
//		return (*itData).second;
//	}
//	else
//	{
//		CDynamicStringInfos * infos = new CDynamicStringInfos();
//		if( infos )
//		{
//			_Data.insert( make_pair(index,infos) );
//			return infos;
//		}
//		else
//		{
//			nlwarning("<CChatDynamicDatabase::getInfos> new infos allocation failed for string %d",index);
//			return NULL;
//		}
//	}
//
//} // getDynamicStringInfos //
//
//
//
//
////-----------------------------------------------
////	~CChatDynamicDatabase
////
////-----------------------------------------------
//CChatDynamicDatabase::~CChatDynamicDatabase()
//{
//	map< uint32, CDynamicStringInfos *>::iterator itData;
//	for( itData = _Data.begin(); itData != _Data.end(); ++itData )
//	{
//		delete (*itData).second;
//	}
//
//} // ~CChatDynamicDatabase //
//
//#endif

//-----------------------------------------------
//	ctor
//
//-----------------------------------------------
CClientChatManager::CClientChatManager()
{
	_ChatMode = (uint8) CChatGroup::nbChatMode;
	_ChatDynamicChannelId = 0;
	_NumTellPeople = 0;
	_MaxNumTellPeople = 5;

	// default to NULL
	for(uint i=0;i<CChatGroup::MaxDynChanPerPlayer;i++)
	{
		_DynamicChannelNameLeaf[i]= NULL;
		_DynamicChannelIdLeaf[i]= NULL;
		_DynamicChannelIdCache[i]= DynamicChannelEmptyId;
	}
}

//-------------------------------------------------------
const ucstring *CClientChatManager::cycleLastTell()
{
	if (_TellPeople.empty()) return NULL;
	_TellPeople.push_front(_TellPeople.back());
	_TellPeople.pop_back();
	return &(_TellPeople.front());
}

//-------------------------------------------------------
void CClientChatManager::setTellListSize(uint numPeople)
{
	if (numPeople <= _NumTellPeople)
	{
		for(uint k = 0; k < (_NumTellPeople - numPeople); ++k)
		{
			_TellPeople.pop_front();
		}
		_NumTellPeople = numPeople;
	}
	_MaxNumTellPeople = numPeople;
}

//-----------------------------------------------
//	init
//
//-----------------------------------------------
void CClientChatManager::init( const string& /* staticDBFileName */ )
{
//#ifdef OLD_STRING_SYSTEM //
//// temp
//	_StaticDB.load( staticDBFileName );
//#endif

} // init //



//-----------------------------------------------
//	chat
//
//-----------------------------------------------
void CClientChatManager::chat( const ucstring& strIn, bool isChatTeam )
{
	// Truncate to 255 chars max (because of server restriction)
	ucstring	str= strIn.substr(0,255);

	// send str to IOS
	CBitMemStream bms;
	string msgType;

	if (isChatTeam)
	{
		if (NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GROUP:0:PRESENT")->getValueBool())
			msgType = "STRING:CHAT_TEAM";
		else
			return;		// don't display team chat message if there is no team chat
	}
	else
		msgType = "STRING:CHAT";

	if( GenericMsgHeaderMngr.pushNameToStream(msgType,bms) )
	{
		bms.serial( str );
		NetMngr.push( bms );
		//nlinfo("impulseCallBack : %s %s sent", msgType.c_str(), str.toString().c_str());
	}
	else
	{
		nlwarning("<CClientChatManager::chat> unknown message name : %s", msgType.c_str());
	}

	if (UserEntity != NULL) UserEntity->setAFK(false);

} // chat //


//-----------------------------------------------
//	tell
//
//-----------------------------------------------
void CClientChatManager::tell( const string& receiverIn, const ucstring& strIn )
{
	// Truncate to 255 chars max (because of server restriction)
	string		receiver= receiverIn.substr(0,255);
	ucstring	str= strIn.substr(0,255);

	// *** send str
	CBitMemStream bms;
	string msgType = "STRING:TELL";
	if( GenericMsgHeaderMngr.pushNameToStream(msgType,bms) )
	{
		bms.serial( receiver );
		bms.serial( str );
		NetMngr.push( bms );
		//nlinfo("impulseCallBack : %s %s %s sent", msgType.c_str(), receiver.c_str(), str.toString().c_str());
	}
	else
	{
		nlwarning("<CClientChatManager::tell> unknown message name : STRING:TELL");
	}


	// *** manage list of last telled people
	// remove the telled people from list (if present)
	std::list<ucstring>::iterator it = _TellPeople.begin();
	while(it != _TellPeople.end())
	{
		if (*it == ucstring(receiver))
		{
			it = _TellPeople.erase(it);
			nlassert(_NumTellPeople != 0);
			-- _NumTellPeople;
		}
		else
		{
			++it;
		}
	}

	// readd to back of the list (most recent telled people)
	_TellPeople.push_back(receiver);
	++ _NumTellPeople;

	// if too much people, remove the older one
	if (_NumTellPeople > _MaxNumTellPeople)
	{
		-- _NumTellPeople;
		_TellPeople.pop_front();
	}

	// tell => the user is no more AFK.
	if (UserEntity != NULL) UserEntity->setAFK(false);

} // tell //



//-----------------------------------------------
//	filter
//
//-----------------------------------------------
void CClientChatManager::filter( uint8 filter )
{
	CBitMemStream bms;
	string msgType = "STRING:FILTER";
	if( GenericMsgHeaderMngr.pushNameToStream(msgType,bms) )
	{
		bms.serial( filter );
		NetMngr.push( bms );
		//nlinfo("impulseCallBack : %s %d sent", msgType.c_str(), filter);
	}
	else
	{
		nlwarning("<CClientChatManager::filter> unknown message name : STRING:FILTER");
	}

	if (UserEntity != NULL) UserEntity->setAFK(false);

} // filter //



//-----------------------------------------------
//	setChatMode
//
//-----------------------------------------------
void CClientChatManager::setChatMode(CChatGroup::TGroupType group, TChanID dynamicChannelId)
{
	uint8 mode = group;
	 // mode really changed ?
	if (mode == _ChatMode && dynamicChannelId==_ChatDynamicChannelId) return;

	// Chat team don't need swap mode
	if (group != CChatGroup::team)
	{
		CBitMemStream bms;
		string msgType = "STRING:CHAT_MODE";
		if( GenericMsgHeaderMngr.pushNameToStream(msgType,bms) )
		{
			bms.serial( mode );
			bms.serial( dynamicChannelId );
			NetMngr.push( bms );
			//nlinfo("impulseCallBack : %s %d sent", msgType.c_str(), mode);
		}
		else
		{
			nlwarning("<CClientChatManager::setChatMode> unknown message name : STRING:CHAT_MODE");
		}
	}

	// update cache
	_ChatMode = mode;
	_ChatDynamicChannelId = dynamicChannelId;

	if (UserEntity != NULL) UserEntity->setAFK(false);

} // filter //

/// Reset the chat mode to force the client to resend it. Used during far TP.
void CClientChatManager::resetChatMode()
{
	_ChatMode = (uint8)CChatGroup::nbChatMode;
}


// ***************************************************************************
void CClientChatManager::processTellString(NLMISC::CBitMemStream& bms, IChatDisplayer &chatDisplayer)
{
	CChatMsg chatMsg;

	// Serial. For tell message, there is no chat mode, coz we know we are in tell mode !
	bms.serial (chatMsg.CompressedIndex);
	bms.serial (chatMsg.SenderNameId);
	bms.serial (chatMsg.Content);

	if (PermanentlyBanned) return;

	chatMsg.ChatMode = (uint8) CChatGroup::tell;

	// If !complete, wait
	ucstring senderStr;
	bool	complete = true;
	complete &= STRING_MANAGER::CStringManagerClient::instance()->getString(chatMsg.SenderNameId, senderStr);
	if (!complete)
	{
		_ChatBuffer.push_back(CChatMsgNode(chatMsg, true));
		nldebug("<impulseTell> Received TELL, put in buffer : waiting association");
		return;
	}

	// display
	ucstring	ucstr;
	buildTellSentence(senderStr, chatMsg.Content, ucstr);
	chatDisplayer.displayTell(/*chatMsg.CompressedIndex, */ucstr, senderStr);
}

// ***************************************************************************
void CClientChatManager::processFarTellString(NLMISC::CBitMemStream& bms, IChatDisplayer &chatDisplayer)
{
	CFarTellMsg farTellMsg;

	// Serial. For far tell message, there is no chat mode nor sender index, and the sender is a string literal!
	farTellMsg.serial(bms);

	if (PermanentlyBanned) return;

	// display
	ucstring	ucstr;
	buildTellSentence(farTellMsg.SenderName, farTellMsg.Text, ucstr);
	chatDisplayer.displayTell(/*chatMsg.CompressedIndex, */ucstr, farTellMsg.SenderName);
}


// ***************************************************************************
void	CClientChatManager::processChatString( NLMISC::CBitMemStream& bms, IChatDisplayer &chatDisplayer)
{
	// before displaying anything, must ensure dynamic channels are up to date
	// NB: only processChatString() have to do this. Other methods cannot be in dyn_chat mode
	updateDynamicChatChannels(chatDisplayer);

	// serial
	CChatMsg chatMsg;
	bms.serial( chatMsg );
	CChatGroup::TGroupType	type = static_cast<CChatGroup::TGroupType>(chatMsg.ChatMode);
	ucstring	senderStr;

	bool complete = true;
	complete &= STRING_MANAGER::CStringManagerClient::instance()->getString(chatMsg.SenderNameId, senderStr);

	if (type == CChatGroup::dyn_chat)
	{
		// retrieve the DBIndex from the dynamic chat id
		sint32 dbIndex = ChatMngr.getDynamicChannelDbIndexFromId(chatMsg.DynChatChanID);
		// if the client database is not yet up to date, put the chat message in buffer
		if (dbIndex < 0)
			complete = false;
	}

	// if !complete, wait
	if (!complete)
	{
		_ChatBuffer.push_back(CChatMsgNode(chatMsg, false));
		//nldebug("<impulseChat> Received CHAT, put in buffer : waiting association");
		return;
	}

	// display
	ucstring	ucstr;
	buildChatSentence(chatMsg.CompressedIndex, senderStr, chatMsg.Content, type, ucstr);
	chatDisplayer.displayChat(chatMsg.CompressedIndex, ucstr, chatMsg.Content, type, chatMsg.DynChatChanID, senderStr);
}


// ***************************************************************************
void CClientChatManager::processTellString2(NLMISC::CBitMemStream& bms, IChatDisplayer &chatDisplayer)
{
	// serial
	CChatMsg2 chatMsg;
	chatMsg.ChatMode = CChatGroup::tell;
	bms.serial(chatMsg.CompressedIndex);
	bms.serial(chatMsg.SenderNameId);
	bms.serial(chatMsg.PhraseId);

	// if !complete, wait
	ucstring senderStr;
	ucstring rawMessage;
	bool	complete = true;
	complete &= STRING_MANAGER::CStringManagerClient::instance()->getString(chatMsg.SenderNameId, senderStr);
	complete &= STRING_MANAGER::CStringManagerClient::instance()->getDynString(chatMsg.PhraseId, rawMessage);
	if (!complete)
	{
		_ChatBuffer.push_back(CChatMsgNode(chatMsg, true));
		nldebug("<impulseTell> Received TELL, put in buffer : waiting association");
		return;
	}

	// display
	ucstring	ucstr;
	buildTellSentence(senderStr, rawMessage, ucstr);
	chatDisplayer.displayTell(/*chatMsg.CompressedIndex, */ucstr, senderStr);
}


// ***************************************************************************
void CClientChatManager::processChatString2(NLMISC::CBitMemStream& bms, IChatDisplayer &chatDisplayer)
{
	CChatMsg2 chatMsg;
	bms.serial( chatMsg );
	if (PermanentlyBanned) return;
	CChatGroup::TGroupType	type = static_cast<CChatGroup::TGroupType>(chatMsg.ChatMode);
	ucstring	senderStr;
	ucstring	rawMessage;

	// here, the type cannot be dyn_chat (no DynChatId in the message) => discard
	if(type==CChatGroup::dyn_chat)
	{
		nlwarning("Client don't support dyn_chat with CChatMsg2 messages => '%x' aborted", chatMsg.PhraseId);
		return;
	}

	// if !complete, wait
	bool	complete = true;
	complete &= STRING_MANAGER::CStringManagerClient::instance()->getString(chatMsg.SenderNameId, senderStr);
	complete &= STRING_MANAGER::CStringManagerClient::instance()->getDynString(chatMsg.PhraseId, rawMessage);
	if (!complete)
	{
		_ChatBuffer.push_back(CChatMsgNode(chatMsg, false));
		//nldebug("<impulseChat> Received CHAT, put in buffer : waiting association");
		return;
	}

	rawMessage += ucstring(" ");
	rawMessage += chatMsg.CustomTxt;

	// display
	ucstring	ucstr;
	buildChatSentence(chatMsg.CompressedIndex, senderStr, rawMessage, type, ucstr);
	chatDisplayer.displayChat(chatMsg.CompressedIndex, ucstr, rawMessage, type, CEntityId::Unknown, senderStr);
}

// ***************************************************************************
void CClientChatManager::processChatStringWithNoSender( NLMISC::CBitMemStream& bms, CChatGroup::TGroupType type, IChatDisplayer &chatDisplayer)
{
	nlassert(type!=CChatGroup::dyn_chat);

	// serial
	CChatMsg2 chatMsg;
	uint32 phraseID;
	bms.serial(phraseID);
	if (PermanentlyBanned) return;
	chatMsg.CompressedIndex = INVALID_DATASET_INDEX;
	chatMsg.SenderNameId = 0;
	chatMsg.ChatMode = type;
	chatMsg.PhraseId = phraseID;
	ucstring ucstr;

	// if !complete, wait
	bool	complete = STRING_MANAGER::CStringManagerClient::instance()->getDynString(chatMsg.PhraseId, ucstr);
	if (!complete)
	{
		_ChatBuffer.push_back(CChatMsgNode(chatMsg, false));
		//nldebug("<impulseDynString> Received CHAT, put in buffer : waiting association");
		return;
	}

	// diplay
	ucstring senderName("");
	chatDisplayer.displayChat(INVALID_DATASET_INDEX, ucstr, ucstr, type, CEntityId::Unknown, senderName);
}

// ***************************************************************************
void CClientChatManager::flushBuffer(IChatDisplayer &chatDisplayer)
{
	// before displaying anything, must ensure dynamic channels are up to date
	updateDynamicChatChannels(chatDisplayer);

	// **** Process waiting messages
	{
		list<CChatMsgNode>::iterator itMsg;
		for( itMsg = _ChatBuffer.begin(); itMsg != _ChatBuffer.end(); )
		{
			CChatGroup::TGroupType	type = static_cast<CChatGroup::TGroupType>(itMsg->ChatMode);
			ucstring sender, content;

			// all strings received?
			bool complete = true;
			if (itMsg->SenderNameId != 0)
				complete &= STRING_MANAGER::CStringManagerClient::instance()->getString(itMsg->SenderNameId, sender);
			if(itMsg->UsePhraseId)
				complete &= STRING_MANAGER::CStringManagerClient::instance()->getDynString(itMsg->PhraseId, content);
			else
				content= itMsg->Content;

			if (type == CChatGroup::dyn_chat)
			{
				// retrieve the DBIndex from the dynamic chat id
				sint32 dbIndex = ChatMngr.getDynamicChannelDbIndexFromId(itMsg->DynChatChanID);
				// if the client database is not yet up to date, leave the chat message in buffer
				if (dbIndex < 0)
					complete = false;
			}

			// if complete, process
			if (complete)
			{
				ucstring ucstr;
				if (itMsg->SenderNameId == 0)
				{
					ucstr = content;
				}
				else
				{
					if(itMsg->DisplayAsTell)
						buildTellSentence(sender, content, ucstr);
					else
						buildChatSentence(itMsg->CompressedIndex, sender, content, type, ucstr);
				}

				// display
				if(itMsg->DisplayAsTell)
					chatDisplayer.displayTell(/*itMsg->CompressedIndex, */ucstr, sender);
				else
					chatDisplayer.displayChat(itMsg->CompressedIndex, ucstr, content, type, itMsg->DynChatChanID, sender);

				list<CChatMsgNode>::iterator itTmp = itMsg++;
				_ChatBuffer.erase( itTmp );
			}
			else
			{
				++itMsg;
			}
		}
	}
} // flushBuffer //




//-----------------------------------------------
//	getString
//
//-----------------------------------------------
ucstring CClientChatManager::getString( CBitMemStream& bms, ucstring& ucstr )
{

	// deal with parameters
	uint32 dynParamIdx = 0;
	bool dynParamSearch = true;
	char chTmp[1024];
	while( dynParamSearch )
	{
		// search if a parameter exists in the string
		sprintf(chTmp,"$%d",dynParamIdx);
		ucstring ucstrTmp( chTmp );
		ucstring::size_type idx = ucstr.find(ucstrTmp);

		// if there's a parameter in the string
		if( idx != ucstring::npos )
		{
			char c = (char)ucstr[idx+ucstrTmp.size()];
			switch( c )
			{
				// parameter is an entry in the dynamic database
				case 'e':
				{
					bool huff;
					bms.serialBit(huff);
					const ucstring dynStr("???");
					if( huff )
					{
						nldebug("<CClientChatManager::getString> receiving huffman dynamic parameter in static string");
//						#ifdef OLD_STRING_SYSTEM
//							_DynamicDB.decodeString( dynStr, bms );
//						#endif
					}
					else
					{
						//if( (sint32)bms.length()*8 - bms.getPosInBit() >= 32 )
						{
							uint32 nameIndex;
							bms.serial(nameIndex);
//							#ifdef OLD_STRING_SYSTEM
//								dynStr = _DynamicDB.getDynamicStringInfos(nameIndex)->Str;
//							#endif
						}
					}
					ucstr.replace( idx, ucstrTmp.size()+1, dynStr );
				}
				break;

				// parameter is a string
				case 's':
				{
					string dynStr;
					bms.serial( dynStr );
					ucstring ucDynStr(dynStr);
					ucstr.replace( idx, ucstrTmp.size()+1, ucDynStr );
				}
				break;

				// parameter is an unsigned integer
				case 'u':
				{
					uint32 nb;
					bms.serial( nb );
					ucstr.replace( idx, ucstrTmp.size()+1, ucstring(toString(nb)) );
				}
				break;
				/*
				case 'u':
				{
					uint i = idx + strTmp.size() + 1;
					string bitCountStr;
					while( isdigit(str[i]) )
					{
						bitCountStr += str[i];
						i++;
					}
					nlassert( !bitCountStr.empty() );
					uint32 bitCount;
					fromString(bitCountStr, bitCount);
					nlassert( bitCount <= 64 );
					uint64 nb;
					bms.serial( nb, bitCount );
					str.replace( idx, strTmp.size() + 1 + bitCountStr.size(), toString(nb) );
				}
				break;
				*/
				// parameter is a signed integer
				case 'i':
				{
					sint32 nb;
					bms.serial( nb );
					ucstr.replace( idx, ucstrTmp.size()+1, ucstring(toString(nb)) );
				}
				break;
				/*
				case 'i':
				{
					uint i = idx + strTmp.size() + 1;
					string bitCountStr;
					while( isdigit(str[i]) )
					{
						bitCountStr += str[i];
						i++;
					}
					nlassert( !bitCountStr.empty() );
					uint32 bitCount;
					fromString(bitCountStr, bitCount);
					nlassert( bitCount <= 64 );
					uint64 nb;
					bms.serial( nb, bitCount );
					str.replace( idx, strTmp.size() + 1 + bitCountStr.size(), toString(nb) );
				}
				break;
				*/

				// parameter is a float
				case 'f':
				{
					float nb;
					bms.serial( nb );
					ucstr.replace( idx, ucstrTmp.size()+1, ucstring(toString(nb)) );
				}
				break;

				// parameter type is unknown
				default :
				{
					nlwarning("<CClientChatManager::getString> The dynamic type %c is unknown",c);
				}
			}
			dynParamIdx++;
		}
		else
		{
			dynParamSearch = false;
		}
	};

	return ucstr;

} // getString //

//-----------------------------------------------
//	getString
//
//-----------------------------------------------
bool CClientChatManager::getString( ucstring &result, std::vector<uint64>& args, const ucstring &ucstrbase )
{
	result = ucstrbase;

	bool finalString = true;

	// deal with parameters
	uint32 dynParamIdx = 0;
	bool dynParamSearch = true;
	char chTmp[1024];
	while( dynParamSearch )
	{
		// search if a parameter exists in the string
		sprintf(chTmp,"$%d",dynParamIdx);
		ucstring ucstrTmp( chTmp );
		ucstring::size_type idx = result.find(ucstrTmp);

		// if there's a parameter in the string
		if( idx != ucstring::npos )
		{
			ucstring rep;
			rep = "???";
			if (dynParamIdx >= args.size())
			{
				nlwarning ("Missing args for string '%s', only %d args, need arg %d", ucstrbase.toString().c_str(), args.size(), dynParamIdx);
			}
			else
			{
				char c = (char)result[idx+ucstrTmp.size()];
				switch( c )
				{
					// parameter is an entry in the dynamic database
					case 'e':
					{
//							#ifdef OLD_STRING_SYSTEM
//								CDynamicStringInfos *res = _DynamicDB.getDynamicStringInfos ((uint32)args[dynParamIdx]);
//								if (!res->Associated)
//							#endif
							finalString = false;
//							#ifdef OLD_STRING_SYSTEM
//								rep = res->Str;
//							#endif
					}
					break;

					// parameter is a string
					case 's':
					{
						nlwarning ("string param not implemented in the vector<uint64> decoding");
					}
					break;

					// parameter is an unsigned integer
					case 'u':
					{
						uint32 nb = (uint32) args[dynParamIdx];
						rep = toString(nb);
					}
					break;

					// parameter is a signed integer
					case 'i':
					{
						sint32 nb = (sint32) args[dynParamIdx];
						rep = toString(nb);
					}
					break;

					// parameter is a float
					case 'f':
					{
						float nb = *(float *) &(args[dynParamIdx]);
						rep = toString(nb);
					}
					break;

					// parameter type is unknown
					default :
					{
						nlwarning("<CClientChatManager::getString> The dynamic type %c is unknown",c);
					}
					break;
				}
			}
			result.replace( idx, ucstrTmp.size()+1, rep );

			dynParamIdx++;
		}
		else
		{
			dynParamSearch = false;
		}
	};

	return finalString;

} // getString //

// ***************************************************************************
void CClientChatManager::buildTellSentence(const ucstring &sender, const ucstring &msg, ucstring &result)
{
	// If no sender name was provided, show only the msg
	if ( sender.empty() )
		result = msg;
	else
	{
		ucstring name = CEntityCL::removeTitleAndShardFromName(sender);
		ucstring csr;

		// special case where there is only a title, very rare case for some NPC
		if (name.empty())
		{
			// we need the gender to display the correct title
			CCharacterCL *entity = dynamic_cast<CCharacterCL*>(EntitiesMngr.getEntityByName(sender, true, true));
			bool bWoman = entity && entity->getGender() == GSGENDER::female;

			name = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(CEntityCL::getTitleFromName(sender), bWoman);
			{
				// Sometimes translation contains another title
				ucstring::size_type pos = name.find('$');
				if (pos != ucstring::npos)
				{
					name = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(CEntityCL::getTitleFromName(name), bWoman);
				}
			}


		}
		else
		{
			// Does the char have a CSR title?
			csr = CHARACTER_TITLE::isCsrTitle(CEntityCL::getTitleFromName(sender)) ? ucstring("(CSR) ") : ucstring("");
		}

		result = csr + name + ucstring(" ") + CI18N::get("tellsYou") + ucstring(": ") + msg;
	}
}


// ***************************************************************************
void CClientChatManager::buildChatSentence(TDataSetIndex /* compressedSenderIndex */, const ucstring &sender, const ucstring &msg, CChatGroup::TGroupType type, ucstring &result)
{
	// if its a tell, then use buildTellSentence
	if(type==CChatGroup::tell)
	{
		buildTellSentence(sender, msg, result);
		return;
	}

	// If no sender name was provided, show only the msg
	if ( sender.empty() )
	{
		result = msg;
		return;
	}

	// get the category if any. Note, in some case (chat from other player), there is not categories
	// and we do not want getStringCategory to return 'SYS' category.
	ucstring finalMsg;
	string catStr = getStringCategory(msg, finalMsg, false);
	ucstring cat;
	if (!catStr.empty())
		cat = string("&")+catStr+"&";

	if ( ! cat.empty())
	{
		result = msg;
		return;
	}

	// Format the sentence with the provided sender name
	ucstring senderName = CEntityCL::removeTitleAndShardFromName(sender);

	ucstring csr;
	// Does the char have a CSR title?
	csr = CHARACTER_TITLE::isCsrTitle(CEntityCL::getTitleFromName(sender)) ? ucstring("(CSR) ") : ucstring("");

	if (UserEntity && senderName == UserEntity->getDisplayName())
	{
		// The player talks
		switch(type)
		{
			case CChatGroup::shout:
				result = cat + csr + CI18N::get("youShout") + ucstring(": ") + finalMsg;
			break;
			default:
				result = cat + csr + CI18N::get("youSay") + ucstring(": ") + finalMsg;
			break;
		}
	}
	else
	{
		// Special case where there is only a title, very rare case for some NPC
		if (senderName.empty())
		{
			CCharacterCL *entity = dynamic_cast<CCharacterCL*>(EntitiesMngr.getEntityByName(sender, true, true));
			// We need the gender to display the correct title
			bool bWoman = entity && entity->getGender() == GSGENDER::female;

			senderName = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(CEntityCL::getTitleFromName(sender), bWoman);
			{
				// Sometimes translation contains another title
				ucstring::size_type pos = senderName.find('$');
				if (pos != ucstring::npos)
				{
					senderName = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(CEntityCL::getTitleFromName(senderName), bWoman);
				}
			}


		}

		switch(type)
		{
			case CChatGroup::shout:
				result = cat + csr + senderName + ucstring(" ") + CI18N::get("heShout") + ucstring(": ") + finalMsg;
			break;
			default:
				result = cat + csr + senderName + ucstring(" ") + CI18N::get("heSays") + ucstring(": ") + finalMsg;
			break;
		}
	}
}

// ***************************************************************************
void	CClientChatManager::initInGame()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	for(uint i=0;i<CChatGroup::MaxDynChanPerPlayer;i++)
	{
		// default
		_DynamicChannelNameLeaf[i]= NULL;
		_DynamicChannelIdLeaf[i]= NULL;
		_DynamicChannelIdCache[i]= DynamicChannelEmptyId;
		// get
		CCDBNodeLeaf	*name= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:DYN_CHAT:CHANNEL%d:NAME", i), false);
		CCDBNodeLeaf	*id= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:DYN_CHAT:CHANNEL%d:ID", i), false);
		if(name && id)
		{
			_DynamicChannelNameLeaf[i]= name;
			_DynamicChannelIdLeaf[i]= id;
		}
	}
}

// ***************************************************************************
void	CClientChatManager::releaseInGame()
{
	for(uint i=0;i<CChatGroup::MaxDynChanPerPlayer;i++)
	{
		_DynamicChannelNameLeaf[i]= NULL;
		_DynamicChannelIdLeaf[i]= NULL;
		_DynamicChannelIdCache[i]= DynamicChannelEmptyId;
	}
}

// ***************************************************************************
TChanID	CClientChatManager::getDynamicChannelIdFromDbIndex(uint32 dbIndex)
{
	if(dbIndex>=CChatGroup::MaxDynChanPerPlayer || _DynamicChannelIdLeaf[dbIndex]==NULL)
		return CEntityId::Unknown;
	else
		return TChanID(uint64(_DynamicChannelIdLeaf[dbIndex]->getValue64()));
}

// ***************************************************************************
sint32	CClientChatManager::getDynamicChannelDbIndexFromId(TChanID channelId)
{
	for(uint i=0;i<CChatGroup::MaxDynChanPerPlayer;i++)
	{
		if(_DynamicChannelIdLeaf[i]!= NULL)
		{
			if(uint64(_DynamicChannelIdLeaf[i]->getValue64()) == channelId.getRawId())
				return i;
		}
	}

	return -1;
}

// ***************************************************************************
bool	CClientChatManager::isDynamicChannelExist(TChanID channelId)
{
	sint32	dbid= getDynamicChannelDbIndexFromId(channelId);
	return dbid>=0 && dbid<CChatGroup::MaxDynChanPerPlayer;
}

// ***************************************************************************
uint32	CClientChatManager::getDynamicChannelNameFromDbIndex(uint32 dbIndex)
{
	if(dbIndex>=CChatGroup::MaxDynChanPerPlayer || _DynamicChannelNameLeaf[dbIndex]==NULL)
		return 0;
	else
		return _DynamicChannelNameLeaf[dbIndex]->getValue32();
}


// ***************************************************************************
void	CClientChatManager::updateDynamicChatChannels(IChatDisplayer &chatDisplayer)
{
	// For all channels
	for(uint i=0;i<CChatGroup::MaxDynChanPerPlayer;i++)
	{
		// if the NAME is 0, force also an "empty id". because server may not release Id to 0 while NAME is.
		sint32	curActualChannelId= DynamicChannelEmptyId;
		if(_DynamicChannelIdLeaf[i] && _DynamicChannelNameLeaf[i])
		{
			if(_DynamicChannelNameLeaf[i]->getValue32()!=0)
				curActualChannelId= _DynamicChannelIdLeaf[i]->getValue32();
		}

		// if different from precend, clear the channel
		if(curActualChannelId !=(sint32)_DynamicChannelIdCache[i])
		{
			_DynamicChannelIdCache[i]= curActualChannelId;
			// flush
			chatDisplayer.clearChannel(CChatGroup::dyn_chat, i);
		}
	}
}



// ***************************************************************************

class CHandlerTell : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		string receiver = getParam (sParams, "player");
		ucstring message;
		message.fromUtf8(getParam (sParams, "text"));
//		message = getParam (sParams, "text");
		if (receiver.empty() || message.empty())
			return;

		// Get the chat window (if any)
		CChatWindow *cw = NULL;
		CGroupEditBox *eb = pCaller?dynamic_cast<CGroupEditBox *>(pCaller):NULL;
		if (eb)
			cw = getChatWndMgr().getChatWindowFromCaller(eb);

		// Send the message.
		ChatMngr.tell(receiver, message);

		// display in the good window
		CInterfaceProperty prop;
		prop.readRGBA("UI:SAVE:CHAT:COLORS:SPEAKER"," ");
		ucstring finalMsg;
		CChatWindow::encodeColorTag(prop.getRGBA(), finalMsg, false);

		ucstring csr(CHARACTER_TITLE::isCsrTitle(UserEntity->getTitleRaw()) ? "(CSR) " : "");
		finalMsg += csr + CI18N::get("youTell") + ": ";
		prop.readRGBA("UI:SAVE:CHAT:COLORS:TELL"," ");
		CChatWindow::encodeColorTag(prop.getRGBA(), finalMsg, true);
		finalMsg += message;
		// display msg with good color
//		TDataSetIndex dsi; // not used ....
		PeopleInterraction.ChatInput.Tell.displayTellMessage(/*dsi, */finalMsg, receiver, prop.getRGBA());

		ucstring s = CI18N::get("youTellPlayer");
		strFindReplace(s, "%name", receiver);
		strFindReplace(finalMsg, CI18N::get("youTell"), s);
		CInterfaceManager::getInstance()->log(finalMsg);
	}
};
REGISTER_ACTION_HANDLER( CHandlerTell, "tell");

// ***************************************************************************

class CHandlerEnterTell : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		string receiver = getParam (sParams, "player");
		if (receiver.empty())
			return;

		CChatGroupWindow *pCGW = PeopleInterraction.getChatGroupWindow();
		if (pCGW != NULL)
		{
			CGroupContainer *pGC = pCGW->createFreeTeller(receiver);
			if (pGC != NULL)
				pGC->setActive(true);
			CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(pGC->getGroup("eb"));
			if (eb)
			{
				CWidgetManager::getInstance()->setCaptureKeyboard(eb);
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerEnterTell, "enter_tell");


//-----------------------------------------------
//	updateChatModeAndButton
//
//-----------------------------------------------
void CClientChatManager::updateChatModeAndButton(uint mode, uint32 dynamicChannelDbIndex)
{
	// Check if USER chat is active
	bool userActive = false;
	CChatGroupWindow *pCGW = PeopleInterraction.getChatGroupWindow();
	if (pCGW)
	{
		CInterfaceGroup* pIG = pCGW->getContainer()->getGroup("content:cb:user");
		if (pIG)
		userActive = pIG->getActive();
	}

	CChatGroup::TGroupType m = (CChatGroup::TGroupType)mode;

	if (userActive)
	{
		// Change the button of the user chat to the corresponding chat target
		if (pCGW)
		{
			CCtrlTextButton *pUserBut = dynamic_cast<CCtrlTextButton*>(pCGW->getContainer()->getCtrl("content:but_user"));
			CCtrlTextButton *pEmoteBut = dynamic_cast<CCtrlTextButton*>(pCGW->getContainer()->getCtrl("content:but_emote"));
			CInterfaceGroup *pEditBox = dynamic_cast<CInterfaceGroup*>(pCGW->getContainer()->getGroup("content:ebw"));

			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			const bool teamActive = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GROUP:0:PRESENT")->getValueBool();
			const bool guildActive = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:NAME")->getValueBool();

			if (m == CChatGroup::team && ! teamActive)
				m = PeopleInterraction.TheUserChat.Filter.getTargetGroup();

			if (m == CChatGroup::guild && ! guildActive)
				m = PeopleInterraction.TheUserChat.Filter.getTargetGroup();

			if (pUserBut)
			{
				switch(m)
				{
					default:
					case CChatGroup::arround:
					case CChatGroup::say:		pUserBut->setHardText("uiFilterAround");	break;
					case CChatGroup::region:	pUserBut->setHardText("uiFilterRegion");	break;
					case CChatGroup::universe:	pUserBut->setHardText("uiFilterUniverse");	break;
					case CChatGroup::team:		if (teamActive) pUserBut->setHardText("uiFilterTeam");		break;
					case CChatGroup::guild:		if (guildActive) pUserBut->setHardText("uiFilterGuild");	break;
					case CChatGroup::dyn_chat:
						uint32 textId = ChatMngr.getDynamicChannelNameFromDbIndex(dynamicChannelDbIndex);
						ucstring title;
						STRING_MANAGER::CStringManagerClient::instance()->getDynString(textId, title);
						if (title.empty())
						{
							// Dyn channel does not exist, don't change
							m = PeopleInterraction.TheUserChat.Filter.getTargetGroup();
							dynamicChannelDbIndex = PeopleInterraction.TheUserChat.Filter.getTargetDynamicChannelDbIndex();
						}
						else
						{
							pUserBut->setHardText(title.toUtf8());
						}
						break;
					// NB: user chat cannot have yubo_chat target
				}

				pUserBut->setActive(true);
				pUserBut->getParent()->updateCoords();
				pUserBut->updateCoords();
			}

			if (pEmoteBut)
			{
				pEmoteBut->setActive(true);
				pEmoteBut->updateCoords();
			}

			if (pEditBox && pUserBut && pEmoteBut)
			{
				pEditBox->setW(-pUserBut->getWReal()-pEmoteBut->getWReal()-8);
				pEditBox->setX(pUserBut->getWReal()+4);
			}

			PeopleInterraction.TheUserChat.Filter.setTargetGroup(m, dynamicChannelDbIndex);
			PeopleInterraction.ChatGroup.Filter.setTargetGroup(m, dynamicChannelDbIndex);
		}
	}
}

// ***************************************************************************

class CHandlerTalk : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// Param
		uint mode;
		fromString(getParam (sParams, "mode"), mode);
		ucstring text;
		text.fromUtf8 (getParam (sParams, "text"));
//		text = getParam (sParams, "text");

		// Parse any tokens in the text
		if ( ! CInterfaceManager::parseTokens(text))
		{
			return;
		}

		// Find the base group
		if ((mode<CChatGroup::nbChatMode) && !text.empty())
		{
			if(text[0] == '/')
			{
				string str = text.toUtf8();
				string cmdWithArgs = str.substr(1);

				// Get the command name from the string, can contain spaces
				string cmd = cmdWithArgs.substr(0, cmdWithArgs.find(' '));
				if (cmdWithArgs.find('"') == 0)
				{
					string::size_type pos = cmdWithArgs.find('"', 1);
					if (string::npos != pos)
					{
						cmd = cmdWithArgs.substr(1, pos - 1);
					}
				}

				if ( NLMISC::ICommand::exists( cmd ) )
				{
					NLMISC::ICommand::execute( cmdWithArgs, g_log );
				} 
				else
				{
					CInterfaceManager *im = CInterfaceManager::getInstance();
					im->displaySystemInfo (ucstring(cmd+": ")+CI18N::get ("uiCommandNotExists"));
				}
			}
			else
			{
				ChatMngr.setChatMode((CChatGroup::TGroupType)mode);
				ChatMngr.chat(text, mode == CChatGroup::team);
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerTalk, "talk");

// ***************************************************************************

class CHandlerEnterTalk : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// Param
		uint mode;
		fromString(getParam (sParams, "mode"), mode);

		ChatMngr.updateChatModeAndButton(mode);
	}
};
REGISTER_ACTION_HANDLER( CHandlerEnterTalk, "enter_talk");

// ***************************************************************************

class CHandlerTalkMessage : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// Param
		ucstring text = CI18N::get ("uiTalkMemMsg"+sParams);

		// Find the base group
		if (!text.empty())
		{
			ChatMngr.setChatMode (CChatGroup::say);
			ChatMngr.chat(text);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerTalkMessage, "talk_message");

// ***************************************************************************

class CHandlerSwapChatMode : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		bool	updateCapture= getParam(sParams, "update_capture")=="1";

		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:ENTER_DONT_QUIT_CB", false);
		if(node)
		{
			// if "chatmode" is active
			if(node->getValue32())
			{
				// leave it (enter quit CB)
				node->setValue32(0);
				// also leave Chat Focus (important if comes from command)
				if (updateCapture)
					CWidgetManager::getInstance()->setCaptureKeyboard(NULL);
			}
			else
			{
				// enter chat mode (enter dont quit CB)
				node->setValue32(1);
				// enter Chat focus if '/c' entered
				if (updateCapture && !CWidgetManager::getInstance()->getCaptureKeyboard())
				{
					// reset to the old captured keyboard (should be the one that have launched the command)
					if(CWidgetManager::getInstance()->getOldCaptureKeyboard())
						CWidgetManager::getInstance()->setCaptureKeyboard(CWidgetManager::getInstance()->getOldCaptureKeyboard());
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerSwapChatMode, "swap_chat_mode");

