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

#ifndef RY_OUTPOST_SQUAD_H
#define RY_OUTPOST_SQUAD_H

#include "nel/misc/types_nl.h"
#include "mission_manager/ai_alias_translator.h"
#include "game_share/outpost.h"

/**
 * All classes in this file : 
 * \author Nicolas Brigand
 * \author Olivier Cado
 * \author Sebastien Guignot
 * \author Jerome Vuarand
 * \author Nevrax France
 * \date 2005
 */

/// forward declarations
class CStaticOutpostSquad;

/// a spawn zone used to spawn squads in the outpost
class COutpostSpawnZone
{
public:
	COutpostSpawnZone();
	COutpostSpawnZone(TAIAlias alias, const NLMISC::CVector & center, float radius);

	/// get the alias of the spawn zone
	TAIAlias alias() const { return _Alias; }
	/// get the center of the spawn zone
	const NLMISC::CVector & getCenter() const { return _Center; }
	/// get the radius of the spawn zone (in meters)
	const float getRadius() const { return _Radius; }

	/// get a one line description string (for debug)
	std::string toString() const;

private:
	/// alias of the spawn zone
	TAIAlias					_Alias;
	/// center of the spawn zone
	NLMISC::CVector				_Center;
	/// radius of the spawn zone
	float						_Radius;
};

/// a template of squad of NPC that can defend the outpost
class COutpostSquadDescriptor
{
public:
	DECLARE_PERSISTENCE_METHODS
	
	COutpostSquadDescriptor();
	COutpostSquadDescriptor(NLMISC::CSheetId sheet, TAIAlias alias, const std::string& name);

	void init(NLMISC::CSheetId sheet, TAIAlias alias, const std::string& name);
	TAIAlias alias() const { return _Alias; }
	NLMISC::CSheetId sheet() const { return _Sheet; }

	/// get static form
	const CStaticOutpostSquad * getStaticForm() const { return _Form; }

	/// get a one line description string (for debug)
	std::string toString() const;

private:
	/// called before storing a pdr save record for all generic processing work
	void preStore() const;
	/// called after applying a pdr save record for all generic processing work
	void postLoad();

private:
	/// sheet id describing the squad
	NLMISC::CSheetId			_Sheet;
	/// alias of the npc group
	TAIAlias					_Alias;
	/// static form
	const CStaticOutpostSquad *	_Form;
};

/// a squad of NPC that can defend the outpost
class COutpostSquad
: public NLMISC::CRefCount
{
public:
	/// @name Constructors and destructor
	//@{
	COutpostSquad();
	explicit COutpostSquad(TAIAlias outpost, const COutpostSquadDescriptor & desc, TAIAlias spawnZone, OUTPOSTENUMS::TPVPSide side);
	virtual ~COutpostSquad();
	//@}

	/// called each time a squad is created
	static NLMISC::CSmartPtr<COutpostSquad> getSquadFromCreateOrder(uint32 createOrder);

	/// @name Transitions orders
	//@{
public:
	void spawn(TAIAlias outpost);
	void despawn();
private:
	void create();
	void destroy();
	/// recreate the squad in AIS, used to update the spawn zone of the squad in AIS
	void recreate();
	//@}
public:
	
	/// @name Transitions notifications
	//@{
	void created(uint32 createOrder, uint32 groupId);
	void spawned();
	void despawned();
	//@}
	
	/// @name AIS callbacks
	//@{
	void died();
	void leaderDied();
	void AISDown();
	void AISUp();
	//@}
	
	/// @name Accessors
	//@{
	bool isSpawned();
	bool isDead();
	bool isReady();
	/// get sheet id
	NLMISC::CSheetId getSheet() { return _Desc.sheet(); }
	/// get the group id, identifying the group in the AIS
	uint32 getGroupId() { return _GroupId; }
	/// get the squad state
	OUTPOSTENUMS::TSquadState getSquadState() const { return _State; }
	
	/// set the zone where the squad will be spawned
	/// WARNING: if necessary it recreates the group in the AIS to update spawn zone
	/// \param spawnZone: must be a valid spawn zone of the outpost
	/// \return false if spawn cannot be changed
	bool setSpawnZone(TAIAlias spawnZone);
	/// return the spawn zone alias
	TAIAlias getSpawnZone() const { return _SpawnZone; }

	/// set the outpost alias (not saved but set back after loading)
	void setOutpostAlias(TAIAlias alias) { _OwnerOutpostAlias = alias; }
	//@}
	
	/** update the squad state depending on the elapsing time
	 * \param currentTime : seconds since 1970
	 * \return false if the squad must be removed, otherwise true
	 */
	bool updateSquad(uint32 currentTime);

	/// get a one line description string (for debug)
	std::string toString() const;

private:
	/// get next AIS squad create order
	static uint32 nextCreateOrder();

	/// last AIS squad create order
	static uint32 _LastCreateOrder;
	/// AIS squad create orders
	static std::map<uint32, NLMISC::CRefPtr<COutpostSquad> > _CreateOrders;

private:
	/// outpost alias to send messages to AIS
	TAIAlias					_OwnerOutpostAlias;
	/// structure describing the squad
	COutpostSquadDescriptor		_Desc;
	/// index of the spawn zone where the squad must be spawned
	TAIAlias					_SpawnZone;
	/// 
	OUTPOSTENUMS::TSquadState	_State;
	/// pending spawn order, 0 if none
	uint32						_CreateOrder;
	uint32						_GroupId;
	
	/// true if the squad is being recreated in AIS
	bool						_Recreate;
	/// false=belong to outpost owner, true=belong to outpost attacker
	OUTPOSTENUMS::TPVPSide		_Side;
};
typedef NLMISC::CSmartPtr<COutpostSquad> COutpostSquadPtr;

/// everything that is necessary to describe a squad before creating it
/// this is used to configure the squads that will be created in next fight round
class COutpostSquadData
: public NLMISC::CRefCount
{
public:
	DECLARE_PERSISTENCE_METHODS
	
	COutpostSquadData();
	explicit COutpostSquadData(TAIAlias outpost, const COutpostSquadDescriptor & desc, TAIAlias spawnZone);

	/// set the outpost alias (not saved but set back after loading)
	void setOutpostAlias(TAIAlias alias) { _OwnerOutpostAlias = alias; }
	/// get the owner outpost alias
	TAIAlias getOwnerOutpost() const { return _OwnerOutpostAlias; }

	/// set the spawn zone
	void setSpawnZone(TAIAlias alias) { _SpawnZone = alias; }
	/// get the spawn zone alias
	TAIAlias getSpawnZone() const { return _SpawnZone; }

	/// set the squad descriptor
	void setSquadDescriptor(COutpostSquadDescriptor & desc) { _Desc = desc; }
	/// get the squad descriptor
	const COutpostSquadDescriptor & getSquadDescriptor() const { return _Desc; }

	/// get a one line description string (for debug)
	std::string toString() const;

private:
	/// outpost alias to send messages to AIS
	TAIAlias				_OwnerOutpostAlias;
	/// structure describing the squad
	COutpostSquadDescriptor	_Desc;
	/// index of the spawn zone where the squad must be spawned
	TAIAlias				_SpawnZone;
};
typedef NLMISC::CSmartPtr<COutpostSquadData> COutpostSquadDataPtr;


#endif // RY_OUTPOST_SQUAD_H

/* End of outpost_squad.h */
