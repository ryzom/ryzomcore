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

#ifndef R2_VISION_H
#define R2_VISION_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"

#include "game_share/base_types.h"
#include "game_share/player_vision_delta.h"
#include "r2_variables.h"


namespace R2_VISION
{
	//-----------------------------------------------------------------------------------------------------------------
	// forward declarations
	//-----------------------------------------------------------------------------------------------------------------

	class	CUniverse;			// the root container
	class	CInstance;			// an ai instnce container within a CUniverse
	class	CVisionGroup;		// a vision group object within a CInstance
	class	CViewer;			// a viewer entity record within a CVisionGroup (can migrate from one vision group to another)

	struct	SUniverseEntity;	// an entity record within a CUniverse
	struct	SInstanceEntity;	// an entity record within a CInstance
	struct	SViewedEntity;		// an element of the vision descriptions of both CVisionGroup and CViewer

	struct	SDelayedPlayerPosition;	// a teleport position for a player to be applied with a delay

	class	IVisionDeltaManager;	// an interface class for defining a callback object that manages vision deltas

	//-----------------------------------------------------------------------------------------------------------------
	// constants for visibility / invisibility management
	//-----------------------------------------------------------------------------------------------------------------

	// the following invisibility levels are stored in the "WHO_SEES_ME" mirror property
	enum TInvisibilityLevel
	{
		// the basic invisibility levels
		BAD_VALUE,
		VISIBLE,			// for players +(dms, ems, gms, sgms, dev)
		INVISIBLE_MOB,		// for super players +(dms, ems, gms, sgms, dev)
		INVISIBLE_PLAYER,	// for dms +(ems, gms, sgms, dev)
		INVISIBLE_DM,		// for ems +(gms, sgms, dev)
		INVISIBLE_EG,		// for gms +(sgms, dev)
		INVISIBLE_SG,		// for gms +(sgms, dev)
		INVISIBLE_VG,		// for gms +(sgms, dev)
		INVISIBLE_EM,		// for gms +(sgms, dev)
		INVISIBLE_GM,		// for sgms +(dev)
		INVISIBLE_SGM,		// for dev
		INVISIBLE_DEV,		// for dev

		// the number of bits used to store the invisibility level
		NUM_WHOSEESME_BITS= 4,

		// presets defining who I can see and who sees me (who I can see * 16 + who sees me)
		WHOSEESME_VISIBLE_MOB=	 VISIBLE,
		WHOSEESME_INVISIBLE_MOB= INVISIBLE_MOB,

		WHOSEESME_VISIBLE_PLAYER=	(VISIBLE<<NUM_WHOSEESME_BITS) | VISIBLE,
		WHOSEESME_INVISIBLE_PLAYER= (VISIBLE<<NUM_WHOSEESME_BITS) | INVISIBLE_PLAYER,

		WHOSEESME_VISIBLE_DM=	 (INVISIBLE_DM<<NUM_WHOSEESME_BITS) | VISIBLE,
		WHOSEESME_INVISIBLE_DM=  (INVISIBLE_DM<<NUM_WHOSEESME_BITS) | INVISIBLE_DM,

		WHOSEESME_VISIBLE_EG=	 (INVISIBLE_SG<<NUM_WHOSEESME_BITS) | VISIBLE,
		WHOSEESME_INVISIBLE_EG=  (INVISIBLE_SG<<NUM_WHOSEESME_BITS) | INVISIBLE_EG,

		WHOSEESME_VISIBLE_SG=	 (INVISIBLE_SG<<NUM_WHOSEESME_BITS) | VISIBLE,
		WHOSEESME_INVISIBLE_SG=  (INVISIBLE_SG<<NUM_WHOSEESME_BITS) | INVISIBLE_SG,

		WHOSEESME_VISIBLE_VG=	 (INVISIBLE_VG<<NUM_WHOSEESME_BITS) | VISIBLE,
		WHOSEESME_INVISIBLE_VG=  (INVISIBLE_VG<<NUM_WHOSEESME_BITS) | INVISIBLE_VG,

