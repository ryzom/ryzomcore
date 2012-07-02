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



#ifndef RY_PET_INTERFACE_MESSAGES_H
#define RY_PET_INTERFACE_MESSAGES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"

#include "game_share/synchronised_message.h"
#include "game_share/mirror.h"


// Pet interface message class for AIS / EGS communication

//----------------------------------------------------------------
// pet spawn
//----------------------------------------------------------------
class CPetSpawnMsg : public CMirrorTransportClass
{
public:
	enum TSpawnMode { NEAR_PLAYER = 0, NEAR_POINT };
	uint32				AIInstanceId;
	uint16				SpawnMode;
	TDataSetRow			CharacterMirrorRow;
	NLMISC::CSheetId	PetSheetId;
	uint16				PetIdx;	// must be returned to EGS with CPetSpawnConfirmationMsg class
	sint32				Coordinate_X, Coordinate_Y, Coordinate_H; //For NEAR_POINT mode
	float				Heading; //For NEAR_POINT mode
	ucstring			CustomName;

	virtual void description ()
	{
		className ("CPetSpawnMsg");
		property ("AIInstanceId", PropUInt32, (uint32)~0, AIInstanceId);
		property ("SpawnMode", PropUInt16, (uint16)NEAR_PLAYER, SpawnMode);
		property ("CharacterMirrorRow",	PropDataSetRow,	TDataSetRow(), CharacterMirrorRow);
		property ("PetSheetId", PropSheetId, NLMISC::CSheetId::Unknown, PetSheetId);
		property ("PetIndex", PropUInt16, (uint16)0, PetIdx);
		property ("Coordinate_X", PropSInt32, (sint32)0, Coordinate_X);
		property ("Coordinate_Y", PropSInt32, (sint32)0, Coordinate_Y);
		property ("Coordinate_H", PropSInt32, (sint32)0, Coordinate_H);
		property ("Heading", PropFloat, 0.0f, Heading);
		property ("CustomName", PropUCString, ucstring(""), CustomName);
	}

	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

		
//----------------------------------------------------------------
// spawn confirmation
//----------------------------------------------------------------
class CPetSpawnConfirmationMsg : public CMirrorTransportClass
{
public:
	enum TSpawnError 
	{ 
		NO_ERROR_SPAWN = 0,
		CHARATER_UNKNOWN, 
		PET_SHEETID_UNKNOWN, 
		CHARACTER_LAND_MAP_UNKNOWN, 
		COORDINATE_NOT_IN_WORLD, 
		NOT_IMPLEMENTED, 
		INTERNAL_ERROR, 
		PET_ALREADY_SPAWNED 
	};
	uint16 SpawnError;
	TDataSetRow	CharacterMirrorRow;
	TDataSetRow	PetMirrorRow; //If spawned
	uint16 PetIdx;

	virtual void description ()
	{		
		className ("CPetSpawnConfirmationMsg");
		property ("SpawnError", PropUInt16, (uint16)NO_ERROR_SPAWN, SpawnError);
		property ("CharacterMirrorRow", PropDataSetRow,	TDataSetRow(), CharacterMirrorRow);
		property ("PetMirrorRow",		PropDataSetRow,	TDataSetRow(), PetMirrorRow);
		property ("PetIndex", PropUInt16, (uint16)0, PetIdx);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};


//----------------------------------------------------------------
// pet command
//----------------------------------------------------------------
class CPetCommandMsg : public CMirrorTransportClass
{
public:
	enum TCommand { STAND = 0, FOLLOW, GOTO_POINT, GOTO_POINT_DESPAWN, LIBERATE, DESPAWN };
	enum TCommandError { COLLISION_NOT_VALIDNO_ERROR_SPAWN = 0, CHARATER_UNKNOWN, PET_SHEETID_UNKNOWN, CHARACTER_LAND_MAP_UNKNOWN, COORDINATE_NOT_IN_WORLD, NOT_IMPLEMENTED, INTERNAL_ERROR };
	uint16 Command;
	TDataSetRow	CharacterMirrorRow;
	TDataSetRow PetMirrorRow;
	sint32	Coordinate_X, Coordinate_Y, Coordinate_H; //For GOTO_POINT, GOTO_POINT_DESPAWN and SPAWN_POINT commands
	float Heading; // For GOTO_POINT_DESPAWN commands (via stable)
	
	virtual void description ()
	{
		className ("CPetCommandMsg");
		property ("Command", PropUInt16, (uint16)STAND, Command);
		property ("CharacterMirrorRow", PropDataSetRow,	TDataSetRow(), CharacterMirrorRow);
		property ("PetMirrorRow", PropDataSetRow, TDataSetRow(), PetMirrorRow);
		property ("Coordinate_X", PropSInt32, (sint32)0, Coordinate_X);
		property ("Coordinate_Y", PropSInt32, (sint32)0, Coordinate_Y);
		property ("Coordinate_H", PropSInt32, (sint32)0, Coordinate_H);
		property ("Heading", PropFloat, 0.0f, Heading);
	}

	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------
// command confirmation
//----------------------------------------------------------------
class CPetCommandConfirmationMsg : public CMirrorTransportClass
{
public:
	enum TCommandError { NO_ERROR_COMMAND = 0, POSITION_COLLISION_NOT_VALID };
	uint16 CommandError;
	TDataSetRow	CharacterMirrorRow;
	TDataSetRow PetMirrorRow;

	CPetCommandConfirmationMsg()
	{}

	CPetCommandConfirmationMsg(uint16 commandError,	const	CPetCommandMsg	&msg)
	:CommandError(commandError)
	,CharacterMirrorRow(msg.CharacterMirrorRow)
	,PetMirrorRow(msg.PetMirrorRow)
	{}

	virtual void description ()
	{		
		className ("CPetCommandConfirmationMsg");
		property ("CommandError", PropUInt16, (uint16)NO_ERROR_COMMAND, CommandError);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------
// pet owner
//----------------------------------------------------------------
class CPetSetOwner : public CMirrorTransportClass
{
public:

	TDataSetRow	OwnerMirrorRow;
	TDataSetRow PetMirrorRow;

	virtual void description ()
	{
		className ("CPetSetOwner");
		property ("OwnerMirrorRow", PropDataSetRow, TDataSetRow(), OwnerMirrorRow);
		property ("PetMirrorRow", PropDataSetRow, TDataSetRow(), PetMirrorRow);
	}
	
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

#endif //RY_PET_INTERFACE_MESSAGES_H
