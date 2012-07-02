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

// Nel Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/matrix.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/variable.h"

//// Nel 3d
//#include "nel/3d/u_instance_group.h"

//// Nel Net
//#include "nel/net/service.h"

//// Game share
//#include "game_share/tick_event_handler.h"
//#include "game_share/mode_and_behaviour.h" //TEMP!!!
//#include "game_share/light_ig_loader.h"
//#include "game_share/synchronised_message.h"

//// Nel georges
//#include "nel/georges/u_form.h"
//#include "nel/georges/u_form_elm.h"
//#include "nel/georges/u_form_loader.h"

//Nel pacs
#include "nel/pacs/u_primitive_block.h"
#include "nel/pacs/u_global_position.h"

#include "nel/misc/hierarchical_timer.h"

// Game share
#include "server_share/combat_vision_delta.h"
#include "server_share/animal_hunger.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/utils.h"

// R2 share
#include "server_share/r2_variables.h"

// local
#include "gpm_service.h"
#include "gpm_defs.h"
#include "world_position_manager.h"
#include "sheets.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLPACS;
using namespace NLGEORGES;

//--------------------
// GLOBALS
//--------------------

const string	FrontEndVisionMessageType = string("VISIONS_DELTA_2");
const string	CombatVisionMessageType = string("VISIONS_ARROUND_ENTITIES");

bool			EVSUp = false;

sint			NbTicksInFuture = 0;
sint			NbTicksInFuture2 = 0;

CVariable<double>				SecuritySpeedFactor("gpms","SecuritySpeedFactor", "Security Margin For Player Speed", 1.0, 0, true);
CVariable<bool>					VerboseSpeedAbuse("gpms", "VerboseSpeedAbuse", "Allows GPMS to log speed abuses", false, 0, true);

CGenericXmlMsgHeaderManager		GenericXmlMsgManager;


//CEarlyPositionMap EarlyPositions;














/****************************************************************\
 ****************************************************************
					CWorldPositionManager
 ****************************************************************
\****************************************************************/

// statics members allocation

CWorldPositionManager::TWorldEntityContainer			CWorldPositionManager::_EntitiesInWorld;
CWorldPositionManager::TWorldEntityContainerByEId		CWorldPositionManager::_EntitiesInWorldByEId;
TWorldEntityList										CWorldPositionManager::_EntityList;
TWorldEntityList										CWorldPositionManager::_PrimitivedList;

//
CWorldPositionManager::TWorldCellsMap					CWorldPositionManager::_WorldCellsMap = NULL;
CWorldPositionManager::TWorldCellsMap					CWorldPositionManager::_WorldCellsEffectiveMap = NULL;
CWorldPositionManager::TIndoorCellContainer				CWorldPositionManager::_IndoorCellContainer;
uint32													CWorldPositionManager::_WorldMapX = 0,
														CWorldPositionManager::_WorldMapEffectiveX = 0,
														CWorldPositionManager::_WorldMapY = 0,
														CWorldPositionManager::_WorldMapEffectiveY = 0;
CWorldPositionManager::TCellOffsetContainer				CWorldPositionManager::_VisionCellOffsets;
CWorldPositionManager::TCellOffsetContainer				CWorldPositionManager::_ObjectVisionCellOffsets;

//
TMapFrontEndData										CWorldPositionManager::_MapFrontEndData;
TMapServiceData											CWorldPositionManager::_MapServiceData;
sint32													CWorldPositionManager::_TotalPlayers;

//
CWorldPositionManager::TRemovedEntityContainer			CWorldPositionManager::_RemovedEntities;
CObjectList<CWorldEntity>								CWorldPositionManager::_OutOfVisionEntities;
TPlayerList												CWorldPositionManager::_UpdatePlayerList;

//
CWorldPositionManager::TPacsPrimMap						CWorldPositionManager::_PacsPrimMap;
CWorldPositionManager::TPrimBlockMap					CWorldPositionManager::_PrimBlockMap;

//
std::list< CCell * >									CWorldPositionManager::_SelectedCells;	// Cells selected by one of select methode

// Asked list of entities arround an entity for mirror properties delta update
CWorldPositionManager::TEntitiesAroundEntityContainer	CWorldPositionManager::_EntitiesAround;

CContinentContainer										CWorldPositionManager::_ContinentContainer;

uint32													CWorldPositionManager::_CellSize;				// Cell size in coordinate unit

double													CWorldPositionManager::_PrimitiveMaxSize;
uint8													CWorldPositionManager::_NbWorldImages;			// Number images in world for collide management
uint8													CWorldPositionManager::_NbDynamicWorldImages;	// Number of dynamic world image (must be _NbWorldImages - _FirstDynamicWorldImage )
uint8													CWorldPositionManager::_FirstDynamicWorldImage;	// First dynamique world image;
uint8													CWorldPositionManager::_CurrentWorldImage;		// Current world image
uint16													CWorldPositionManager::_NbVisionPerTick = 200;

//
CPatatSubscribeManager									CWorldPositionManager::_PatatSubscribeManager;
float													CWorldPositionManager::_fXMin;
float													CWorldPositionManager::_fYMin;
float													CWorldPositionManager::_fXMax;
float													CWorldPositionManager::_fYMax;

CWorldPositionManager::TCellOffsetContainer				CWorldPositionManager::_VisionCellOffsetsCheck;
CWorldPositionManager::TCellOffsetContainer				CWorldPositionManager::_ObjectVisionCellOffsetsCheck;

bool													CWorldPositionManager::_LoadPacsCol = true;


//



/****************************************************************\
						init
\****************************************************************/
void CWorldPositionManager::init( uint32 nbCellX, uint32 nbCellY, uint32 visionDistance, uint32 PrimitiveMaxSize, uint8 NbWorldImages, bool loadPacsPrims, bool loadPacsCol)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	nlassert(NbWorldImages >= 2);

	nlinfo("Memory sizeofs:");
	nlinfo("sizeof(CCell)=%d", sizeof(CCell));
	nlinfo("sizeof(CWorldEntity)=%d", sizeof(CWorldEntity));
	nlinfo("sizeof(CPlayerInfos)=%d", sizeof(CPlayerInfos));

	_CellSize = CELL_SIZE;

	_LoadPacsCol = loadPacsCol;

	// init cells map
	const sint16 maxWidthInCells = (sint16) (visionDistance / _CellSize);
	const sint16 maxHeightInCells = (sint16) (visionDistance / _CellSize);

	_WorldMapX = nbCellX;
	_WorldMapY = nbCellY;
	_WorldMapEffectiveX = _WorldMapX+maxWidthInCells;
	_WorldMapEffectiveY = _WorldMapY+2*maxHeightInCells;

	_WorldCellsMap = new CCell* [_WorldMapEffectiveX*_WorldMapEffectiveY];

	nlinfo("Allocated %d effective cells ptr (%.1f Mb)", _WorldMapEffectiveX*_WorldMapEffectiveY, (float)(_WorldMapEffectiveX*_WorldMapEffectiveY*4)/(1024.0f*1024.0f));

	uint	i;
	for (i=0; i<_WorldMapEffectiveX*_WorldMapEffectiveY; ++i)
		_WorldCellsMap[i] = NULL;

	_WorldCellsEffectiveMap = _WorldCellsMap + maxHeightInCells*_WorldMapEffectiveX + maxWidthInCells/2;

	loadCellsSkimTableForVision(visionDistance);

	//
	_PrimitiveMaxSize = PrimitiveMaxSize;
	_NbWorldImages = NbWorldImages;					// Number of images
	_NbDynamicWorldImages = _NbWorldImages - 1;		// Number of dynamique world image
	_FirstDynamicWorldImage = 1;					// First dynamic world image
	_CurrentWorldImage = _FirstDynamicWorldImage;	// Current world image (corresponding to real game time)

	// init continents
	_ContinentContainer.init(10, 10, _PrimitiveMaxSize, _NbWorldImages, IService::getInstance()->WriteFilesDirectory.toString(), 32.0, loadPacsPrims);

	//
	initPacsPrim();

	// inits bounding box for patat manager
	_fXMin = +1.0e10f;
	_fXMax = -1.0e10f;
	_fYMin = +1.0e10f;
	_fYMax = -1.0e10f;

	GenericXmlMsgManager.init(CPath::lookup("msg.xml"));

} // constructor CWorldPositionManager


/****************************************************************\
			loadCellsSkimTable()
\****************************************************************/


class CSkimItem
{
public:
	CSkimItem(double distance, sint32 offset, sint32 mask) : Distance(distance), Offset(offset), Mask(mask)	{}

	double	Distance;
	sint32	Offset;
	sint32	Mask;

	bool		operator < (const CSkimItem &item) const
	{
		return Distance < item.Distance || (Distance == item.Distance && Offset < item.Offset);
	}
};

void CWorldPositionManager::loadCellsSkimTableForVision( uint32 visionDistance )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	const sint16 maxWidthInCells = (sint16) (visionDistance / _CellSize);
	const sint16 maxHeightInCells = (sint16) (visionDistance / _CellSize);

	vector<CSkimItem>	distanceVector;

	sint	i, j;

	for (i=-maxWidthInCells; i<=maxWidthInCells; ++i)
	{
		for (j=-maxHeightInCells; j<=maxHeightInCells ; ++j)
		{
			// skip the center cell (always included in vision, no need to add it the _CellsSkimTable)
			if (i == 0 && j == 0)
				continue;

			double	d = sqrt(double(i*i + j*j)) * _CellSize;

			// only add the zones for which the zone center is inside the vision circle
			if (d < visionDistance)
				distanceVector.push_back(CSkimItem(d, i+j*_WorldMapEffectiveX, 0xffffffff << (sint32)(32*d/visionDistance)));
		}
	}

	sort(distanceVector.begin(), distanceVector.end());

	uint	k;
	for (k=0; k<distanceVector.size(); ++k)
	{
		CCellOffset	co;
		co.Offset = distanceVector[k].Offset;
		co.Mask = distanceVector[k].Mask;
		co.Distance = (uint32)(distanceVector[k].Distance/1000.0);

		if (distanceVector[k].Distance < 24000)
			_ObjectVisionCellOffsets.push_back(co);
		_VisionCellOffsets.push_back(co);

		if (Verbose)
			nlinfo("VisionCell[%d] Offset=%d Distance=%.1f Mask=%8X", k, distanceVector[k].Offset, distanceVector[k].Distance, distanceVector[k].Mask);
	}

	_ObjectVisionCellOffsetsCheck = _ObjectVisionCellOffsets;
	_VisionCellOffsetsCheck = _VisionCellOffsets;

} // loadCellsSkimTable


/****************************************************************\
						getEntityIndex()
\****************************************************************/
TDataSetRow	CWorldPositionManager::getEntityIndex(const CEntityId &id)
{
	TDataSetRow result= TheDataset.getDataSetRow(id);
	STOP_IF(!result.isValid(),"Failed to get datasetrow for entity: "<<id.toString());
	return result;
}


/****************************************************************\
						setCurrentTick
\****************************************************************/
void	CWorldPositionManager::setCurrentTick( NLMISC::TGameCycle tick )
{ 
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	//_GameTick = tick; 
	_CurrentWorldImage = _FirstDynamicWorldImage ;

	// check all entities
	TWorldEntityContainer::iterator	it;
	for (it=_EntitiesInWorld.begin(); it!=_EntitiesInWorld.end(); ++it)
	{
		CWorldEntity	*entity = (*it).second;

		// reset entities in pacs
		NLMISC::CVectorD pos( entity->X()*0.001, entity->Y()*0.001, entity->Z()*0.001 );

		bool	water = false;

		if (entity->Primitive != NULL)
		{
			if (entity->getType() == CWorldEntity::Trigger)
			{
				entity->Primitive->insertInWorldImage(0);
				entity->Primitive->setGlobalPosition( pos, 0 );
				entity->Primitive->setOrientation( entity->Theta() , 0 );
				pos = entity->Primitive->getFinalPosition( 0 );
				entity->MoveContainer->evalCollision(1, 0);
			}
			else
			{
				uint tmpimage = (_CurrentWorldImage + _NbDynamicWorldImages - 1) % _NbDynamicWorldImages + 1;
				entity->Primitive->setGlobalPosition( pos, tmpimage );
				entity->Primitive->setOrientation( entity->Theta() , tmpimage );
				entity->MoveContainer->evalCollision(1, tmpimage);
				//_PatatSubscribeManager.processPacsTriggers(entity->MoveContainer);
				processPacsTriggers(entity->MoveContainer);

				pos = entity->Primitive->getFinalPosition( _CurrentWorldImage );
			}
		}

		entity->setPosition((sint32)(1000*pos.x), (sint32)(1000*pos.y), (sint32)(1000*pos.z), false, (entity->Z()&2)!=0, false);
		entity->Tick = tick;

		if (entity->PlayerInfos != NULL)
			entity->PlayerInfos->LastVisionTick = tick;
	}

	uint	i;
	for (i=0; i<_WorldMapX*_WorldMapY; ++i)
		if (_WorldCellsMap[i] != NULL)
			_WorldCellsMap[i]->_LastVisionUpdate = tick;
}


/****************************************************************\
						initPacsPrim
\****************************************************************/
void	CWorldPositionManager::initPacsPrim()
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	_ContinentContainer.initPacsPrim();
}

/****************************************************************\
						initPatatManager
\****************************************************************/
void	CWorldPositionManager::initPatatManager()
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	_PatatSubscribeManager.init();
}

/****************************************************************\
						loadPatatsInFile
\****************************************************************/
void	CWorldPositionManager::loadPatatsInFile(const string &file)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	if (Verbose)
		nldebug("Load '%s' patat file", file.c_str());

	// check extension
	if (strlwr(CFile::getExtension(file)) != "primitive")
	{
		nlwarning("Couldn't load '%s' primitive file, not a '.primitive' file", file.c_str());
		return;
	}

	try
	{
		_PatatSubscribeManager.usePrim(file);
	}
	catch (const NLMISC::EStream &e)
	{
		nlwarning("Couldn't load '%s' : %s", file.c_str(), e.what());
	}
}

