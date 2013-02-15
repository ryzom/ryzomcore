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

#ifndef RY_BUILDING_PHYSICAL_H
#define RY_BUILDING_PHYSICAL_H

#include "building_template.h"
#include "egs_pd.h"

class CTPDestination;
class CCharacter;
class CGuild;

/**
 * Interface for physical buildings
 * a physical building ( i.e : each physical building represents an outdoor building. They references building templates )
 * For example there can be only 1 matis guild building template. But the template is referenced by every giant tree that contains guild appartments
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class IBuildingPhysical
{
	friend class CBuildingTest;

public:
	/// ctor from name and template
	inline IBuildingPhysical(CBuildingTemplate* templ, TAIAlias alias);
	virtual ~IBuildingPhysical(){}
	/// build the building additionnal properties from data. 
	bool build( const NLLIGO::IPrimitive* prim, CBuildingParseData & parseData );
	
	///\name accessors
	//@{
	/// return the template used by this building
	inline const CBuildingTemplate * getTemplate() const;
	/// get the name of the building
	inline const std::string & getName() const;
	/// return the state counter of the building. Incremented each time a guild / player room is added to the building. Useful to check if client and server are sync
	inline uint16 getStateCounter()const;
	/// return the alias of the building
	inline TAIAlias getAlias()const;
	/// return an exit from an exit index
	inline const CTPDestination* getExit( uint8 idx )const;
	/// return the default exit spawn zone
	inline uint16 getDefaultExitSpawn() const;
	//@}
	
	/// get the cell id of a physical room of this building. Return true if no error. 
	/// If the room was not already instanciated, this method instanciate it
	/// ownerIdx is the index of the owner ( player / guild ) in the room owner vector
	bool addUser(CCharacter * user, uint16 roomIdx,uint16 ownerIdx, sint32 & cellId);
	/// return false if the user was not present in the building
	inline bool removeUser(const TDataSetRow & row);
	/// return true if user is in building
	bool isUserInsideBuilding( const TDataSetRow & user );

	///\name overridable
	//@{
	/// fill the icon param, send the description text to the user and store the resulting text id in the textId param. 
	virtual void getClientDescription(uint16 roomIdx, uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const = 0;
	/// callback called when all the appartments of a user must be removed
	virtual void onPlayerDeletion( const NLMISC::CEntityId & userId ){}
	/// callback called when all the appartments of a guild must be removed
	virtual void onGuildDeletion( uint32 guild ){}
	/// return true if the user is allowed to go in the building
	virtual bool isUserAllowed(CCharacter * user, uint16 ownerIdx, uint16 roomIdx) = 0;
	///return the number of owner in this building ( return 1 for common buildings )
	virtual uint16 getOwnerCount(){ return 1; }
	/// dump building infos
	virtual void dumpBuilding(NLMISC::CLog & log) const =0;
	//@}

protected:
	/// overridable helper used to init the room vector
	virtual void initRooms() = 0;
	
	/// alias ( unique id ) of the building
	TAIAlias						_Alias;
	/// building template used by this destination
	const CBuildingTemplate *		_Template;
	/// name of the building
	std::string						_Name;
	/// exits
	std::vector<CTPDestination*>	_Exits;
	/// id of the default exit spawn zone
	uint16							_DefaultExitSpawn;
	/// state counter of the building
	uint16							_StateCounter;
	
	/// the rooms of the building
	struct CRoomPhysical
	{
		std::vector<sint32> Cells;
	};
	std::vector<CRoomPhysical>		_Rooms;

	/// the players in the building
	std::vector<TDataSetRow>		_UsersInside;

	// list of players
	std::vector<NLMISC::CEntityId> _Players;
};


/// physical building that are owned by nobody
class CBuildingPhysicalCommon : public IBuildingPhysical
{
	friend class CBuildingTest;

	NL_INSTANCE_COUNTER_DECL(CBuildingPhysicalCommon);
public:

	/// ctor from name and template
	inline CBuildingPhysicalCommon(CBuildingTemplate* templ, TAIAlias alias);	
	/// reset a room of this building
	inline void resetRoomCell( uint16 roomIdx );
	
	virtual void getClientDescription(uint16 roomIdx, uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const;
	virtual bool isUserAllowed(CCharacter * user, uint16 ownerIdx, uint16 roomIdx){ return true; }

	virtual void dumpBuilding(NLMISC::CLog & log) const;

protected:
	/// overridable helper used to init the room vector
	virtual void initRooms();
};



/// a physical guild building 
class CBuildingPhysicalGuild : public IBuildingPhysical
{
	friend class CBuildingTest;

	NL_INSTANCE_COUNTER_DECL(CBuildingPhysicalGuild);
public:

	inline CBuildingPhysicalGuild(CBuildingTemplate* templ,TAIAlias alias);
	/// add a guild to this building
	inline void addGuild( uint32 guildId );
	/// reset a room of this building
	inline void resetRoomCell( uint16 roomIdx,uint32 guild );
	// get the id of an owner guild
	inline EGSPD::TGuildId getOwnerGuildId(uint idx);

	virtual void getClientDescription(uint16 roomIdx, uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const;
	virtual void onGuildDeletion( uint32 guild );
	virtual bool isUserAllowed(CCharacter * user, uint16 ownerIdx, uint16 roomIdx);
	virtual uint16 getOwnerCount();

	virtual void dumpBuilding(NLMISC::CLog & log) const;

private:
	virtual void initRooms();
	std::vector<EGSPD::TGuildId> _Guilds;
};

/// a physical player building 
class CBuildingPhysicalPlayer : public IBuildingPhysical
{
	friend class CBuildingTest;

	NL_INSTANCE_COUNTER_DECL(CBuildingPhysicalPlayer);
public:

	inline CBuildingPhysicalPlayer(CBuildingTemplate* templ, TAIAlias alias);
	/// add a player to this building
	inline void addPlayer( const NLMISC::CEntityId & userId );
	/// deinstanciate a room of this building
	inline void resetRoomCell( uint16 roomIdx,const NLMISC::CEntityId & userId );
	/// get an owner player
	inline const NLMISC::CEntityId & getPlayer(uint idx);
	
	virtual void getClientDescription(uint16 roomIdx, uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const;
	virtual void onPlayerDeletion( const NLMISC::CEntityId & userId );
	virtual bool isUserAllowed(CCharacter * user, uint16 ownerIdx, uint16 roomIdx);
	virtual uint16 getOwnerCount();

	virtual void dumpBuilding(NLMISC::CLog & log) const;
	
private:
	virtual void initRooms();

};

#include "building_physical_inline.h"

#endif // RY_BUILDING_PHYSICAL_H

/* End of building_physical.h */
