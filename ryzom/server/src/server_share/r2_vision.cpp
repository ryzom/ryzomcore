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

/*
  TODO:
	- Deal with invisibility masks
	- Deal with biasing
	- add a solution for split() if all of the entities in the group are super-imposed
*/


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"

#include "nel/misc/types_nl.h"
#include "r2_vision.h"

#ifdef NL_DEBUG
	// #define ENABLE_TESTS
#endif
#include "game_share/sadge_tests.h"
#include "game_share/tick_event_handler.h"

//#undef TEST
//#define TEST(txt_and_args) nldebug txt_and_args

//-----------------------------------------------------------------------------
// NLMISC variables
//-----------------------------------------------------------------------------

NLMISC::CVariable<bool> VerboseVisionDelta("r2vision","VerboseVisionDelta","enable verbose logging of vision changes",false, 0, true);
NLMISC::CVariable<uint32> VisionResetDuration("r2vision","VisionResetDuration","the number of updates to be skipped when reseting player vision",10, 0, true);


//-----------------------------------------------------------------------------
// namespace R2_VISION
//-----------------------------------------------------------------------------

namespace R2_VISION
{
	//-----------------------------------------------------------------------------
	// Constants
	//-----------------------------------------------------------------------------

	// the maximum radius that we consider correct for a vision group
	static const uint32 MaxVisionGroupRadius= 50*1000;		// 50 meters
	static const uint32 MaxVisionGroupDiameter= 2*MaxVisionGroupRadius;

	// the maximum number of viewers permitted in a vision group
	static const uint32 MaxViewersPerGroup= 100;

	// max viewer dist beyond which we start to split the group into sub groups 
	static const uint32 MinVisionGroupSplitDiameter= ((MaxVisionGroupDiameter*5)/4);

	// max dist beyond which entities are never seen
	static const uint32 MaxVisionDist= 250*1000;	// 250 meter
	static const uint32 VisionBucketShift= 15;		// divide by 32768
	static const uint32 NumVisionBuckets= 1+(MaxVisionDist>>VisionBucketShift);
	static const uint32 SmallestBucketSize= 1<<VisionBucketShift;

	// a big and nasty hack to get a zero value for a TDataSetRow (needed for sorting etc)
	static const uint32 Zero=0;
	static const TDataSetRow& ZeroDataSetRow=*(TDataSetRow*)&Zero;

	// the dimentions for the different vision vector operations
	static const uint32 AllocatedVisionVectorSize= 256;
	static const uint32 MaxVisionEntries= 250;			// max 256 - (1 slot for invalid entry, 1 slot for viewer when invisible)


	//-----------------------------------------------------------------------------
	// Handy Utility Routines
	//-----------------------------------------------------------------------------

	inline uint32 quickDist(uint32 x0, uint32 y0, uint32 x1, uint32 y1)
	{
		uint32 xdist= abs((sint32)(x0-x1));
		uint32 ydist= abs((sint32)(y0-y1));
		return (xdist>ydist)? (xdist+(ydist>>1)): (ydist+(xdist>>1));
	}


	inline bool visionGroupsOverlap(const CVisionGroup* va, const CVisionGroup* vb)
	{
		// if either of the vision groups are empty return false
		if (va==NULL || vb==NULL || va->numViewers()==0 || vb->numViewers()==0)
			return false;

		// if the vision groups don't share the same vision level then return false
		if (va->getVisionLevel() != vb->getVisionLevel())
			return false;

		// calculate the minima of x and Y coordinates from the 2 groups
		uint32 combinedXMin= std::min(va->xMin(),vb->xMin());
		uint32 combinedYMin= std::min(va->yMin(),vb->yMin());
		uint32 combinedXMax= std::max(va->xMax(),vb->xMax());
		uint32 combinedYMax= std::max(va->yMax(),vb->yMax());

		// if the combined groups' bounding space would be too large then don't bother looking any further
		if (quickDist(	combinedXMin,	combinedYMin,	combinedXMax,	combinedYMax ) > MaxVisionGroupDiameter)
			return false;

		// the overlap is confirmed if the sum of the players in the 2 groups is small enough
		if (va->numViewers() + vb->numViewers() < MaxViewersPerGroup)
			return true;

		// calculate the distance from extreme x min to extreme x max and extreme y min to exetrem y max
		uint32 extemeXdist= combinedXMax - combinedXMin;
		uint32 extemeYdist= combinedYMax - combinedYMin;

		uint32 xDistA= (va->xMax() - va->xMin());
		uint32 yDistA= (va->yMax() - va->yMin());
		uint32 xDistB= (vb->xMax() - vb->xMin());
		uint32 yDistB= (vb->yMax() - vb->yMin());

		// calculate the sum of the 2 vision group lengths in each of x and y
		uint32 sumXdist= xDistA + xDistB;
		uint32 sumYdist= yDistA + yDistB;

		// we consider that a real overlap occurs when both extreme values are < 3/4 of the equivalent sum values
		if ( (4*extemeXdist<=3*sumXdist) && (4*extemeYdist<=3*sumYdist) )
			return true;

		// we consider the vision groups don't overlap
		return false;
	}


	//-----------------------------------------------------------------------------
	// METHODS CUniverse
	//-----------------------------------------------------------------------------

	void CUniverse::createInstance(uint32 aiInstance, uint32 groupId)
	{
		// just ignore attempts to create the std::numeric_limits<uint32>::max() instance
		if (aiInstance==std::numeric_limits<uint32>::max())
		{
			return;
		}

		// ensure that the new AIInstance value is reasonable & increase _Instances vector size if required
		BOMB_IF(aiInstance>65535,"Failed to create Instance with implausible AIInstance value",return);
		if (_Instances.size()<=aiInstance)
		{
			nlinfo("Increasing Universe::AIInstance vector size to: %d",aiInstance+1);
			_Instances.resize(aiInstance+1);
		}
		NLMISC::CSmartPtr<CInstance>& theInstance= _Instances[aiInstance];

		// allocating the new instance
		if (theInstance==NULL)
		{
			theInstance= new CInstance(this,groupId);
			nlinfo("Allocating new AIInstance: %d at address: %p",aiInstance,&*theInstance);
		}
	}

	void CUniverse::removeInstance(uint32 aiInstance)
	{
		// just ignore attempts to remove the std::numeric_limits<uint32>::max() instance
		if (aiInstance==std::numeric_limits<uint32>::max())
		{
			return;
		}

		// make sure the instance exists
		DROP_IF(aiInstance>=_Instances.size()||_Instances[aiInstance]==NULL,
			NLMISC::toString("Ignoring attempt to remove non-existant instance: %d",aiInstance),return);

		// allow the instance to do housekeeping before being destroyed
		_Instances[aiInstance]->release();

		// destroy the instance
		_Instances[aiInstance]=NULL;
	}

	void CUniverse::removeInsancesByGroup(uint32 groupId)
	{
		// iterate over complete vector of instances releasing any that match the given group id
		for (uint32 i=0;i<_Instances.size();++i)
		{
			if (_Instances[i]!=NULL && _Instances[i]->getGroupId()==groupId)
			{
				removeInstance(i);
			}
		}
	}

	void CUniverse::addEntity(TDataSetRow dataSetRow,uint32 aiInstance,sint32 x,sint32 y,TInvisibilityLevel invisibilityLevel,bool isViewer)
	{
		H_AUTO(CUniverse_addEntity)

		// strip junk from data set row to leave a pure index value
		uint32 row= dataSetRow.getIndex();

		// ensure that the row value is valid
		BOMB_IF(row==0,"Illegal attempt to manipulate entity with dataset row=0",return);
		BOMB_IF(row>(1<<20),NLMISC::toString("Ignoring entity with implausible row value: %d",row),return);

		// delegate to workhorse routine (converting y coordinate to +ve axis)
		_addEntity(dataSetRow,aiInstance,x,-y,invisibilityLevel,isViewer);
	}

