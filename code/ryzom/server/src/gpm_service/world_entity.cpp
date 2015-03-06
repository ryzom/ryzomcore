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

#include "world_entity.h"
#include "sheets.h"
#include "world_position_manager.h"
#include "gpm_service.h"

using namespace std;
using namespace NLMISC;
using namespace NLPACS;

#define DUMP_VAR(var)	log->displayNL("%28s: %s", #var, toString(var).c_str());
#define DUMP_VARPTR(var)	log->displayNL("%28s: %s", #var, toStringPtr(var).c_str());

/****************************************************************\
 ****************************************************************
						CWorldEntity
 ****************************************************************
\****************************************************************/

// Static entity allocator
CBlockMemory<CWorldEntity>	CWorldEntity::_EntityAllocator;


template<class T>
void	registerEntityProperty(const string &propertyName, CMirrorPropValue1DS<T> *property, const CEntityId &id)
{
	// TODO: optimize: use entity index instead of Id, prop index instead of name
	property->init( TheDataset, id, propertyName );
}


/// Creates a new entity (new equivalent). This must be initialised later using init();
CWorldEntity*	CWorldEntity::create()
{
	CWorldEntity*	entity = _EntityAllocator.allocate();
	if (entity != NULL)
	{
		entity->ListIterator = CWorldPositionManager::_EntityList.insert(CWorldPositionManager::_EntityList.end(), entity);
		entity->PrimIterator = CWorldPositionManager::_PrimitivedList.end();
	}
	return entity;
}

/// Removes an entity (delete equivalent).
void			CWorldEntity::remove(CWorldEntity* entity)
{
	CWorldPositionManager::_EntityList.erase(entity->ListIterator);
	if (entity->PrimIterator != CWorldPositionManager::_PrimitivedList.end())
	{
		CWorldPositionManager::_PrimitivedList.erase(entity->PrimIterator);
	}
	_EntityAllocator.free(entity);
}



/****************************************************************\
						init
\****************************************************************/
void	CWorldEntity::init( const CEntityId& id, const TDataSetRow &index )
{
	Id = id;
	Index = index;

	// Keep properties local for invisible group entities (aiVision).
	// This is obsolete, as aiVision are not used anymore. Because these entities were not
	// in mirror, their property values always stayed in temp storage. All the code was
	// the same for aiVision and visible entities, i.e. we used the CMirrorPropValueAlice.
	// Now the 'Alice' version is not needed anymore, because all entities go into
	// registerEntityProperty().
//	if ( Id.getType() != RYZOMID::aiVision )
//	{

	registerEntityProperty("X", &X, id);
	registerEntityProperty("Y", &Y, id);
	registerEntityProperty("Z", &Z, id);
	registerEntityProperty("LocalX", &LocalX, id);
	registerEntityProperty("LocalY", &LocalY, id);
	registerEntityProperty("LocalZ", &LocalZ, id);
	registerEntityProperty("Theta", &Theta, id);
	registerEntityProperty("Sheet", &Sheet, id);
	registerEntityProperty("TickPos", &Tick, id);
	registerEntityProperty("Cell", &Cell, id);
	registerEntityProperty("VisionCounter", &VisionCounter, id);
	registerEntityProperty("WhoSeesMe", &WhoSeesMe, id);
	if (Cell > 0)
	{
		nlwarning("Entity %s Cell is preset to %d which should be 0 or negative, forced to 0", id.toString().c_str(), Cell());
		Cell = 0;
	}
//	}
//	else
//		Cell = -1;

	//IsStaticObject = false;
	//IsInvisible = false;
	//IsAgent = false;
	//IsTrigger = false;
	//IsInvisibleToPlayer = false;

	RefCounter = 0;
	TickLock = 0;

	TempVisionState = false;
	TempControlInVision = false;
	TempParentInVision = false;

	HasVision = false;

	Previous = NULL;
	Next = NULL;

	PlayerInfos = NULL;

	Parent = NULL;
	Control = NULL;

	PosInitialised = false;
	Continent = INVALID_CONTINENT_INDEX;
	MoveContainer = NULL;
	Primitive = NULL;
	UsePrimitive = false;
	ForceUsePrimitive = false;
	ForceDontUsePrimitive = false;

	VisionCounter = (uint8)0x0;
	PlayersSeeingMe = 0;
	ClosestPlayer = NULL;

	CellPtr = NULL;

	PatatEntryIndex = 0;

	if (id.getType() == RYZOMID::object )
	{
		_Type = Object;
	}
	else if (id.getType() == RYZOMID::player)
	{
		_Type = Player;
	}
	else if (id.getType() == RYZOMID::trigger)
	{
		_Type = Trigger;
	}
	else if ((id.getType() >= RYZOMID::bot_ai_begin) && (id.getType() <= RYZOMID::bot_ai_end))
	{
		_Type = AI;
	}
	else
	{
		_Type = Unknown;
	}


	// commented, now set by the EGS
	//WhoSeesMe = 0xffffffff;

	CheckMotion = true;

} // CWorldEntity constructor

