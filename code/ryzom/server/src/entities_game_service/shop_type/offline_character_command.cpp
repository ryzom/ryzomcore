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

#include "offline_character_command.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/sstring.h"
#include "nel/misc/command.h"
#include "nel/net/service.h"
#include "nel/net/message.h"

#include "../egs_variables.h"

#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "entity_manager/entity_callbacks.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(IOfflineCommand);
NL_INSTANCE_COUNTER_IMPL(COfflineCharacterCommand);

COfflineCharacterCommand * COfflineCharacterCommand::_Instance = 0;
const string CSoldItem::_Token = string("ItemSold");
const string CMaximumShopStoreTimeReached::_Token = string("ItemMaxSaleStoreReached");
const string CAdminOfflineCommand::_Token = string("AdminOfflineCommand");
const string CModifyContactCommand::_Token = string("ModifyContactCommand");
string DummyOfflineCommand = string("Dummy");



//-----------------------------------------------------------------------------
COfflineCharacterCommand * COfflineCharacterCommand::getInstance()
{
	if( _Instance == 0 )
	{
		_Instance = new COfflineCharacterCommand();
		nlassert( _Instance != 0 );
	}
	return _Instance;
}

//-----------------------------------------------------------------------------
COfflineCharacterCommand::COfflineCharacterCommand()
{
	CFile::createDirectory( Bsi.getLocalPath() + toString("characters_offline_commands") );
}

//-----------------------------------------------------------------------------
IOfflineCommand * COfflineCharacterCommand::factory( const std::string& command )
{
	if( command == DummyOfflineCommand )
	{
		return 0;
	}
	if( command.empty() == false )
	{
		CSString c = command;
		string commandToken = c.strtok(",");
		
		IOfflineCommand * commandInterface = 0;

		if( commandToken == CSoldItem::_Token )
		{
			commandInterface = new CSoldItem( command );
		}
		else if( commandToken == CMaximumShopStoreTimeReached::_Token )
		{
			commandInterface = new CMaximumShopStoreTimeReached( command );
		}
		else if( commandToken == CAdminOfflineCommand::_Token )
		{
			commandInterface = new CAdminOfflineCommand( command );
		}
		else if( commandToken == CModifyContactCommand::_Token )
		{
			commandInterface = new CModifyContactCommand( command );
		}
		if( commandInterface == 0 )
		{
			nlwarning("Can't create command %s", command.c_str());
		}
		return commandInterface;
	}
	nlwarning("command are empty !");
	return 0;
}

//-----------------------------------------------------------------------------
bool COfflineCharacterCommand::addOfflineCommand( const std::string& command )
{
	IOfflineCommand * commandInterface = factory( command );

	if( commandInterface == 0 )
	{
		return false;
	}

	// try apply command
	if( commandInterface->apply(false) == false )
	{
		/* ben's code */
		std::string	filename = getOfflineCommandsFilename(commandInterface->getEntityId());
		Bsi.append(filename, command);
	}
	delete commandInterface;
	return true;
}

//-----------------------------------------------------------------------------
bool COfflineCharacterCommand::addOfflineCommandWithoutApply( const std::string& command )
{
	IOfflineCommand * commandInterface = factory( command );
	
	if( commandInterface == 0 )
	{
		return false;
	}
	
	std::string	filename = getOfflineCommandsFilename(commandInterface->getEntityId());
	Bsi.append(filename, command);

	delete commandInterface;
	return true;
}
//-----------------------------------------------------------------------------
void COfflineCharacterCommand::characterOnline( const NLMISC::CEntityId& entity )
{
	std::string	filename = getOfflineCommandsFilename(entity);

//	nlinfo("BSIF: requesting file...");
	Bsi.requestFile(filename, new COfflineCommandFileCallback(entity));
	return;
}