	void CUniverse::forceRefreshVision(TDataSetRow dataSetRow)
	{
		uint32 row = dataSetRow.getIndex();

		// ensure that the row value is valid
		BOMB_IF(row==0,"Illegal attempt to manipulate entity with dataset row=0",return);
		BOMB_IF(row>(1<<20),NLMISC::toString("Ignoring entity with implausible row value: %d",row),return);

		SUniverseEntity& theEntity= _Entities[row];

		// if the entity was already allocated then remove it
		if (theEntity.AIInstance == std::numeric_limits<uint32>::max()) { return; }

		if ( theEntity.ViewerRecord)
		{
			uint32 x = theEntity.ViewerRecord->getX();
			uint32 y = theEntity.ViewerRecord->getY();
			TInvisibilityLevel invisibilityLevel = theEntity.ViewerRecord->getVisionLevel();
			uint32 aiInstance = theEntity.AIInstance;
			
			_addEntity(dataSetRow,aiInstance, static_cast<sint32>(x), static_cast<sint32>(y),invisibilityLevel, true);	
		}		
		
	}
	
	void CUniverse::forceResetVision(TDataSetRow dataSetRow)
	{
		uint32 row = dataSetRow.getIndex();

		// ensure that the row value is valid
		BOMB_IF(row==0,"Illegal attempt to manipulate entity with dataset row=0",return);
		BOMB_IF(row>(1<<20),NLMISC::toString("Ignoring entity with implausible row value: %d",row),return);

		// get the pointer to the entity record and ensure that the entity is a viewer
		SUniverseEntity& theEntity= _Entities[row];
		BOMB_IF(theEntity.ViewerRecord==NULL,"IGNORING ResetVision for non-viewer entity"+dataSetRow.toString(),return);

		// if we're in verbose logging mode then do some logging...
		if (VerboseVisionDelta)
		{
			nlinfo("FRR- Force Reset Vision received for entitiy: %s",dataSetRow.toString().c_str());
		}

		// flag the ViewerRecord to force a vision reset
		theEntity.ViewerRecord->forceResetVision();
	}

	void CUniverse::removeEntity(TDataSetRow dataSetRow)
	{
		H_AUTO(CUniverse_removeEntity)

		// strip junk from data set row to leave a pure index value
		uint32 row= dataSetRow.getIndex();

		// ensure that the row value is valid
		BOMB_IF(row==0,"Illegal attempt to manipulate entity with dataset=0",return);
		BOMB_IF(row>=_Entities.size(),NLMISC::toString("Ignoring attempt to remove entity with invalid row value: %d",row),return);

		// delegate to workhorse routine
		_removeEntity(dataSetRow);
	}

	SUniverseEntity* CUniverse::getEntity(TDataSetRow dataSetRow)
	{
		return getEntity(dataSetRow.getIndex());
	}

	SUniverseEntity* CUniverse::getEntity(uint32 row)
	{
		BOMB_IF(row==0,"Illegal attempt to manipulate entity with dataset=0",return NULL);
		BOMB_IF(row>=_Entities.size(),NLMISC::toString("Attempting to get hold of entity with invalid row value: %d",row),return NULL);
		return &_Entities[row];
	}

	void CUniverse::setEntityPosition(TDataSetRow dataSetRow,sint32 x,sint32 y)
	{
		H_AUTO(CUniverse_setEntityPosition)

		// strip junk from data set row to leave a pure index value
		uint32 row= dataSetRow.getIndex();

		// ensure that the row value is valid
		BOMB_IF(row==0,"Illegal attempt to manipulate entity with dataset=0",return);
		BOMB_IF(row>=_Entities.size(),NLMISC::toString("Ignoring attempt to set entity position with invalid row value: %d",row),return);

		// delegate to workhorse routine (converting y coordinate to +ve axis)
		_setEntityPosition(dataSetRow,x,-y);
	}

	void CUniverse::setEntityPositionDelayed(TDataSetRow dataSetRow,sint32 x,sint32 y,uint32 ticks)
	{
		H_AUTO(CUniverse_setEntityPositionDelayed)

		nlinfo("adding delayed entity position to the list: %s (%d,%d) @ time: %d",dataSetRow.toString().c_str(),x,y,ticks);
		_DelayedPositions.push_back(SDelayedPlayerPosition(ticks,dataSetRow,x,y));
	}

	void CUniverse::teleportEntity(TDataSetRow dataSetRow,uint32 aiInstance,sint32 x,sint32 y,TInvisibilityLevel invisibilityLevel)
	{
		H_AUTO(CUniverse_teleportEntity)

		// strip junk from data set row to leave a pure index value
		uint32 row= dataSetRow.getIndex();

		// ensure that the row value is valid
		BOMB_IF(row==0,"Illegal attempt to manipulate entity with datasetrow=0",return);
		BOMB_IF(row>=_Entities.size(),NLMISC::toString("Ignoring attempt to set entity position with invalid row value: %d",row),return);

		// ensure that the new AIInstance exists
		BOMB_IF(aiInstance!=std::numeric_limits<uint32>::max() && (aiInstance>=_Instances.size() || _Instances[aiInstance]==NULL),
			NLMISC::toString("ERROR: Failed to add entity %d to un-initialised instance: %d",row,aiInstance),aiInstance=std::numeric_limits<uint32>::max());

		// delegate to workhorse routine (converting y coordinate to +ve axis)
		_teleportEntity(dataSetRow,aiInstance,x,-y,invisibilityLevel);
	}

	void CUniverse::setEntityInvisibilityInfo(TDataSetRow dataSetRow,uint32 whoSeesMe)
	{
		// get a handle to the entity
		SUniverseEntity& theEntity= _Entities[dataSetRow.getIndex()];

		// tell the instance that the entity has changed visiblity
		if (theEntity.AIInstance<_Instances.size())
		{
			NLMISC::CSmartPtr<CInstance>& theInstance= _Instances[theEntity.AIInstance];

			// extract the invisibility level from the whoSeesMe value
			TInvisibilityLevel invisibilityLevel= (TInvisibilityLevel)(whoSeesMe&((1<<NUM_WHOSEESME_BITS)-1));

			// set the visibility info for us as a viewed entity:
			theInstance->setEntityInvisibility(theEntity.InstanceIndex, invisibilityLevel);

			// if the entity is a viewer then update the viewer info
			if (theEntity.ViewerRecord!=NULL)
			{
				// determine the vision level (the bit of who_sees_me used by the viewer)
				TInvisibilityLevel visionLevel= (TInvisibilityLevel)(whoSeesMe>>NUM_WHOSEESME_BITS);

				// if the vision level has changed then we have stuff to do
				if (theEntity.ViewerRecord->getVisionLevel()!=visionLevel)
				{
					if (VerboseVisionDelta)
					{
						nlinfo("Vision Delta: %s: Changes vision level from %d to %d",dataSetRow.toString().c_str(),theEntity.ViewerRecord->getVisionLevel(),visionLevel);
					}

					// change the vision level for ourselves
					theEntity.ViewerRecord->setVisionLevel(visionLevel);

					// remove the entity from their current vision group
					theInstance->isolateViewer(theEntity);
				}
			}
		}
	}

	void CUniverse::registerVisionDeltaManager(IVisionDeltaManager* manager)
	{
		_VisionDeltaManager= manager;
	}

	void CUniverse::addVisionDelta(const CPlayerVisionDelta& visionDelta)
	{
		if (_VisionDeltaManager!=NULL)
		{
			_VisionDeltaManager->addVisionDelta(visionDelta);
		}
	}

