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

#include "id_impulsions.h"
#include "uid_impulsions.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/sphrase_com.h"
#include "game_share/synchronised_message.h"
#include "entity_container.h"
#include "frontend_service.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


extern CGenericXmlMsgHeaderManager GenericXmlMsgHeaderMngr;
extern NLNET::TServiceId SelfServiceId;

void cbImpulsionIdGatewayOpen( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId );
void cbImpulsionIdGatewayMessage( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId );
void cbImpulsionIdGatewayClose( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId );


/*
 * Callbacks of messages with special data handling
 */

//-----------------------------------------------
//	impulsionRdy :
//
//-----------------------------------------------
void impulsionRdy( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId )
{
	// Tell EGS that the character is ready. When processing this message, the EGS will
	// add the entity into mirror. Thus the sending must be done using sendMessageViaMirror
	// for proper synchronization
	CMessage msgout("RDY");
	msgout.serial( sender );
	sendMessageViaMirror("EGS", msgout);

/*	// Forward message from client to IOS (for string info)
	CMessage msgout2("READY_STRING");
	msgout2.serial( sender );
	uint8 nbBitsToSkip = (uint8)bms.getPosInBit(); // instead of deleting the bits already read by the GenericXMLManager, tell how many there are
	msgout2.serial( nbBitsToSkip );
	msgout2.serialMemStream( bms );
	CUnifiedNetwork::getInstance()->send("IOS", msgout2 );
*/
}


//-----------------------------------------------
//	impulsionCmd : send a command to be executed on the destination
//
//-----------------------------------------------
void impulsionCmd( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId )
{
	bool addentity;
	string dest, cmd, arg;
	try
	{
		bms.serial(addentity);
		bms.serial(dest);
		bms.serial(cmd);
		bms.serial(arg);
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionCmd> %s",e.what());
		return;
	}
	
	nlinfo("<impulsionCmd> CMD received : %s %s to %s",cmd.c_str(), arg.c_str(), dest.c_str());
	
	CMessage msgout("EXEC_COMMAND");
	string res;
	res = cmd + " ";
	if (addentity)
	{
		res += sender.toString() + " ";
	}
	res += arg;
	msgout.serial (res);

	nlinfo("<impulsionCmd> CMD forwarded : '%s'",res.c_str());

	CUnifiedNetwork::getInstance()->send(dest, msgout);
}

//-----------------------------------------------
//	impulsionAdmin OUTOFDATE
//
//-----------------------------------------------
/*void impulsionAdmin( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId )
{
	string cmd, arg;
	bool onTarget;
	try
	{
		bms.serial(onTarget);
		bms.serial(cmd);
		bms.serial(arg);
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionAdmin> %s",e.what());
		return;
	}

	nlinfo ("User %s wants to execute '%s' '%s' from the client, forward it to the EGS", sender.toString().c_str(), cmd.c_str(), arg.c_str());

	CMessage msgout("CLIENT:COMMAND:ADMIN");
	msgout.serial (sender);
	msgout.serial (onTarget);
	msgout.serial (cmd);
	msgout.serial (arg);
	CUnifiedNetwork::getInstance()->send("EGS", msgout);
}*/


//-----------------------------------------------
//	impulsionStringRequestId
//
//-----------------------------------------------
//void impulsionStringRequestId( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId )
//{
//	uint32 stringId;
//	try
//	{
//		bms.serial(stringId);
//	}
//	catch(const Exception &e)
//	{
//		nlwarning("<impulsionStringRequestId> %s", e.what());
//		return;
//	}
//
//	CMessage msgout( "STRING_RQ_ID" );
//	msgout.serial( sender );
//	msgout.serial( stringId );
//	CUnifiedNetwork::getInstance()->send("IOS", msgout);
//
//}

//-----------------------------------------------
//	impulsionPhraseLearn : learn a phrase
//-----------------------------------------------
void impulsionPhraseLearn( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId )
{
	uint16			phraseId;
	CSPhraseCom		phraseDesc;
	
	try
	{
		bms.serial( phraseId );
		bms.serial( phraseDesc );		
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionPhraseLearn> %s",e.what());
		return;
	}


	CMessage msgout("CLIENT:PHRASE:LEARN");
	msgout.serial( sender );
	msgout.serial( phraseId );
	msgout.serial( phraseDesc );
	CUnifiedNetwork::getInstance()->send("EGS", msgout);
}

