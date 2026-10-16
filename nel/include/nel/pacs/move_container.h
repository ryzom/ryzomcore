// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_MOVE_CONTAINER_H
#define NL_MOVE_CONTAINER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/pool_memory.h"
#include "move_cell.h"
#include "collision_ot.h"
#include "nel/pacs/u_move_container.h"
#include "collision_surface_temp.h"

#define NELPACS_CONTAINER_TRIGGER_DEFAULT_SIZE 100

namespace NLPACS
{

class CMovePrimitive;
class CMoveElement;
class CGlobalRetriever;
class CPrimitiveWorldImage;

/**
 * A container for movable objects
 * Some constraints:
 * * The move bounding box must be lower than the cell size
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */

class CMoveContainer: public UMoveContainer
{
	friend class CMovePrimitive;
	friend class CPrimitiveWorldImage;
public:
	/// Constructor
	CMoveContainer (double xmin, double ymin, double xmax, double ymax, uint widthCellCount, uint heightCellCount, double primitiveMaxSize,
		uint8 numWorldImage, uint maxIteration, uint otSize)
	{
		init (xmin, ymin, xmax, ymax, widthCellCount, heightCellCount, primitiveMaxSize, numWorldImage, maxIteration, otSize);
	}

	/// Init the container with a global retriever
	CMoveContainer (CGlobalRetriever* retriever, uint widthCellCount, uint heightCellCount, double primitiveMaxSize,
		uint8 numWorldImage, uint maxIteration, uint otSize)
	{
		init (retriever, widthCellCount, heightCellCount, primitiveMaxSize, numWorldImage, maxIteration, otSize);
	}

	/// Destructor
	virtual ~CMoveContainer ();

	/// Init the container without global retriever
	void init (double xmin, double ymin, double xmax, double ymax, uint widthCellCount, uint heightCellCount, double primitiveMaxSize,
		uint8 numWorldImage, uint maxIteration, uint otSize);

	/// Init the container with a global retriever
	void init (CGlobalRetriever* retriever, uint widthCellCount, uint heightCellCount, double primitiveMaxSize,
		uint8 numWorldImage, uint maxIteration, uint otSize);

	/// Add a collisionable primitive in the container. Return the pointer on the primitive.
	UMovePrimitive*				addCollisionablePrimitive (uint8 firstWorldImage, uint8 numWorldImage, const UMovePrimitive *copyFrom = NULL);

	/// Add a noncollisionable primitive in the container. Return the pointer on the primitive.
	UMovePrimitive*				addNonCollisionablePrimitive (const UMovePrimitive *copyFrom = NULL);

	/// Load a block of collisionable primitive
	bool						loadCollisionablePrimitiveBlock (const char *filename, uint8 firstWorldImage, uint8 numWorldImage, std::vector<UMovePrimitive*> *primitives, float orientation, const NLMISC::CVector &position, bool dontSnapToGround = false);

	/// Add a block of collsionnable primitives
	void						addCollisionnablePrimitiveBlock(UPrimitiveBlock *pb, uint8 firstWorldImage, uint8 numWorldImage, std::vector<UMovePrimitive*> *primitives, float orientation, const NLMISC::CVector &position, bool dontSnapToGround = false, const NLMISC::CVector &scale = NLMISC::CVector(1.0f, 1.0f, 1.0f));

	/// Set world image as static world image.
	void						setAsStatic (uint8 worldImage);

	/// Duplicate world image
	void						duplicateWorldImage (uint8 source, uint8 dest);

	/// Remove a primitive from the container.
	void						removePrimitive (UMovePrimitive* primitive);

	/// Evaluation of the collision system
	void						evalCollision (double deltaTime, uint8 worldImage);

	// Evaluation of collision for one non-collisionable primitive
	bool						evalNCPrimitiveCollision (double deltaTime, UMovePrimitive *primitive, uint8 worldImage);

	/// Make a move test
	bool						testMove (UMovePrimitive* primitive, const NLMISC::CVectorD& speed, double deltaTime, uint8 worldImage,
											NLMISC::CVectorD *contactNormal);

	/// Allocate a move element
	CMoveElement				*allocateMoveElement ();

	/// Free move element
	void						freeMoveElement (CMoveElement *element);

	/// Get the retriever
	CGlobalRetriever			*getGlobalRetriever() const
	{
		return _Retriever;
	}

	/// Get number of trigger information
	uint						getNumTriggerInfo() const
	{
		return (uint)_Triggers.size();
	}

	/// Get the n-th trigger information
	const UTriggerInfo			&getTriggerInfo (uint id) const
	{
		// check
		nlassert (id<_Triggers.size());

		return _Triggers[id];
	}

	/// Get all the primitives in the container
	virtual	void				getPrimitives(std::vector<const UMovePrimitive *> &dest) const;

private:
	/// Current test time
	uint32						_TestTime;

	/// Max test iterations
	uint32						_MaxTestIteration;

	/// Set of primitives
	std::set<CMovePrimitive*>	_PrimitiveSet;

	/// Root of modified primitive for each world image
	std::vector<CMovePrimitive*>	_ChangedRoot;

	/// Set of primitives
	std::set<uint8>				_StaticWorldImage;

	/// The time ordered table size
	uint						_OtSize;

	/// The time ordered table
	std::vector<CCollisionOT>	_TimeOT;

	/// Previous collision node in the OT
	CCollisionOT				*_PreviousCollisionNode;