	void CUniverse::update()
	{
		H_AUTO(CUniverse_Update)

		// deal with any delayed player positions who's timers have expired
		NLMISC::TGameCycle currentTick= CTickEventHandler::getGameCycle();
		TDelayedPositions::iterator it= _DelayedPositions.begin();
		TDelayedPositions::iterator itEnd= _DelayedPositions.end();
		while(it!=itEnd)
		{
			// copy 'it' to 'curIt' and post incrment it
			TDelayedPositions::iterator curIt= it++;

			// if next entry in list's time is up then treat it
			if (curIt->GameCycle <= currentTick)
			{
				setEntityPosition(curIt->DataSetRow,curIt->X,curIt->Y);
				_DelayedPositions.erase(curIt);
			}
		}

		// deal with the updates for all of the instances
		for (uint32 i=0;i<_Instances.size();++i)
		{
			if (_Instances[i]!=NULL)
			{
				_Instances[i]->updateVision();
			}
		}
	}

	inline void CUniverse::_addEntity(TDataSetRow dataSetRow,uint32 aiInstance,sint32 x,sint32 y,TInvisibilityLevel invisibilityLevel,bool isViewer)
	{
		uint32 row= dataSetRow.getIndex();

		// increase _Entities vector size if required
		if (_Entities.size()<=row)
		{
			nlinfo("Increasing Universe::Entities vector size to: %d",row+1);
			_Entities.resize(row+1);
		}
		SUniverseEntity& theEntity= _Entities[row];

		// if the entity was already allocated then remove it
		if (theEntity.AIInstance!=std::numeric_limits<uint32>::max())
		{
			_removeEntity(dataSetRow);
		}

		// if the entity is a viewer then setup their vision record
		if (isViewer)
		{
			theEntity.ViewerRecord= new CViewer;
			theEntity.ViewerRecord->init(dataSetRow,this);
		}

		// delegate to the teleport code to setup coordinates etc
		_teleportEntity(dataSetRow,aiInstance,x,y,invisibilityLevel);
	}

	inline void CUniverse::_removeEntity(TDataSetRow dataSetRow)
	{
		// get a handle to the entity
		SUniverseEntity& theEntity= _Entities[dataSetRow.getIndex()];

		// If the entity was previously in an AIInstance then remove them
		if (theEntity.AIInstance<_Instances.size() && _Instances[theEntity.AIInstance]!=NULL)
		{
			// detach the entity from the instance it's currently in
			_Instances[theEntity.AIInstance]->removeEntity(theEntity);
			theEntity.AIInstance= std::numeric_limits<uint32>::max();
			theEntity.ViewerRecord= NULL;
		}
	}

	inline void CUniverse::_setEntityPosition(TDataSetRow dataSetRow,sint32 x,sint32 y)
	{
		// get a handle to the entity
		SUniverseEntity& theEntity= _Entities[dataSetRow.getIndex()];

		#ifdef NL_DEBUG
		nlassert(theEntity.AIInstance==std::numeric_limits<uint32>::max() || theEntity.AIInstance<_Instances.size());
		nlassert(theEntity.AIInstance==std::numeric_limits<uint32>::max() || _Instances[theEntity.AIInstance]!=NULL);
		#endif

		// if the entity is a viewer then move the view coordinates
		if (theEntity.ViewerRecord!=NULL)
		{
			theEntity.ViewerRecord->setEntityPosition(x,y);
		}

		// if the entity is not currently in an AIInstance then stop here
		if (theEntity.AIInstance==std::numeric_limits<uint32>::max())
			return;
		
		// set the instance entity record position
		_Instances[theEntity.AIInstance]->setEntityPosition(theEntity.InstanceIndex,x,y);
	}

	inline void CUniverse::_teleportEntity(TDataSetRow dataSetRow,uint32 aiInstance,sint32 x,sint32 y,TInvisibilityLevel invisibilityLevel)
	{
		// get a handle to the entity
		SUniverseEntity& theEntity= _Entities[dataSetRow.getIndex()];

		// If the entity was previously in a different AIInstance then remove them
		if (theEntity.AIInstance<_Instances.size() && _Instances[theEntity.AIInstance]!=NULL)
		{
			_Instances[theEntity.AIInstance]->removeEntity(theEntity);
		}

		// set the new AIInstance value for the entity
		theEntity.AIInstance= aiInstance; 

		// if the aiInstance is set to std::numeric_limits<uint32>::max() (a reserved value) then we stop here
		if (aiInstance==std::numeric_limits<uint32>::max())
		{
			// clear out the vision for an entity in aiInstance std::numeric_limits<uint32>::max()
			if (getEntity(dataSetRow)->ViewerRecord!=NULL)
			{
				TVision emptyVision(2);
				nlctassert(sizeof(TDataSetRow)==sizeof(uint32));
				emptyVision[0].DataSetRow= ZeroDataSetRow;
				emptyVision[1].DataSetRow=dataSetRow;
				getEntity(dataSetRow)->ViewerRecord->updateVision(emptyVision,this,false);
			}
			return;
		}

		// if the entity is a viewer then move the view coordinates
		if (theEntity.ViewerRecord!=NULL)
		{
			theEntity.ViewerRecord->setEntityPosition(x,y);
		}

		// tell the instance to receive the new entity
		_Instances[aiInstance]->addEntity(dataSetRow,x,y,invisibilityLevel,theEntity);
	}

	void CUniverse::dump(NLMISC::CLog& log)
	{
		log.displayNL("--------------------------------- Start of Ring Vision Universe");
		log.displayNL("- Instance Vector Size: %u",_Instances.size());
		log.displayNL("- Entities Vector Size: %u",_Entities.size());
		log.displayNL("- Delayed Positions List Size: %u",_DelayedPositions.size());
		for (uint32 i=0;i<_Instances.size();++i)
		{
			if (_Instances[i]!=NULL)
			{
				log.displayNL("--------------------------------- Start of Instance %u",i);
				_Instances[i]->dump(log);
			}
		}
		log.displayNL("--------------------------------- End of Ring Vision Universe");
	}


	//-----------------------------------------------------------------------------
	// METHODS CInstance
	//-----------------------------------------------------------------------------

	CInstance::CInstance(CUniverse* theUniverse,uint32 groupId)
	{
		_TheUniverse= theUniverse;
		_GroupId=groupId;
	}

	uint32 CInstance::getGroupId() const
	{
		return _GroupId;
	}

	void CInstance::addEntity(TDataSetRow dataSetRow,uint32 x,uint32 y,TInvisibilityLevel invisibilityLevel,const SUniverseEntity& entity)
	{
		H_AUTO(CInstance_addEntity)

		// setup the field in the entity to say where it is in the instance's _Entities vector
		entity.InstanceIndex= (uint32)_Entities.size();

		// setup the new record for the entity (at the end of the _Entities vector)
		SInstanceEntity& theEntity=	vectAppend(_Entities);
		theEntity.DataSetRow= dataSetRow;

		// call setEntityPosition() and setEntityInvisibilityLevel() to set the entity position and invisibility level 
		setEntityPosition(entity.InstanceIndex,x,y);
		setEntityInvisibility(entity.InstanceIndex,invisibilityLevel);

		// if the entity is a viewer then add them to the viewers vector
		if (entity.ViewerRecord!=NULL)
		{
			// try to add add the entity into an existing vision group (must be within 50 meters of the group)
			bool foundVisionGroup= false;
			for (uint32 i=(uint32)_VisionGroups.size();i--;)
			{
				// if the group is empty then skip it
				if (_VisionGroups[i]==NULL)
					continue;

				// if the group has a different vision level to us then skip it
				if (_VisionGroups[i]->getVisionLevel()!=entity.ViewerRecord->getVisionLevel())
					continue;

				// determine the vision group size if the entity were a member
				uint32 dist= quickDist( x, y,
					(_VisionGroups[i]->xMin()+_VisionGroups[i]->xMax())>>1,
					(_VisionGroups[i]->yMin()+_VisionGroups[i]->yMax())>>1);

				// if the distance is ok then go ahead and add the entityt to the group
				if (dist<MaxVisionGroupDiameter)
				{
					_VisionGroups[i]->addViewer(entity.ViewerRecord);
					foundVisionGroup=true;
					break;
				}
			}

			// no group was found so scan vector for an unused slot to use
			if (!foundVisionGroup)
			{
				for (uint32 i=(uint32)_VisionGroups.size();i--;)
				{
					NLMISC::CSmartPtr<CVisionGroup>& theVision= _VisionGroups[i];
					if (theVision==NULL)
					{
						theVision= new CVisionGroup;
						theVision->setVisionId(i);
					}
					if (theVision->numViewers()==0)
					{
						foundVisionGroup= true;
						theVision->addViewer(entity.ViewerRecord);
						break;
					}
				}
			}

			// if still no group was found then create a new group for the entity
			if (!foundVisionGroup)
			{
				NLMISC::CSmartPtr<CVisionGroup>& theVision= vectAppend(_VisionGroups);
				theVision= new CVisionGroup;
				theVision->setVisionId((uint32)_VisionGroups.size()-1);
				theVision->addViewer(entity.ViewerRecord);
			}
		}
	}