/****************************************************************\
			CWorldEntity destructor
\****************************************************************/
CWorldEntity::~CWorldEntity()
{
	// the Primitive should have been removed in the UMoveContainer, and so is already deleted
	
} // CWorldEntity destructor


  
/**
 * Display debug
 */
void	CWorldEntity::display(NLMISC::CLog *log) const
{
	if (this == NULL)
		log->displayNL("CWorldEntity::display called on NULL");

	log->displayNL("--- Display Entity %s E%u ---", Id.toString().c_str(), Index.getIndex());

	DUMP_VAR(X());
	DUMP_VAR(Y());
	DUMP_VAR(Z());
	DUMP_VAR(LocalX());
	DUMP_VAR(LocalY());
	DUMP_VAR(LocalZ());
	DUMP_VAR(Theta());
	DUMP_VAR(Sheet());
	DUMP_VAR(Tick());
	DUMP_VAR(Cell());
	DUMP_VAR(VisionCounter());
	DUMP_VAR(WhoSeesMe());
	DUMP_VAR(PatatEntryIndex);
	DUMP_VARPTR(CellPtr);
	DUMP_VAR(PosInitialised);
	DUMP_VAR(Continent);
	DUMP_VARPTR(Primitive);
	DUMP_VARPTR(MoveContainer);
	DUMP_VAR(UsePrimitive);
	DUMP_VAR(ForceUsePrimitive);
	DUMP_VAR(ForceDontUsePrimitive);
	DUMP_VAR(TickLock);
	DUMP_VARPTR((CWorldEntity*)Previous);
	DUMP_VARPTR((CWorldEntity*)Next);
	DUMP_VARPTR((CWorldEntity*)Parent);
	DUMP_VARPTR((CWorldEntity*)Control);
	DUMP_VAR(HasVision);
	DUMP_VAR(CheckMotion);
	DUMP_VAR(TempVisionState);
	DUMP_VAR(RefCounter);

	log->displayNL("--- End of Display ---");

	if (PlayerInfos != NULL)
		PlayerInfos->display(log);
}


/****************************************************************\
			create primitive for fiche type entity
\****************************************************************/
void CWorldEntity::createPrimitive(NLPACS::UMoveContainer *pMoveContainer, uint8 worldImage)
{
	UMovePrimitive		*primitive;

	if (PrimIterator != CWorldPositionManager::_PrimitivedList.end())
	{
		CWorldPositionManager::_PrimitivedList.erase(PrimIterator);
		PrimIterator = CWorldPositionManager::_PrimitivedList.end();
	}

	if (!UsePrimitive && !ForceUsePrimitive)
	{
		nlwarning("Entity %s asked to create a PACS primitive, whereas shouldn't not, abort", Id.toString().c_str());
		return;
	}

	if (pMoveContainer == NULL)
	{
		nlwarning("pMoveContainer == NULL in createPrimitive()");
		return;
	}

	// if a primitive already exists
	if (Primitive != NULL)
	{
		// previous move container must not be null
		if (MoveContainer == NULL)
		{
			nlwarning("MoveContainer == NULL in createPrimitive()");
			return;
		}

		// check the previous move container is different (otherwise does not create a primitive)
		if (MoveContainer == pMoveContainer)
			return;

		// depending on the type of the previous primitive, creates a copy of it
		if (Primitive->isCollisionable())
		{
			primitive = pMoveContainer->addCollisionablePrimitive(CWorldPositionManager::_FirstDynamicWorldImage, CWorldPositionManager::_NbDynamicWorldImages, Primitive);
		}
		else
		{
			primitive = pMoveContainer->addNonCollisionablePrimitive(Primitive);
		}

		// removes previous primitive
		MoveContainer->removePrimitive(Primitive);

		// and set new one as entity primitive
		Primitive = primitive;
		MoveContainer = pMoveContainer;

		if (Primitive == NULL)
		{
			MoveContainer = NULL;
			nlwarning("Can't create PACS primitive for entity %s", Id.toString().c_str());
			return;
		}

		PrimIterator = CWorldPositionManager::_PrimitivedList.insert(CWorldPositionManager::_PrimitivedList.end(), this);

		// and leave...
		return;
	}

	Primitive = NULL;
	MoveContainer = NULL;

	const CGpmSheets::CSheet	*sheet = CGpmSheets::lookup(CSheetId(Sheet()));

	float	primRadius = 0.5f;
	float	primHeight = 2.0f;
	
	if (sheet != NULL)
	{
		primRadius = sheet->Radius * sheet->Scale;
		primHeight = sheet->Height * sheet->Scale;
	}

	primitive = pMoveContainer->addNonCollisionablePrimitive();

	if (primitive == NULL)
	{
		nlwarning("Can't create PACS primitive for entity %s", Id.toString().c_str());
		return;
	}
	primitive->UserData = ((uint64)(Index.getIndex()) << 16);
	//nldebug("Set entity E%u to %"NL_I64"d", Index.getIndex(), primitive->UserData);
	primitive->setPrimitiveType( UMovePrimitive::_2DOrientedCylinder );
	primitive->setReactionType( UMovePrimitive::Slide );
	primitive->setTriggerType( UMovePrimitive::NotATrigger );
	primitive->setCollisionMask( 0xffffffff );
	primitive->setOcclusionMask( 0x00000000 );
	primitive->setObstacle( false );
	primitive->setAbsorbtion( 0 );
	primitive->setHeight( primHeight );
	primitive->setRadius( primRadius-0.1f );	// decrease primitive usable radius to lessen pacs load (avoid collision test)

	Primitive = primitive;
	MoveContainer = pMoveContainer;

	PrimIterator = CWorldPositionManager::_PrimitivedList.insert(CWorldPositionManager::_PrimitivedList.end(), this);
}