		WHOSEESME_VISIBLE_EM=	 (INVISIBLE_GM<<NUM_WHOSEESME_BITS) | VISIBLE,
		WHOSEESME_INVISIBLE_EM=  (INVISIBLE_GM<<NUM_WHOSEESME_BITS) | INVISIBLE_EM,

		WHOSEESME_VISIBLE_GM=	 (INVISIBLE_GM<<NUM_WHOSEESME_BITS) | VISIBLE,
		WHOSEESME_INVISIBLE_GM=  (INVISIBLE_GM<<NUM_WHOSEESME_BITS) | INVISIBLE_GM,

		WHOSEESME_VISIBLE_SGM=	 (INVISIBLE_SGM<<NUM_WHOSEESME_BITS) | VISIBLE,
		WHOSEESME_INVISIBLE_SGM= (INVISIBLE_SGM<<NUM_WHOSEESME_BITS) | INVISIBLE_SGM,

		WHOSEESME_VISIBLE_DEV=	 (INVISIBLE_DEV<<NUM_WHOSEESME_BITS) | VISIBLE,
		WHOSEESME_INVISIBLE_DEV= (INVISIBLE_DEV<<NUM_WHOSEESME_BITS) | INVISIBLE_DEV,

	};


	//-----------------------------------------------------------------------------------------------------------------
	// STRUCT SInstanceEntity
	//-----------------------------------------------------------------------------------------------------------------
	
	struct SInstanceEntity
	{
		TDataSetRow	DataSetRow;	// mirror row for this entity
		uint32	X;				// x coord of this entity
		uint32	Y;				// y coord of this entity
		TInvisibilityLevel InvisibilityLevel; // value used to determine which groups can see me

		// ctor
		SInstanceEntity()
		{
			X=0;
			Y=0;
			InvisibilityLevel= VISIBLE;
		}
	};

	//-----------------------------------------------------------------------------------------------------------------
	// STRUCT SViewedEntity
	//-----------------------------------------------------------------------------------------------------------------

	struct SViewedEntity
	{
				TDataSetRow	DataSetRow;
		mutable uint32		VisionSlot;	// this is mutable to allow for easy use in CViewer::updateVision()

		// operator required for std::sort() routine to work
		bool operator<(const SViewedEntity& other) const
		{
			return DataSetRow.getIndex()< other.DataSetRow.getIndex();
		}

		// ctor
		SViewedEntity()
		{
			VisionSlot=std::numeric_limits<uint32>::max();
		}
	};

	//-----------------------------------------------------------------------------------------------------------------
	// type declarations
	//-----------------------------------------------------------------------------------------------------------------

	// a vector of SInstanceEntity - as managed by a CInstance object
	typedef std::vector<SInstanceEntity> TInstanceEntities;

	// a vision vector as managed by both CVisionGroup and CViewer objects
	typedef std::vector<SViewedEntity> TVision;


	inline bool isEntityVisibleToPlayers(const uint64& whoSeesMeVal)
	{
		if (IsRingShard)
		{
			if ((uint32)whoSeesMeVal==0xffffffffu)
				return true;
			return (((uint32)whoSeesMeVal)&((1<<NUM_WHOSEESME_BITS)-1))==VISIBLE;
		}
		else
		{
			return ((uint32)whoSeesMeVal)!=0;
		}
	}

	inline bool isEntityVisibleToMobs(const uint64& whoSeesMeVal)
	{
		return (uint32)(whoSeesMeVal>>32)!=0;
	}

	inline uint64 buildWhoSeesMe(TInvisibilityLevel playerInvisibiltyLevel,bool visibleToMobs)
	{
		return (visibleToMobs?UINT64_CONSTANT(0xffffffff00000000):UINT64_CONSTANT(0x0000000000000000)) | (uint64)playerInvisibiltyLevel;
	}

	inline uint32 extractVisionLevel(const uint64& whoSeesMeVal)
	{
		return ((uint32)whoSeesMeVal) >> NUM_WHOSEESME_BITS;		
	}

	inline uint32 extractInvisibilityLevel(const uint64& whoSeesMeVal)
	{
		return ((uint32)whoSeesMeVal) & ((1<<NUM_WHOSEESME_BITS)-1);		
	}