	inline void CInstance::setEntityPosition(uint32 entityIndex,uint32 x,uint32 y)
	{
		if(entityIndex<_Entities.size())
		{
			_Entities[entityIndex].X=x;
			_Entities[entityIndex].Y=y;
		}
		else
		{
			BOMB(NLMISC::toString("BIG BAD BUG - Failed to move entity with invalid index: %d",entityIndex),return);
		}
	}

	inline void CInstance::setEntityInvisibility(uint32 entityIndex,TInvisibilityLevel invisibilityLevel)
	{
		if(entityIndex<_Entities.size())
		{
			if ( VerboseVisionDelta && (_Entities[entityIndex].InvisibilityLevel != invisibilityLevel) )
			{
				nlinfo("Vision Delta: %s: Changes invisibility level from %x to %x",_Entities[entityIndex].DataSetRow.toString().c_str(),_Entities[entityIndex].InvisibilityLevel,invisibilityLevel);
			}

			_Entities[entityIndex].InvisibilityLevel= invisibilityLevel;
		}
		else
		{
			BOMB(NLMISC::toString("BIG BAD BUG - Failed to set invisibilty level for entity with invalid index: %d",entityIndex),return);
		}
	}

	void CInstance::updateVision()
	{
		H_AUTO(CInstance_updateVision)

		// if we have no vision groups then don't waste our time
		if (_VisionGroups.empty())
			return;

		// setup a local index to the used vision group objects
		static std::vector<NLMISC::CSmartPtr<CVisionGroup> > visionGroups;
		visionGroups.clear();

		// run throught the vision groups once updating the bounding coordinates and sort keys
		for (uint32 i=(uint32)_VisionGroups.size();i--;)
		{
			NLMISC::CSmartPtr<CVisionGroup>& theVisionGroup= _VisionGroups[i];

			// skip empty groups
			if (theVisionGroup==NULL)
				continue;
			if (theVisionGroup->numViewers()==0)
			{
				theVisionGroup=NULL;
				continue;
			}

			// update bouding coordinates and sort keys
			theVisionGroup->updateBoundingCoords();
			theVisionGroup->recalcSortKey();

			// add the vision group to the local vision group list (sorting as we go)
			uint32 theSortKey= theVisionGroup->getSortKey();
			uint32 j=(uint32)visionGroups.size();	// point to back entry in vision group
			visionGroups.push_back(NULL);	// create the new space that we're going to fill
			while (j!=0 && visionGroups[j-1]->getSortKey()>theSortKey)
			{
				uint32 oldj= j--;
				visionGroups[oldj]= visionGroups[j];
			}
			visionGroups[j]= theVisionGroup;
		}

		// if no vision groups were found then skip out of instance update
		if (visionGroups.empty())
			return;

		// run through the vision groups to update them
		for (uint32 i=(uint32)visionGroups.size();i--;)
		{
			NLMISC::CSmartPtr<CVisionGroup>& vi= visionGroups[i];

			// if there are no viewers in the group (it was probably merged with another group) then skip 
			if (vi->numViewers()==0)
				continue;

			// attempt to merge with groups that are nearby
			for (uint32 j=i;j--;)
			{
				NLMISC::CSmartPtr<CVisionGroup>& vj= visionGroups[j];

				// stop if the remaining groups are all too far away
				if (vi->getSortKey()-vj->getSortKey()>MaxVisionGroupRadius)
					break;

				if (visionGroupsOverlap(vi,vj))
				{
					// merge the smaller vision object into the larger one
					if (vi->numViewers()>=vj->numViewers())
					{
						vi->merge(vj);
					}
					else
					{
						vj->merge(vi);
					}
					// break out of the loop because one merge per group per cycle is enough
					break;
				}
			}

			// if we need to split the group then do it now...
			while (	quickDist(vi->xMin(),vi->yMin(),vi->xMax(),vi->yMax())> MinVisionGroupSplitDiameter || 
					(	vi->numViewers()>MaxViewersPerGroup &&
						quickDist(vi->xMin(),vi->yMin(),vi->xMax(),vi->yMax())> SmallestBucketSize ) )
			{
				// find a free vision id to use
				uint32 visionSlot;
				for (visionSlot=0;visionSlot<_VisionGroups.size();++visionSlot)
				{
					if (_VisionGroups[visionSlot]==NULL || _VisionGroups[visionSlot]->numViewers()==0)
						break;
				}

				// if no free slot was found in the vision group vector then add a new group
				if (visionSlot==_VisionGroups.size())
					_VisionGroups.push_back(NULL);

				// initialise the new group object
				NLMISC::CSmartPtr<CVisionGroup>& theVision= _VisionGroups[visionSlot];
				theVision= new CVisionGroup;
				theVision->setVisionId(visionSlot);

				// do the splitting
				vi->split(theVision);
				vi->updateBoundingCoords();
			}

			// recalculate the group's vision
			vi->buildVision(_Entities);
			vi->updateViewers(_TheUniverse);
		}
	}

	void CInstance::isolateViewer(SUniverseEntity& entity)
	{
		H_AUTO(CInstance_isolateViewer)

		// if the entity is a viewer...
		CViewer* viewerRecord= &*entity.ViewerRecord;
		if (viewerRecord!=NULL)
		{
			// locate the old vision group (the one we're allocated to before isolation)
			uint32 visionId= viewerRecord->VisionId;
			NLMISC::CSmartPtr<CVisionGroup>& oldVisionGroup= _VisionGroups[visionId];
			BOMB_IF(visionId>=_VisionGroups.size() ||oldVisionGroup==NULL,"Trying to remove entity from vision group with unknown vision id",viewerRecord->VisionId=std::numeric_limits<uint32>::max();return);

			// if we're the only viewer then already isolated so just return
			if (oldVisionGroup->numViewers()==1)
				return;

			// remove from the old vision group
			oldVisionGroup->removeViewer(viewerRecord);

			// find a free vision id to use for the new vision group we're going to create
			uint32 visionSlot;
			for (visionSlot=0;visionSlot<_VisionGroups.size();++visionSlot)
			{
				if (_VisionGroups[visionSlot]==NULL || _VisionGroups[visionSlot]->numViewers()==0)
					break;
			}

			// if no free slot was found in the vision group vector then add a new group
			if (visionSlot==_VisionGroups.size())
				_VisionGroups.push_back(NULL);

			// initialise the new group object
			NLMISC::CSmartPtr<CVisionGroup>& newVisionGroup= _VisionGroups[visionSlot];
			newVisionGroup= new CVisionGroup;
			newVisionGroup->setVisionId(visionSlot);

			// add the viewer into the new group
			newVisionGroup->addViewer(viewerRecord);

			if (VerboseVisionDelta)
			{
				nlinfo("Vision Delta: %s: Isolating viewer from vision group %d to new group %d",viewerRecord->getViewerId().toString().c_str(),oldVisionGroup->getVisionId(),visionSlot);
			}
		}
	}