/****************************************************************\
						loadPatatsInPath
\****************************************************************/
void	CWorldPositionManager::loadPatatsInPath(const string &path)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	if (Verbose)
		nldebug("Load '%s' patat path", path.c_str());

	// check if path exists
	if (!NLMISC::CFile::isExists(path))
	{
		nlwarning("Couldn't load patats in path '%s': path not found", path.c_str());
		return;
	}

	// get path content
	std::vector<std::string> fileNames;
	NLMISC::CPath::getPathContent(path, true, false, true, fileNames);

	// check if there are files in path
	if (fileNames.empty())
	{
		nlwarning("The given path '%s' has no prim patats!", path.c_str());
		return;
	}

	// load all .prim files
	for(uint k = 0; k < fileNames.size(); ++k)
	{
		// check extension
		if (strlwr(CFile::getExtension(fileNames[k])) != "prim")
			continue;

		loadPatatsInFile(fileNames[k]);
	}
}

/****************************************************************\
						loadPatatManagerFile
\****************************************************************/
void	CWorldPositionManager::loadPatatManagerFile(const string &file)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	try
	{
		string	filepath = CPath::lookup(file);
		CIFile	f(filepath);
		nlinfo("Load PatatManager file '%s' (including path)", filepath.c_str());
		f.serial(_PatatSubscribeManager);
		_PatatSubscribeManager.displayPatatGridInfo();
	}
	catch (const Exception &e)
	{
		nlwarning("Couldn't load manager file '%s': %s", file.c_str(), e.what());
	}
}

/****************************************************************\
						savePatatManagerFile
\****************************************************************/
void	CWorldPositionManager::savePatatManagerFile(const string &file)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	try
	{
		string	filepath = IService::getInstance()->WriteFilesDirectory.toString()+CFile::getFilename(file);
		nlinfo("PatatManager file saved as '%s' (including path)", filepath.c_str());
		COFile	f(filepath);
		f.serial(_PatatSubscribeManager);
		_PatatSubscribeManager.displayPatatGridInfo();
	}
	catch (const Exception &e)
	{
		nlwarning("Couldn't save manager file '%s': %s", file.c_str(), e.what());
	}
}

/****************************************************************\
						triggerSubscribe
\****************************************************************/
void	CWorldPositionManager::triggerSubscribe(NLNET::TServiceId serviceId, const string &name, uint16 id)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	if (_PatatSubscribeManager.exist(name))
	{
		_PatatSubscribeManager.subscribe(serviceId, make_pair<string,uint16>(name, id));
	}
}

/****************************************************************\
						triggerUnsubscribe
\****************************************************************/
void	CWorldPositionManager::triggerUnsubscribe(NLNET::TServiceId serviceId, uint16 id)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	_PatatSubscribeManager.unsubscribe(serviceId, id);
}

/****************************************************************\
						triggerUnsubscribe
\****************************************************************/
void	CWorldPositionManager::triggerUnsubscribe(NLNET::TServiceId serviceId)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	_PatatSubscribeManager.unsubscribe(serviceId);
}







/****************************************************************\
						loadContinent
\****************************************************************/
void	CWorldPositionManager::loadContinent(const string &name, const string &file, sint index, bool allowAutoSpawn)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	if (_LoadPacsCol)
	{
		_ContinentContainer.loadContinent(name, file, index, allowAutoSpawn);
	}
}

/****************************************************************\
						removeContinent
\****************************************************************/
void	CWorldPositionManager::removeContinent(sint index)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	_ContinentContainer.removeContinent(index);
}




/****************************************************************\
						processPacsTriggers
\****************************************************************/
void	CWorldPositionManager::processPacsTriggers(UMoveContainer *moveContainer)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	uint	num = moveContainer->getNumTriggerInfo();
	uint	i;

	for (i=0; i<num; ++i)
	{
		const UTriggerInfo	&info = moveContainer->getTriggerInfo(i);

		uint64			trigger = info.Object0;
		uint64			entity = info.Object1;

		//nldebug("In processPacsTriggers: %"NL_I64"d versus %"NL_I64"d", entity, trigger);

		if ((entity&0xffff) != 0)
			swap(trigger, entity);

		if ((entity&0xffff) != 0 || (trigger&0xffff) != 1)
			continue;

		sint32			triggerId = (sint32)((trigger & 0xffff0000) >> 16);
		TDataSetRow		entityIndex = TheDataset.getCurrentDataSetRow((TDataSetIndex)(entity >> 16));

		switch (info.CollisionType)
		{
		case UTriggerInfo::In:
			{
				//nldebug("PTRIG_IN: E%u enters trigger %d", entityIndex.getIndex(), triggerId);
				CMessage	msgout("PTRIG_IN");
				msgout.serial(triggerId, entityIndex);
				CVectorD	pos = _ContinentContainer.getTriggerPosition(triggerId);
				msgout.serial(pos);
				sendMessageViaMirror("EGS", msgout);
				nlinfo("TRIG: Entity %d entered trigger %d", entityIndex.getIndex(), triggerId);
			}
			break;
		case UTriggerInfo::Out:
			{
				//nldebug("PTRIG_OUT: E%u leaves trigger %d", entityIndex.getIndex(), triggerId);
				CMessage	msgout("PTRIG_OUT");
				msgout.serial(triggerId, entityIndex);
				sendMessageViaMirror("EGS", msgout);
				nlinfo("TRIG: Entity %d left trigger %d", entityIndex.getIndex(), triggerId);
			}
			break;
		case UTriggerInfo::Inside:
			break;
		}
	}
}









/****************************************************************\
						update
\****************************************************************/
void CWorldPositionManager::update()
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	uint	i;

	// inc tick and set new Current World Image to ( _CurrentWorldImage % _NbDynamicWorldImages ) + 1
	uint	currentWorldImage = ( _CurrentWorldImage % _NbDynamicWorldImages ) + 1;

	{
		H_AUTO(PACSEvalCollision);
		for (i=0; (sint)i<_ContinentContainer.size(); ++i)
		{
			UMoveContainer	*moveContainer = _ContinentContainer.getMoveContainer(i);

			if (moveContainer == NULL)
				continue;

			// duplicate current world image into the next one
			{
				H_AUTO(DupWorldImage);
				moveContainer->duplicateWorldImage( _CurrentWorldImage, ( _CurrentWorldImage % _NbDynamicWorldImages ) + 1);
			}
			
			{
				// eval static collision
				H_AUTO(Eval0);
				moveContainer->evalCollision(1, 0);
			}

			{
				H_AUTO(ManageTriggers0);
				//managePACSTriggers(moveContainer);
				//_PatatSubscribeManager.processPacsTriggers(moveContainer);
				processPacsTriggers(moveContainer);
			}

			{
				// eval dynamic collision
				H_AUTO(EvalCurrentWI);
				moveContainer->evalCollision(1, currentWorldImage);
			}

			{
				H_AUTO(ManageTriggersWI);
				//_PatatSubscribeManager.processPacsTriggers(moveContainer);
				processPacsTriggers(moveContainer);
			}
		}
		//++_GameTick; 
	}

	// inc tick and set new Current World Image to ( _CurrentWorldImage % _NbDynamicWorldImages ) + 1
	_CurrentWorldImage = currentWorldImage;

	{
		// get the final position of every entity and update the position in the World Position Manager
		H_AUTO(PACSGetFinalPosition);
		CVector finalPos;

		for (TWorldEntityList::iterator ite = _PrimitivedList.begin(); ite != _PrimitivedList.end(); ++ite)
		{
			CWorldEntity	*entity = *ite;

			// don't use pacs move for agent and static objects
			if (!entity->hasPrimitive() || !entity->CheckMotion)
				continue;

			/// \todo remove ben
			// check somewhere position is initialised
			if ((entity->getType() == CWorldEntity::Player) && (!entity->PosInitialised && (entity->X() != 0 || entity->Y() != 0)))
				nlwarning("E%u wasn't warned of its position but position seems ready yet!", entity->Index.getIndex());

			entity->updatePositionUsingMovePrimitive(_CurrentWorldImage);
		}

/*
		for (TWorldEntityList::iterator ite = _EntityList.begin(); ite != _EntityList.end(); ++ite)
		{
			CWorldEntity	*entity = *ite;

			// don't use pacs move for agent and static objects
			if (!entity->hasPrimitive() || !entity->CheckMotion)
				continue;

			/// \todo remove ben
			// check somewhere position is initialised
			if ((entity->getType() == CWorldEntity::Player) && (!entity->PosInitialised && (entity->X() != 0 || entity->Y() != 0)))
				nlwarning("E%u wasn't warned of its position but position seems ready yet!", entity->Index.getIndex());

			entity->updatePositionUsingMovePrimitive(_CurrentWorldImage);
		}
*/

/*
		for( TWorldEntityContainer::iterator ite = _EntitiesInWorld.begin(); ite != _EntitiesInWorld.end(); ++ite )
		{
			CWorldEntity	*entity = (*ite).second;

			/// \todo remove ben
			// check somewhere position is initialised
			if ((entity->getType() == CWorldEntity::Player) && (!entity->PosInitialised && (entity->X() != 0 || entity->Y() != 0)))
				nlwarning("E%u wasn't warned of its position but position seems ready yet!", entity->Index.getIndex());

			// don't use pacs move for agent and static objects
			if (!entity->hasPrimitive() || !entity->CheckMotion)
				continue;

			entity->updatePositionUsingMovePrimitive(_CurrentWorldImage);
		}
*/
	}

	H_TIME(ComputeVision, computeVision(););

	H_TIME(UpdateEntitiesAround, updateEntitiesAround(););

	H_TIME(UpdatePatatSubscribeManager, _PatatSubscribeManager.emitChanges(););

	// actually delete entites that have been removed from the manager
	// only removes entities that are no more in vision of any other entity
	TRemovedEntityContainer::iterator	itRemove;
	for (itRemove=_RemovedEntities.begin(); itRemove!=_RemovedEntities.end(); )
	{
		if ((*itRemove)->RefCounter == 0)
		{
			CWorldEntity::remove(*itRemove);
			itRemove = _RemovedEntities.erase(itRemove);
		}
		else
		{
			++itRemove;
		}
	}

	// TEMP CHECK
	/*
	for (i=0; i<_ObjectVisionCellOffsets.size(); ++i)
		nlassert(_ObjectVisionCellOffsets[i] == _ObjectVisionCellOffsetsCheck[i]);

	for (i=0; i<_VisionCellOffsets.size(); ++i)
		nlassert(_VisionCellOffsets[i] == _VisionCellOffsetsCheck[i]);
	*/

	if ( NbTicksInFuture != 0 )
		nlwarning( "%u: %d positions in the future (small diff)", CTickEventHandler::getGameCycle(), NbTicksInFuture );
	NbTicksInFuture = 0;
	if ( NbTicksInFuture2 != 0 )
		nlwarning( "%u: %d positions in the future (BIG DIFF)", CTickEventHandler::getGameCycle(), NbTicksInFuture2 );
	NbTicksInFuture2 = 0;

}// update




/****************************************************************\
						destructor
\****************************************************************/
void CWorldPositionManager::release( )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	// ANTIBUG: avoids gpms to assert on release (in CBlockMemory::purge())
	NL3D_BlockMemoryAssertOnPurge = false;

	// free entities
	TWorldEntityContainer::iterator		ite;
	while (!_EntitiesInWorld.empty())
	{
		CWorldEntity *entity = (*_EntitiesInWorld.begin()).second;
		onRemoveEntity( entity->Index );
		// TODO: remove the entities created by hand (using the command addEntity) in the mirror
	}

	//
	TRemovedEntityContainer::iterator	itRemove;
	for (itRemove=_RemovedEntities.begin(); itRemove!=_RemovedEntities.end(); )
	{
		if ((*itRemove)->RefCounter == 0)
		{
			CWorldEntity::remove(*itRemove);
			itRemove = _RemovedEntities.erase(itRemove);
		}
		else
		{
			++itRemove;
		}
	}
	//

	// check there are no more entities to delete
	TRemovedEntityContainer::iterator	itr;
	for (itr = _RemovedEntities.begin(); itr != _RemovedEntities.end(); ++itr)
	{
		nlwarning("Entity %s has not been removed (RefCounter=%d)", (*itr)->Id.toString().c_str(), (*itr)->RefCounter);
		CWorldEntity::remove(*itr);
	}

	// Release cells map
	uint	i;
	for (i=0; i<_WorldMapEffectiveX*_WorldMapEffectiveY; ++i)
		if (_WorldCellsMap[i] != NULL)
			CCell::remove(_WorldCellsMap[i]);

	delete [] _WorldCellsMap;

	for (i=0; i<_IndoorCellContainer.size(); ++i)
		if (_IndoorCellContainer[i] != NULL)
			CCell::remove(_IndoorCellContainer[i]);

	_IndoorCellContainer.clear();

	_WorldCellsMap = NULL;
	_WorldCellsEffectiveMap = NULL;

	for (i=0; (sint)i<_ContinentContainer.size(); ++i)
		removeContinent(i);

	_ContinentContainer.clear();
} // destructor


/****************************************************************\
					createIndoorCell()
\****************************************************************/
void	CWorldPositionManager::createIndoorCell(sint32 cellId)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	nlassert(cellId > 0);

	if (cellId >= (sint32)_IndoorCellContainer.size())
		_IndoorCellContainer.resize(cellId+1, NULL);

	_IndoorCellContainer[cellId] = CCell::create();
	_IndoorCellContainer[cellId]->init(-cellId);
}