	/// Current deltaTime
	double						_DeltaTime;

	/// Max primitive size
	double						_PrimitiveMaxSize;

	/// Area size
	double						_Xmin;
	double						_Ymin;
	double						_Xmax;
	double						_Ymax;

	/// Cells width and height
	double						_CellWidth;
	double						_CellHeight;

	/// Cells count
	uint						_CellCountWidth;
	uint						_CellCountHeight;

	/// Cells array
	std::vector<std::vector<CMoveCell> >		_VectorCell;

	/// Retriver pointner
	CGlobalRetriever			*_Retriever;
	CCollisionSurfaceTemp		_SurfaceTemp;

	/// Memory manager for CCollisionOTInfo
	std::vector <UTriggerInfo>						_Triggers;
	NLMISC::CPoolMemory<CCollisionOTDynamicInfo>	_AllocOTDynamicInfo;
	NLMISC::CPoolMemory<CCollisionOTStaticInfo>		_AllocOTStaticInfo;

private:

	// Clear the container
	void						clear ();

	// Update modified primitives bounding box
	void						updatePrimitives (double deltaTime, uint8 worldImage);

	// Update cells list for this primitive
	void						updateCells (CMovePrimitive *primitive, uint8 worldImage);

	// Fill the elementArray descriptor.
	void						getCells (CMovePrimitive *primitive, uint8 worldImage, uint8 primitiveWorldImage, CMoveElement **elementArray);


	// Clear the time ordered table
	void						clearOT ();

	// Check the OT is cleared and linked
	void						checkOT ();

	// Eval one terrain collision
	bool						evalOneTerrainCollision (double beginTime, CMovePrimitive *primitive, uint8 primitiveWorldImage,
															bool testMove, bool &testMoveValid, CCollisionOTStaticInfo *staticColInfo,
															NLMISC::CVectorD *contactNormal);

	// Eval one primitive collision
	bool						evalOnePrimitiveCollision (double beginTime, CMovePrimitive *primitive, uint8 worldImage,
													uint8 primitiveWorldImage, bool testMove, bool secondIsStatic,
													bool &testMoveValid, CCollisionOTDynamicInfo *dynamicColInfo,
													NLMISC::CVectorD *contactNormal);

	// Eval final step
	bool						evalPrimAgainstPrimCollision (double beginTime, CMovePrimitive *primitive, CMovePrimitive *otherPrimitive,
													CPrimitiveWorldImage *wI, CPrimitiveWorldImage *otherWI, bool testMove,
													uint8 firstWorldImage, uint8 secondWorldImage, bool secondIsStatic,
													CCollisionOTDynamicInfo *dynamicColInfo,
													NLMISC::CVectorD *contactNormal);

	// Eval all collision for modified primitives
	void						evalAllCollisions (double beginTime, uint8 worldImage);

	// Add a collision in the time ordered table
	void						newCollision (CMovePrimitive* first, CMovePrimitive* second, const CCollisionDesc& desc,
												bool collision, bool enter, bool exit, bool inside, uint firstWorldImage, uint secondWorldImage,
												bool secondIsStatic, CCollisionOTDynamicInfo *dynamicColInfo);

	// Add a collision in the time ordered table
	void						newCollision (CMovePrimitive* first, const CCollisionSurfaceDesc& desc, uint8 worldImage, double beginTime,
												CCollisionOTStaticInfo *staticColInfo);

	// Add a trigger in the trigger array
	void						newTrigger (CMovePrimitive* first, CMovePrimitive* second, const CCollisionDesc& desc, uint triggerType);

	// Clear modified primitive list
	void						clearModifiedList (uint8 worldImage);

	// Remove modified primitive from time ordered table
	void						removeModifiedFromOT (uint8 worldImage);

	// Check sorted list
	void						checkSortedList ();

	// Allocate ordered table info
	CCollisionOTDynamicInfo		*allocateOTDynamicInfo ();

	// Allocate ordered table info
	CCollisionOTStaticInfo		*allocateOTStaticInfo ();

	// Free all ordered table info
	void						freeAllOTInfo ();

	// Allocate a primitive
	CMovePrimitive				*allocatePrimitive (uint8 firstWorldImage, uint8 numWorldImage);

	// Free a primitive
	void						freePrimitive (CMovePrimitive *primitive);

	// Allocate world image pointers
	CPrimitiveWorldImage		**allocateWorldImagesPtrs (uint numPtrs);

	// Free world image pointers
	void						freeWorldImagesPtrs (CPrimitiveWorldImage **ptrs);

	// Allocate a world image
	CPrimitiveWorldImage		*allocateWorldImage ();

	// Free world image pointers
	void						freeWorldImage (CPrimitiveWorldImage *worldImage);

	// Called by CMovePrimitive when a change occurred on the primitive BB
	void						changed (CMovePrimitive* primitive, uint8 worldImage);

	// Remove the collisionable primitive from the modified list
	void						removeFromModifiedList (CMovePrimitive* primitive, uint8 worldImage);

	// Remove the non collisionable primitive from the modified lists
	void						removeNCFromModifiedList (CMovePrimitive* primitive, uint8 worldImage);

	// Unlink this move element
	void						unlinkMoveElement  (CMoveElement *element, uint8 worldImage);

	// Reaction of the collision between two primitives. Return true if one object has been modified.
	void						reaction (const CCollisionOTInfo& first);
};


} // NLPACS


#endif // NL_MOVE_CONTAINER_H

/* End of move_container.h */