	void CInstance::removeEntity(SUniverseEntity& entity)
	{
		H_AUTO(CInstance_removeEntity)

		// if the entity is a viewer...
		CViewer const * viewerRecord= &*entity.ViewerRecord;
		if (viewerRecord!=NULL)
		{
			uint32 visionId= viewerRecord->VisionId;
			BOMB_IF(visionId>=_VisionGroups.size() ||_VisionGroups[visionId]==NULL,"Trying to remove entity with unknown vision id",viewerRecord->VisionId=std::numeric_limits<uint32>::max();return);
			_VisionGroups[visionId]->removeViewer(viewerRecord);
		}

		// make sure the entity's InstanceIndex is valid
		uint32 entityIndex= entity.InstanceIndex;
		BOMB_IF(entityIndex>=_Entities.size(),"BIG BAD BUG - Entity's InstanceIndex is invalid!",return);

		// move the entity currently at the back of the vector to the slot that we're liberating (NOTE: this could be us!)
		SInstanceEntity& theEntitySlot= _Entities[entityIndex];
		theEntitySlot=_Entities.back();
		_TheUniverse->getEntity(theEntitySlot.DataSetRow)->InstanceIndex= entityIndex;

		// pop the back element off the entities vector
		_Entities.pop_back();

		// invalidate the InstanceIndex value for the entity we just removed
		entity.InstanceIndex=std::numeric_limits<uint32>::max();
		entity.AIInstance=std::numeric_limits<uint32>::max();
	}

	void CInstance::release()
	{
		// iterate over vision groups getting them to clear their childrens' visions
		for (uint32 i=(uint32)_VisionGroups.size();i--;)
		{
			if (_VisionGroups[i]!=NULL)
			{
				_VisionGroups[i]->release(_TheUniverse);
			}
		}

		// remove all of the entities from the instance
		for (uint32 i=(uint32)_Entities.size();i--;)
		{
			SUniverseEntity* theEntity= _TheUniverse->getEntity(_Entities[i].DataSetRow);
			BOMB_IF(theEntity==NULL,"Failed to locate universe entity corresponding to instance entity in CInstance::realease()",continue);
			removeEntity(*theEntity);
		}
	}

	void CInstance::dump(NLMISC::CLog& log)
	{
		// display a few stats
		log.displayNL("- GroupId: %u",_GroupId);
		log.displayNL("- Entities Vector Size: %u",_Entities.size());
		log.displayNL("- Vision Groups Vector Size: %u",_VisionGroups.size());

		// display all viewable entities in the instance
		for (uint32 i=0;i<_Entities.size();++i)
		{
			log.displayNL("- Entity %u: %s (%d,%d) InvisibilityLevel(%x)",i,_Entities[i].DataSetRow.toString().c_str(),_Entities[i].X,_Entities[i].Y,_Entities[i].InvisibilityLevel);
		}

		// display all vision groups
		for (uint32 i=0;i<_VisionGroups.size();++i)
		{
			log.display("- Group %u: ",i);
			_VisionGroups[i]->dump(log);
		}
	}


	//-----------------------------------------------------------------------------
	// METHODS CVisionGroup
	//-----------------------------------------------------------------------------

	CVisionGroup::CVisionGroup()
	{
		_XMin=std::numeric_limits<uint32>::max();
		_XMax=0;
		_YMin=std::numeric_limits<uint32>::max();
		_YMax=0;
		_VisionId=std::numeric_limits<uint32>::max();
		// setup the dummy entry in the vision buffer
		_Vision.reserve(AllocatedVisionVectorSize);
		vectAppend(_Vision).DataSetRow= ZeroDataSetRow;
	}

	void CVisionGroup::release(CUniverse* theUniverse)
	{
		// clear out vision (leaving only the reserved entry in slot 0)
		_Vision.resize(1);

		// iterate over the viewers to have them update their visions
		updateViewers(theUniverse);
	}

	void CVisionGroup::addViewer(CViewer* viewer)
	{
		// ensure that the viewer wasn't aleady attached to another vision group
		#ifdef NL_DEBUG
		nlassert(viewer!=NULL);
		nlassert(viewer->VisionId==std::numeric_limits<uint32>::max());
		nlassert(viewer->VisionIndex==std::numeric_limits<uint32>::max());
		#endif
		TEST(("add viewer %d to grp %d",viewer->getViewerId().getIndex(),_VisionId));


		// flag the viewer as belongning to this vision, with this vision index
		viewer->VisionId=_VisionId;
		viewer->VisionIndex=(uint32)_Viewers.size();

		// add the new viewer to the viewers vector
		_Viewers.push_back(viewer);

		// update the boudary coordinates of the vision group
		_XMin= std::min(_XMin,viewer->getX());
		_YMin= std::min(_YMin,viewer->getY());
		_XMax= std::max(_XMax,viewer->getX());
		_YMax= std::max(_YMax,viewer->getY());
	}

	void CVisionGroup::removeViewer(const CViewer* viewer)
	{
		// ensure that the viewer was aleady attached to this vision group
		#ifdef NL_DEBUG
		nlassert(viewer!=NULL);
		nlassert(viewer->VisionId==_VisionId);
		nlassert(viewer->VisionIndex<_Viewers.size());
		nlassert(_Viewers[viewer->VisionIndex]==viewer);
		#endif
		TEST(("remove viewer %d from grp %d",viewer->getViewerId().getIndex(),_VisionId));

		// move the back entry in the viewers buffer into our slot (NOTE: the back entry can be us!)
		uint32 visionIndex= viewer->VisionIndex;
		_Viewers[visionIndex]= _Viewers.back();
		_Viewers[visionIndex]->VisionIndex= visionIndex;

		// pop the back entry off the vector and flag oursleves as unused
		_Viewers.pop_back();
		viewer->VisionId= std::numeric_limits<uint32>::max();
		viewer->VisionIndex= std::numeric_limits<uint32>::max();

		// NOTE: after this the boundary may be out of date - this will be recalculated at the next
		// vision update so we don't take time to do it here
	}

	void CVisionGroup::merge(CVisionGroup* other)
	{
		H_AUTO(CVisionGroup_merge)

		#ifdef NL_DEBUG
		nlassert(other!=NULL);
		#endif
		TEST(("merge %d with %d",_VisionId,other->_VisionId));

		// merge the viewer vectors
		for (uint32 i=(uint32)other->_Viewers.size();i--;)
		{
			// get a pointer to the viewer object that we're moving from the other vision group
			NLMISC::CSmartPtr<CViewer>& theViewer= other->_Viewers[i];

			// update the vision id to point to ourselves and the next slot in our viewers vector
			theViewer->VisionId=_VisionId; 
			theViewer->VisionIndex=(uint32)_Viewers.size(); 

			TEST(("moveGroup ent=%d old grp=%d new grp=%d",theViewer->getViewerId().getIndex(),other->_VisionId,_VisionId));

			// push the element onto our viewers vector
			_Viewers.push_back(theViewer);
		}

		// merge the viewer bounding coordinates
		_XMin= std::min(_XMin,other->_XMin);
		_YMin= std::min(_YMin,other->_YMin);
		_XMax= std::max(_XMax,other->_XMax);
		_YMax= std::max(_YMax,other->_YMax);

		// empty out the merged vision group
		other->_Viewers.clear();
		other->_Vision.resize(1); // keep the dummy entry intact at the base of the vision vector
		other->_XMin=	other->_XMax=	other->_YMin=	other->_YMax=	0;
	}