/****************************************************************\
					createBuildingInstance()
\****************************************************************/
void	CWorldPositionManager::createBuildingInstance(uint8 continent, const string &id, const CVectorD &position)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TTime	start = CTime::getLocalTime();
	if (continent >= _ContinentContainer.size() ||
		_ContinentContainer.getRetriever(continent) == NULL ||
		_ContinentContainer.getRetrieverBank(continent) == NULL)
	{
		nlwarning("createBuildingInstance(): continent %d is not used", continent);
		return;
	}

	sint32	instance = -1;

	if (!_ContinentContainer.getRetriever(continent)->buildInstance(id, position, instance))
	{
		nlwarning("createBuildingInstance(): failed to instanciate building '%s' at position (%.3f,%.3f,%.3f)", id.c_str(), position.x, position.y, position.z);
		return;
	}

	TTime	end = CTime::getLocalTime();
	if (Verbose)
		nlinfo("createBuildingInstance(): instanciated building '%s' successfully: %d ms", id.c_str(), (uint32)(end-start));

	return;
}


/****************************************************************\
					instanciatePacsPrim()
\****************************************************************/
void	CWorldPositionManager::instanciatePacsPrim(const string &id, const string &file, const CVectorD &pos, float angle)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TPacsPrimMap::iterator	it = _PacsPrimMap.find(CFile::getFilenameWithoutExtension(file));
	if (it == _PacsPrimMap.end())
	{
		nlwarning("Can't instanciate prim '%s', because file '%s.pacs_prim' not found", id.c_str(), file.c_str());
		return;
	}

	UPrimitiveBlock	*block = (*it).second;

	TPrimBlockMap::iterator	itb = _PrimBlockMap.find(id);
	if (itb != _PrimBlockMap.end())
	{
		nlwarning("Can't instanciate prim '%s', already instanciated", id.c_str());
		return;
	}

	uint8	continent = _ContinentContainer.findContinent(pos);
	if (continent == INVALID_CONTINENT_INDEX)
	{
		nlwarning("Can't instanciate prim '%s', invalid position (%.3f, %.3f, %.3f)", id.c_str(), pos.x, pos.y, pos.z);
		return;
	}

	if (Verbose)
		nldebug("Instanciating obstacle '%s'", id.c_str());

	pair<TPrimBlockMap::iterator, bool>	res = _PrimBlockMap.insert(TPrimBlockMap::value_type(id, CPrimBlock()));
	itb = res.first;

	UMoveContainer	*mc = _ContinentContainer.getMoveContainer(continent);
	(*itb).second.MoveContainer = mc;
	mc->addCollisionnablePrimitiveBlock((*it).second, 0, 1, &((*itb).second.MovePrimitives), angle, pos, true);

	if (Verbose)
		nldebug("'%s': %d primitives", id.c_str(), (*itb).second.MovePrimitives.size());
}

/****************************************************************\
					removePacsPrim()
\****************************************************************/
void	CWorldPositionManager::removePacsPrim(const string &id)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TPrimBlockMap::iterator	itb = _PrimBlockMap.find(id);
	if (itb == _PrimBlockMap.end())
	{
		nlwarning("Can't remove prim '%s', not instanciated instanciated", id.c_str());
		return;
	}

	if (Verbose)
		nldebug("Removing obstacle '%s', %d primitives", id.c_str(), (*itb).second.MovePrimitives.size());

	uint	i;
	for (i=0; i<(*itb).second.MovePrimitives.size(); ++i)
		(*itb).second.MoveContainer->removePrimitive((*itb).second.MovePrimitives[i]);

	_PrimBlockMap.erase(itb);
}

/****************************************************************\
					setObstacle()
\****************************************************************/
void	CWorldPositionManager::setObstacle(const string &id, bool obstacle)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TPrimBlockMap::iterator	itb = _PrimBlockMap.find(id);
	if (itb == _PrimBlockMap.end())
	{
		nlwarning("Can't set obstacle '%s' state, not instanciated yet", id.c_str());
		return;
	}

	if (Verbose)
		nldebug("Setting obstacle '%s' as %s", id.c_str(), obstacle ? "true" : "false");

	uint	i;
	for (i=0; i<(*itb).second.MovePrimitives.size(); ++i)
		(*itb).second.MovePrimitives[i]->setObstacle(obstacle);
}







/****************************************************************\
					link()
\****************************************************************/
void	CWorldPositionManager::link(CWorldEntity* entity)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	// check entity is valid
	nlassert(entity != NULL);

	// if cell id was set to a negative value, was an indoor cell, so link there
	link(entity, entity->Cell);
}

/****************************************************************\
					link()
\****************************************************************/
void	CWorldPositionManager::link(CWorldEntity* entity, sint32 cell)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	// check cell and entity are valid
	//nlassert(cell >= 0);	// can be anything now !!
	nlassert(entity != NULL);

	// don't link triggers
	if (entity->getType() == CWorldEntity::Trigger)
		return;

	CCell	*pcell = NULL;

	// if cell is 0, find cell from entity coordinates
	if (cell >= 0)
	{
		//sint32	vx = entity->X();
		//sint32	vy = entity->Y();
		
		uint16	cx = (uint16) ( + entity->X()/_CellSize );
		uint16	cy = (uint16) ( - entity->Y()/_CellSize );

		if (checkCellBounds(cx, cy))
		{
			pcell = getCell(cx, cy);

			if (pcell == NULL)
			{
				pcell = CCell::create();
				if (pcell != NULL)
					pcell->init(cx, cy);

				setCell(cx, cy, pcell);
			}
		}
	}
	// if cell is explicited, find indoor cell
	else if (cell < -1 && -cell < (sint32)_IndoorCellContainer.size())
	{
		pcell = _IndoorCellContainer[-cell];
	}

	link(entity, pcell);
}

/****************************************************************\
					link()
\****************************************************************/
void	CWorldPositionManager::link(CWorldEntity* entity, CCell *cell)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	// check cell and entity are valid
	nlassert(entity != NULL);

	if (cell != entity->getCell())
	{
		if (entity->isLinked())
			entity->CellPtr->remove(entity);

		if (cell != NULL)
			cell->add(entity);
		else
			entity->Cell = 0;
	}
}


/****************************************************************\
						onAddEntity()
\****************************************************************/
void CWorldPositionManager::onAddEntity(const TDataSetRow &entityIndex )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	const CEntityId& id = TheDataset.getEntityId( entityIndex );

	if (getEntity(entityIndex) != NULL)
	{
		nlwarning( "CWorldPositionManager::onAddEntity E%u/%s already in GPMS", entityIndex.getIndex(), id.toString().c_str() );
		//setEntityPosition( id, x, y, z, theta, tick );
		return;
	}

	const uint8 image = _CurrentWorldImage;

	// Create entity
	CWorldEntity *pWorldEntity = CWorldEntity::create();

	// check succeded
	if( !pWorldEntity )
	{
		nlwarning("CWorldEntity::createEntity() for E%u/%s failed", entityIndex.getIndex(), id.toString().c_str());
		return;
	}

	TServiceId frontendId(id.getDynamicId());

	// init entity
	pWorldEntity->init( id, entityIndex );

	if (pWorldEntity->getType() == CWorldEntity::Player)
	{
		// init player info
		CPlayerInfos	*infos = CPlayerInfos::create();

		// check succeded
		if (infos == NULL)
		{
			nlwarning("Couldn't create player infos for player %s, whole entity is removed", id.toString().c_str());
			CWorldEntity::remove(pWorldEntity);
			return;
		}

		infos->init(id, frontendId, pWorldEntity);

		TMapFrontEndData::iterator	itfe = _MapFrontEndData.find(frontendId);
		// if no list yet, create a node
		if (itfe == _MapFrontEndData.end())
		{
			pair<TMapFrontEndData::iterator, bool>	result = _MapFrontEndData.insert(make_pair( frontendId, CFrontEndData() ));
			if (!result.second)
			{
				nlwarning("Unable to create frontend data for entity %s (associated to service %d)", pWorldEntity->Id.toString().c_str(), frontendId.get());
				nlstop;
				return;
			}
			itfe = result.first;
		}

		++((*itfe).second.NumPlayers);
		updateMaxVisions();

		pWorldEntity->UsePrimitive = true;
		pWorldEntity->HasVision = true;		// indicate this entity has vision to be updated, used for cell vision building
		pWorldEntity->PlayerInfos = infos;

		//_Players[id] = infos;

		infos->ItFrontEnd = itfe;
		infos->ItUpdatePlayer = _UpdatePlayerList.insert(_UpdatePlayerList.end(), infos);	// add player to the end of the update player list
		infos->LastVisionTick = CTickEventHandler::getGameCycle();
		infos->DelayVision = CTickEventHandler::getGameCycle()+1;

		// always self in slot 0
		infos->Slots[0] = pWorldEntity;

#ifndef HANDLE_SLOT0_SPECIAL
		activateSelfSlot( pWorldEntity );
#endif

		++NumPlayers;
	}
/*
	if (id.getType() == RYZOMID::multiTarget)
	{
		// do something here
		CMirrorPropValueRO<TYPE_RIDER_ENTITY_ID>	rider(TheDataset, entityIndex, DSPropertyENTITY_MOUNTED_ID);

		TDataSetRow		riderIndex = TDataSetRow::createFromRawIndex(rider());
		if (riderIndex.isValid())
		{
			attach(riderIndex, entityIndex, 0, 0, 0);
		}
	}
*/
	++NumEntities;

	// insert in world
	_EntitiesInWorldByEId[id] = pWorldEntity;
	_EntitiesInWorld[entityIndex] = pWorldEntity;

	/// \todo: fix for egs that doesn't set Cell at entity creation
	// Creatures have to spawn outside of the indoor continent
	//if ( pWorldEntity->Cell >= 0 ) 
	//	pWorldEntity->Cell = 0;

	// After init of the world entity's mirror values, Cell can't be stricly positive
	// 0 means we don't know which outdoor cell
	nlassert( pWorldEntity->Cell <= 0 );

	// if pos was initialised before entity appeared, reset its position
	if (pWorldEntity->X.getTimestamp() != 0 && pWorldEntity->Y.getTimestamp() != 0)
	{
		pWorldEntity->PosInitialised = true;
		resetPrimitive(pWorldEntity);
	}

	// force entity to spawn
	//resetPrimitive(pWorldEntity);

} // onAddEntity



/****************************************************************\
						removeEntity(CEntityId)
\****************************************************************/
void CWorldPositionManager::cmdRemoveEntity( const TDataSetRow &index )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TheMirror.removeEntity( TheDataset.getEntityId(index) );
} // removeEntity


/****************************************************************\
						removeEntity(iterator)
\****************************************************************/
void CWorldPositionManager::onRemoveEntity( const TDataSetRow &index )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	CWorldEntity *entity = getEntityPtr(index);
	if (entity == NULL)
	{
		nlwarning("removeEntity(): E%u_%hu not found", index.getIndex(), index.counter());
		return;
	}


	// if entity has control, remove control
	if (entity->Parent != NULL && (CWorldEntity*)(entity->Parent->Control) == entity)
	{
		leaveControl(entity->Index);
	}

	// if entity is controlled, remove control
	if ((CWorldEntity*)(entity->Control) != NULL)
	{
		leaveControl(entity->Control->Index);
	}

	// if entity is only attached, detach
	if (entity->Parent != NULL)
	{
		detach(entity->Index);
	}

	// if entity has children attached, detach them
	while (!entity->Children.empty())
	{
		detach(entity->Children[0]->Index);
	}

	// removes visions so vision reference counters are decremented the right way
	removePlayerVision(entity);
	removeCombatVision(entity);

	// Unrequest local mirrors subscribes arround entity
	unrequestAllForEntityAround( entity->Id );

	// remove the entity from it's Cell
	unlink(entity);

	// delete the primitive
	entity->removePrimitive();

	// if the entity is a player, plan it for deletion
	if (entity->HasVision)
	{
		CPlayerInfos	*infos = entity->PlayerInfos;
		if (infos == NULL)
		{
			if (Verbose)
				nlwarning("Player %s has no player info", entity->Id.toString().c_str());
		}
		else
		{
			// remove player from the update list
			_UpdatePlayerList.erase(infos->ItUpdatePlayer);

			// decrease number of player for this fe
			TMapFrontEndData::iterator	itfe = infos->ItFrontEnd;
			if (itfe != _MapFrontEndData.end())
				--((*itfe).second.NumPlayers);

			// removes player from players map
			//_Players.erase(entity->Id);
			CPlayerInfos::remove(infos);
			entity->PlayerInfos = NULL;
		}

		updateMaxVisions();
		--NumPlayers;
	}

	//
	entity->Parent = NULL;
	entity->Children.clear();
	entity->Content.clear();

	// remove entity from all patats
	_PatatSubscribeManager.setNewEntryIndex(entity->Id, 0, entity->PatatEntryIndex);

	--NumEntities;

	// delete the CWorldEntity object -> move it to a temp container
	_RemovedEntities.push_back(entity);

	_EntitiesInWorld.erase(entity->Index);
	_EntitiesInWorldByEId.erase(entity->Id);

} // removeEntity


/****************************************************************\
						removeAllEntities()
\****************************************************************/
void CWorldPositionManager::removeAllEntities()
{	
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TWorldEntityContainer::iterator it;
	list<TDataSetRow>						delItList;

	for (it = _EntitiesInWorld.begin() ; it != _EntitiesInWorld.end() ; ++it)
		delItList.push_back( (*it).first );

	list<TDataSetRow>::iterator		itDel;
	for ( itDel = delItList.begin() ; itDel != delItList.end() ; ++itDel)
		onRemoveEntity( *itDel );

} // removeAllEntities



/****************************************************************\
						processWaitingEntities()
\****************************************************************/
void CWorldPositionManager::processWaitingEntities()
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
} // processWaitingEntities




