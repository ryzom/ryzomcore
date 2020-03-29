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
#include "cdb_group.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "nel/net/unified_network.h"
#include "game_item_manager/guild_inv.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

extern CGenericXmlMsgHeaderManager	GenericMsgManager;


CBitMemStream DBOutput( false, 2048 ); // global to avoid reallocation


/*
 * Handy logging macros & variables
 */

CVariable<bool>	VerboseCDBGroup("egs", "VerboseCDBGroup", "Flag to enable or dissable verbose logging in cdb_group.cpp", true,0,true);
#define LOG if (VerboseCDBGroup==false) {} else nldebug


/*
 * Add a recipient (recipient must not move in memory after calling addRecipient! Beware vectors
 */
void	CCDBGroup::addRecipient( const CCDBRecipient& recipient )
{
	// Add to list of new recipients (will be moved to normal list in sendDelta()
	// because if doing it here, the new recipient will receive twice the changes
	// done since the latest sendDeltas(), one of which is from the 'permanent
	// changes')
	_NewRecipients.push_back( recipient );
	
	LOG( "CDB: %s added into group", recipient.toString().c_str() );
}


/*
 * Remove a recipient
 */
void	CCDBGroup::removeRecipient( const CCDBRecipient& recipient )
{
	// Remove from list
	if ( _Recipients.erase( recipient ) == 0 )
	{
		// Not found in normal list, search in the list of new recipients
		std::vector<CCDBRecipient>::iterator inr = std::find( _NewRecipients.begin(), _NewRecipients.end(), recipient );
		if ( inr != _NewRecipients.end() )
		{
			_NewRecipients.erase( inr );
			// The group database was not sent to recipient yet => return (don't send reset bank)
		}
		else
		{
			nlwarning( "Recipient %s not found when removing from CDBGroup", recipient.toString().c_str() );
		}
		return;
	}

	// Send message for the client to clear its properties corresponding to the bank
	// If it did that when receving a new INIT_BANK, he would lose any database update
	// that occur after addRecipient() but arrive before INIT_BANK (because the impulsion
	// arriving order is not guaranteed).
	DBOutput.resetBufPos();
	GenericMsgManager.pushNameToStream( "DB_GROUP:RESET_BANK", DBOutput );
	// write the server tick, to ensure old DB update are not applied after newer
	TGameCycle	serverTick= CTickEventHandler::getGameCycle();
	DBOutput.serial(serverTick);
	// encode the bank
	uint32 bank = (uint32)Database.bank();
	uint nbits;
	FILL_nbits_WITH_NB_BITS_FOR_CDBBANK
	DBOutput.serial( bank, nbits );
	CMessage msgout( "CDB_IMPULSION" );
	msgout.serial( const_cast<CEntityId&>(recipient) );
	msgout.serialBufferWithSize( (uint8*)DBOutput.buffer(), DBOutput.length() );
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(recipient.getDynamicId()), msgout );

	LOG( "CDB: %s removed from group", recipient.toString().c_str() );
}



/*
 *
 */
void	CCDBGroup::sendDeltas( uint32 maxBitSize, IDataProvider& dataProvider, uint8 sendFlags )
{
	// Check if there are changes
	if ( (Database.getChangedPropertyCount() != 0) ||
		 dataProvider.nonEmpty() )
	{
		// Send a message to all frontends, for multiple delivery to recipients (except new ones)
		DBOutput.resetBufPos();
		GenericMsgManager.pushNameToStream( "DB_GROUP:UPDATE_BANK", DBOutput );
		// Write the server tick, to ensure old DB update are not applied after newer
		TGameCycle serverTick = CTickEventHandler::getGameCycle();
		DBOutput.serial( serverTick );
		// Encode the bank
		uint32 bank = (uint32)Database.bank();
		uint nbits;
		FILL_nbits_WITH_NB_BITS_FOR_CDBBANK
		DBOutput.serial( bank, nbits );
		// Write the shared database delta
		if ( Database.getChangedPropertyCount() != 0 )
		{
			Database.writeDelta( DBOutput, maxBitSize );
		}
		else
		{
			uint32 noChange = 0;
			DBOutput.serial( noChange, CDBChangedPropertyCountBitSize );
		}
		// Write additional provided data (for guild inventory)
		dataProvider.provideUpdate( DBOutput );
		// Send
		CMessage msgout( "CDB_MULTI_IMPULSION" );
		msgout.serial (sendFlags);
		if (sendFlags & SendDeltasToAll )		// Broadcasting event; no need to include recipient list
		{
			// All users; bypass recipient list
		}
		else
		{
			msgout.serialCont( _Recipients );	// Explicit multi-recipients
		}
	
		msgout.serialBufferWithSize( (uint8*)DBOutput.buffer(), DBOutput.length() );
		CUnifiedNetwork::getInstance()->send( "FS", msgout, false ); // viaMirror not needed because sending entity ids instead of datasetrows
		LOG( "Sent CDB_MULTI_IMPULSION to all FS (%u bytes)", msgout.length() );
	}
	// Send a message to each new recipient, with the history (including the latest changes)
	// and move them to the normal recipient list
	for ( std::vector<CCDBRecipient>::const_iterator inr=_NewRecipients.begin(); inr!=_NewRecipients.end(); ++inr )
	{
		const CCDBRecipient& recipient = (*inr);
		
		// Sent history of what was modified before its arrival in the group
		// As this is a specific message, the client can react differently from an update
		DBOutput.resetBufPos();
		GenericMsgManager.pushNameToStream( "DB_GROUP:INIT_BANK", DBOutput );
		// Write the server tick, to ensure old DB update are not applied after newer
		TGameCycle serverTick = CTickEventHandler::getGameCycle();
		DBOutput.serial( serverTick );
		// Encode the bank
		uint32 bank = (uint32)Database.bank();
		uint nbits;
		FILL_nbits_WITH_NB_BITS_FOR_CDBBANK
		DBOutput.serial( bank, nbits );
		// Write the shared database delta (from the beginning)
		if ( ! Database.writePermanentDelta( DBOutput ) )
		{
			uint32 noInitialDelta = 0;
			DBOutput.serial( noInitialDelta, 16 );
		}
		// Write additional provided data (for guild inventory)
		dataProvider.provideContents( DBOutput ); // provideUpdate() must have been called before, otherwise would not be empty()
		// Send
		CMessage msgout( "CDB_IMPULSION" );
		msgout.serial( const_cast<CEntityId&>(recipient) );
		msgout.serialBufferWithSize( (uint8*)DBOutput.buffer(), DBOutput.length() );
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(recipient.getDynamicId()), msgout );
		
		// Add to normal recipient list
		if ( ! _Recipients.insert( recipient ).second )
			nlwarning( "Recipient %s added twice into CDBGroup", recipient.toString().c_str() );
	}
	_NewRecipients.clear();
}