	void CVisionGroup::split(CVisionGroup* other)
	{
		H_AUTO(CVisionGroup_split)

		#ifdef NL_DEBUG
		nlassert(other!=NULL);
		#endif
		TEST(("split  grp %d into grp %d",_VisionId,other->_VisionId));

		// make sure we don't try splitting an empty vision group
		BOMB_IF(_Viewers.empty(),"Attempting to split a vision group with no members",return);

		// setup some handy locals
		uint32 viewerSize= (uint32)_Viewers.size();

		// setup a couple of vectors to hold x coordinates and y coordinates
		static std::vector <uint32> xvect;
		static std::vector <uint32> yvect;
		xvect.resize(viewerSize);
		yvect.resize(viewerSize);

		// run through the viewers, filling in the x and y vectors
		for (uint32 i=viewerSize;i--;)
		{
			const NLMISC::CSmartPtr<CViewer>& theViewer= _Viewers[i];
			xvect[i]=theViewer->getX();
			yvect[i]=theViewer->getY();
		}

		// sort the 2 vectors
		std::sort(xvect.begin(),xvect.end());
		std::sort(yvect.begin(),yvect.end());

		// locate the biggest gap along each axis
		uint32 longestGap=0;
		bool longestIsX=true;
		uint32 cutPos=0;

		// setup a bias to favour splitting in the middle of large evenly spread groups
		sint32 biasDelta=1000;	// a bias of 1 meter per individual who we split off
		sint32 bias= sint32(-biasDelta*viewerSize)>>1;
		sint32 maxBias= -bias;

		// iterate down from entry (viewerSize-1)..1 (skip 0)
		for (uint32 i=viewerSize;--i;)
		{
			// update the bias score
			bias+=biasDelta;
			sint32 absBias= maxBias-abs(bias);

			// calculate distances from entry i-1 to entry i in each of x and y vectors
			uint32 xdist= xvect[i]-xvect[i-1]+absBias;
			uint32 ydist= yvect[i]-yvect[i-1]+absBias;

			// if our xdist is a new record then take note, use it as the new refference
			if (xdist > longestGap)
			{
				longestGap=xdist;
				longestIsX=true;
				cutPos=i;
			}

			// if our ydist is a new record then take note, use it as the new refference
			if (ydist > longestGap)
			{
				longestGap=ydist;
				longestIsX=false;
				cutPos=i;
			}
		}

		// migrate entries over to the new vector
		uint32 splitCount= viewerSize-cutPos;
		other->_Viewers.resize(splitCount);
		if (longestIsX)
		{
			uint32 leftIdx=0, rightIdx=viewerSize-1, otherIdx=0, splitVal= xvect[cutPos];
			while (otherIdx!=splitCount)
			{
				// move elements from the right end of this vector to the other vector
				while (_Viewers[rightIdx]->getX()>= splitVal)
				{
					TEST(("moveGroup ent=%d old grp=%d new grp=%d",_Viewers[rightIdx]->getViewerId().getIndex(),_VisionId,other->_VisionId));

					other->_Viewers[otherIdx]= _Viewers[rightIdx];
					// update the vision index info
					other->_Viewers[otherIdx]->VisionIndex=otherIdx;
					other->_Viewers[otherIdx]->VisionId=other->_VisionId;
					// update the iterators
					otherIdx++;
					rightIdx--;
				}
				// deal with elements off the right end of this vector that need to be re-housed
				while (_Viewers[rightIdx]->getX()< splitVal && otherIdx!=splitCount)
				{
					// skip elements at the left end of this vector that are to be left untouched
					while (_Viewers[leftIdx]->getX()< splitVal)
					{
						++leftIdx;
					}

					TEST(("moveGroup ent=%d old grp=%d new grp=%d",_Viewers[leftIdx]->getViewerId().getIndex(),_VisionId,other->_VisionId));

					// move the element reffered to by leftIdx to the other vector and replace it from the rightIdx
					other->_Viewers[otherIdx]= _Viewers[leftIdx];
					_Viewers[leftIdx]= _Viewers[rightIdx];
					// update the vision index info
					other->_Viewers[otherIdx]->VisionIndex=otherIdx;
					other->_Viewers[otherIdx]->VisionId=other->_VisionId;
					_Viewers[leftIdx]->VisionIndex=leftIdx;
					// update the iterators
					otherIdx++;
					leftIdx++;
					rightIdx--;
				}
			}
			_Viewers.resize(cutPos);
		}
		else
		{
			uint32 leftIdx=0, rightIdx=viewerSize-1, otherIdx=0, splitVal= yvect[cutPos];
			while (otherIdx!=splitCount)
			{
				// move elements from the right end of this vector to the other vector
				while (_Viewers[rightIdx]->getY()>= splitVal)
				{
					other->_Viewers[otherIdx]= _Viewers[rightIdx];
					// update the vision index info
					other->_Viewers[otherIdx]->VisionIndex=otherIdx;
					other->_Viewers[otherIdx]->VisionId=other->_VisionId;
					// update the iterators
					otherIdx++;
					rightIdx--;
				}
				// deal with elements off the right end of this vector that need to be re-housed
				while (_Viewers[rightIdx]->getY()< splitVal && otherIdx!=splitCount)
				{
					// skip elements at the left end of this vector that are to be left untouched
					while (_Viewers[leftIdx]->getY()< splitVal)
					{
						++leftIdx;
					}
					// move the element reffered to by leftIdx to the other vector and replace it from the rightIdx
					other->_Viewers[otherIdx]= _Viewers[leftIdx];
					_Viewers[leftIdx]= _Viewers[rightIdx];
					// update the vision index info
					other->_Viewers[otherIdx]->VisionIndex=otherIdx;
					other->_Viewers[otherIdx]->VisionId=other->_VisionId;
					_Viewers[leftIdx]->VisionIndex=leftIdx;
					// update the iterators
					otherIdx++;
					leftIdx++;
					rightIdx--;
				}
			}
			_Viewers.resize(cutPos);
		}
	}

	void CVisionGroup::updateBoundingCoords()
	{
		// if there are no viewers then stop here and don't waste any more time
		if (_Viewers.empty())
			return;

		// calculate the bouding box for our viewers
		uint32 xmin= std::numeric_limits<uint32>::max();
		uint32 ymin= std::numeric_limits<uint32>::max();
		uint32 xmax= 0;
		uint32 ymax= 0;
		for (uint32 i=(uint32)_Viewers.size();i--;)
		{
			const CViewer& viewer=*_Viewers[i];
			xmin= std::min(xmin,viewer.getX());
			ymin= std::min(ymin,viewer.getY());
			xmax= std::max(xmax,viewer.getX());
			ymax= std::max(ymax,viewer.getY());
		}
		_XMin= xmin;
		_YMin= ymin;
		_XMax= xmax;
		_YMax= ymax;

	}