/****************************************************************\
						setPlayerVisionProcessing()
\****************************************************************/
void	CWorldPositionManager::setPlayerVisionProcessing(const TDataSetRow &index, bool enabled)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	CWorldEntity	*entity = getEntityPtr(index);
	if (entity == NULL || entity->PlayerInfos == NULL)
		return;

	entity->PlayerInfos->EnableVisionProcessing = enabled;
}


/****************************************************************\
						updateMaxVisions()
\****************************************************************/
void	CWorldPositionManager::updateMaxVisions()
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TMapFrontEndData::iterator	it;
	sint	totalPlayers = 0;

	// compute total players
	for (it=_MapFrontEndData.begin(); it!=_MapFrontEndData.end(); ++it)
		totalPlayers += (*it).second.NumPlayers;

	_TotalPlayers = totalPlayers;

	//if more players than total vision
	if (totalPlayers > _NbVisionPerTick)
	{
		// max per tick is the ratio of players on this fe
		for (it=_MapFrontEndData.begin(); it!=_MapFrontEndData.end(); ++it)
			(*it).second.MaxVisionsPerTick = (_NbVisionPerTick*(*it).second.NumPlayers + totalPlayers-1) / totalPlayers;
	}
	else
	{
		// max per tick is straight number of player...
		for (it=_MapFrontEndData.begin(); it!=_MapFrontEndData.end(); ++it)
			(*it).second.MaxVisionsPerTick = (*it).second.NumPlayers;
	}
}


/****************************************************************\
						computeVision()
\****************************************************************/
void	CWorldPositionManager::computeVision()
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	//static pair<CWorldEntity*, sint32>				cellVisionArray[MAX_SEEN_ENTITIES+1];	// sint32 used for masking
	static CVisionEntry				cellVisionArray[MAX_SEEN_ENTITIES+1];	// sint32 used for masking
	uint							numEntities;
	TMapFrontEndData::iterator		itFE;
	
	{
		// clear all message towards frontends (to reuse messages)
		H_AUTO(MessageClearUp);
		for (itFE=_MapFrontEndData.begin(); itFE!=_MapFrontEndData.end(); ++itFE)
		{
			(*itFE).second.CurrentVisionsAtTick = 0;
			CMessage	&msg = (*itFE).second.Message;
			msg.clear();
			/*if (msg.isReading())
				msg.invert();*/
			msg.setType(FrontEndVisionMessageType);
			(*itFE).second.MessageHeaderSize = msg.getPos();

			(*itFE).second.VisionIn = 0;
			(*itFE).second.VisionOut = 0;
		}
	}

	//
	// some checks (famine de vision)
	//
	{
		static	sint	maxdt = 0;
		TPlayerList::iterator	itpl;
		for (itpl=_UpdatePlayerList.begin(); itpl!=_UpdatePlayerList.end(); )
		{
			CPlayerInfos	*pl = (*itpl);

			if (pl->Entity == NULL || pl->Entity->CellPtr == NULL)
			{
				++itpl;
				continue;
			}
			
			sint	dt = CTickEventHandler::getGameCycle() - (*itpl)->LastVisionTick;
			if (dt > maxdt)
			{
				if (Verbose)
					nlwarning("New max cycle %d for player %s fe=%d", dt, pl->Entity->Id.toString().c_str(), pl->FeId.get());
				maxdt = dt;
			}

			// above 20 ticks since last vision update
			// vision state is considered as abnormal
			if (dt > 20)
			{
				CFrontEndData	&fedata = (*(pl->ItFrontEnd)).second;
				if (Verbose)
					nlwarning("Player %s vision prioritized (dt=%d, feId=%d, %d players on fe, CurrentVisionAtTick=%d, MaxVisionPerTick=%d", pl->Entity->Id.toString().c_str(), dt, pl->FeId.get(), fedata.NumPlayers, fedata.CurrentVisionsAtTick, fedata.MaxVisionsPerTick);

				itpl = prioritizeVisionState(itpl);
			}
			else
			{
				++itpl;
			}
		}
	}
	//
	//
	//


	//
	TPlayerList::iterator	itpl = _UpdatePlayerList.begin();
	sint					numVision = 0;
	sint					maxVision = std::min((sint)_TotalPlayers, (sint)_NbVisionPerTick);

	while (numVision < maxVision && itpl != _UpdatePlayerList.end())
	{
		CPlayerInfos	*player = *itpl;
		nlassert(player->Entity != NULL);

		CCell			*cell = player->Entity->CellPtr;

		if (cell == NULL)
		{
			++itpl;
			continue;
		}

		if (cell->visionUpdateCycle() >= CTickEventHandler::getGameCycle())
			break;

		cell->setVisionUpdateCycle(CTickEventHandler::getGameCycle());
		H_TIME(ComputeCellVision, computeCellVision(cell, cellVisionArray, numEntities););

		CPlayerInfos	*plv;
		for (plv=cell->getPlayersList(); plv!=NULL; plv=plv->Next)
		{
			CFrontEndData	&fe = (*(plv->ItFrontEnd)).second;
			// if player's fe reached its maximum for this tick, get to next entity
			if (fe.CurrentVisionsAtTick >= fe.MaxVisionsPerTick || plv->DelayVision > CTickEventHandler::getGameCycle())
				continue;

			// else set cell vision to the player
			H_TIME(SetVisionToPlayer, setCellVisionToEntity(plv->Entity, cellVisionArray, numEntities););
			++numVision;
			plv->LastVisionTick = CTickEventHandler::getGameCycle();

			//
			++(fe.CurrentVisionsAtTick);

			// put player at the end of the update list
			if (plv == *itpl)
				itpl = updateVisionState(itpl);
			else
				updateVisionState(plv->ItUpdatePlayer);
		}
	}

	// treat vision for players that are no more in a cell
	// all visions are treated in one tick (assuming there aren't many players in this case and it is quite fast)
	while (_OutOfVisionEntities.getHead() != NULL)
	{
		setCellVisionToEntity(_OutOfVisionEntities.getHead(), cellVisionArray, 0);
		_OutOfVisionEntities.remove(_OutOfVisionEntities.getHead());
	}

	{
		H_AUTO(SendVisionDelta);
		// build and send all the messages for the different front Ends
		for (itFE=_MapFrontEndData.begin(); itFE!=_MapFrontEndData.end(); ++itFE)
		{
			CMessage	&msg = (*itFE).second.Message;
			if ((*itFE).second.MessageHeaderSize == msg.getPos())
				continue;
			
			sendMessageViaMirror( (*itFE).first, msg );
			msg.clear();
		}
	}
}

/****************************************************************\
						setContent()
\****************************************************************/
void	CWorldPositionManager::setContent(const TDataSetRow &index, const vector<CEntitySheetId> &content)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	CWorldEntity	*entity = getEntityPtr(index);

	if (entity == NULL)
		return;

	entity->Content = content;
}

/****************************************************************\
						activateSelfSlot()
\****************************************************************/
void	CWorldPositionManager::activateSelfSlot(CWorldEntity *entity)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	if (entity == NULL || entity->PlayerInfos == NULL)
	{
		if (Verbose)
			nlwarning("Unable to activate self slot for player: can't find player");
		return;
	}

	if (entity->PlayerInfos->DesactivateSlot0)
	{
		entity->PlayerInfos->DesactivateSlot0 = false;
		return;
	}

	entity->PlayerInfos->ActivateSlot0 = true;
}

/****************************************************************\
						desactivateSelfSlot()
\****************************************************************/
void	CWorldPositionManager::desactivateSelfSlot(CWorldEntity *entity)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	if (entity == NULL || entity->PlayerInfos == NULL)
	{
		if (Verbose)
			nlwarning("Unable to desactivate self slot for player: can't find player");
		return;
	}

	if (entity->PlayerInfos->ActivateSlot0)
	{
		entity->PlayerInfos->ActivateSlot0 = false;
		return;
	}

	entity->PlayerInfos->DesactivateSlot0 = true;
}

/****************************************************************\
						updateVisionState()
\****************************************************************/
TPlayerList::iterator	CWorldPositionManager::updateVisionState(TPlayerList::iterator itpl)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	// get player info block
	CPlayerInfos			*infos = *itpl;

	// remove iterator from the update list
	TPlayerList::iterator	next = _UpdatePlayerList.erase(itpl);

	// and push it to the back of the list
	infos->ItUpdatePlayer = _UpdatePlayerList.insert(_UpdatePlayerList.end(), infos);

	// returns next player iterator
	return next;
}

/****************************************************************\
						prioritizeVisionState()
\****************************************************************/
TPlayerList::iterator	CWorldPositionManager::prioritizeVisionState(TPlayerList::iterator itpl)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	// get player info block
	CPlayerInfos			*infos = *itpl;
	CWorldEntity			*entity = infos->Entity;

	nlassert(entity != NULL);
	CCell					*cell = entity->CellPtr;

	// remove iterator from the update list
	TPlayerList::iterator	next = _UpdatePlayerList.erase(itpl);

	// and push it to the front of the list
	infos->ItUpdatePlayer = _UpdatePlayerList.insert(_UpdatePlayerList.begin(), infos);

	// move player to the head of player list in cell so it is updated as soon as possible
	if (cell != NULL)
	{
		cell->_PlayersList.remove(infos);
		cell->_PlayersList.insertAtHead(infos);
	}

	// returns next player iterator
	return next;
}

/****************************************************************\
						computeCellVision()
\****************************************************************/
void	CWorldPositionManager::computeCellVision( CCell *cell, CVisionEntry* entitiesSeenFromCell, uint &numEntities)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	CVisionEntry*	fillPtr;				// entities pointer fill buffer
	CVisionEntry*	endPtr;					// the end of the buffer
	//pair<CWorldEntity*,sint32> *fillPtr;				// entities pointer fill buffer
	//pair<CWorldEntity*,sint32> *endPtr;					// the end of the buffer

	// get the coords of the player's Cell
	CCell			*pCell = NULL;
	uint32			startOffset = 0;
	CCell			**centerCell = NULL;

	if (cell->id() >= 0)
	{
		startOffset = getCellOffset(cell->x(), cell->y());
		centerCell = _WorldCellsEffectiveMap+startOffset;
	}

	uint32			centerCellMask = 0xffffffff;

	// First adds objects
	fillPtr = entitiesSeenFromCell;
	endPtr = entitiesSeenFromCell+MAX_SEEN_OBJECTS;
	fillPtr = cell->addObjects(fillPtr, endPtr, centerCellMask, 0);

	// if the cell has vision on other cells
	if (!cell->isIndoor())
	{

		uint					numOffsets = (uint)_ObjectVisionCellOffsets.size();
		CCellOffset*			offsetPtr = &(_ObjectVisionCellOffsets[0]);					// warning!! _VisionCellOffsets must be filled
		CCellOffset*			offsetEnd = &(_ObjectVisionCellOffsets[numOffsets-1]);			// warning!! _VisionCellOffsets must be filled
		//pair<sint32, uint32>	*offsetPtr = &(_ObjectVisionCellOffsets[0]);					// warning!! _VisionCellOffsets must be filled
		//pair<sint32, uint32>	*offsetEnd = &(_ObjectVisionCellOffsets[numOffsets-1]);			// warning!! _VisionCellOffsets must be filled

		do
		{
			pCell = centerCell[ (*offsetPtr).Offset ];
			if (!pCell)
			{
				++offsetPtr;
				continue;
			}

			CWorldEntity	*ent = pCell->getObjectsList();
			while (ent != NULL && fillPtr < endPtr)
			{
				sint32	mask = (*offsetPtr).Mask & (uint32)(ent->WhoSeesMe);
/*
				if (mask == 0)
					nlwarning("TEMP: visibility check failed for entity %s [entitymask=%8X, cellmask=%8X, offsetIndex=%d]", ent->Id.toString().c_str(), (sint32)(ent->WhoSeesMe()), (*offsetPtr).second, offsetPtr-&(_ObjectVisionCellOffsets[0]));
*/
				if (mask != 0)
				{
					fillPtr->Entity = ent;
					fillPtr->Mask = mask;
					fillPtr->Distance = offsetPtr->Distance;
					++fillPtr;
				}
				ent = ent->Next;
			}

			if (fillPtr >= endPtr)
				break;

			++offsetPtr;
		}
		while (offsetPtr <= offsetEnd);
	}


	// then adds entities
	endPtr = entitiesSeenFromCell+MAX_SEEN_ENTITIES;
	fillPtr = cell->addEntities(fillPtr, endPtr, centerCellMask, 0);

	// if the cell has vision on other cells
	if (!cell->isIndoor())
	{
		uint					numOffsets = (uint)_VisionCellOffsets.size();
		CCellOffset*			offsetPtr = &(_VisionCellOffsets[0]);								// warning!! _VisionCellOffsets must be filled
		CCellOffset*			offsetEnd = &(_VisionCellOffsets[_VisionCellOffsets.size()-1]);	// warning!! _VisionCellOffsets must be filled
		//pair<sint32, uint32>	*offsetPtr = &(_VisionCellOffsets[0]);								// warning!! _VisionCellOffsets must be filled
		//pair<sint32, uint32>	*offsetEnd = &(_VisionCellOffsets[_VisionCellOffsets.size()-1]);	// warning!! _VisionCellOffsets must be filled

		do
		{
			pCell = centerCell[ (*offsetPtr).Offset ];
			if (!pCell)
			{
				++offsetPtr;
				continue;
			}

			CWorldEntity	*ent = pCell->getEntitiesList();
			while (ent != NULL && fillPtr < endPtr)
			{
				sint32	mask = (*offsetPtr).Mask & (uint32)(ent->WhoSeesMe);
/*
				if (mask == 0)
					nlwarning("TEMP: visibility check failed for entity %s [entitymask=%8X, cellmask=%8X, offsetIndex=%d]", ent->Id.toString().c_str(), (sint32)(ent->WhoSeesMe()), (*offsetPtr).second, offsetPtr-&(_ObjectVisionCellOffsets[0]));
*/
				if (mask != 0)
				{
					fillPtr->Entity = ent;
					fillPtr->Mask = mask;
					fillPtr->Distance = offsetPtr->Distance;
					++fillPtr;
				}
				ent = ent->Next;
			}

			if (fillPtr >= endPtr)
				break;

			++offsetPtr;
		}
		while (offsetPtr <= offsetEnd);
	}

	numEntities = (uint)(fillPtr-entitiesSeenFromCell);
}