//-----------------------------------------------------------------------------
std::string	COfflineCharacterCommand::getOfflineCommandsFilename(const CEntityId& entity)
{
	uint32	userId = PlayerManager.getPlayerId( entity );
	uint32	charIndex = PlayerManager.getCharIndex( entity );
	return toString("%s/account_%u_%u.offline_commands", PlayerManager.getOfflineCommandPath(userId, true).c_str(), userId, charIndex);
}

//-----------------------------------------------------------------------------
COfflineCharacterCommand::COfflineCommandFileCallback::COfflineCommandFileCallback(const NLMISC::CEntityId& id)
{
	Id = id;
}

//-----------------------------------------------------------------------------
void COfflineCharacterCommand::COfflineCommandFileCallback::callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
{
	// finalize client ready first for register charId in front end before send system message to client
	finalizeClientReady( PlayerManager.getPlayerId( Id ), PlayerManager.getCharIndex( Id ) );

	// if no offline commands
	if (fileDescription.FileName.empty() == false && fileDescription.FileSize != 0 )
	{
		while( true )
		{
			string line;
			getLine( line, dataStream );
			if( line.empty() )
				break;
			
			IOfflineCommand * commandInterface = COfflineCharacterCommand::getInstance()->factory( line );
			if( commandInterface )
			{
				commandInterface->apply(true);
				delete commandInterface;
			}
		}
	}
	if (!fileDescription.FileName.empty())
		Bsi.deleteFile( fileDescription.FileName, false );
}
	
//-----------------------------------------------------------------------------
void COfflineCharacterCommand::COfflineCommandFileCallback::getLine( std::string& line, NLMISC::IStream& dataStream )
{
	while (true)
	{
		char c;
		try
		{
			// read one byte
			dataStream.serialBuffer ((uint8 *)&c, 1);
		}
		catch(...)
		{
			return;
		}
		
		// if end line
		if (c == '\n')
		{
			return;
		}
		
		// skip '\r' char
		if (c != '\r')
		{
			line += c;
		}
	}
}

//-----------------------------------------------------------------------------
CSoldItem::CSoldItem( const std::string& command )
{
	if( command.size() > 0 )
	{
		CSString c = command;
		string extract = c.strtok(",");
		nlassert( extract == _Token );
		extract = c.strtok(",");
		_EntityId.fromString( extract.c_str() );
		extract = c.strtok(",");
		_ItemSheet = CSheetId( extract );
		extract = c.strtok(",");
		NLMISC::fromString(extract, _Quantity);
		extract = c.strtok(",");
		NLMISC::fromString(extract, _Price);
		extract = c.strtok(",");
		NLMISC::fromString(extract, _BasePrice);
		extract = c.strtok(",");
		NLMISC::fromString(extract, _Identifier);
		_Buyer.fromString( c.c_str() );
	}
}

