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

#ifndef RYZOM_OFFLINE_CHARACTER_COMMAND_H
#define RYZOM_OFFLINE_CHARACTER_COMMAND_H

// Misc
#include "nel/misc/entity_id.h"

// game share
#include "game_share/backup_service_interface.h"

// STL
#include <string>
#include <map>

extern void finalizeClientReady( uint32 userId, uint32 index );

/**
 * Interface for offline commands
 */
class IOfflineCommand
{
	NL_INSTANCE_COUNTER_DECL(IOfflineCommand);
public:
	// apply command, offline = true if command are sended during character are offline
	virtual bool apply(bool offline) = 0;

	// return character id of concerned character
	virtual const NLMISC::CEntityId& getEntityId() = 0;
};


/**
 * Sold item
 * An item in shop store are solded
 */
class CSoldItem : public IOfflineCommand
{
public:
	static const std::string	_Token;
 
	// factory
	CSoldItem( const std::string& command );

	// apply
	bool apply(bool offline);

	// return character id of concerned character
	const NLMISC::CEntityId& getEntityId() { return _EntityId; }

	// make string command
	static void makeStringCommande( std::string& command, const NLMISC::CEntityId& id, const NLMISC::CSheetId& item, uint32 quantity, uint32 unitPrice, uint32 unitBasePrice, uint32 identifier, const NLMISC::CEntityId& buyer );

private:
	NLMISC::CEntityId	_EntityId;
	NLMISC::CSheetId	_ItemSheet;
	uint32				_Quantity;
	uint32				_Price;
	uint32				_BasePrice;
	uint32				_Identifier;
	NLMISC::CEntityId	_Buyer;
};


/**
 * Item reach maximum time in shop store
 * it's destroyed
 */
class CMaximumShopStoreTimeReached : public IOfflineCommand
{
public:
	static const std::string	_Token;
	
	// factory
	CMaximumShopStoreTimeReached( const std::string& command );
	
	// apply
	bool apply(bool offline);
	
	// return character id of concerned character
	const NLMISC::CEntityId& getEntityId() { return _EntityId; }
	
	// make string command
	static void makeStringCommande( std::string& command, const NLMISC::CEntityId& id, const NLMISC::CSheetId& item, uint32 quantity, uint32 identifier );

private:
	NLMISC::CEntityId	_EntityId;
	NLMISC::CSheetId	_ItemSheet;
	uint32				_Quantity;
	uint32				_Identifier;
};

/**
 * Admin command delayed execution due to character offline
 */
class CAdminOfflineCommand : public IOfflineCommand
{
public:
	static const std::string	_Token;

	// factory
	CAdminOfflineCommand( const std::string& command );

	// apply
	bool apply(bool offline);

	// return character id of concerned character
	const NLMISC::CEntityId& getEntityId() { return _EntityId; }
	
	// make string command
	static void makeStringCommande( std::string& command, const NLMISC::CEntityId& id, const std::string& adminCommand );
		
private:
	NLMISC::CEntityId	_EntityId;
	std::string			_AdminCommand;
};


// ***************************************************************************
/**
 * A command that modify a contact 
 */
class CModifyContactCommand : public IOfflineCommand
{
public:
	static const std::string	_Token;
 
	// factory
	CModifyContactCommand( const std::string& command );

	// apply
	bool apply(bool offline);

	// return character id of concerned character
	const NLMISC::CEntityId& getEntityId() { return _EntityId; }

	// make string command
	static void makeStringCommande( std::string& command, const NLMISC::CEntityId& id, const std::string &contactOperation, const NLMISC::CEntityId& other );

private:
	NLMISC::CEntityId	_EntityId;
	NLMISC::CEntityId	_Other;
	std::string			_Operation;
};

// ***************************************************************************
/**
 * Keep commands for offline character and apply it when character become online
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2004
 */
class COfflineCharacterCommand
{
	NL_INSTANCE_COUNTER_DECL(COfflineCharacterCommand);
public:
	// get instance
	static COfflineCharacterCommand * getInstance();
	
	// factory for IOfflineCommand
	IOfflineCommand * factory( const std::string& command );

	// add offline command: try to apply, if fail send command to backup service for apply when character login
	bool addOfflineCommand( const std::string& command );

	// add offline command: do not try to apply it, just send it to backup service
	bool addOfflineCommandWithoutApply( const std::string& command );

	// a character goes online
	void characterOnline( const NLMISC::CEntityId& entity );

	// return filename of offline command for a character
	std::string	getOfflineCommandsFilename(const NLMISC::CEntityId& entity);
		
	// file callback class
	class COfflineCommandFileCallback : public IBackupFileReceiveCallback
	{
	public:
		NLMISC::CEntityId			Id;
		COfflineCommandFileCallback(const NLMISC::CEntityId& id);

		// call back for bs file asynchronous read
		void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream);

		// get line from IStream
		void getLine( std::string& line, NLMISC::IStream& dataStream );
	};
	
private:
	// private constuctor
	COfflineCharacterCommand();

	// singleton pointer
	static COfflineCharacterCommand *	_Instance;
};

#endif // RYZOM_OFFLINE_CHARACTER_COMMAND_H

/* offline_character_command.h */
