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

#ifndef NL_CDB_GROUP_H
#define NL_CDB_GROUP_H

#include "nel/misc/entity_id.h"
#include "player_manager/cdb_synchronised.h"
#include <set>
#include "game_item_manager/guild_inv.h"


class CCharacter;
class IDataProvider;

typedef NLMISC::CEntityId CCDBRecipient;


/**
 * Database group.
 *
 * A group type is identified by a TCDBBank enum.
 * One recipient CAN'T belong to several groups of the same bank (ex: one player can belong
 * to 0 or 1 guild).
 *
 * What are the main advantages of using a CCDBGroup instead of duplicating changes
 * on several character's databases?
 * - Less user code.
 * - The data are shared, and the change trackers are shared as well (less memory needed).
 * - The delta buffer is built only once per tick (less CPU).
 * - The delta buffer is sent in one message to a FS (less network traffic).
 * - The number of bits used to locate properties is lower, as there is one tree per bank (less
 * network traffic).
 *
 * How to use the database system?
 * Here is a tutorial, it explains how CCDBGroup is used in the EGS.
 *
 * A. Put a CCDBSynchronised database in the character class.
 *    Put a CCDBGroup in each group class (eg. in a guild class).
 *    Init them with the corresponding banks.
 *
 * B. At each game cycle, here is the using process:
 *
 * 1. To populate a CCDBGroup, use addRecipient()/removeRecipient(). addRecipient() will send to it all
 *    modifications in the group database since the beginning. removeRecipient() will ask the client to
 *    reset the corresponding data, so that a new addRecipient() would work (the delta values need be
 *    done with 0 as the reference value).
 *
 * 2. Your code sets some properties using Database.setProp(), either database of a particular character
 *    or the database in a CCDBGroup (in database.xml, the branch banks must have been set likewise).
 *
 * 3. Now you want to build a delta buffer for all characters, including modifications for groups and
 *    particular characters.
 *    Call sendDeltas() on each CCDBGroup object. You may provide maxBitSize to limit the space used by
 *    these groups. You need not call writeDelta() on the Database object in a CCDBGroup. A message will
 *    be emitted to all front-ends, to send the changes to all the recipients of the group.
 *
 * 4. Call writeDelta() on the CCDBSynchronised database of every character.
 *    You should provide maxBitSize to limit the size of the message.
 *    The recommended available size should be provided by the network emitting system (front-end service).
 *    Push the outbox bitmemstream of every character into an output message. You should differenciate the
 *    initial message with other updates so that the client can react differently (it will not call any
 *    observers for the initial message). Note: you can know the "not sent yet" flag in CCDBSynchronized
 *    database, using setAsSent()/notSentYet(). Send the message.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2004
 */
class CCDBGroup
{
public:

	/// Flags for sendDeltas
	enum {
		SendDeltasToRecipients		=	0,			///< If specified, send the deltas to the group's recipent list
		SendDeltasToAll				=	1,			///< If specified, send the deltas to everyong (broadcast), ignores the recipent list
	};

	/// Init (the singleton of CCDBStructBanks must have been initialized before)
	void				init( TCDBBank bank ) { Database.init( bank, true ); }
	
	/**
	 * Add a recipient.
	 * - recipient must be ready for writing (e.g. at state 1 of above tutorial), because it will
	 * be filled immediately with the modifications that occured before its arrival.
	 * //Obsolete:
	 * //- recipient must neither move in memory nor be removed between calls of addRecipient() and
	 * //removeRecipient(). Beware with vectors and reallocation.
	 */
	void				addRecipient( const CCDBRecipient& recipient );

	/// Remove a recipient
	void				removeRecipient( const CCDBRecipient& recipient );

	/// Use Database to set properties
	CCDBSynchronised	Database;

	/// Write and send the delta to all recipients, if there's something to write. Set maxBitSize to ~0 for no limit.
	/// @param[in] sendFlags A combination of Flags (see enum above) indicating how the deltas are to be sent
	void				sendDeltas( uint32 maxBitSize, IDataProvider& dataProvider, uint8 sendFlags);

	/// Simpler version
	/// @param[in] sendFlags A combination of Flags (see enum above) indicating how the deltas are to be sent
	void				sendDeltas( uint32 maxBitSize, uint8 sendFlags)
	{
		CFakeDataProvider dp;
		sendDeltas( maxBitSize, dp, sendFlags );
	}
	

private:

	typedef std::set< CCDBRecipient > CRecipientList;

	/// Recipients
	CRecipientList					_Recipients;

	/// New recipients since the latest sendDeltas()
	std::vector< CCDBRecipient >	_NewRecipients;
};



#endif // NL_CDB_GROUP_H

/* End of cdb_group.h */