	void CVisionGroup::buildVision(const TInstanceEntities& entities)
	{
		H_AUTO(CVisionGroup_buildVision)

		// if there are no viewers then stop here and don't waste any more time
		if (_Viewers.empty())
			return;

		// generate the ref coordinates for the centre of our vision group
		uint32 xref= _XMin/2+_XMax/2;	// note - we do divide before add to avoid math errors !!!
		uint32 yref= _YMin/2+_YMax/2;	// note - we do divide before add to avoid math errors !!!

		// build a little set of buckets to sort our entities into
		// these are static purely for reasons of optimisation
		static std::vector<SInstanceEntity const*> buckets(MaxVisionEntries*NumVisionBuckets);
		uint32 bucketIndex[NumVisionBuckets];
		for (uint32 i=0;i<NumVisionBuckets;++i)
		{
			bucketIndex[i]=i;
		}

		// get hold of our vision level
		TInvisibilityLevel visionLevel= getVisionLevel();

		// calculate the distance from each of the entities to our ref position
		for (uint32 i=(uint32)entities.size();i--;)
		{
			// get a refference to the nth entity
			const SInstanceEntity& theEntity= entities[i];

			// make sure the entity is not invisible to us
			if (theEntity.InvisibilityLevel>visionLevel)
			{
				continue;
			}

			// calculate the distance value for this entity
			uint32 dist= quickDist(theEntity.X,theEntity.Y,xref,yref);

			// if this entity is in range then add a pointer to it to our index
			if (dist<=MaxVisionDist)
			{
				// get a bucket index for the entity
				uint32 bucket= dist>>VisionBucketShift;

				// get a refference to the appropriate bucket index
				uint32& theBucketIndex= bucketIndex[bucket];
				if (theBucketIndex>=buckets.size())
					continue;
				buckets[theBucketIndex]= &theEntity;
				theBucketIndex+= NumVisionBuckets;
			}
		}

		// reset the vision vector and add in the dummy entry with DataSetRow=0
		_Vision.resize(AllocatedVisionVectorSize);
		_Vision[0].DataSetRow= ZeroDataSetRow;
		_Vision[0].VisionSlot= std::numeric_limits<uint32>::max(); 

		// setup a vision slot iterator for filling in the vision buffer (=1 to skip passed the dummy entry)
		uint32 nextVisionSlot=1;

		// run through the buckets adding their contents to the vision vector
		for (uint32 i=0;i<NumVisionBuckets;++i)
		{
			uint32& theBucketIndex= bucketIndex[i];
			uint32 numBucketEntries= theBucketIndex/NumVisionBuckets;

			// check whether this bucket fits into the vision vector
			if (numBucketEntries+nextVisionSlot>MaxVisionEntries)
			{
				BOMB_IF(i==0,"ERROR: Vision calculated with >250 entities in first vision bucket!",break);
				break;
			}

			// add the contents of this bucket to the vision vector
			while(numBucketEntries--)
			{
				theBucketIndex-= NumVisionBuckets;
				_Vision[nextVisionSlot++].DataSetRow= buckets[theBucketIndex]->DataSetRow;
			}
		}
		// resize the vision buffer down to fit the size really used
		_Vision.resize(nextVisionSlot);

		// sort the vision buffer
		std::sort(_Vision.begin(),_Vision.end());
	}

	void CVisionGroup::updateViewers(CUniverse* theUniverse)
	{
		// iterate over the viewers, udating their vision
		for (uint32 i=(uint32)_Viewers.size();i--;)
		{
			_Viewers[i]->updateVision(_Vision,theUniverse,false);
		}
	}

	void CVisionGroup::dump(NLMISC::CLog& log)
	{
		// display a few stats
		log.displayNL("VisionId(%u) Min(%d,%d)..Max(%d,%d) sortKey(%u)",_VisionId,_XMin,_YMin,_XMax,_YMax,_SortKey);
		log.displayNL("-- Viewers Vector Size: %u",_Viewers.size());
		log.displayNL("-- Vision Vector Size: %u",_Vision.size());

		// the vector of viewers
		for (uint32 i=0;i<_Viewers.size();++i)
		{
			if (_Viewers[i]==NULL)
				continue;

			log.display("-- Viewer %u: ",i);
			_Viewers[i]->dump(log);
		}

		// the group's vision
		std::string visionString;
		for (uint32 i=0;i<_Vision.size();++i)
		{
			if (!visionString.empty())
				visionString+= ' ';
			visionString+= _Vision[i].DataSetRow.toString();
		}
		log.displayNL("-- Vision: %s",visionString.c_str());
	}

	inline uint32 CVisionGroup::xMin() const 
	{
		return _XMin;
	}

	inline uint32 CVisionGroup::xMax() const 
	{
		return _XMax;
	}

	inline uint32 CVisionGroup::yMin() const 
	{
		return _YMin;
	}

	inline uint32 CVisionGroup::yMax() const 
	{
		return _YMax;
	}

	inline uint32 CVisionGroup::getSortKey() const 
	{
		return _SortKey;
	}

	inline void CVisionGroup::setSortKey(uint32 sortKey)
	{
		_SortKey= sortKey;
	}

	inline void CVisionGroup::recalcSortKey()
	{
		_SortKey= quickDist(0,0,(_XMin+_XMax)/2,(_YMin+_YMax)/2);
	}

	inline uint32 CVisionGroup::numViewers() const
	{
		return (uint32)_Viewers.size();
	}

	inline void CVisionGroup::setVisionId(uint32 visionId)
	{
		_VisionId= visionId;
	}

	inline uint32 CVisionGroup::getVisionId() const
	{
		return _VisionId;
	}

	inline TInvisibilityLevel CVisionGroup::getVisionLevel() const
	{
		if (_Viewers.empty())
			return VISIBLE;
		return (**_Viewers.begin()).getVisionLevel();
	}


	//-----------------------------------------------------------------------------
	// METHODS CViewer
	//-----------------------------------------------------------------------------
	
	CViewer::CViewer()
	{
		VisionId=std::numeric_limits<uint32>::max();
		VisionIndex=std::numeric_limits<uint32>::max();
		_VisionResetCount= 0;
	}

	void CViewer::init(TDataSetRow viewerId,CUniverse* theUniverse)
	{
		// ensure that we don't try to init the same object more than once
		BOMB_IF(_ViewerId!=TDataSetRow(),"Call to init() for already initialised CViewer object",return);

		// setup the viewer id required for filling in vision update messages
		_ViewerId= viewerId;

		// setup the viewed entities vector with reserved slot 0
		_Vision.reserve(AllocatedVisionVectorSize);
		_Vision.resize(1);

		// setup the dummy entry with DataSetRow=0
		_Vision[0].DataSetRow= ZeroDataSetRow;
		_Vision[0].VisionSlot= std::numeric_limits<uint32>::max(); 

		// setup the vision slots in reverse order from 254..0 (because they're popped from the back)
		_FreeVisionSlots.clear();
		_FreeVisionSlots.resize(AllocatedVisionVectorSize-1);
		for (uint32 i=0;i<AllocatedVisionVectorSize-1;++i)
		{
			_FreeVisionSlots[i]= AllocatedVisionVectorSize-2-i;
		}

		// add the viewer themselves to their own visions
		TVision vision(2);
		vision[0].DataSetRow= ZeroDataSetRow;	// the reserved slot
		vision[1].DataSetRow= viewerId;			// the viewer themselves
		updateVision(vision,theUniverse,true);
	}

	void CViewer::_updateVisionBlind(CUniverse* theUniverse,CPlayerVisionDelta& result)
	{
		// if the vision is already empty then just return...
		if (_Vision.size()<=2)
			return;

		uint32 playerVisionEntryIndex=1;
		
		// iterate down from the last vision entry to vision entry 1 (don't touch entry 0 because it's reserved)
		for (uint32 i=(uint32)_Vision.size()-1; i>0; i--)
		{
			// get a refference to the vision entry to be dealt with (removed)
			SViewedEntity& visionEntry= _Vision[i];
			
			// special case - never delete self from vision
			if (visionEntry.VisionSlot==0)
			{
				playerVisionEntryIndex= i;
				continue;
			}
			
			// delete a vision entry that no longer exists
			// add to the 'removed vision entries' result vector
			result.EntitiesOut.push_back(CPlayerVisionDelta::CIdSlot(visionEntry.DataSetRow,(uint8)visionEntry.VisionSlot));
			TEST(("Vision DROP entity %d[%d] x %d for vision reset",_ViewerId.getIndex(),visionEntry.VisionSlot,visionEntry.DataSetRow.getIndex()));

			// if we're in verbose log mode then do a bit of logging
			if (VerboseVisionDelta)
			{
				nlinfo("Vision Delta: %s: Generating empty vision because _VisionResetCounter!=0",_ViewerId.toString().c_str());
			}
			
			// add to the vector of free vision slots
			_FreeVisionSlots.push_back(visionEntry.VisionSlot);
		}

		// only keep 2 slots: slot #1: self , #0: the front-of-vision marker
		_Vision[1]= _Vision[playerVisionEntryIndex];
		_Vision.resize(2);
		
	}