/****************************************************************\
						setCellVisionToEntity()
\****************************************************************/
void	CWorldPositionManager::setCellVisionToEntity( CWorldEntity *entity, CVisionEntry* cellVisionArray, uint numEntities)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	// discard non player entities
	if (!entity->HasVision)
		return;

	// get the player infos of this entity
	CPlayerInfos	*infos = entity->PlayerInfos;

	if (!infos)
	{
		nlwarning("The entity %s doesn't have associated PlayerInfos to compute vision", entity->Id.toString().c_str() );
		return;
	}

	// discard players if vision processing disabled
	if (!infos->EnableVisionProcessing)
		return;

	// update the player vision, new entities in vision are stored in inVision, old ones in outVision
	static CPlayerVisionDelta	visionDelta;
	H_TIME(ComputePlayerDeltaVision, computePlayerDeltaVision( infos, cellVisionArray, numEntities, visionDelta ););

	if (visionDelta.EntitiesIn.empty() && visionDelta.EntitiesOut.empty())
		return;

	{
		H_AUTO(SerialVisionDelta);
		// look for the good list of vision to insert delta in
		TMapFrontEndData::iterator	itFE;
		itFE = _MapFrontEndData.find(infos->FeId);
		
		// if no list yet, create a node
		if (itFE == _MapFrontEndData.end())
		{
			nlwarning("Unable to store vision update, couldn't find FrontEndData %d", infos->FeId.get());
			return;
		}
		
		(*itFE).second.VisionIn += (sint32)visionDelta.EntitiesIn.size();
		(*itFE).second.VisionOut += (sint32)visionDelta.EntitiesOut.size();
		//(*itFE).second.VisionReplace += visionDelta.EntitiesReplace.size();
		
		(*itFE).second.Message.serial(visionDelta);
	}
}

/****************************************************************\
						releaseSlot()
\****************************************************************/
inline void releaseSlot( CPlayerInfos *infos, CWorldEntity* e, uint slot )
{
	// if I was the closest player, then set entity to be not seen
	if (((CWorldEntity*)infos->Entity) == e->ClosestPlayer)
	{
		e->ClosestPlayer = NULL;
		e->VisionCounter = 0x0;
	}

	// free slot
	e->PlayersSeeingMe = e->PlayersSeeingMe-1;
	infos->Slots[slot] = NULL;
	infos->FreeSlots.push_back(slot);
}

/****************************************************************\
						addToEntitiesOut()
\****************************************************************/
inline void addToEntitiesOut( CPlayerInfos *infos, CWorldEntity* e, uint slot, CPlayerVisionDelta &visionDelta )
{
	releaseSlot(infos, e, slot);
	visionDelta.EntitiesOut.push_back(CPlayerVisionDelta::CIdSlot(e->Index, slot));
}

/****************************************************************\
						addToEntitiesIn()
\****************************************************************/
inline void addToEntitiesIn( CPlayerInfos *infos, CWorldEntity* e, CPlayerVisionDelta &visionDelta )
{
	// add the entity into the EntitiesIn delta
	uint	slot = infos->FreeSlots.back();
	visionDelta.EntitiesIn.push_back(CPlayerVisionDelta::CIdSlot(e->Index, slot));
	infos->FreeSlots.pop_back();
	nlassert(infos->Slots[slot] == NULL);
	infos->Slots[slot] = e;
	infos->Slots[slot]->PlayersSeeingMe = infos->Slots[slot]->PlayersSeeingMe+1;
}

/****************************************************************\
						removeFromVisionAndEntitiesIn()
\****************************************************************/
void removeFromVisionAndEntitiesIn( CPlayerInfos *infos, CWorldEntity* e, uint slot, CPlayerVisionDelta &visionDelta)
{
	// If the entity is in the pending EntitiesIn vector (not sent yet), remove it from it,
	// and we must not add it into EntitiesOut.

	/*
		NB: this looks like a O(n2) in the end but no!
		1/ removeFromVisionAndEntitiesIn() is called very rarely
		2/ visionDelta.EntitiesIn is not 255 in size, but something more like 10
	*/
	for (std::vector<CPlayerVisionDelta::CIdSlot>::iterator ite=visionDelta.EntitiesIn.begin(); ite!=visionDelta.EntitiesIn.end(); ++ite)
	{
		if ((*ite).Slot == slot)
		{
			// Remove entity from EntitiesIn (swap with last element then pop back)
			std::vector<CPlayerVisionDelta::CIdSlot>::iterator nextIte = ite;
			++nextIte;
			if (nextIte != visionDelta.EntitiesIn.end())
			{
				*ite = visionDelta.EntitiesIn.back();
			}
			visionDelta.EntitiesIn.pop_back();

			// Remove from vision but do not send the removal as the addition has not been sent yet
			releaseSlot(infos, e, slot);
			return;
		}
	}

	// Otherwise act as a regular removal of entity from vision
	addToEntitiesOut(infos, e, slot, visionDelta);
}



/****************************************************************\
						computePlayerDeltaVision()
\****************************************************************/
void	CWorldPositionManager::computePlayerDeltaVision( CPlayerInfos *infos, CVisionEntry* cellVision, uint &numEntities, CPlayerVisionDelta &visionDelta )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	uint	i;

	visionDelta.PlayerIndex = infos->Entity->Index;
	visionDelta.EntitiesIn.clear();
	visionDelta.EntitiesOut.clear();

	if (infos->ActivateSlot0)
	{
		if (infos->Slot0Active)
		{
			nlwarning("Slot 0 for entity/player %s activated twice consecutively, state unchanged", infos->Entity->Id.toString().c_str());
		}
		else
		{
			if (Verbose)
				nlinfo("Activated Slot 0 for entity/player %s", infos->Entity->Id.toString().c_str());
			visionDelta.EntitiesIn.push_back(CPlayerVisionDelta::CIdSlot(infos->Entity->Index, 0));
			infos->Slot0Active = true;
		}
	}

	if (infos->DesactivateSlot0)
	{
		if (!infos->Slot0Active)
		{
			nlwarning("Slot 0 for entity/player %s desactivated twice consecutively, state unchanged", infos->Entity->Id.toString().c_str());
		}
		else
		{
			if (Verbose)
				nlinfo("Desactivated Slot 0 for entity/player %s", infos->Entity->Id.toString().c_str());
			visionDelta.EntitiesOut.push_back(CPlayerVisionDelta::CIdSlot(infos->Entity->Index, 0));
			infos->Slot0Active = false;
		}
	}
	infos->DesactivateSlot0 = false;
	infos->ActivateSlot0 = false;

	// first mark all entities in new vision: browse the computed vision and set TempVisionState to true for each visible entity
	for (i=0; i<numEntities; ++i)
	{
		CWorldEntity *e = cellVision[i].Entity;

		// Check if entity in new vision
		if ((cellVision[i].Mask & infos->WhoICanSee) != 0)
			e->TempVisionState = true;

		if (cellVision[i].Distance < (0xffu-e->VisionCounter()))
		{
			// if I am closer to this entity, then update entity's closest player
			e->ClosestPlayer = infos->Entity;
			e->VisionCounter = 0xff-(uint8)(cellVision[i].Distance);
		}
		else if (e->ClosestPlayer == infos->Entity)
		{
			// if I was the closest player, the update distance
			e->VisionCounter = 0xff-(uint8)(cellVision[i].Distance);
		}
	}

	// mark own entity as seen (to avoid allocating a slot for it)
	infos->Entity->TempVisionState = true;

	// check all entities that are no more in vision: browse the previous entities in vision and check their new state
	for (i=1; i<MAX_SEEN_ENTITIES; ++i)
	{
		CWorldEntity *e = infos->Slots[i];
		if (e == NULL)
			continue;

		// if flag is not set -> entity is out
		if (!e->TempVisionState)
		{
			addToEntitiesOut(infos, e, i, visionDelta);
		}
		else
		{
			// unset the flag so that only the entities not browsed yet still have the flag set
			e->TempVisionState = false;
		}
	}

	infos->Entity->TempVisionState = false;

	// check all entities that were not in vision before (as long as there are still free slots)
	for (i=0; i<numEntities && !infos->FreeSlots.empty(); ++i)
	{
		// if flag not changed -> entity is in
		CWorldEntity *e = cellVision[i].Entity;
		if (e->TempVisionState)
		{
			addToEntitiesIn(infos, e, visionDelta);
		}

		cellVision[i].Entity->TempVisionState = false;
	}

	// reset vision state for entities that were not checked (not enough slots)
	for (; i<numEntities; ++i)
		cellVision[i].Entity->TempVisionState = false;


	// *** Prevent from splitting vision of controller/controlled entity.
	// Ex: mounted mounts must not be visible if their rider is not visible.
	// It might waste some free slot space but this way we are sure the players won't have invisible riders.

	// First build a short list of entities that may have the problem
	static std::vector< pair<uint, CWorldEntity*> >	entityLinked;
	entityLinked.clear();
	// Must add the slot 0 (user) in the loop, to be sure its parent (eg: mektoub) is correctly handled
	for (i=0; i!=MAX_SEEN_ENTITIES; ++i)
	{
		CWorldEntity *e = infos->Slots[i];
		if (e == NULL)
			continue;
		
		// Set the InVision flag for each entity on its related entity
		if (e->isControlled())
			e->Control->TempParentInVision = true;	// My controller now knows that its parent (ie me) is in vision
		if (e->hasControl())
			e->Parent->TempControlInVision = true;	// My parent now knows that its controler (ie me) is in vision

		// add in the list
		if (e->isControlled() || e->hasControl())
			entityLinked.push_back(make_pair(i,e));
	}
	
	// Then parse this (short) list to potentially remove control/parent that have not their relative in the vision
	for (i=0; i!=entityLinked.size(); ++i)
	{
		uint		slot = entityLinked[i].first;
		CWorldEntity *e = entityLinked[i].second;
		// in any case never remove the slot 0 !
		if(slot==0)
			continue;
		
		// If I am controlled (eg a mektoub) and my controller (eg my rider) is not in vision, then remove me!
		if ((e->isControlled()) && (!e->TempControlInVision))
			removeFromVisionAndEntitiesIn(infos, e, slot, visionDelta);

		// If I am a controller (eg a rider) and my parent (eg my mektoub) is not in vision, then remove me!
		// else if important to not remove twice
		else if ((e->hasControl()) && (!e->TempParentInVision))
			removeFromVisionAndEntitiesIn(infos, e, slot, visionDelta);
	}

	// Clean: parse the (short) list to reset flags
	for (i=0; i!=entityLinked.size(); ++i)
	{
		CWorldEntity *e = entityLinked[i].second;
		if (e->isControlled())
			e->Control->TempParentInVision = false;
		if (e->hasControl())
			e->Parent->TempControlInVision = false;
	}
}

/****************************************************************\
						removePlayerVision()
\****************************************************************/
void CWorldPositionManager::removePlayerVision(CWorldEntity *entity)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	if (entity->HasVision && entity->PlayerInfos != NULL)
	{
		CPlayerInfos	*plInfo = entity->PlayerInfos;
		uint	i;
		for (i=0; i<MAX_SEEN_ENTITIES; ++i)
		{
			if (plInfo->Slots[i] != NULL)
			{
				if (entity == plInfo->Slots[i]->ClosestPlayer)
				{
					// if I was the closest player, then entity is set as not seen
					// This may be false if another player has this entity in vision
					// but it will be properly set next time player vision is computed
					plInfo->Slots[i]->ClosestPlayer = NULL;
					plInfo->Slots[i]->VisionCounter = 0x0;
				}

				plInfo->Slots[i]->PlayersSeeingMe = plInfo->Slots[i]->PlayersSeeingMe-1;
				plInfo->Slots[i] = NULL;
				plInfo->FreeSlots.push_back(i);
			}
		}
	}
}

/****************************************************************\
						removeCombatVision()
\****************************************************************/
void CWorldPositionManager::removeCombatVision(CWorldEntity *entity)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TEntitiesAroundEntityContainer::iterator it = _EntitiesAround.find(entity->Id);
	if (it != _EntitiesAround.end())
		_EntitiesAround.erase(it);
}









/****************************************************************\
						movePlayer()
\****************************************************************/
void CWorldPositionManager::movePlayer(CWorldEntity *entity, sint32 x, sint32 y, sint32 z, float theta, TGameCycle tick, bool forceTick)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	H_AUTO(CWorldPositionManager_movePlayer);

	nlassert(entity != NULL);

#ifdef RECORD_LAST_PLAYER_POSITIONS
	if (entity->PlayerInfos != NULL)
	{
		if (entity->PlayerInfos->DistanceHistory.size() >= 20)
			entity->PlayerInfos->DistanceHistory.pop_back();
		entity->PlayerInfos->DistanceHistory.push_front(make_pair<CVectorD,	uint>(CVectorD(x*0.001, y*0.001, z*0.001), tick));
	}
