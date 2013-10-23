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

/*****************************************************************************/
//						IBuildingPhysical implementation
/*****************************************************************************/

//----------------------------------------------------------------------------
inline IBuildingPhysical::IBuildingPhysical(CBuildingTemplate* templ, TAIAlias alias)
:_Template(templ),_Alias(alias),_StateCounter(0)
{
}

//----------------------------------------------------------------------------
inline const CBuildingTemplate * IBuildingPhysical::getTemplate() const
{
	return _Template;
}

//----------------------------------------------------------------------------
inline const std::string & IBuildingPhysical::getName() const
{
	return _Name; 
}

//----------------------------------------------------------------------------
inline uint16 IBuildingPhysical::getStateCounter()const
{
	return _StateCounter;
}

//----------------------------------------------------------------------------
inline TAIAlias IBuildingPhysical::getAlias()const
{
	return _Alias;
}

//----------------------------------------------------------------------------
inline const CTPDestination* IBuildingPhysical::getExit( uint8 idx )const
{
	if ( idx >= _Exits.size() ) return NULL;
	return _Exits[idx];
}

//----------------------------------------------------------------------------
inline uint16 IBuildingPhysical::getDefaultExitSpawn()const
{
	return _DefaultExitSpawn;
}

//----------------------------------------------------------------------------
inline bool IBuildingPhysical::removeUser(const TDataSetRow & row)
{
	std::vector<TDataSetRow>::iterator it = std::find( _UsersInside.begin(), _UsersInside.end(), row );
	if ( it != _UsersInside.end() )
	{
		(*it) = _UsersInside.back();
		_UsersInside.pop_back();
		return true;
	}
	return false;
}

/*****************************************************************************/
//						CBuildingPhysicalCommon implementation
/*****************************************************************************/

//----------------------------------------------------------------------------
CBuildingPhysicalCommon::CBuildingPhysicalCommon(CBuildingTemplate* templ, TAIAlias alias)
:IBuildingPhysical(templ,alias)
{
}

//----------------------------------------------------------------------------
inline void CBuildingPhysicalCommon::resetRoomCell( uint16 roomIdx )
{
	BOMB_IF( (roomIdx >= _Rooms.size()),
		NLMISC::toString("<BUILDING> in building '%s' roomIdx %hu asked, count is %u", _Name.c_str(), roomIdx, _Rooms.size()),
		return
		);

	BOMB_IF( (_Rooms[roomIdx].Cells.size() < 1), "<BUILDING> trying to reset a cell out of bound", return );
	_Rooms[roomIdx].Cells[0] = 0;
}

/*****************************************************************************/
//						CBuildingPhysicalGuild implementation
/*****************************************************************************/
inline CBuildingPhysicalGuild::CBuildingPhysicalGuild(CBuildingTemplate* templ,TAIAlias alias)
:IBuildingPhysical(templ,alias)
{
}

//----------------------------------------------------------------------------
void CBuildingPhysicalGuild::addGuild( uint32 guildId )
{
	if ( std::find(_Guilds.begin(), _Guilds.end(), guildId) != _Guilds.end() )
	{
		return;
	}

	_StateCounter++;
	_Guilds.push_back( guildId );
	// add an instance cell to each room for this guild
	const uint size = (uint)_Rooms.size();
	for ( uint i = 0; i < size; i++ )
	{
		_Rooms[i].Cells.push_back( 0 );
	}
}

//----------------------------------------------------------------------------
void CBuildingPhysicalGuild::resetRoomCell( uint16 roomIdx,uint32 guild )
{
	BOMB_IF( (roomIdx >= _Rooms.size()),
		NLMISC::toString("<BUILDING> in building '%s' roomIdx %hu asked, count is %u", _Name.c_str(), roomIdx, _Rooms.size()),
		return
		);

	for ( uint i = 0; i < _Guilds.size(); i++ )
	{
		if ( _Guilds[i] == guild )
		{
			BOMB_IF( (i >= _Rooms[roomIdx].Cells.size()), "<BUILDING> trying to reset a cell out of bound", return );
			_Rooms[roomIdx].Cells[i] = 0;
			return;
		}
	}
	nlwarning("<BUILDING> in building '%s' guild %u asked and not found",_Name.c_str(), guild );
}

//----------------------------------------------------------------------------
EGSPD::TGuildId CBuildingPhysicalGuild::getOwnerGuildId(uint idx)
{
	if ( idx < _Guilds.size() )
		return _Guilds[idx];
	return 0;
}



/*****************************************************************************/
//					CBuildingPhysicalPlayer implementation
/*****************************************************************************/

//----------------------------------------------------------------------------
inline CBuildingPhysicalPlayer::CBuildingPhysicalPlayer(CBuildingTemplate* templ, TAIAlias alias)
:IBuildingPhysical(templ,alias)
{
}

//----------------------------------------------------------------------------
inline void CBuildingPhysicalPlayer::addPlayer( const NLMISC::CEntityId & userId )
{
	if ( std::find(_Players.begin(), _Players.end(), userId) != _Players.end() )
	{
		nlwarning("<BUILDING> trying to add player %s which is already present in the building", userId.toString().c_str());
		return;
	}

	_StateCounter++;
	_Players.push_back( userId );
	// add an instance cell to each room for this player
	const uint size = (uint)_Rooms.size();
	for ( uint i = 0; i < size; i++ )
	{
		_Rooms[i].Cells.push_back( 0 );
	}
}

//----------------------------------------------------------------------------
inline void CBuildingPhysicalPlayer::resetRoomCell( uint16 roomIdx,const NLMISC::CEntityId & userId )
{
	BOMB_IF( (roomIdx >= _Rooms.size()),
		NLMISC::toString("<BUILDING> in building '%s' roomIdx %hu asked, count is %u", _Name.c_str(), roomIdx, _Rooms.size()),
		return
		);

	for ( uint i = 0; i < _Players.size(); i++ )
	{
		if ( _Players[i] == userId )
		{
			BOMB_IF( (i >= _Rooms[roomIdx].Cells.size()), "<BUILDING> trying to reset a cell out of bound", return );
			_Rooms[roomIdx].Cells[i] = 0;
			return;
		}
	}
	nlwarning("<BUILDING> in building '%s' player %s asked and not found",_Name.c_str(), userId.toString().c_str() );
}


//----------------------------------------------------------------------------
inline const NLMISC::CEntityId & CBuildingPhysicalPlayer::getPlayer(uint idx)
{
	if ( idx < _Players.size() )
		return _Players[idx];
	return NLMISC::CEntityId::Unknown;
}