	void CViewer::_updateVisionNormal(const TVision& vision,CUniverse* theUniverse,bool firstTime,CPlayerVisionDelta& result)
	{
		// prepare to apply our group's vision to our own...
		sint32 oldVisionIdx= (sint32)_Vision.size()-1;			// note that there is always a dummy entry at start of vision vector
		sint32 newVisionIdx= (sint32)vision.size()-1;			// note that there is always a dummy entry at start of vision vector
		static std::vector<uint32> newVisionEntries;	// temp vector - static for reasons of optimisation
		newVisionEntries.clear();

		// run through the two sorted vision vectors from top to bottom, stopping when they both reach the bottom
		while ((newVisionIdx|oldVisionIdx)!=0)
		{
			const SViewedEntity& newVisionEntry= vision[newVisionIdx];
			SViewedEntity& oldVisionEntry= _Vision[oldVisionIdx];

			// see whether we have a new entry to add, an unchanged entry to ignore or an out of date entry to delete
			if (newVisionEntry.DataSetRow==oldVisionEntry.DataSetRow)
			{
				// we have an entry that hasn't changed (its in both old and new vision)
				newVisionEntry.VisionSlot= oldVisionEntry.VisionSlot;
				--newVisionIdx;
				--oldVisionIdx;
			}
			else
			{
				if (newVisionEntry.DataSetRow.getIndex()>oldVisionEntry.DataSetRow.getIndex())
				{
					// make sure we don't add ourselves back into our own vision a second time after teleportation etc
					// but do allow us to be added in on the first pass when _Vision is empty (there is always a vision
					// terminator entry so the empty vision case comes when _Vision.size()==1)
					if (newVisionEntry.DataSetRow!=_ViewerId || firstTime)
					{
						// add a new entry to the vision
						// add to a temp vector of new entries that haven't yet had a vision slot allocation
						newVisionEntries.push_back(newVisionIdx);
					}
					else
					{
						// force ourselves into vision slot 0 (where we belong)
						newVisionEntry.VisionSlot= 0;

						if (VerboseVisionDelta)
						{
							nlinfo("Vision Delta: %s: Becomes visible to self",_ViewerId.toString().c_str());
						}
					}
					--newVisionIdx;
				}
				else
				{
					// make sure we never delete ourselves from our own vision...
					uint32 visionSlot= oldVisionEntry.VisionSlot;
					if (visionSlot!=0)
					{
						// delete a vision entry that no longer exists
						// add to the 'removed vision entries' result vector
						result.EntitiesOut.push_back(CPlayerVisionDelta::CIdSlot(oldVisionEntry.DataSetRow,(uint8)visionSlot));
						TEST(("Vision DROP entity %d[%d] x %d (grp %d)",_ViewerId.getIndex(),visionSlot,oldVisionEntry.DataSetRow.getIndex(),VisionId));

						// add to the vector of free vision slots
						_FreeVisionSlots.push_back(visionSlot);
					}
					else
					{
						if (VerboseVisionDelta)
						{
							nlinfo("Vision Delta: %s: Rescuing Vision entry when going invisible",_ViewerId.toString().c_str());
						}
					}
					--oldVisionIdx;
				}
			}
		}

		// calculate the number of free vision slots remaining once we deal with the new vision entries
		sint32 freeVisionSlotOffset= (sint32)(_FreeVisionSlots.size()-newVisionEntries.size());
		BOMB_IF(freeVisionSlotOffset<0,"BIG BAD BUG generating vision ... too few free vision slots found",newVisionEntries.resize(_FreeVisionSlots.size()));

		// deal with new vision entries
		for (uint32 i= (uint32)newVisionEntries.size();i--;)
		{
			// derefference the new vision entry
			const SViewedEntity& visionEntry= vision[newVisionEntries[i]];

			// lookup the next free vision slot
			visionEntry.VisionSlot= _FreeVisionSlots[freeVisionSlotOffset+i];

			// add the new entry to the result vector
			result.EntitiesIn.push_back(CPlayerVisionDelta::CIdSlot(visionEntry.DataSetRow,(uint8)visionEntry.VisionSlot));
			TEST(("Vision ADD entity %d[%d] = %d (grp %d)",_ViewerId.getIndex(),visionEntry.VisionSlot,visionEntry.DataSetRow.getIndex(),VisionId));
		}

		// resize down the free slots vector to deal with the slots we just allocated
		_FreeVisionSlots.resize(freeVisionSlotOffset);

		// update our vision vector
		_Vision= vision;
	}

	void CViewer::updateVision(const TVision& vision,CUniverse* theUniverse,bool firstTime)
	{
		H_AUTO(CViewer_UpdateVision)

		// we conserve a static variable that we use for building our results in as it saves time
		static CPlayerVisionDelta result;
		result.reset(_ViewerId);

		// make sure we've been properly initialised
		BOMB_IF(_ViewerId==TDataSetRow(),"Call to updateVision() for uninitialised CViewer object",return);

		// if we're resting the player vision then treat ourselves as blind unless this is the first update in which case we need to
		// be allowed to add ourselves into our own vision
		if (_VisionResetCount!=0 && !firstTime)
		{
			// if we're in verbose log mode then do a bit of logging
			if (VerboseVisionDelta)
			{
				nlinfo("Vision Delta: %s: Generating empty vision because _VisionResetCounter!=0",_ViewerId.toString().c_str());
			}
						
			// decrement the count of vision iterations still to be skipped to complete this reset
			--_VisionResetCount;
			
			// perform a vision update that clears out all vision contents other than self
			_updateVisionBlind(theUniverse,result);
		}
		else
		{
			// perform a normal vision update
			_updateVisionNormal(vision,theUniverse,firstTime,result);
		}

		// if the vision delta isn't empty then dispatch it
		if (!result.empty())
		{
			// if we're in verbose log mode then do a bit of logging
			if (VerboseVisionDelta)
			{
				nlinfo("Vision Delta: %s: %s",_ViewerId.toString().c_str(),result.toString().c_str());
			}
			
			// add our result to the messages to be dispatched to the FES at end of tick...
			theUniverse->addVisionDelta(result);
		}
	}

	void CViewer::dump(NLMISC::CLog& log)
	{
		// display a few stats
		log.displayNL("%s (%d,%d) VisionLvl(%u)",_ViewerId.toString().c_str(),_X,_Y,_VisionLevel);

		// the entity's vision
		std::string visionString;
		for (uint32 i=0;i<_Vision.size();++i)
		{
			if (!visionString.empty())
				visionString+= ' ';
			visionString+= NLMISC::toString("[%u]",_Vision[i].VisionSlot)+ _Vision[i].DataSetRow.toString();
		}
		log.displayNL("--- Vision: %s",visionString.c_str());

		// the entity's free vision slots
		std::string freeVisionSlotsString;
		for (uint32 i=(uint32)_FreeVisionSlots.size();i--;)
		{
			if (!freeVisionSlotsString.empty())
				freeVisionSlotsString+= ' ';
			freeVisionSlotsString+= NLMISC::toString("%u",_FreeVisionSlots[i]);
		}
		log.displayNL("--- Free Vision Slots: %s",visionString.c_str());
	}

	inline TDataSetRow CViewer::getViewerId() const
	{
		return _ViewerId;
	}

	inline uint32 CViewer::getViewerIdx() const
	{
		return _ViewerId.getIndex();
	}

	inline void CViewer::setEntityPosition(uint32 x, uint32 y)
	{
		_X=x;
		_Y=y;
	}

	inline uint32 CViewer::getX() const
	{
		return _X;
	}

	inline uint32 CViewer::getY() const
	{
		return _Y;
	}

	inline void CViewer::forceResetVision()
	{
		// if we're not already resetting vision...
		if (_VisionResetCount == 0)
		{
			// setup the vision counter to reset vision for next n updates in order to allow 
			// FES time to clear out player vision
			_VisionResetCount = VisionResetDuration;
		}
	}

	inline void CViewer::setVisionLevel(TInvisibilityLevel visionLevel)
	{
		_VisionLevel= visionLevel;
	}

	inline TInvisibilityLevel CViewer::getVisionLevel() const
	{
		return _VisionLevel;
	}

} // namespace R2_VISION