	//-----------------------------------------------------------------------------------------------------------------
	// STRUCT SUniverseEntity
	//-----------------------------------------------------------------------------------------------------------------
	
	struct SUniverseEntity
	{
		TDataSetRow DataSetRow;						// the complete data set row for the entity
		uint32 AIInstance;							// the id of the instance that we're currently in (std::numeric_limits<uint32>::max() by default)
		mutable uint32 InstanceIndex;				// the index within the instance's _Entities vector
		NLMISC::CSmartPtr<CViewer> ViewerRecord;	// pointer to the CViewer record for viewers (or NULL)

		// ctor
		SUniverseEntity()
		{
			AIInstance=std::numeric_limits<uint32>::max();
			InstanceIndex=std::numeric_limits<uint32>::max();
		}
	};

	
	//-----------------------------------------------------------------------------------------------------------------
	// CLASS CUniverse
	//-----------------------------------------------------------------------------------------------------------------

	class CUniverse: public NLMISC::CRefCount
	{
	public:
		// create a new AIInstance with the given Id
		// - instance Id must be a positive number in range 0..65535
		// - group id is arbitrary - generally use the id of the service responsible for this instance
		// BOMB_IF( the instance already exists )
		void createInstance(uint32 aiInstance, uint32 groupId);

		// destroy the AIInstance with the given Id
		// WARN_IF( the instance doesn't exist )
		void removeInstance(uint32 id);

		// destroy all AIInstance with the given group Id
		// - used generally on serviceDown of a service that manages instances
		void removeInsancesByGroup(uint32 groupId);

		// add an entity to the universe and assign them to an AIInstance
		// create a new viewer record if necessary
		// if an entity already existed with the same id it's removed cleanly before the add operation begins
		void addEntity(TDataSetRow dataSetRow,uint32 aiInstance,sint32 x,sint32 y,TInvisibilityLevel invisibilityLevel,bool isViewer);

		// remove an entity from the universe, whether a viewer or not
		void removeEntity(TDataSetRow dataSetRow);

		// get hod of a SUniverseEntity record for an entity from its dataset row
		SUniverseEntity* getEntity(TDataSetRow dataSetRow);
		SUniverseEntity* getEntity(uint32 row);

		// set the coordinates of a pre-existing entity
		void setEntityPosition(TDataSetRow dataSetRow,sint32 x,sint32 y);
		void setEntityPositionDelayed(TDataSetRow dataSetRow,sint32 x,sint32 y,uint32 ticks);

		// set the coordinates and change the aiInstance assignment for a pre-existing entity
		void teleportEntity(TDataSetRow dataSetRow,uint32 aiInstance,sint32 x,sint32 y,TInvisibilityLevel invisibilityLevel);
		
		// set the 'whoSeesMe property for an entity
		void setEntityInvisibilityInfo(TDataSetRow dataSetRow,uint32 whoSeesMe);

		// register an object to manage vision deltas
		void registerVisionDeltaManager(IVisionDeltaManager* manager);

		// add a vision delta record to the messages to be sent to the front ends
		void addVisionDelta(const CPlayerVisionDelta& visionDelta);

		// perform an update cycle, recalculate a sub-set of the universe's viewers' visions
		void update();

		// dump state info to the given log for debugging purposes
		void dump(NLMISC::CLog& log);

		// force the refresh of the vision
		void forceRefreshVision(TDataSetRow dataSetRow);

		// clear out the vision for an entity for a certain number of iterations to allow FES to clean vision buffers
		void forceResetVision(TDataSetRow dataSetRow);

	private:
		// private implementations of above public routines - validity checks on parameters are skipped
		// as they are assumed to have been carried out in the public interface
		void _addEntity(TDataSetRow dataSetRow,uint32 aiInstance,sint32 x,sint32 y,TInvisibilityLevel invisibilityLevel,bool isViewer);
		void _removeEntity(TDataSetRow dataSetRow);
		void _setEntityPosition(TDataSetRow dataSetRow,sint32 x,sint32 y);
		void _teleportEntity(TDataSetRow dataSetRow,uint32 aiInstance,sint32 x,sint32 y,TInvisibilityLevel invisibilityLevel);
		

