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

#include "uid_impulsions.h"

#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/msg_client_server.h"
#include "game_share/synchronised_message.h"

#include "frontend_service.h"
#include "fe_receive_sub.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


extern CGenericXmlMsgHeaderManager	GenericXmlMsgHeaderMngr;

void cbImpulsionUidGatewayOpen(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle);
void cbImpulsionUidGatewayMessage(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle);
void cbImpulsionUidGatewayClose(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle);


/*
 * Callbacks of messages with special data handling
 */

//-----------------------------------------------
//	impulsionSelectChar :
//-----------------------------------------------
static void	impulsionSelectChar(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle)
{
//	uint64 longUid = uid;

	CSelectCharMsg selectCharMsg;
	selectCharMsg.serial( bms );

	// look for the user datas
	CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();
	CClientHost *ch = frs->findClientHostByUid(uid);

	if (ch == NULL)
	{
		nlwarning("SELECT_CHAR: Nothing known about uid %u", uid);
		return;
	}

	// TODO : ring : add character/role checking
//#pragma message (NL_LOC_MSG "Ring TODO : add R2 character/role checking")

	// Check authorized character slot
	if (((ch->AuthorizedCharSlot != 0xF) && (selectCharMsg.c != ch->AuthorizedCharSlot)) &&
		(!CLoginServer::acceptsInvalidCookie()))
	{
		frs->rejectReceivedMessage( UnauthorizedCharacterSlot, uid );
		cbDisconnectClient( uid , "FS impulsionSelectChar");
		return;
	}

	CMessage msgout("SELECT_CHAR");
//	msgout.serial( longUid );
	msgout.serial( uid );
	msgout.serial( selectCharMsg.c );
	msgout.serial( ch->InstanceId );
	CUnifiedNetwork::getInstance()->send("EGS", msgout);
	nldebug( "User %u: SELECT_CHAR %u", uid, selectCharMsg.c );
}


//-----------------------------------------------
//	impulsionRetMainland :
//-----------------------------------------------
static void	impulsionRetMainland(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle)
{
	uint8 charIndex;
	bms.serial( charIndex );
	TSessionId rejectedSessionId;
	bms.serial( rejectedSessionId );

	CMessage msgout("RET_MAINLAND" ); // always userId and index, never CEntityId even if client has is ingame
	msgout.serial( uid );
	msgout.serial( charIndex );
	msgout.serial( rejectedSessionId );
	CUnifiedNetwork::getInstance()->send("EGS", msgout);
}

//-----------------------------------------------
//	impulsionAskChar :
//-----------------------------------------------
static void	impulsionAskChar(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle)
{
	uint64 longUid = uid;
	
	CCheckNameMsg checkNameMsg;
	checkNameMsg.serialBitMemStream( bms );
	
	CMessage msgout("CHECK_NAME");
	msgout.serial( longUid );
	msgout.serial( checkNameMsg );
	CUnifiedNetwork::getInstance()->send("EGS", msgout);
}


//-----------------------------------------------
//	impulsionCreateChar :
//-----------------------------------------------
static void	impulsionCreateChar(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle)
{
	uint64 longUid = uid;

	CCreateCharMsg createCharMsg;
	createCharMsg.serialBitMemStream( bms );

	CMessage msgout("CREATE_CHAR");
	msgout.serial( longUid );
	msgout.serial( createCharMsg );
	CUnifiedNetwork::getInstance()->send("EGS", msgout);
}


//-----------------------------------------------
//	impulsionStringRequestUid
//
//-----------------------------------------------
static void impulsionStringRequestUid(uint32 uid, CBitMemStream &bms, TGameCycle gamecycle)
{
	uint32 stringId;
	try
	{
		bms.serial(stringId);
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionStringRequestUid> %s", e.what());
		return;
	}

	CMessage msgout( "STRING_RQ_UID" );
	msgout.serial( uid );
	msgout.serial( stringId );
	CUnifiedNetwork::getInstance()->send("IOS", msgout);

}


/*
 * General receiving function for impulsions by Uid from client to server
 */
