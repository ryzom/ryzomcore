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
#include "animal_position_state.h"
#include "../entities.h"
#include "interface_manager.h"
#include "group_compas.h"
#include "game_share/animal_status.h"

using NLMISC::CCDBNodeLeaf;

// ***************************************************************************
// CPositionState
// ***************************************************************************



// ***************************************************************************
bool CPositionState::getPos(sint32 &px, sint32 &py)
{
	// only if the animal is spawned
	if (dbOk())
	{
		CEntityCL	*entity = getEntity();
		// If the entity is not in vision, take Database position
		if(!entity)
		{
			return getDbPos(px, py);
		}
		// else take VisualProp more precise position
		else
		{
			px = (sint32)((float)entity->lastFramePos().x * 1000);
			py = (sint32)((float)entity->lastFramePos().y * 1000);
		}

		return true;
	}
	else
	{
		px= py= 0;
		return false;
	}

}

// ***************************************************************************
void CPositionState::serialNodeLeaf(NLMISC::IStream &f, CCDBNodeLeaf *&dbNode)
{
	f.serialCheck(NELID("NL__"));
	f.serialVersion(0);
	std::string dbPath;
	if (f.isReading())
	{
		f.serial(dbPath);
		dbNode = NULL;
		if (!dbPath.empty())
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			dbNode = NLGUI::CDBManager::getInstance()->getDbProp(dbPath, false);
		}
	}
	else
	{
		if (dbNode)
		{
			dbPath = dbNode->getFullName();
		}
		f.serial(dbPath);
	}
	f.serialCheck(NELID("END_"));
}


// ***************************************************************************
void CUIDEntityPositionState::serial(NLMISC::IStream &f)
{
	f.serialCheck(NELID("UIDE"));
	f.serialVersion(0);
	serialNodeLeaf(f, _DBPos);
	serialNodeLeaf(f, _Uid);
	f.serialCheck(NELID("_END"));
}

// ***************************************************************************
CEntityCL *CUIDEntityPositionState::getEntity()
{
	if (!dbOk()) return NULL;
	CLFECOMMON::TClientDataSetIndex	uid= _Uid->getValue32();
	// If the _EntitySlot is not set, try to acquire the animal in vision.
	if(_EntitySlot == CLFECOMMON::INVALID_SLOT)
	{
		// O(n=256)
		CEntityCL	*entity= EntitiesMngr.getEntityByCompressedIndex(uid);
		// the entity position must be valid, else don't take it.
		if((entity != NULL) && entity->firstPositionReceived())
			_EntitySlot = entity->slot();

	}
	// else check that the cache slot is OK with this UID
	else
	{
		// O(1)
		CEntityCL	*entity= EntitiesMngr.entity(_EntitySlot);
		// Is data slot changed ? animal no more in vision -> try to acquire it at next pass
		if((entity == NULL) || ((entity != NULL) && (entity->dataSetId() != uid)))
			_EntitySlot= CLFECOMMON::INVALID_SLOT;
	}
	CEntityCL	*entity= EntitiesMngr.entity(_EntitySlot);
	return entity;

}

// ***************************************************************************
bool CUIDEntityPositionState::getDbPos(sint32 &px, sint32 &py)
{
	if (!dbOk()) return false;
	px = (sint32) (_DBPos->getValue64() >> 32);
	py = _DBPos->getValue32();
	return true;
}


// ***************************************************************************
// CUIDEntityPositionState
// ***************************************************************************


// ***************************************************************************
CUIDEntityPositionState::CUIDEntityPositionState()
{
	_DBPos= NULL;
	_Uid= NULL;
	_EntitySlot= CLFECOMMON::INVALID_SLOT;
}

// ***************************************************************************
void CUIDEntityPositionState::build(const std::string &baseDB)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	_DBPos= NLGUI::CDBManager::getInstance()->getDbProp(baseDB+":POS", false);
	_Uid= NLGUI::CDBManager::getInstance()->getDbProp(baseDB+":UID", false);
	// reset
	_EntitySlot= CLFECOMMON::INVALID_SLOT;
}