//-----------------------------------------------
//	impulsionPhraseMemorize : memorize a phrase
//-----------------------------------------------
void impulsionPhraseMemorize( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId )
{
	uint8			set,slot;
	uint16			phraseId;
	CSPhraseCom		phraseDesc;
	
	try
	{
		bms.serial( set );
		bms.serial( slot );
		bms.serial( phraseId );
		bms.serial( phraseDesc );		
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionPhraseMemorize> %s",e.what());
		return;
	}


	CMessage msgout("CLIENT:PHRASE:MEMORIZE");
	msgout.serial( sender );
	msgout.serial( set );
	msgout.serial( slot );	
	msgout.serial( phraseId );
	msgout.serial( phraseDesc );
	CUnifiedNetwork::getInstance()->send("EGS", msgout);
}


//-----------------------------------------------
//	impulsionExecuteFaber : client execute faber phrase
//-----------------------------------------------
void impulsionExecuteFaber( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId )
{
	NLMISC::CSheetId craftPlan;
	uint8 memory, slot;
	std::vector< CFaberMsgItem > rmSelected;
	std::vector< CFaberMsgItem > rmFormulaSelected;

	try
	{
		bms.serial( craftPlan );
		bms.serial( memory );
		bms.serial( slot );
		bms.serialCont( rmSelected );
		bms.serialCont( rmFormulaSelected );
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionExecuteFaber> %s",e.what());
		return;
	}

	CMessage msgout("CLIENT:PHRASE:EXECUTE_FABER");
	msgout.serial( sender );
	msgout.serial( craftPlan );
	msgout.serial( memory );
	msgout.serial( slot );
	msgout.serialCont( rmSelected );
	msgout.serialCont( rmFormulaSelected );
	CUnifiedNetwork::getInstance()->send("EGS", msgout);
}


//-----------------------------------------------
//	cbImpulsionGetNpcIconDesc
//-----------------------------------------------
void cbImpulsionGetNpcIconDesc( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId )
{
	try
	{
		vector<uint32> npcKeys;
		uint8 nb8;
		bms.serial( nb8 );
		npcKeys.resize( nb8 );
		for (uint i=0; i!=(uint)nb8; ++i)
		{
			bms.serial( npcKeys[i] );
		}
		CMessage msgout("CLIENT:NPC_ICON:GET_DESC");
		msgout.serial( sender );
		msgout.serialCont( npcKeys );
		CUnifiedNetwork::getInstance()->send("EGS", msgout);
		//nldebug("Forwarding GET_DESC to EGS (%hu NPCs)", nb8 );
	}
	catch (const Exception &e)
	{
		nlwarning("<cbImpulsionGetNpcIconDesc> %s", e.what());
		return;
	}
}


/*
 * General receiving function for impulsions by CEntityId from client to server
 */
void routeImpulsionIdFromClient( NLMISC::CBitMemStream& bms, const NLMISC::CEntityId& sender, const NLMISC::TGameCycle& gamecycle )
{
	string msgName;
	try
	{
/*		// Check player behaviour in case the player is stunned
		const TDataSetRow& dataSetRow = TheDataset.getDataSetRow( sender );
		if ( dataSetRow.isValid() )
		{
			CMirrorPropValueRO<bool> stunned( TheDataset, dataSetRow, DSPropertyStunned );
			if ( stunned() )
			{
				nldebug("Entity %s is stunned, reject impulsion", sender.toString().c_str());
				return;
			}
		}
*/
		// Decode XML header
		CGenericXmlMsgHeaderManager::CNodeId	msgNodeId = GenericXmlMsgHeaderMngr.getNodeId(bms, msgName);

		// check if message decoded safely
		if (!msgNodeId.isValid())
		{
			nlwarning("Unable to decode message from sender %s", sender.toString().c_str());
		}
		else
		{
			// get format and sendto of message
			const CGenericXmlMsgHeaderManager::TMessageFormat	&format = GenericXmlMsgHeaderMngr.getNodeFormat(msgNodeId);
			const string										&sendto = GenericXmlMsgHeaderMngr.getNodeSendTo(msgNodeId);

			// check sendto
			if (sendto == "")
			{
				TImpulsionIdCb	cb = (TImpulsionIdCb)(GenericXmlMsgHeaderMngr.getNodeUserData(msgNodeId, 0));
				if (cb != NULL)
				{
					cb(sender, bms, gamecycle, SelfServiceId);
					//nldebug( "RTG: Routed impulsion %s via user callback [%s]", msgName.c_str(), sender.toString().c_str() );
				}
				else
				{
					// Try with the "impulsions uid" callbacks.
					// After a CEntityId has been stored for a user, the FS routes all by id. The client
					// may want to send a message that has a uid callback.
					// Warning: the message should contain the character index, as the FS callback knows only the userId
					TImpulsionUidCb ucb = (TImpulsionUidCb)(GenericXmlMsgHeaderMngr.getNodeUserData(msgNodeId, 1));
					if (ucb != NULL)
					{
						ucb((uint32)(sender.getShortId() >> 4), bms, gamecycle);
						nldebug( "RTG: Routed impulsion %s (redir uid) via user callback [%s]", msgName.c_str(), sender.toString().c_str() );
					}
					else
					{
						nlwarning( "RTG: Can't route impulsion %s [%s], no 'sendto' nor user callback", msgName.c_str(), sender.toString().c_str() );
						return;
					}
				}
			}
			else
			{
				// create forward message
				CMessage	msgout("CLIENT:"+msgName);
				msgout.serial(const_cast<CEntityId&>(sender));

				if (GenericXmlMsgHeaderMngr.nodeUsesCycle(msgNodeId))
					msgout.serial(const_cast<NLMISC::TGameCycle&>(gamecycle));

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
				ucstring	ucs;

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
					case CGenericXmlMsgHeaderManager::UCString:
						bms.serial(ucs);
						msgout.serial(ucs);
						break;
					}
				}

				// send message to concerned service
				//CUnifiedNetwork::getInstance()->send(sendto, msgout );
				sendMessageViaMirror( sendto, msgout ); // because of guild management
				//nldebug( "RTG: Routed impulsion %s to %s [%s]", msgName.c_str(), sendto.c_str(), sender.toString().c_str() );
			}
		}

		// transfer the message to the BBS
		/*if( BBSUp )
		{
			CMessage msgout("IMPULSION_ID");

			msgout.serial( sender );
			msgout.serialCont( v );
			msgout.serial( gamecycle );

			CUnifiedNetwork::getInstance()->send("BBS", msgout);
		}*/
	}
	catch(const Exception &e)
	{
		nlwarning("<routeImpulsionIdFromClient> %s %s", msgName.c_str(), e.what() );
		return;
	}

} // cbImpulsionId //