void routeImpulsionUidFromClient( NLMISC::CBitMemStream& bms, const uint32& userId, const NLMISC::TGameCycle& gamecycle )
{
	string msgName;
	try
	{
		// Decode XML header
		CGenericXmlMsgHeaderManager::CNodeId	msgNodeId = GenericXmlMsgHeaderMngr.getNodeId(bms, msgName);

		// check if message decoded safely
		if (!msgNodeId.isValid())
		{
			nlwarning("Unable to decode message from client %d", userId);
		}
		else
		{
			// get format and sendto of message
			const CGenericXmlMsgHeaderManager::TMessageFormat	&format = GenericXmlMsgHeaderMngr.getNodeFormat(msgNodeId);
			const string										&sendto = GenericXmlMsgHeaderMngr.getNodeSendTo(msgNodeId);

			// check sendto
			if (sendto == "")
			{
				TImpulsionUidCb	cb = (TImpulsionUidCb)(GenericXmlMsgHeaderMngr.getNodeUserData(msgNodeId, 1));
				if (cb == NULL)
				{
					nlwarning( "RTG: Can't route impulsion %s [uid %u], no 'sendto' nor user callback", msgName.c_str(), userId );
				}
				else
				{
					cb(userId, bms, gamecycle);
					//nldebug( "RTG: Routed impulsion %s via user callback [uid %u]", msgName.c_str(), userId );
				}
			}
			else
			{
				// create forward message
				CMessage	msgout("CLIENT:"+msgName);

				uint64 longUserId = userId;
				msgout.serial(longUserId);

				union
				{
					bool	b;
					uint8	u8;
					uint16	u16;
					uint32	u32;
					uint64	u64;
					sint8	s8;
					sint16	s16;
					sint32	s32;
					sint64	s64;
					float	f;
					double	d;
				} store;
				CEntityId	e;
				string		s;

				uint	i;
				// for each message field, serial in and serial out
				for (i=0; i<format.size(); ++i)
				{

					switch (format[i].Type)
					{
					case CGenericXmlMsgHeaderManager::Bool:
						bms.serial(store.b);
						msgout.serial(store.b);
						break;
					case CGenericXmlMsgHeaderManager::Uint8:
						bms.serial(store.u8);
						msgout.serial(store.u8);
						break;
					case CGenericXmlMsgHeaderManager::Uint16:
						bms.serial(store.u16);
						msgout.serial(store.u16);
						break;
					case CGenericXmlMsgHeaderManager::Uint32:
						bms.serial(store.u32);
						msgout.serial(store.u32);
						break;
					case CGenericXmlMsgHeaderManager::Uint64:
						bms.serial(store.u64);
						msgout.serial(store.u64);
						break;
					case CGenericXmlMsgHeaderManager::Sint8:
						bms.serial(store.s8);
						msgout.serial(store.s8);
						break;
					case CGenericXmlMsgHeaderManager::Sint16:
						bms.serial(store.s16);
						msgout.serial(store.s16);
						break;
					case CGenericXmlMsgHeaderManager::Sint32:
						bms.serial(store.s32);
						msgout.serial(store.s32);
						break;
					case CGenericXmlMsgHeaderManager::Sint64:
						bms.serial(store.s64);
						msgout.serial(store.s64);
						break;
					case CGenericXmlMsgHeaderManager::BitSizedUint:
						bms.serial(store.u32, format[i].BitSize);
						if (format[i].BitSize <= 8)
						{
							store.u8 = (uint8)store.u32;
							msgout.serial(store.u8);
						}
						else if (format[i].BitSize <= 16)
						{
							store.u16 = (uint16)store.u32;
							msgout.serial(store.u16);
						}
						else
						{
							msgout.serial(store.u32);
						}
						break;
					case CGenericXmlMsgHeaderManager::Float:
						bms.serial(store.f);
						msgout.serial(store.f);
						break;
					case CGenericXmlMsgHeaderManager::Double:
						bms.serial(store.d);
						msgout.serial(store.d);
						break;
					case CGenericXmlMsgHeaderManager::EntityId:
						bms.serial(e);
						msgout.serial(e);
						break;
					case CGenericXmlMsgHeaderManager::String:
						bms.serial(s);
						msgout.serial(s);
						break;
					}
				}

				// send message to concerned service
				sendMessageViaMirror( sendto, msgout ); // because of guild management (ex: cbDeleteChar)
				//nldebug( "RTG: Routed impulsion %s to %s [uid %u]", msgName.c_str(), sendto.c_str(), userId );
			}
		}
	}
	catch(const Exception &e)
	{
		nlwarning("<routeImpulsionUidFromClient> %s %s", msgName.c_str(), e.what() );
		return;
	}

} // cbImpulsionUid //


/*
 * Map the callbacks for messages with special data handling
 */
void	initImpulsionUid()
{
	GenericXmlMsgHeaderMngr.setUserData("CONNECTION:SELECT_CHAR",	(uint64)impulsionSelectChar, 1);
	GenericXmlMsgHeaderMngr.setUserData("CONNECTION:CREATE_CHAR",	(uint64)impulsionCreateChar, 1);
	GenericXmlMsgHeaderMngr.setUserData("STRING_MANAGER:STRING_RQ",	(uint64)impulsionStringRequestUid, 1);
	GenericXmlMsgHeaderMngr.setUserData("CONNECTION:ASK_NAME",		(uint64)impulsionAskChar, 1);
	GenericXmlMsgHeaderMngr.setUserData("CONNECTION:RET_MAINLAND",		(uint64)impulsionRetMainland, 1);
	GenericXmlMsgHeaderMngr.setUserData("MODULE_GATEWAY:FEOPEN",		(uint64)cbImpulsionUidGatewayOpen, 1);	
	GenericXmlMsgHeaderMngr.setUserData("MODULE_GATEWAY:GATEWAY_MSG",	(uint64)cbImpulsionUidGatewayMessage, 1);	
	GenericXmlMsgHeaderMngr.setUserData("MODULE_GATEWAY:FECLOSE",		(uint64)cbImpulsionUidGatewayClose, 1);	
}