// ***************************************************************************
// CTeammatePositionState
// ***************************************************************************

// ***************************************************************************
CTeammatePositionState::CTeammatePositionState() : CUIDEntityPositionState()
{
	_Present= NULL;
}


// ***************************************************************************
void CTeammatePositionState::build(const std::string &baseDB)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	_Present = NLGUI::CDBManager::getInstance()->getDbProp(baseDB+":NAME", false);

	CUIDEntityPositionState::build(baseDB);
}


// ***************************************************************************
bool CTeammatePositionState::getPos(sint32 &px, sint32 &py)
{

	CEntityCL	*entity = getEntity();

	// only if the animal is spawned
	if( dbOk() && (_Present->getValue32() != 0))
	{
		return CPositionState::getPos(px, py);
	}
	else
	{
		px= py= 0;
		return false;
	}

}

// ***************************************************************************
// CAnimalPositionState
// ***************************************************************************

// ***************************************************************************
CAnimalPositionState::CAnimalPositionState() : CUIDEntityPositionState()
{
	_Status= NULL;
}


// ***************************************************************************
void			CAnimalPositionState::build(const std::string &baseDB)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	_Status= NLGUI::CDBManager::getInstance()->getDbProp(baseDB+":STATUS", false);

	CUIDEntityPositionState::build(baseDB);
}


// ***************************************************************************
bool		CAnimalPositionState::getPos(sint32 &px, sint32 &py)
{
	// only if the animal is spawned
	if( dbOk() && ANIMAL_STATUS::isSpawned((ANIMAL_STATUS::EAnimalStatus)_Status->getValue32()))
	{
		return CPositionState::getPos(px, py);
	}
	else
	{
		px= py= 0;
		return false;
	}
}

// ***************************************************************************
void CAnimalPositionState::serial(NLMISC::IStream &f)
{
	f.serialCheck(NELID("APS_"));
	f.serialVersion(0);
	CUIDEntityPositionState::serial(f);
	serialNodeLeaf(f, _Status);
	f.serialCheck(NELID("END_"));
}


// ***************************************************************************
// CNamedEntityPositionState
// ***************************************************************************


// ***************************************************************************
CEntityCL *CNamedEntityPositionState::getEntity()
{
	if (!dbOk()) return false;
	return EntitiesMngr.getEntityByName(_Name->getValue32());
}

// ***************************************************************************
bool CNamedEntityPositionState::getDbPos(sint32 &px, sint32 &py)
{
	if (!dbOk()) return false;
	px = _X->getValue32();
	py = _Y->getValue32();
	return true;
}

// ***************************************************************************
void CNamedEntityPositionState::build(CCDBNodeLeaf *name, CCDBNodeLeaf *x, CCDBNodeLeaf *y)
{
	_Name = name;
	_X = x;
	_Y = y;
}


// ***************************************************************************
CEntityCL	*CDialogEntityPositionState::getEntity()
{
	if ( CCompassDialogsManager::getInstance().getEntries().size() >= _DialogIndex )
		return NULL;
	return EntitiesMngr.getEntityByName(CCompassDialogsManager::getInstance().getEntries()[_DialogIndex].Text);
}

bool		 CDialogEntityPositionState::getDbPos(sint32 &px, sint32 &py)
{
	if ( CCompassDialogsManager::getInstance().getEntries().size() < _DialogIndex )
		return false;
	px = CCompassDialogsManager::getInstance().getEntries()[_DialogIndex].X;
	py = CCompassDialogsManager::getInstance().getEntries()[_DialogIndex].Y;
	return true;
}

// ***************************************************************************
void CNamedEntityPositionState::serial(NLMISC::IStream &f)
{
	f.serialCheck(NELID("NEPS"));
	f.serialVersion(0);
	serialNodeLeaf(f, _Name);
	serialNodeLeaf(f, _X);
	serialNodeLeaf(f, _Y);
	f.serialCheck(NELID("END_"));
}