#endif

	// Is the player controlling a mount?
	if (entity->hasControl())
	{
		// Call movePlayer() on the mount instead of the player
		movePlayer(entity->getControlled(), x, y, z, theta, tick, forceTick);
		return;
	}

	// check time not in future
	if (tick > CTickEventHandler::getGameCycle())
	{
		if (tick - CTickEventHandler::getGameCycle() < 5)
			++NbTicksInFuture;
		else
		{
			++NbTicksInFuture2;
			nldebug( "CWorldPositionManager::movePlayer%s: Future tick big diff: %u (tick=%u now)%u", entity->Id.toString().c_str(), tick-CTickEventHandler::getGameCycle(), tick, CTickEventHandler::getGameCycle() );
		}

		tick = CTickEventHandler::getGameCycle();
	}

	// check time not too old
	if (CTickEventHandler::getGameCycle()-tick >= _NbDynamicWorldImages)
	{
		if (Verbose)
			nlwarning("CWorldPositionManager::movePlayer%s: received a position too old (by %d ticks)", entity->Id.toString().c_str(), 1 + CTickEventHandler::getGameCycle()-tick - _NbDynamicWorldImages);
		return;
	}

	// check position received is not older than last received
	if (entity->Tick > tick && !forceTick)
	{
		if (Verbose)
			nlwarning("CWorldPositionManager::movePlayer%s: received a position older (t=%d) than previous (t=%d)", entity->Id.toString().c_str(), tick, entity->Tick.getValue());
		return;
	}

	// check entity has motion check enabled
	if (!entity->CheckMotion && !entity->isControlled())
	{
		nlwarning("CWorldPositionManager::movePlayer%s: CheckMotion not enabled, strange behaviour may happen", entity->Id.toString().c_str());
	}


	bool								interior = (z&2)!=0;
	bool								water = (z&4)!=0;
	bool								correctPos = false;

	// 
	CVectorD							movVector = entity->localMotion() ? 
												CVectorD((double)(x-entity->LocalX())*0.001, (double)(y-entity->LocalY())*0.001, (double)(z-entity->LocalZ())*0.001) :
												CVectorD((double)(x-entity->X())*0.001,		 (double)(y-entity->Y())*0.001,		 (double)(z-entity->Z())*0.001);
	CVectorD							targetPos = CVectorD(x*0.001, y*0.001, z*0.001);
	CVectorD							finalPos;

	NLMISC::TGameCycle ticksSinceLastUpdate = tick - entity->Tick();

	// Get master (player) entity (in case of a mount)
	CWorldEntity	*master = entity;
	while ((CWorldEntity*)(master->Control) != NULL)
		master = (CWorldEntity*)(master->Control);

	// Get the proper speed
	CMirrorPropValueRO<TYPE_RUNSPEED>	maxSpeed( TheDataset, entity->Index, DSPropertyCURRENT_RUN_SPEED );
	float limitSpeedToUse = maxSpeed();
	if ( entity != master )
	{
		// If the player is on a mount, handle the hunger of the mount
		if ( (movVector.x > 0.001) && (movVector.y > 0.001) )
		{
			CMirrorPropValueRO<TYPE_WALKSPEED> walkSpeed( TheDataset, entity->Index, DSPropertyCURRENT_WALK_SPEED );
			CSpeedLimit speedLimit( TheDataset, entity->Index );
			limitSpeedToUse = speedLimit.getSpeedLimit( walkSpeed, maxSpeed );
		}
	}

	// maxDist = speed(in mm/s)*0.001(to meter)*sec_per_ticks*num_ticks
	//double								maxDist = speed()*0.001*CTickEventHandler::getGameTimeStep()*(tick-entity->Tick());
	// maxDist = speed(in m/s)*sec_per_ticks*num_ticks
	double	maxDist = limitSpeedToUse * SecuritySpeedFactor * CTickEventHandler::getGameTimeStep() * ticksSinceLastUpdate;

	// Check player speed
	// only consider (x,y) motion for speed and position correction
	if (master->PlayerInfos != NULL && master->PlayerInfos->CheckSpeed && CheckPlayerSpeed && fabs(movVector.x)+fabs(movVector.y) > maxDist)
	{
		double		movNorm = sqr(movVector.x)+sqr(movVector.y); // already done if (entity != master) but here is a rare overspeed case

		if (movNorm > sqr(maxDist))
		{
			if (VerboseSpeedAbuse)
			{
				nlwarning("CWorldPositionManager::movePlayer%s: limited speed (movNorm=%.2f, movMax=%.2f, maxSpeed=%.2f)", entity->Id.toString().c_str(), sqrt(movNorm), maxDist, limitSpeedToUse*0.001*SecuritySpeedFactor);
			}

			movVector *= (maxDist / sqrt(movNorm));
		}
	}

	// check entity has a primitive
	if (entity->hasPrimitive())
	{
		uint8	wi = (uint8)((_CurrentWorldImage+_NbDynamicWorldImages-1-CTickEventHandler::getGameCycle()+tick)%_NbDynamicWorldImages+1);

		CPlayerInfos	*pi = entity->PlayerInfos;
		if (pi != NULL)
		{
			// Log plyer motion
			CPlayerInfos::CPlayerPos	ppos;
			ppos.AtTick = CTickEventHandler::getGameCycle();
			entity->Primitive->getGlobalPosition(ppos.GPos, wi);
			ppos.Motion = movVector;
			ppos.Theta = theta;

			if (pi->PosHistory.size() >= 50)
				pi->PosHistory.pop_back();

			pi->PosHistory.push_front(ppos);
		}

		// set entity orientation
		entity->Primitive->setOrientation(theta, wi);

		// do move
		entity->Primitive->move(movVector, wi);

		// eval collision and get triggers state
		entity->MoveContainer->evalNCPrimitiveCollision(1, entity->Primitive, wi);
		//_PatatSubscribeManager.processPacsTriggers(entity->MoveContainer);
		processPacsTriggers(entity->MoveContainer);

		// get final position and interior flag
		finalPos = entity->Primitive->getFinalPosition(wi);
		UGlobalPosition	gp;
		entity->Primitive->getGlobalPosition(gp, wi);

		NLPACS::UGlobalRetriever	*retriever = _ContinentContainer.getRetriever(entity->Continent);

		if (retriever != NULL)
		{
			interior = retriever->isInterior(gp);
			float	dummy;
			water = retriever->isWaterPosition(gp, dummy);
		}

		// if the final position is more than one meter away from the param position, correct entity position
		CVectorD	diff2d = targetPos-finalPos;
		diff2d.z = 0.0;
		if (diff2d.sqrnorm() > 1.0 )
		{
			correctPos = true;
			if (VerboseSpeedAbuse)
			{
				nlwarning("CWorldPositionManager::movePlayer%s: corrected position: real=(%.1f,%.1f) targeted=(%.1f,%.1f)", master->Id.toString().c_str(), finalPos.x, finalPos.y, targetPos.x, targetPos.y);
			}
		}

		// copy pos into targetPos
		targetPos = finalPos;

		// Update entity coordinates directly in mirror
		entity->updatePosition((sint32)(finalPos.x * 1000), (sint32)(finalPos.y * 1000), (sint32)(finalPos.z * 1000), theta, tick, interior, water);
	}
	else
	{
		nlwarning("CWorldPositionManager::movePlayer(%s): entity has no NLPACS::MovePrimitive, motion cannot be checked!", entity->Id.toString().c_str(), tick, entity->Tick.getValue());

		// Update entity coordinates directly in mirror
		entity->updatePosition(x, y, z, theta, tick, interior, water);
	}

	// and update entity links (and children) in cell map
	updateEntityPosition(entity);

	if (correctPos)
	{
		CMessage msgout( "IMPULSION_ID" );
		msgout.serial( master->Id );
		CBitMemStream bms;
		GenericXmlMsgManager.pushNameToStream( "TP:CORRECT", bms );
		bms.serial( const_cast<sint32&>(entity->X()) );	
		bms.serial( const_cast<sint32&>(entity->Y()) );	
		bms.serial( const_cast<sint32&>(entity->Z()) );	
		msgout.serialMemStream( bms );
		CUnifiedNetwork::getInstance()->send( TServiceId(master->Id.getDynamicId()), msgout );
	}
}



/****************************************************************\
					 updateEntityPosition()
\****************************************************************/
void	CWorldPositionManager::updateEntityPosition(CWorldEntity *entity)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	nlassert(entity != NULL);

	entity->PatatEntryIndex = _PatatSubscribeManager.getNewEntryIndex(entity->Id, CVector(entity->X()*0.001f, entity->Y()*0.001f, entity->Z()*0.001f), entity->PatatEntryIndex);

	link(entity);

	deque<CWorldEntity*>	queue;

	bool	interior = ((entity->Z() & 2) != 0);
	bool	water =((entity->Z() & 4) != 0);

	uint	i;
	for (i=0; i<entity->Children.size(); ++i)
		queue.push_back(entity->Children[i]);

	while (!queue.empty())
	{
		CWorldEntity	*child = queue.front();
		queue.pop_front();

		CWorldEntity	*parent = child->Parent;
		if (parent == NULL)
			continue;

		child->updatePosition(interior, water);

		child->PatatEntryIndex = _PatatSubscribeManager.getNewEntryIndex(child->Id, CVector(child->X()*0.001f, child->Y()*0.001f, child->Z()*0.001f), child->PatatEntryIndex);

		link(child);

		for (i=0; i<child->Children.size(); ++i)
			queue.push_back(child->Children[i]);
	}

} // updateEntityPosition

/****************************************************************\
					 updateEntityPosition()
\****************************************************************/
void	CWorldPositionManager::updateEntityPosition(CWorldEntity *entity, sint32 cell)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	nlassert(entity != NULL);

	entity->PatatEntryIndex = _PatatSubscribeManager.getNewEntryIndex(entity->Id, CVector(entity->X()*0.001f, entity->Y()*0.001f, entity->Z()*0.001f), entity->PatatEntryIndex);

	link(entity, cell);

	deque<CWorldEntity*>	queue;

	// push first children
	uint	i;
	for (i=0; i<entity->Children.size(); ++i)
		queue.push_back(entity->Children[i]);

	bool	interior = ((entity->Z() & 2) != 0);
	bool	water =((entity->Z() & 4) != 0);

	while (!queue.empty())
	{
		CWorldEntity	*child = queue.front();
		queue.pop_front();

		CWorldEntity	*parent = child->Parent;
		if (parent == NULL)
			continue;

		child->updatePosition(interior, water);

		child->PatatEntryIndex = _PatatSubscribeManager.getNewEntryIndex(child->Id, CVector(child->X()*0.001f, child->Y()*0.001f, child->Z()*0.001f), child->PatatEntryIndex);

		link(child, cell);

		for (i=0; i<child->Children.size(); ++i)
			queue.push_back(child->Children[i]);
	}

} // updateEntityPosition





/****************************************************************\
					 enablePrimitive()
\****************************************************************/
void CWorldPositionManager::enablePrimitive(CWorldEntity *entity)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	nlassert(entity != NULL);

	// in any case, disable primitive first
	disablePrimitive(entity);

	// find continent to create primitive
	CVectorD	worldPosition(entity->X()*0.001, entity->Y()*0.001, entity->Z()*0.001);
	entity->Continent = _ContinentContainer.findContinent(worldPosition, entity->Id);

	if (entity->Continent == INVALID_CONTINENT_INDEX)
	{
		// couldn't find valid continent, abort
		nlwarning("CWorldPositionManager::enablePrimitive(%s): unable to find valid continent", entity->Id.toString().c_str());
		entity->Continent = NO_CONTINENT_INDEX;
		return;
	}

	// don't force entity to lose primitive
	entity->ForceDontUsePrimitive = false;

	// force entity to have a primitive
	if (!entity->UsePrimitive)
		entity->ForceUsePrimitive = true;

	UMoveContainer	*moveContainer = _ContinentContainer.getMoveContainer(entity->Continent);

	entity->createPrimitive(moveContainer, _CurrentWorldImage);

	// check succeded
	if (entity->Primitive == NULL)
	{
		nlwarning("CWorldPositionManager::enablePrimitive(%s): unable to create primitive", entity->Id.toString().c_str());
		return;
	}

	entity->Primitive->setGlobalPosition(worldPosition, _CurrentWorldImage);
	entity->Primitive->setOrientation(entity->Theta() , _CurrentWorldImage);
	entity->MoveContainer->evalCollision(1, _CurrentWorldImage);
	//_PatatSubscribeManager.processPacsTriggers(entity->MoveContainer);
	processPacsTriggers(entity->MoveContainer);

	//worldPosition = entity->Primitive->getFinalPosition( _CurrentWorldImage );
	entity->updatePositionUsingMovePrimitive(_CurrentWorldImage);
}

/****************************************************************\
					 disablePrimitive()
\****************************************************************/
void CWorldPositionManager::disablePrimitive(CWorldEntity *entity)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	nlassert(entity != NULL);

	entity->removePrimitive();
}

/****************************************************************\
					 resetPrimitive()
\****************************************************************/
void	CWorldPositionManager::resetPrimitive(CWorldEntity *entity)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	teleport(entity, entity->X, entity->Y, entity->Z, entity->Theta, entity->Continent, entity->Cell(), CTickEventHandler::getGameCycle());
}





/****************************************************************\
					 teleport()
\****************************************************************/
void CWorldPositionManager::teleport(const TDataSetRow &index, sint32 x, sint32 y, sint32 z, float theta, uint8 continent, sint32 cell, TGameCycle tick)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	CWorldEntity	*entity = getEntityPtr(index);
	if (entity == NULL)
	{
		if (Verbose)
			nlwarning("teleport(): Entity E%u not in GPMS", index.getIndex());
		return;
	}

	teleport(entity, x, y, z, theta, continent, cell, tick);
}

/****************************************************************\
					 teleport()
\****************************************************************/
void CWorldPositionManager::teleport(CWorldEntity *entity, sint32 x, sint32 y, sint32 z, float theta, uint8 continent, sint32 cell, TGameCycle tick)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	if (entity == NULL)
	{
		nlwarning("teleport(): entity is NULL");
		return;
	}

	// check cell id is valid
