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

#ifdef NL_OS_WINDOWS
#pragma warning (disable : 4355) // warning C4355: 'this' : used in base member initializer list
#endif // NL_OS_WINDOWS

#ifndef RYAI_BOT_PET_H
#define RYAI_BOT_PET_H

#include "ai_bot.h"
#include "path_behaviors.h"

class CBotPet;
class CGrpPet;
class CPetOwner;

class IAIBotCAIProfile;
class CSpawnGroupPet;

//////////////////////////////////////////////////////////////////////////////
// CSpawnBotPet                                                             //
//////////////////////////////////////////////////////////////////////////////

class CSpawnBotPet
: public NLMISC::CDbgRefCount<CSpawnBotPet>
, public CSpawnBot
{
public:
	CSpawnBotPet(TDataSetRow const& row, CBot& owner, NLMISC::CEntityId const& id, float radius, uint32 level, RYAI_MAP_CRUNCH::TAStarFlag denyFlags);
	
	void processEvent(CCombatInterface::CEvent const& event) { }
	
	RYZOMID::TTypeId getRyzomType() const { return RYZOMID::pack_animal; }
	
	CBotPet& getPersistent	();
	CBotPet const& getPersistent() const;
	
	CSpawnGroupPet& spawnGrp();
	
	void sendInfoToEGS() const { }
	
	// pets are always attackable by bots
	virtual	bool isBotAttackable() const { return true; }
	
	// Take position from mirror
	void updatePos();
	
	// Return true if the animal is mounted (thus controlled by the GPMS)
	bool isMounted() const;
	
	CPathPosition& pathPos() { return _PathPos; }
	
	uint32 _DeathTime;

	void setVisualPropertiesName();
	
private:

	CPathPosition	_PathPos;
};

//////////////////////////////////////////////////////////////////////////////
// CBotPet                                                                  //
//////////////////////////////////////////////////////////////////////////////

class CBotPet
: public CBot
{
public:
	CBotPet(CGroup* owner, CAIAliasDescriptionNode* alias = NULL);
	~CBotPet();
	
	void getSpawnPos(CAIVector& triedPos, RYAI_MAP_CRUNCH::CWorldPosition& spawnPos, RYAI_MAP_CRUNCH::CWorldMap const& worldMap, CAngle& spawnTheta);
	void setSpawnPos(CAIPos const& spawnPos) { _SpawnPos = spawnPos; }
	CGrpPet& getPetGroup();
	
	void update(uint32 ticks, CAIEntityPhysical const* petOwner);
	
	CSpawnBotPet* getSpawn() { return static_cast<CSpawnBotPet*>(getSpawnObj()); }
	
	CAIS::CCounter& getSpawnCounter();
	
	void setDespawn() { _MustDespawn = true; }
	bool haveToDespawn() const { return _MustDespawn; }
	
	void changeOwner(NLMISC::CEntityId const& newOwner);
	
	virtual std::string	getOneLineInfoString() const { return std::string("Pet bot '") + getName() + "'"; }
	
	virtual void triggerSetSheet(AISHEETS::ICreatureCPtr const& sheet);
	
protected:
	RYZOMID::TTypeId getRyzomType() const { return RYZOMID::pack_animal; }
	
	CSpawnBot* getSpawnBot(TDataSetRow const& row, NLMISC::CEntityId const& id, float radius)
	{
		return new CSpawnBotPet(row, *this, id, radius, getSheet()->Level(), getGroup().getAStarFlag());
	}
	
private:
	CAIPos	_SpawnPos;
	bool	_MustDespawn;
};

/****************************************************************************/
/* Inlined methods                                                          */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CSpawnBotPet                                                             //
//////////////////////////////////////////////////////////////////////////////

inline
CSpawnBotPet::CSpawnBotPet(TDataSetRow const& row, CBot& owner, NLMISC::CEntityId const& id, float radius, uint32 level, RYAI_MAP_CRUNCH::TAStarFlag denyFlags)
: CSpawnBot(row, owner, id, radius, level, denyFlags)
, _DeathTime(0)
, _PathPos(CAngle())
{
}

inline
CBotPet& CSpawnBotPet::getPersistent()
{
	return static_cast<CBotPet&>(CSpawnBot::getPersistent());
}

inline
CBotPet const& CSpawnBotPet::getPersistent() const
{
	return static_cast<const CBotPet&>(CSpawnBot::getPersistent());
}

//////////////////////////////////////////////////////////////////////////////
// CBotPet                                                                  //
//////////////////////////////////////////////////////////////////////////////

inline
CBotPet::CBotPet(CGroup* owner, CAIAliasDescriptionNode* alias)
: CBot(owner, alias)
, _MustDespawn(false)
{
}

inline
CBotPet::~CBotPet()
{
	if (!isSpawned())
		return;
	despawnBot();
}

inline
CAIS::CCounter& CBotPet::getSpawnCounter()
{
	return CAIS::instance()._PetBotCounter;
}

#endif