	private:
		std::vector<NLMISC::CSmartPtr<CInstance> >	_Instances;	// the instances in the service (vector index corresponds to AIInstance ids)
		std::vector<SUniverseEntity>				_Entities;	// basic info for all of the entities in the universe

		typedef std::list<SDelayedPlayerPosition>	TDelayedPositions;
		TDelayedPositions							_DelayedPositions;

		NLMISC::CRefPtr<IVisionDeltaManager>		_VisionDeltaManager;	// a callback object used to treat vision deltas
	};


	//-----------------------------------------------------------------------------------------------------------------
	// CLASS CInstance
	//-----------------------------------------------------------------------------------------------------------------

	class CInstance: public NLMISC::CRefCount
	{
	public:
		// ctor
		CInstance(CUniverse* theUniverse,uint32 groupId);

		// read accessor for group Id
		uint32 getGroupId() const;

		// add an entity to this instance (either a new entity or a move from another instance)
		// if the entity was attached to another instance on entry to this routine it is detached cleanly
		void addEntity(TDataSetRow dataSetRow,uint32 x,uint32 y,TInvisibilityLevel invisibilityLevel,const SUniverseEntity& entity);

		// remove an entity from this instance (either to add to another instance or to destroy)
		void removeEntity(SUniverseEntity& entity);

		// remove a viewer from their current vision group and create a new vision group for them to use
		// this method is used after viewer visionLevel changes
		void isolateViewer(SUniverseEntity& entity);

		// set the position of an entity already assigned to this instance
		void setEntityPosition(uint32 entityIndex,uint32 x,uint32 y);

		// set the invisibility level of an entity already assigned to this instance
		void setEntityInvisibility(uint32 entityIndex,TInvisibilityLevel invisibilityLevel);

		// update the vision of all of the viewers in this instance
		void updateVision();

		// release method for doing house keeping before destruction of the instance
		void release();

		// dump state info to the given log for debugging purposes
		void dump(NLMISC::CLog& log);

	private:
		uint32											_GroupId;		// id of group to which instance belongs
		CUniverse*										_TheUniverse;	// pointer to our parent object
		TInstanceEntities								_Entities;		// the positions cache for all of the entities in the instance
		std::vector<NLMISC::CSmartPtr<CVisionGroup> >	_VisionGroups;	// all of the vision records in the instance
	};


	//-----------------------------------------------------------------------------------------------------------------
	// CLASS CVisionGroup
	//-----------------------------------------------------------------------------------------------------------------
	
	class CVisionGroup: public NLMISC::CRefCount
	{
	public:
		// ctor
		CVisionGroup();

		// release - called just before instance destruction to allow us to clear out player visions
		void release(CUniverse* theUniverse);

		// add a viewer to the vision group
		void addViewer(CViewer* viewer);

		// remove a viewer from the vision group
		void removeViewer(const CViewer* viewer);

		// merge the viewers from another vision group into this one
		void merge(CVisionGroup* other);

		// split the viewers from this group into 2, the second part going into the 'other' CVisionGroup object
		void split(CVisionGroup* other);

		// rebuild the vision list for this vision group
		void buildVision(const TInstanceEntities& entities);

		// update the visions of all viewers
		void updateViewers(CUniverse* theUniverse);

		// accessors for X and Y values
		void updateBoundingCoords();
		uint32 xMin() const;
		uint32 xMax() const;
		uint32 yMin() const;
		uint32 yMax() const;

		// we use a distance from the origin to the centre of the vision unit as a sort key for update() operations
		// a few accessors...
		uint32 getSortKey() const;
		void setSortKey(uint32 sortKey);
		void recalcSortKey();

		// accessors for the viewers list
		uint32 numViewers() const;

		// accessor to get the vision level used for invisibility evaluation
		TInvisibilityLevel getVisionLevel() const;

		// accessors for vision id
		void setVisionId(uint32 visionId);
		uint32 getVisionId() const;