/*
	if (cell < 0)
	{
		nlwarning("teleport(%s): cell=%d was indicated negative, should be zero or positive", entity->Id.toString().c_str(), cell);
		cell = 0;
	}
*/

	// teleport only controlled
	if (entity->hasControl())
	{
		CWorldEntity	*controlled = entity->getControlled();

		entity->ForceDontUsePrimitive = true;

		controlled->ForceDontUsePrimitive = false;
		controlled->ForceUsePrimitive = entity->UsePrimitive;

		teleport(controlled, x, y, z, theta, continent, cell, tick);
		return;
	}

	// if entity is on another, break links
	if (entity->localMotion())
	{
		entity->Parent->removeFromChildren(entity);
	}
	else
	{
		entity->LocalX = GLOBAL_POSITION_TAG;
		entity->LocalY = GLOBAL_POSITION_TAG;
		entity->LocalZ = GLOBAL_POSITION_TAG;
	}

	/// \todo Ben: force interior/water bool if needed
	entity->updatePosition(x, y, z, theta, tick, false, false);

	disablePrimitive(entity);

	if (entity->ForceUsePrimitive || (entity->UsePrimitive && !entity->ForceDontUsePrimitive))
		enablePrimitive(entity);

	// if the entity is a player, tell EGS
	if ( entity->Id.getType() == RYZOMID::player )
	{
		CMessage msgout("ENTER_CONTINENT");
		msgout.serial(entity->Id);
		msgout.serial(continent);
		sendMessageViaMirror("EGS",msgout);
	}

	updateEntityPosition(entity, cell);
}


/****************************************************************\
					 attach()
\****************************************************************/
void CWorldPositionManager::attach(const TDataSetRow &father, const TDataSetRow &child, sint32 x, sint32 y, sint32 z)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	//nlwarning("CWorldPositionManager::attach(): not implemented yet!");

	TWorldEntityContainer::iterator	it;
	it = _EntitiesInWorld.find(father);
	CWorldEntity	*fatherEntity = (it == _EntitiesInWorld.end()) ? NULL : (*it).second;
	it = _EntitiesInWorld.find(child);
	CWorldEntity	*childEntity = (it == _EntitiesInWorld.end()) ? NULL : (*it).second;

	if (fatherEntity == NULL || childEntity == NULL)
	{
		nlwarning("Unable to attach %d to %d: fatherEntity=%p childEntity=%p", child.getIndex(), father.getIndex(), fatherEntity, childEntity);
		return;
	}

	// assumes entities are on the same continent
	if (fatherEntity->MoveContainer != NULL && childEntity->MoveContainer != NULL && fatherEntity->MoveContainer != childEntity->MoveContainer)
	{
		nlwarning("Unable to attach %d to %d: entities are in different continent", child.getIndex(), father.getIndex());
		return;
	}

	// insure no parent
	if (childEntity->Parent != NULL)
	{
		nlwarning("Can't attach %d to %d, child already has a parent %s", child.getIndex(), father.getIndex(), childEntity->Parent->Id.toString().c_str());
		return;
	}
	// insure not already child -- integrity check
	vector<CWorldEntity::CWorldEntitySmartPointer>::iterator        itchild;
		for (itchild=fatherEntity->Children.begin(); itchild!=fatherEntity->Children.end(); ++itchild)
			if ((CWorldEntity*)(*itchild) == childEntity)
				break;
	if (itchild != fatherEntity->Children.end())
	{
		nlwarning("%d already attached to %d", child.getIndex(), father.getIndex());
		return;
	}

	// link child and father
	childEntity->Parent = fatherEntity;
	fatherEntity->Children.push_back(childEntity);

	childEntity->LocalX = x;
	childEntity->LocalY = y;
	childEntity->LocalZ = z;

	updateEntityPosition(childEntity);
}

/****************************************************************\
					 detach()
\****************************************************************/
void CWorldPositionManager::detach(const TDataSetRow &child)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TWorldEntityContainer::iterator	it;
	it = _EntitiesInWorld.find(child);
	CWorldEntity	*childEntity = (it == _EntitiesInWorld.end()) ? NULL : (*it).second;

	if (childEntity == NULL)
	{
		nlwarning("Unable to detach %d: can't find id", child.getIndex());
		return;
	}

	CWorldEntity	*fatherEntity = childEntity->Parent;

	if (fatherEntity == NULL)
	{
		nlwarning("Unable to detach %d: not attached to a father", child.getIndex());
		return;
	}

	vector<CWorldEntity::CWorldEntitySmartPointer>::iterator        itchild;
        for (itchild=fatherEntity->Children.begin(); itchild!=fatherEntity->Children.end(); ++it)
                if ((CWorldEntity*)(*itchild) == childEntity)
                        break;

	//
	// TODO Ben
	// What to do if father also as a father (player on a mektub on a ferry...)
	//

	// insure really child -- integrity check
	if (itchild == fatherEntity->Children.end())
	{
		nlwarning("Child %d not referenced in father %s for detaching. Continue, but incoherences may appear later", child.getIndex(), fatherEntity->Id.toString().c_str());
	}
	else
	{
		fatherEntity->Children.erase(itchild);
	}

	childEntity->Parent = NULL;

	childEntity->LocalX = GLOBAL_POSITION_TAG;	// should be replace be a common value -- for frontend mirror
	childEntity->LocalY = GLOBAL_POSITION_TAG;
	childEntity->LocalZ = GLOBAL_POSITION_TAG;

	updateEntityPosition(childEntity);
/*
	updateEntityPosition(childEntity, fatherEntity->X() + childEntity->LocalX(),
									  fatherEntity->Y() + childEntity->LocalY(),
									  fatherEntity->Z() + childEntity->LocalZ(),
									  childEntity->Theta(),
									  CTickEventHandler::getGameCycle());
*/
	childEntity->ForceDontUsePrimitive = false;
	teleport(childEntity, childEntity->X(), childEntity->Y(), childEntity->Z(), childEntity->Theta(), childEntity->Continent, childEntity->Cell < 0 ? childEntity->Cell : 0, CTickEventHandler::getGameCycle());
}



/****************************************************************\
					 acquireControl()
\****************************************************************/
void	CWorldPositionManager::acquireControl(const TDataSetRow &slave, const TDataSetRow &master, sint32 x, sint32 y, sint32 z)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	if (Verbose)
		nldebug("acquireControl( slave=E%u, master=E%u, x=%d, y=%d, z=%d )", slave.getIndex(), master.getIndex(), x, y, z);

	CWorldEntity	*slaveEntity = getEntityPtr(slave);
	CWorldEntity	*masterEntity = getEntityPtr(master);

	if (slaveEntity == NULL || masterEntity == NULL)
	{
		nlwarning("Unable to acquireControl E%u over E%u: slaveEntity=%p masterEntity=%p", master.getIndex(), slave.getIndex(), slaveEntity, masterEntity);
		return;
	}

	// insure no parent
	if (masterEntity->Parent != NULL)
	{
		nlwarning("Can't acquireControl E%u over E%u, master already has a slave E%u", master.getIndex(), slave.getIndex(), masterEntity->Parent->Index.getIndex());
		return;
	}

	// insure slave not already controlled
	if ((CWorldEntity*)slaveEntity->Control != NULL)
	{
		nlwarning("Can't acquireControl E%u over E%u, slave is already controlled by E%u", master.getIndex(), slave.getIndex(), slaveEntity->Control->Index.getIndex());
		return;
	}

	// insure not already child -- integrity check
	vector<CWorldEntity::CWorldEntitySmartPointer>::iterator        itchild;
	for (itchild=slaveEntity->Children.begin(); itchild!=slaveEntity->Children.end(); ++itchild)
		if ((CWorldEntity*)(*itchild) == masterEntity)
			break;

	bool	alreadyChild = false;
	if (itchild != slaveEntity->Children.end())
	{
		nlwarning("E%u already controls E%u", master.getIndex(), slave.getIndex());
		alreadyChild = true;
	}


	// link slave and master
	masterEntity->Parent = slaveEntity;
	if (!alreadyChild)
		slaveEntity->Children.push_back(masterEntity);
	slaveEntity->Control = masterEntity;

	slaveEntity->ForceUsePrimitive = masterEntity->UsePrimitive;
	slaveEntity->ForceDontUsePrimitive = false;
	masterEntity->ForceUsePrimitive = false;
	masterEntity->ForceDontUsePrimitive = true;

	// set local coordinates
	masterEntity->LocalX = x;
	masterEntity->LocalY = y;
	masterEntity->LocalZ = z;

	// teleport to force creation of a primitive
	// don't need to teleport master, done in slave teleport
	teleport(slaveEntity, slaveEntity->X(), slaveEntity->Y(), slaveEntity->Z(), slaveEntity->Theta(), 0, slaveEntity->Cell, CTickEventHandler::getGameCycle());
}

/****************************************************************\
					 leaveControl()
\****************************************************************/
void	CWorldPositionManager::leaveControl(const TDataSetRow &master)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	if (Verbose)
		nldebug("leaveControl( master=E%u )", master.getIndex());

	CWorldEntity	*masterEntity = getEntityPtr(master);

	if (masterEntity == NULL)
	{
		nlwarning("Unable to recover control from E%u: can't find id", master.getIndex());
		return;
	}

	CWorldEntity	*slaveEntity = masterEntity->Parent;

	if (slaveEntity == NULL)
	{
		nlwarning("Unable to recover control from E%u: no control defined", master.getIndex());
		return;
	}

	if ((CWorldEntity*)(slaveEntity->Control) != masterEntity)
	{
		nlwarning("Unable to recover control from E%u over E%u: doesn't own control", master.getIndex(), slaveEntity->Index.getIndex());
		return;
	}

	vector<CWorldEntity::CWorldEntitySmartPointer>::iterator        itchild;
	for (itchild=slaveEntity->Children.begin(); itchild!=slaveEntity->Children.end(); ++itchild)
		if ((CWorldEntity*)(*itchild) == masterEntity)
			break;

	//
	// TODO Ben
	// What to do if father also as a father (player on a mektub on a ferry...)
	//

	// insure really child -- integrity check
	if (itchild == slaveEntity->Children.end())
	{
		nlwarning("Master E%u not referenced in slave E%u for detaching. Continue, but incoherences may appear later", master.getIndex(), slaveEntity->Index.getIndex());
	}
	else
	{
		slaveEntity->Children.erase(itchild);
	}

	// release control
	masterEntity->Parent = NULL;
	slaveEntity->Control = NULL;

	masterEntity->LocalX = GLOBAL_POSITION_TAG;	// should be replace be a common value -- for frontend mirror
	masterEntity->LocalY = GLOBAL_POSITION_TAG;
	masterEntity->LocalZ = GLOBAL_POSITION_TAG;


	// remove slave primitive if only required for movement of the master
	slaveEntity->ForceUsePrimitive = false;
	slaveEntity->ForceDontUsePrimitive = false;
	masterEntity->ForceUsePrimitive = false;
	masterEntity->ForceDontUsePrimitive = false;

	//
	teleport(masterEntity, masterEntity->X(), masterEntity->Y(), masterEntity->Z(), masterEntity->Theta(), masterEntity->Continent, masterEntity->Cell, CTickEventHandler::getGameCycle());
	teleport(slaveEntity, slaveEntity->X(), slaveEntity->Y(), slaveEntity->Z(), slaveEntity->Theta(), slaveEntity->Continent, slaveEntity->Cell, CTickEventHandler::getGameCycle());
}






/****************************************************************\
					 selectCell()
\****************************************************************/
void CWorldPositionManager::selectCell( CCell *pCell )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	nlassert(pCell != NULL);

	_SelectedCells.push_back( pCell);
}

/****************************************************************\
					 selectOneCellByIndex()
\****************************************************************/
void CWorldPositionManager::selectOneCellByIndex( uint32 indexX, uint32 indexY, bool alwaysSelect )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	// check if index is in limites of world 
	if (checkCellBounds(indexX, indexY))
	{
		CCell*	pCell = getCell(indexX, indexY);

		// check if this Cell is allocated, is not they never are entities in cell
		if( pCell != 0 )
		{
			if( (!pCell->isIndoor()) || alwaysSelect )
			{
				_SelectedCells.push_back( pCell );
			}
		}
	}
	else
	{
		if (Verbose)
			nlwarning("CWorldPositionManager::selectOneCellByIndex gave indexs not in world limits index(%u %u)", indexX, indexY );
	}
} //selectOneCellByIndex

/****************************************************************\
					 selectCell by coordinate
\****************************************************************/
void CWorldPositionManager::selectCell( sint32 x, sint32 y, bool alwaysSelect )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	selectOneCellByIndex( (uint32) ( x / _CellSize ), (uint32) ( -y / _CellSize ), alwaysSelect );
} // selectCell

/****************************************************************\
			selectRoundCells by distance around a coordinate
\****************************************************************/
void CWorldPositionManager::selectRoundCells( sint32 x, sint32 y, sint32 d, bool alwaysSelect )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	static uint32 idx, endIdx;
	static uint32 dx;

	uint32	centerx = x / _CellSize;
	uint32	centery = -y / _CellSize;

	if( d > x )
	{
		idx = 0;
		dx = x;
	}
	else
	{
		idx = (uint32) ( ( x - d ) / _CellSize  );
		dx = d;
	}

	dx += d;
	endIdx = (uint32) ( idx + (dx/_CellSize) );

	//
	static uint32 beginIdy, endIdy;
	static uint32 dy;

	if( d > -y )
	{
		beginIdy = 0;
		dy = -y;
	}
	else
	{
		beginIdy = (uint32) ( ( -y - d ) / _CellSize );
		dy = d;
	}
	dy += d;
	endIdy = (uint32) ( beginIdy + (dy/_CellSize) );
	
	//
	for( ; idx <= endIdx; ++idx )
	{
		for( uint32 idy = beginIdy ; idy <= endIdy; ++idy)
		{
			uint32	dist = (uint32)(_CellSize * sqrt(float((idx-centerx)*(idx-centerx) + (idy-centery)*(idy-centery))));
			if ((sint32)dist < d)
				selectOneCellByIndex( idx, idy, alwaysSelect );
		}
	}
} // selectRoundCells