/****************************************************************\
			removePrimitive
\****************************************************************/
void CWorldEntity::removePrimitive()
{
	if (PrimIterator != CWorldPositionManager::_PrimitivedList.end())
	{
		CWorldPositionManager::_PrimitivedList.erase(PrimIterator);
		PrimIterator = CWorldPositionManager::_PrimitivedList.end();
	}

	if (MoveContainer != NULL && Primitive != NULL)
	{
		MoveContainer->removePrimitive(Primitive);
		Primitive = NULL;
		MoveContainer = NULL;
	}
	else if (MoveContainer != NULL && Primitive == NULL || MoveContainer == NULL && Primitive != NULL)
	{
		nlwarning("Entity %s asked to remove PACS primitive, unable to continue (MoveContainer=%p, Primitive=%p)", Id.toString().c_str(), MoveContainer, Primitive);
	}
}



/****************************************************************\
			updatePositionUsingMovePrimitive
\****************************************************************/
void	CWorldEntity::updatePositionUsingMovePrimitive(uint wi)
{
	if (Primitive == NULL || Continent == INVALID_CONTINENT_INDEX || Continent == NO_CONTINENT_INDEX)
		return;

	NLPACS::UGlobalRetriever	*retriever = CWorldPositionManager::getContinentContainer().getRetriever(Continent);

	if (retriever == NULL)
		return;

	NLPACS::UGlobalPosition		gp;
	Primitive->getGlobalPosition(gp, wi);

	bool		interior = retriever->isInterior(gp);
	bool		local = localMotion();

	float		dummy;
	bool		water = retriever->isWaterPosition(gp, dummy);

	NLMISC::CVectorD	pos = Primitive->getFinalPosition(wi);

	setPosition((sint32)(pos.x*1000), (sint32)(pos.y*1000), (sint32)(pos.z*1000), local, interior, water);
}





/****************************************************************\
 ****************************************************************
							CPlayerInfos
 ****************************************************************
\****************************************************************/

// Static player allocator
CBlockMemory<CPlayerInfos>	CPlayerInfos::_PlayerAllocator;


void	CPlayerInfos::display(CLog *log) const
{
	if (this == NULL)
		log->displayNL("CPlayerInfos::display called on NULL");

	log->displayNL("--- Display PlayerInfos %s E%u ---", _PlayerId.toString().c_str(), (Entity != NULL ? Entity->Index.getIndex() : 0));

	DUMP_VAR(FeId.get());
	DUMP_VAR(LastVisionTick);
	DUMP_VAR(DelayVision);
	DUMP_VAR(ActivateSlot0);
	DUMP_VAR(DesactivateSlot0);
	DUMP_VAR(Slot0Active);
	DUMP_VAR(WhoICanSee);

#ifdef RECORD_LAST_PLAYER_POSITIONS

	log->displayNL("Last positions recorded:");
	uint	i;
	for (i=0; i<DistanceHistory.size(); ++i)
		log->displayNL("(%+9.3f,%+9.3f,%+9.3f) %d ticks", DistanceHistory[i].first.x, DistanceHistory[i].first.y, DistanceHistory[i].first.z, DistanceHistory[i].second);

#endif

	log->displayNL("--- End of Display ---");
}