		// dump state info to the given log for debugging purposes
		void dump(NLMISC::CLog& log);

	private:
		// bounding coordinates for viewers
		uint32 _XMin;
		uint32 _XMax;
		uint32 _YMin;
		uint32 _YMax;
		uint32 _SortKey;	// calculated as quickDist from 0,0 to vision group center

		// the index of this object within it's parent vector
		uint32 _VisionId;

		// the vector of viewers
		typedef std::vector<NLMISC::CSmartPtr<CViewer> > TViewers;
		TViewers _Viewers;

		// a sorted vector of ids of entities in vision
		TVision _Vision;
	};


	//-----------------------------------------------------------------------------------------------------------------
	// CLASS CViewer
	//-----------------------------------------------------------------------------------------------------------------
	
	class CViewer: public NLMISC::CRefCount
	{
	public:
		// ctor
		CViewer();

		// set the dataSetRow for the viewer - required for generating vision delta packets
		void init(TDataSetRow viewerId,CUniverse* theUniverse);

		// update the vision of this entity from the vision defined in visionGroup
		void updateVision(const TVision& vision,CUniverse* theUniverse,bool firstTime);

		// read accessor for _ViewerId
		TDataSetRow getViewerId() const;
		uint32 getViewerIdx() const;

		// position accessors
		void setEntityPosition(uint32 x, uint32 y);
		uint32 getX() const;
		uint32 getY() const;

		// utility method to force vision reset at next update - used to re-initialise vision after editing
		inline void forceResetVision();

		// vision level accessors
		void setVisionLevel(TInvisibilityLevel visionLevel);
		TInvisibilityLevel getVisionLevel() const;

		// dump state info to the given log for debugging purposes
		void dump(NLMISC::CLog& log);

	public:
		// index variables manipulated by external classes to assist in rapid management of deletion operations
		mutable uint32	VisionId;			// the id of the Instance's vision group to which you're assigned
		mutable uint32	VisionIndex;		// the index within the vision group's _Viewers vector

	private:
		// private methods called by updateVision()
		void _updateVisionBlind(CUniverse* theUniverse,CPlayerVisionDelta& result);
		void _updateVisionNormal(const TVision& vision,CUniverse* theUniverse,bool firstTime,CPlayerVisionDelta& result);
		
	
	private:
		// private types
		typedef std::vector<uint32>			TFreeVisionSlots;

		// private data
		uint32				_X;
		uint32				_Y;
		TDataSetRow			_ViewerId;			// this is a datasetrow value used to compose vision messages
		uint32				_VisionResetCount;	// flag meaning that at next update vision should be reset
		TInvisibilityLevel	_VisionLevel;		// value used to determine which entities are too invisible to see
		TVision				_Vision;			// a sorted vector of ids of entities in vision
		TFreeVisionSlots	_FreeVisionSlots;	// a vector of unused vision slots
	};






	//-----------------------------------------------------------------------------------------------------------------
	// STRUCT SDelayedPlayerPosition
	//-----------------------------------------------------------------------------------------------------------------

	struct SDelayedPlayerPosition
	{
		NLMISC::TGameCycle GameCycle;
		TDataSetRow	DataSetRow;
		sint32 X;
		sint32 Y;

		// ctor - default
		SDelayedPlayerPosition()
		{
			X= Y= 0;
		}

		// ctor - parameterised
		SDelayedPlayerPosition(NLMISC::TGameCycle gameCycle,TDataSetRow dataSetRow, sint32 x, sint32 y)
		{
			X= x;
			Y= y;
			DataSetRow= dataSetRow;
			GameCycle= gameCycle;
		}
	};


	//-----------------------------------------------------------------------------------------------------------------
	// CLASS IVisionDeltaManager
	//-----------------------------------------------------------------------------------------------------------------

	class IVisionDeltaManager: public NLMISC::CRefCount
	{
	public:
		// a virtual destructor (for good practice)
		virtual ~IVisionDeltaManager() {}

		// this method is called back each time a new vision delta record is generated
		virtual void addVisionDelta(const CPlayerVisionDelta& visionDelta) =0;
	};

} // namespace R2_VISION

#endif