/****************************************************************\
			selectCells by distance around a coordinate
\****************************************************************/
void CWorldPositionManager::selectCells( sint32 x, sint32 y, sint32 d, bool alwaysSelect )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	sint32	startx, endx;
	sint32	starty, endy;

	startx = (+x-d) / _CellSize;
	endx   = (+x+d) / _CellSize;

	starty = (-y-d) / _CellSize;
	endy   = (-y+d) / _CellSize;

	if (startx < 0)						startx = 0;
	if (startx >= (sint32)_WorldMapX)	startx = _WorldMapX-1;
	if (starty < 0)						starty = 0;
	if (starty >= (sint32)_WorldMapY)	starty = _WorldMapY-1;

	sint32	ix, iy;

	for (iy=starty; iy<=endy; ++iy)
		for (ix=startx; ix<=endx; ++ix)
			selectOneCellByIndex(ix, iy, alwaysSelect);

} // selectCells

/****************************************************************\
		select a groupe of cells around an entity
\****************************************************************/
void CWorldPositionManager::selectCellsAroundEntity(const TDataSetRow &index, bool alwaysSelect )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	struct SIndexOffset
	{
		sint16 iX;
		sint16 iY;
	};

	static const SIndexOffset IndexOffset[] =
	{
		// start at top / left on cell
		{ 0,	-1	},
		{ -1,	0	},
		{ 0,	1	},
		// start at top / right on cell
		{ 0,	-1	},
		{ 1,	0	},
		{ 0,	1	},
		// start at bottom / left on cell
		{ -1,	0	},
		{ 0,	1	},
		{ 1,	0	},
		// start at bottom / right cell
		{ 1,	0	},
		{ 0,	1	},
		{ -1,	0	},
	};

	CWorldEntity	*entity = getEntityPtr(index);

	if( !entity )
	{
		if (Verbose)
			nlwarning("CWorldPositionManager::selectCellsAroundEntity entity E%u for select around is not in GPMS !", index.getIndex());
		return;
	}

	// for entities in indoor cells, only select cell
	if (entity->CellPtr != NULL && entity->CellPtr->isIndoor())
	{
		selectCell( entity->CellPtr );
		return;
	}

	sint32 iX = (+entity->X()) / _CellSize;
	sint32 iY = (-entity->Y()) / _CellSize;

	bool nearLeftBorder = ( (+entity->X()) % _CellSize ) < ( _CellSize / 2 );
	bool nearUpBorder =   ( (-entity->Y()) % _CellSize ) < ( _CellSize / 2 );

	uint32 startTableIndex = nearLeftBorder ? 0: 3;
	startTableIndex += nearUpBorder ? 0 : 6;

	selectOneCellByIndex( (uint16) iX, (uint16) iY, alwaysSelect );

	for( int i = 1; i < 4; ++i )
	{
		iX = iX + IndexOffset[ startTableIndex + i - 1 ].iX;
		iY = iY + IndexOffset[ startTableIndex + i - 1 ].iY;

		if( ( iX >= 0 ) && ( iX < (sint32)_WorldMapX ) )
		{
			if( ( iY >= 0 ) && ( iY < (sint32)_WorldMapY ) )
			{
				selectOneCellByIndex( (uint16) iX, (uint16) iY, alwaysSelect );
			}
		}
	}
}


/****************************************************************\
 resquest for received by mirror all update for asked properties 
			of all entities around gived entity
\****************************************************************/
void CWorldPositionManager::requestForEntityAround( NLNET::TServiceId serviceId, const CEntityId& id, const list< string >& propertiesName )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	unrequestForEntityAround( serviceId, id );

	TEntitiesAroundEntityContainer::iterator it = _EntitiesAround.find( id );
	if( it == _EntitiesAround.end() )
	{
		pair<TEntitiesAroundEntityContainer::iterator, bool>	result = _EntitiesAround.insert( make_pair( id, CAroundEntityInfo() ) );
		it = result.first;
	}

	(*it).second.Subscribers.push_back(CAroundSubscriberInfo());
	CAroundSubscriberInfo	&subscriber = (*it).second.Subscribers.back();

	subscriber.ServiceId = serviceId;
	//subscriber.Properties = propertiesName;
}

/****************************************************************\
		unrequest previous request mirror update for entities 
						around another
\****************************************************************/
void CWorldPositionManager::unrequestForEntityAround( NLNET::TServiceId serviceId, const CEntityId& id )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TEntitiesAroundEntityContainer::iterator it = _EntitiesAround.find( id );
	if( it == _EntitiesAround.end() )
		return;

	for ( list<CAroundSubscriberInfo>::iterator its = (*it).second.Subscribers.begin(); its != (*it).second.Subscribers.end(); )
	{
		if( (*its).ServiceId == serviceId )
		{
			its = (*it).second.Subscribers.erase( its );
		}
		else
		{
			++its;
		}
	}		

	_EntitiesAround.erase( it );
}

/****************************************************************\
	unrequest all previous requests mirror update for entities
						around another
\****************************************************************/
void CWorldPositionManager::unrequestAllForEntityAround( const CEntityId& id )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TEntitiesAroundEntityContainer::iterator it = _EntitiesAround.find( id );
	if( it == _EntitiesAround.end() )
		return;

	for ( list<CAroundSubscriberInfo>::iterator its = (*it).second.Subscribers.begin(); its != (*it).second.Subscribers.end(); )
	{
		its = (*it).second.Subscribers.erase( its );
	}

}

/****************************************************************\
		Update list of asked entities around an entity
\****************************************************************/
void CWorldPositionManager::updateEntitiesAround( void )
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	TMapServiceData::iterator		itService;
	for (itService=_MapServiceData.begin(); itService!=_MapServiceData.end(); ++itService)
	{
		CMessage	&msg = (*itService).second.Message;
		msg.clear();
		msg.setType(CombatVisionMessageType);
		(*itService).second.MessageHeaderSize = msg.getPos();
	}

	for( TEntitiesAroundEntityContainer::iterator it = _EntitiesAround.begin(); it != _EntitiesAround.end(); ++it )
	{
		// compute vision delta for this
		// first select entities in new vision
		clearPreviousSelection();
		selectCellsAroundEntity( getEntityIndex((*it).first) );

		static CCombatVisionDelta		combatVisionDelta;
		static vector<CWorldEntity*>	entitiesInView(256);
		combatVisionDelta.EntitiesIn.clear();
		combatVisionDelta.EntitiesOut.clear();
		entitiesInView.clear();
		combatVisionDelta.EntityId = (*it).first;


		// mark new vision
		CEntityIterator		ita;
		for (ita.begin(); !ita.end(); ++ita)
		{
			CWorldEntity	*entity = *ita;
			entitiesInView.push_back(entity);
			//entity->TempVisionState = CWorldEntity::Checked;
			entity->TempVisionState = true;
		}

		// check old vision
		TEntitySTLList				&entityList = (*it).second.WorldEntitiesAround;
		TEntitySTLList::iterator	ite;
		for (ite = entityList.begin(); ite != entityList.end(); )
		{
			if (!(*ite)->TempVisionState)
			{
				// entity is out of vision
				combatVisionDelta.EntitiesOut.push_back((*ite)->Id);
				ite = entityList.erase( ite );
			}
			else
			{
				(*ite)->TempVisionState = false;
				++ite;
			}
		}

		// check new vision
		vector<CWorldEntity*>::iterator	itne;	// better use a vector iterator, probably faster
		for (itne=entitiesInView.begin(); itne!=entitiesInView.end(); ++itne)
		{
			CWorldEntity	*entity = *itne;

			if (entity->TempVisionState)
			{
				// entity is new in vision
				combatVisionDelta.EntitiesIn.push_back(entity->Id);
				entityList.push_back(entity);
			}
			entity->TempVisionState = false;
		}

		if (combatVisionDelta.EntitiesIn.empty() && combatVisionDelta.EntitiesOut.empty())
			continue;

		list<CAroundSubscriberInfo>::iterator	itSub;
		for (itSub = (*it).second.Subscribers.begin(); itSub != (*it).second.Subscribers.end(); ++itSub)
		{
			// look for the good list of vision to insert delta in
			itService = _MapServiceData.find((*itSub).ServiceId);

			// if no list yet, create a node
			if (itService == _MapServiceData.end())
			{
				pair<TMapServiceData::iterator, bool>	result = _MapServiceData.insert(make_pair( itSub->ServiceId, CServiceData() ));
				if (!result.second)
				{
					nlwarning("Unable to insert vision delta for entity %s (associated to service %d)", it->first.toString().c_str(), itSub->ServiceId.get());
					continue;
				}
				itService = result.first;
				CMessage	&msg = (*itService).second.Message;
				msg.clear();
				msg.setType(CombatVisionMessageType);
				(*itService).second.MessageHeaderSize = msg.getPos();
			}

			(*itService).second.Message.serial(combatVisionDelta);
		}
	}

	for (itService=_MapServiceData.begin(); itService!=_MapServiceData.end(); ++itService)
	{
		CMessage	&msg = (*itService).second.Message;
		if ((*itService).second.MessageHeaderSize == msg.getPos())
			continue;

		sendMessageViaMirror( (*itService).first, msg );

		msg.clear();
	}
}



/****************************************************************\
		vision request
\****************************************************************/
void	CWorldPositionManager::visionRequest(sint32 x, sint32 y, sint32 range, vector<pair<CEntityId, sint32> > &entities)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	clearPreviousSelection();

	selectCells(x, y, range);

	double	frange = range*0.001;

	CEntityIterator	ite;
	for (ite.begin(); !ite.end(); ++ite)
	{
		CWorldEntity	*entity = *ite;
		double			rrange;

		if (abs(x - entity->X) < range && abs(y - entity->Y) < range &&
			(rrange = sqrt(sqr((x - entity->X)*0.001) + sqr((y - entity->Y)*0.001))) < frange)
		{
			entities.push_back(make_pair<CEntityId, sint32>(entity->Id, (sint32)(rrange*1000)));
		}
	}

	CObjectIterator	ito;
	for (ito.begin(); !ito.end(); ++ito)
	{
		CWorldEntity	*entity = *ito;
		double			rrange;

		if (abs(x - entity->X) < range && abs(y - entity->Y) < range &&
			(rrange = sqrt(sqr((x - entity->X)*0.001) + sqr((y - entity->Y)*0.001))) < frange)
		{
			entities.push_back(make_pair<CEntityId, sint32>(entity->Id, (sint32)(rrange*1000)));
		}
	}
}





















// Check
void	CWorldPositionManager::autoCheck(NLMISC::CLog *log)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	for (TPlayerList::iterator itp=_UpdatePlayerList.begin(); itp!=_UpdatePlayerList.end(); ++itp)
	{
		CPlayerInfos	*player = *itp;
		CWorldEntity	*entity = player->Entity;
		if (entity == NULL)
			continue;

		log->displayNL("Check player %s vision (WhoICanSee=%8X)", entity->Id.toString().c_str(), player->WhoICanSee);

		clearPreviousSelection();
		
		selectRoundCells(entity->X(), entity->Y(), 250000);

		CEntityIterator	it;
		for (it.begin(); !it.end(); ++it)
		{
			CWorldEntity	*check = *it;
			check->TempVisionState = true;
		}

		uint	i;
		uint	total = 0, faulty = 0;
		for (i=0; i<MAX_SEEN_ENTITIES; ++i)
		{
			CWorldEntity	*check = player->Slots[i];
			if (check != NULL)
			{
				if (!check->TempVisionState)
				{
					float	dist = (float)sqrt((float)(entity->X()-check->X())*(float)(entity->X()-check->X()) + (float)(entity->Y()-check->Y())*(float)(entity->Y()-check->Y()))*0.001f;
					log->displayNL("entity %s in vision whereas should not be (dist=%.1f, WhoSeesMe=%8X)", check->Id.toString().c_str(), dist, (uint32)check->WhoSeesMe());
					++faulty;
				}
				check->TempVisionState = false;
				++total;
			}
		}
		log->displayNL("%d in slot vision, %d faulty", total, faulty);

		total = 0;
		faulty = 0;
		for (it.begin(); !it.end(); ++it)
		{
			CWorldEntity	*check= *it;
			if (check->TempVisionState)
			{
				float	dist = (float)sqrt((float)(entity->X()-check->X())*(float)(entity->X()-check->X()) + (float)(entity->Y()-check->Y())*(float)(entity->Y()-check->Y()))*0.001f;
				log->displayNL("entity %s not in vision whereas should be (dist=%.1f, WhoSeesMe=%8X)", check->Id.toString().c_str(), dist, (uint32)check->WhoSeesMe());
				++faulty;
			}
			check->TempVisionState = false;
			++total;
		}
		log->displayNL("%d in select vision, %d faulty", total, faulty);
	}
}


void	CWorldPositionManager::displayVisionCells(NLMISC::CLog *log)
{
	STOP_IF(IsRingShard,"Illegal use of CWorldPositionManager on ring shard");
	uint	h;

	for (h=0; h<_ObjectVisionCellOffsets.size(); ++h)
		log->displayNL("_ObjectVisionCellOffsets[%d]: (%d, %X)", h, _ObjectVisionCellOffsets[h].Offset, _ObjectVisionCellOffsets[h].Mask);

	for (h=0; h<_VisionCellOffsets.size(); ++h)
		log->displayNL("_VisionCellOffsets[%d]: (%d, %X)", h, _VisionCellOffsets[h].Offset, _VisionCellOffsets[h].Mask);
}

/*
*/











