/*
 * Receiving function for impulsions by CEntityId, with slot translation, from client to server
 */
void	routeImpulsionIdSlotFromClient( const uint32& targetLootHarvest, const TDataSetRow& targetIndex, const TDataSetRow& senderIndex, const NLMISC::TGameCycle& )
{
	if ( targetLootHarvest == 0 )
	{
		// TARGET
		CMessage msgout( "TARGET" );
		msgout.serial( const_cast<TDataSetRow&>(senderIndex),
					   const_cast<TDataSetRow&>(targetIndex) ); // the gamecycle is not used by EGS
		CUnifiedNetwork::getInstance()->send( "EGS", msgout );
	}
	else
	{
		// PICKUP (loot/harvest)
		CMessage msgout( "ITEM_PICK_UP" );
		const CEntityId& senderId = TheDataset.getEntityId( senderIndex );
		const CEntityId& pickedId = TheDataset.getEntityId( targetIndex );
		msgout.serial( const_cast<CEntityId&>(senderId),
					   const_cast<CEntityId&>(pickedId) );
		uint8 lootHarvestState = (uint8)targetLootHarvest; // targetLootHarvest is one from LHSTATE::TLHState
		msgout.serial( lootHarvestState );
		CUnifiedNetwork::getInstance()->send( "EGS", msgout );
	}
}


/*
 * Map the callbacks for messages with special data handling
 */
void	initImpulsionId()
{
	GenericXmlMsgHeaderMngr.setUserData("CONNECTION:READY",				(uint64)impulsionRdy, 0);
	GenericXmlMsgHeaderMngr.setUserData("DEBUG:CMD",					(uint64)impulsionCmd, 0);
//	GenericXmlMsgHeaderMngr.setUserData("COMMAND:ADMIN",				(uint64)impulsionAdmin, 0);
//	GenericXmlMsgHeaderMngr.setUserData("STRING_MANAGER:STRING_RQ",		(uint64)impulsionStringRequestId, 0);
	GenericXmlMsgHeaderMngr.setUserData("PHRASE:LEARN",					(uint64)impulsionPhraseLearn, 0);
	GenericXmlMsgHeaderMngr.setUserData("PHRASE:MEMORIZE",				(uint64)impulsionPhraseMemorize, 0);	
	GenericXmlMsgHeaderMngr.setUserData("PHRASE:EXECUTE_FABER",			(uint64)impulsionExecuteFaber, 0);	
	GenericXmlMsgHeaderMngr.setUserData("MODULE_GATEWAY:FEOPEN",		(uint64)cbImpulsionIdGatewayOpen, 0);	
	GenericXmlMsgHeaderMngr.setUserData("MODULE_GATEWAY:GATEWAY_MSG",	(uint64)cbImpulsionIdGatewayMessage, 0);	
	GenericXmlMsgHeaderMngr.setUserData("MODULE_GATEWAY:FECLOSE",		(uint64)cbImpulsionIdGatewayClose, 0);	
	GenericXmlMsgHeaderMngr.setUserData("NPC_ICON:GET_DESC",			(uint64)cbImpulsionGetNpcIconDesc, 0);
}