//-----------------------------------------------------------------------------
bool CSoldItem::apply(bool offlineCommand)
{
	CCharacter * c = PlayerManager.getActiveChar( PlayerManager.getPlayerId( _EntityId ) );
	if( c != 0 && c->getId() == _EntityId )
	{
		c->itemSolded( _Identifier, _Quantity, _Price, _BasePrice, _Buyer, offlineCommand );
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CSoldItem::makeStringCommande( std::string& command, const NLMISC::CEntityId& id, const NLMISC::CSheetId& item, uint32 quantity, uint32 unitPrice, uint32 unitBasePrice, uint32 identifier, const NLMISC::CEntityId& buyer )
{
	command = _Token + string(",") + id.toString() + string(",") + item.toString() + string(",") + NLMISC::toString( quantity ) + string(",") + NLMISC::toString( unitPrice ) + string(",") + NLMISC::toString( unitBasePrice ) + string(",") + NLMISC::toString( identifier ) + string(",") + buyer.toString();
}

//-----------------------------------------------------------------------------
CMaximumShopStoreTimeReached::CMaximumShopStoreTimeReached( const std::string& command )
{
	if( command.size() > 0 )
	{
		CSString c = command;
		string extract = c.strtok(",");
		nlassert( extract == _Token );
		extract = c.strtok(",");
		_EntityId.fromString( extract.c_str() );
		extract = c.strtok(",");
		_ItemSheet = CSheetId( extract );
		extract = c.strtok(",");
		NLMISC::fromString(extract, _Quantity);
		NLMISC::fromString(c, _Identifier);
	}
}

//-----------------------------------------------------------------------------
bool CMaximumShopStoreTimeReached::apply(bool offline)
{
	CCharacter * c = PlayerManager.getActiveChar( PlayerManager.getPlayerId( _EntityId ) );
	if( c != 0 && c->getId() == _EntityId )
	{
		c->itemReachMaximumSellStoreTime( _Identifier, _Quantity, offline );
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CMaximumShopStoreTimeReached::makeStringCommande( std::string& command, const NLMISC::CEntityId& id, const NLMISC::CSheetId& item, uint32 quantity, uint32 identifier )
{
	command = _Token + string(",") + id.toString() + string(",") + item.toString() + string(",") + NLMISC::toString( quantity ) + string(",") + NLMISC::toString( identifier );
}

//-----------------------------------------------------------------------------
CAdminOfflineCommand::CAdminOfflineCommand( const std::string& command )
{
	if( command.size() > 0 )
	{
		CSString c = command;
		string extract = c.strtok(",");
		nlassert( extract == _Token );
		extract = c.strtok(",");
		_EntityId.fromString( extract.c_str() );
		_AdminCommand = c;
	}
}

//-----------------------------------------------------------------------------
bool CAdminOfflineCommand::apply(bool offline)
{
	CCharacter * c = PlayerManager.getActiveChar( PlayerManager.getPlayerId( _EntityId ) );
	if( c != 0 && c->getId() == _EntityId )
	{
		if(!c->getEnterFlag())
			return false;
		if(!TheDataset.isAccessible(c->getEntityRowId()))
			return false;
		nlwarning ("CAdminOfflineCommand::apply: Execute client admin offline command '%s' on character", _AdminCommand.c_str(), _EntityId.toString().c_str());
		NLMISC::ICommand::execute(_AdminCommand, *InfoLog);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CAdminOfflineCommand::makeStringCommande( std::string& command, const NLMISC::CEntityId& id, const std::string& adminCommand )
{
	command = _Token + string(",") + id.toString() + string(",") + adminCommand;
}


//-----------------------------------------------------------------------------
NLMISC_COMMAND(addCharacterOfflineCommand,"add command apply when character goes online","string")
{
	if (args.size() != 1)
		return false;
	
	COfflineCharacterCommand::getInstance()->addOfflineCommand( args[0] );
	return true;
}


// ***************************************************************************
CModifyContactCommand::CModifyContactCommand( const std::string& command )
{
	if( command.size() > 0 )
	{
		CSString c = command;
		string extract = c.strtok(",");
		nlassert( extract == _Token );
		// get entity
		extract = c.strtok(",");
		_EntityId.fromString( extract.c_str() );
		// get operation
		extract = c.strtok(",");
		_Operation= extract;
		// get entity referenced
		_Other.fromString( c.c_str() );
	}
}

// ***************************************************************************
bool CModifyContactCommand::apply(bool offlineCommand)
{
	CCharacter * c = PlayerManager.getActiveChar( PlayerManager.getPlayerId( _EntityId ) );
	if( c != 0 && c->getId() == _EntityId )
	{
		c->contactListRefChangeFromCommand( _Other, _Operation );
		return true;
	}
	return false;
}

// ***************************************************************************
void CModifyContactCommand::makeStringCommande( std::string& command, const NLMISC::CEntityId& id, const std::string &contactOperation, const NLMISC::CEntityId& other )
{
	command = _Token + string(",") + id.toString() + string(",") + contactOperation + string(",") + other.toString();
}

