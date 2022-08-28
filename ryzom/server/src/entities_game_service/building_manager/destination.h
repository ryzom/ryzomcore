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


#ifndef RY_DESTINATION_H
#define RY_DESTINATION_H

#include "nel/ligo/primitive.h"
#include "game_share/lift_icons.h"
#include "building_enums.h"

class CCharacter;
struct CBuildingParseData;
class IBuildingPhysical;
class CBuildingTemplate;


/**
 * interface for trigger destinations created from primitive parsing.
 * Used to display infos in the player teleport interface (icon, name,... )
 * Also provides infos on destination restriction ( pets, ... ) and physical telport arrival
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class IDestination
{
public:
	/// ctor
	IDestination(const std::string & name)
		:_Name(name){}

	/// return the name of the destination
	const std::string & getName() const { return _Name; }
	/// return the cellid corresponding to the instance of the room ( 0 if no room is instanciated )
	virtual bool addUser(CCharacter * user,uint16 ownerIdx, sint32 & cellId);
	/// build the destination
	virtual bool build( const NLLIGO::IPrimitive* prim, const NLLIGO::IPrimitive* roomPrim, const CBuildingParseData & parseData ) = 0;
	/// return the state counter of the destination ( useful for client sync )
	virtual uint16 getStateCounter()const{ return 0; }
	/// return the number of entries in the destination ( 1 in most cases, more for player / guild )
	virtual uint16 getEntryCount() const { return 1; }
	/// get the destination icon and description text id ( the text is sent to the user )
	virtual void getClientDescription( uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const = 0;
	/// return true if pets can go to the destination
	virtual bool arePetsAllowed() const{ return false;} 
	/// return the destination spawn zone id. In some case in can depend of the position of the concerned user
	virtual uint16 getSpawnZoneId(const CCharacter * user)const = 0;
	/// return true if user is allowed in building
	virtual bool isUserAllowed(CCharacter * user, uint16 ownerIdx) { return true; }
	/// return true if the destination is a guild room
	virtual bool isGuildRoomDestination(){ return false; }
	
protected:
	
	/// name of the destination
	std::string					_Name;
};

/**
 * interface for destinations have their own an icon / phrase id
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class IDescribedDestination : public IDestination
{
public:
	IDescribedDestination( const std::string & name )
		:IDestination(name){}
protected:
	/// helper used to parse icon / text
	inline bool buildDescription(const NLLIGO::IPrimitive* prim);

	/// icon  to display on client
	LIFT_ICONS::TLiftIcon		_Icon;
	/// name id to display on client
	std::string					_PhraseId;
};



/**
 * a destination that is not instanciated ( example : outdoor teleport ).
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CTPDestination : public IDescribedDestination
{
	NL_INSTANCE_COUNTER_DECL(CTPDestination);
public:

	CTPDestination( const std::string & name )
		:IDescribedDestination(name){}
	virtual bool build( const NLLIGO::IPrimitive* prim, const NLLIGO::IPrimitive* roomPrim, const CBuildingParseData & parseData );
private:
	virtual bool arePetsAllowed()const; 
	virtual uint16 getSpawnZoneId(const CCharacter * user)const;
	virtual void getClientDescription( uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const;

	/// id of the spawn zone
	uint16						_ArrivalSpawn;
	/// true if pets are allowed
	bool						_TeleportPets;
};

/**
 * a destination from outside to a building
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CBuildingDestination : public IDestination
{
	friend class CBuildingTest;

	NL_INSTANCE_COUNTER_DECL(CBuildingDestination);
public:

	CBuildingDestination(const std::string & name)
		:IDestination(name){}
protected:

	virtual uint16 getStateCounter()const;
	virtual void getClientDescription( uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const;
	virtual uint16 getSpawnZoneId(const CCharacter * user)const;
	virtual bool build( const NLLIGO::IPrimitive* prim, const NLLIGO::IPrimitive* roomPrim, const CBuildingParseData & parseData );
	virtual bool addUser(CCharacter * user,uint16 ownerIdx, sint32 & cellId);
	virtual uint16 getEntryCount() const;
	virtual bool isUserAllowed(CCharacter * user, uint16 ownerIdx);
	

	/// id of the arrival spawn zone
	uint16						_ArrivalSpawn;
	/// index of the entry room, in the building template
	uint16						_ArrivalRoomIndex;
	/// physical building where to teleport the user
	IBuildingPhysical *			_ArrivalBuilding;
};

/**
 * an intra building room destination
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CRoomDestination : public IDestination
{
	NL_INSTANCE_COUNTER_DECL(CRoomDestination);
public:

	CRoomDestination(const std::string & name)
		:IDestination(name){}
	
private:
	virtual void getClientDescription( uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const;
	virtual uint16 getSpawnZoneId(const CCharacter * user)const;
	virtual bool build( const NLLIGO::IPrimitive* prim, const NLLIGO::IPrimitive* roomPrim, const CBuildingParseData & parseData );
	virtual bool addUser(CCharacter * user,uint16 ownerIdx, sint32 & cellId);
	virtual bool isUserAllowed(CCharacter * user, uint16 ownerIdx);
	virtual bool isGuildRoomDestination();

	/// index of the entry room, in the building template ( the room where the player must be to have access to this destination )
	uint16						_StartRoomIndex;
	/// index of the arrival room, in the building template
	uint16						_ArrivalRoomIndex;
	/// id of the arrival spawn zone
	uint16						_ArrivalSpawn;
	/// building template where the room is, useful to display only the destination related to the template where the player is
	/// ( it handles the case where a "3D room" is shared by several templates )
	CBuildingTemplate*			_Template;
};

/**
 * an exit building room destination
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CExitDestination : public IDestination
{
	NL_INSTANCE_COUNTER_DECL(CExitDestination);
public:

	CExitDestination(const std::string & name)
		:IDestination(name){}
private:
	virtual void getClientDescription( uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const ;
	virtual uint16 getSpawnZoneId(const CCharacter * user)const;
	virtual bool build( const NLLIGO::IPrimitive* prim, const NLLIGO::IPrimitive* roomPrim, const CBuildingParseData & parseData );
	virtual bool isUserAllowed(CCharacter * user, uint16 ownerIdx);

	/// index of the exit among the building exits
	uint8				_ExitIndex;
	/// building template where the exit is, useful to display only the destination related to the template where the player is
	/// ( it handles the case where a "3D room" is shared by several templates )
	CBuildingTemplate*	_Template;
	/// index of the room (in the template) where the exit is. Useful in case a building template has 2 or more room templates with the same "graphic physical position"
	uint16				_StartRoomIndex;

};

#endif // RY_DESTINATION_H

/* End of destination.h */

