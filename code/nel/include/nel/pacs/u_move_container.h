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

#ifndef NL_U_MOVE_CONTAINER_H
#define NL_U_MOVE_CONTAINER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include <vector>

namespace NLMISC
{
	class CVectorD;
	class CMatrix;
}

namespace NLPACS
{

class UMovePrimitive;
class UTriggerInfo;
class UGlobalRetriever;
class UPrimitiveBlock;

#define NELPACS_DEFAULT_OT_SIZE 100
#define NELPACS_DEFAULT_MAX_TEST_ITERATION 100
#define NELPACS_DEFAULT_NUM_WORLD_IMAGE 1

/**
 * A container for movable objects
 * Some constraints:
 * * The move bounding box must be lower than the cell size
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UMoveContainer
{
public:

	/**
	  * destructor
	  */
	virtual ~UMoveContainer () { }

	/// \name Manage primitives.

	/**
	  * Add a collisionable primitive in the container. Return the pointer on the primitive.
	  * This primitive will generate collisions when the system evaluate other primitives against.
	  *
	  * You must specify the ids of each world image where the primitive can be inserted.
	  * Thoses ids are consecutives. If you choose 5 as first id and 3 as id count,
	  * this primitive could be inserted in the world image #5, #6 and #7.
	  *
	  * This primtive should be inserted in a world image before use. See UMovePrimitive::insertInWorldImage.
	  *
	  * \param firstWorldImage is the first world image where the primitive can be inserted.
	  * \param numWorldImage is the count of world image where the primitive can be inserted.
	  * \return a pointer on the new primitive.
	  */
	virtual UMovePrimitive		*addCollisionablePrimitive (uint8 firstWorldImage, uint8 numWorldImage, const UMovePrimitive *copyFrom = NULL) =0;

	/**
	  * Add a noncollisionable primitive in the container. Return the pointer on the primitive.
	  * This primitive won't generate collisions when the system evaluate other primitives against.
	  *
	  * This primitive can't be inserted in a world image.
	  *
	  * \param copyFrom is an optional primitive to copy attributes from (so the primitive is initialised with same values)
	  * \return a pointer on the new primitive.
	  */
	virtual UMovePrimitive		*addNonCollisionablePrimitive (const UMovePrimitive *copyFrom = NULL) =0;

	/**
	  * Load a PACS primitive block. (*.pacs_prim)
	  *
	  * Add a set of collisionable primitive in the container. If success, fill an array with primitives's pointers.
	  * The primitive are inserted in the requested world image of the container. Then a setGlobalPosition
	  * is done to place the primitives in the world image. The world images are not evaluated.
	  *
	  * You must specify the ids of each world image where the primitives can be inserted.
	  * Thoses ids are consecutives. If you choose 5 as first id and 3 as id count,
	  * those primitives could be inserted in the world image #5, #6 and #7.
	  *
	  * Those primtives should be inserted in a world image before use. See UMovePrimitive::insertInWorldImage.
	  *
	  * Return false if the world image numbers are not present in the move container.
	  *
	  * Can raise unhandled NLMISC::Exception if trouble during serialisation.
	  *
	  * \param filename is the file to load.
	  * \param firstWorldImage is the first world image where the primitive can be inserted.
	  * \param numWorldImage is the count of world image where the primitive can be inserted.
	  * \param primitives is a pointer on an array of primitive pointer to fill if success. If NULL, Do return nothing.
	  * \param orientation is the orientation to give to the primitives.
	  * \param position is the position to give to the primitives.
	  * \param primitives is a pointer on an array of primitive pointer to fill if success. If NULL, Do return nothing.
	  * \param dontSnapToGround force the inserted primitive to be flagged as 'DontSnapToGround'
	  * \see addCollisionablePrimitive
	  * \see getPACSCoordsFromMatrix
	  * \return true if the file is successfully loaded, else return false.
	  */
	virtual bool				loadCollisionablePrimitiveBlock (const char *filename, uint8 firstWorldImage, uint8 numWorldImage, std::vector<UMovePrimitive*> *primitives, float orientation, const NLMISC::CVector &position, bool dontSnapToGround = false) =0;


	/** The same as loadCollisionablePrimitiveBlock, but the primitive block is provided by the caller
	  */
	virtual void                addCollisionnablePrimitiveBlock(UPrimitiveBlock *pb, uint8 firstWorldImage, uint8 numWorldImage, std::vector<UMovePrimitive*> *primitives, float orientation, const NLMISC::CVector &position, bool dontSnapToGround = false, const NLMISC::CVector &scale = NLMISC::CVector(1.0f, 1.0f, 1.0f)) = 0;

	/**
	  * Remove a primitive from the container.
	  *
	  * \param primitive is the pointer on the primitive to remove.
	  */
	virtual void				removePrimitive (UMovePrimitive* primitive) =0;

	/// Get all the primitives in the container
	virtual	void				getPrimitives(std::vector<const UMovePrimitive *> &dest) const = 0;


	/// \name Primitive evaluation.

	/**
	  * Evaluation of a worldImage of the collision system.
	  * This method will evaluate the move of each modified collisionable primitives inserted in the world image.
	  * The method first test collisions against the terrain, then test collisions against primitives
	  * inserted in the world images declared as static,
	  * then test collisions against the primitives inserted in the world image to evaluate.
	  *
	  * \param deltaTime is the delta time of the system evaluation.
	  * \param worldImage is the world image to eval.
	  */
	virtual void				evalCollision (double deltaTime, uint8 worldImage) =0;

	/**
	  * Evaluation of a single non collisionable primitive.
	  * The method first test collisions against the terrain, then test collisions against primitives
	  * inserted in the world images declared as static,
	  * then test collisions against the primitives inserted in the world image to evaluate.
	  *
	  * \param deltaTime is the delta time of the system evaluation.
	  * \param primitive is the primitive pointer
	  * \param worldImage is the world image to eval.
	  * \return false if the primitive is a collisionable primitive.
	  */
	virtual bool				evalNCPrimitiveCollision (double deltaTime, UMovePrimitive *primitive, uint8 worldImage) = 0;

	/**
	  * Test the move of a primitive in a specific world image.
	  *
	  * This method will test the move of each modified primitives inserted in the world image.
	  * The method will first test collisions against primitives inserted in the world images declared as static,
	  * then test collisions against the primitives inserted in the world image choosed to test the move of the primitive.
	  *
	  * \param primitive is a pointer on the primitive
	  * \param speed is the wanted speed of the primitive
	  * \param deltaTime is the deltaTime of the move of the primitive.
	  * \param worldImage is the world image where you want to test the move of the primitive.
	  * \param contactNormal is a pointer to CVectorD to store contactNormal with terrain, if not NULL
	  * \return true if the move is successful, false else.
	  */
	virtual bool				testMove (UMovePrimitive* primitive, const NLMISC::CVectorD& speed, double deltaTime, uint8 worldImage,
											NLMISC::CVectorD *contactNormal) =0;

	/// \name World image management.

	/**
	  * Set world image as static world image.
	  *
	  * This method set this world image as static. It means that primitives inserted there don't move.
	  * Each primitive evaluation methods will first test all the primitives in the world images declared as static.
	  * Then, the evalutation test the primtive in the asked worldImage.
	  *
	  * \param worldImage is the id of the world image to set as static.
	  */
	 virtual void				setAsStatic (uint8 worldImage) =0;

	/**
	  * Duplicate a world image in another.
	  *
	  * All primitive will be removed from the destination world image.
	  * Then, the source world image will be copied in the destination world image.
	  *
	  * Warning, only primitives from the source that have been declared as using the destination
	  * world image will be copied.
	  *
	  * The source world image remain the same.
	  *
	  * \param source is the id of the source world image for the copy.
	  * \param dest is the id of the destination world image for the copy.
	  */
	virtual void				duplicateWorldImage (uint8 source, uint8 dest) =0;

	/// \name Triggers info.

	/// Get number of trigger information
	virtual uint				getNumTriggerInfo() const=0;

	/// Get the n-th trigger information
	virtual const UTriggerInfo &getTriggerInfo (uint id) const=0;

	/// \name Create methods.

	// Create method
	static 	UMoveContainer		*createMoveContainer (double xmin, double ymin, double xmax, double ymax,
		uint widthCellCount, uint heightCellCount, double primitiveMaxSize, uint8 numWorldImage=NELPACS_DEFAULT_NUM_WORLD_IMAGE,
		uint maxIteration=NELPACS_DEFAULT_MAX_TEST_ITERATION, uint otSize=NELPACS_DEFAULT_OT_SIZE);

	// Create method
	static 	UMoveContainer		*createMoveContainer (UGlobalRetriever* retriever, uint widthCellCount,
		uint heightCellCount, double primitiveMaxSize, uint8 numWorldImage=NELPACS_DEFAULT_NUM_WORLD_IMAGE,
		uint maxIteration=NELPACS_DEFAULT_MAX_TEST_ITERATION, uint otSize=NELPACS_DEFAULT_OT_SIZE);

	// Delete method
	static 	void				deleteMoveContainer (UMoveContainer	*container);

	/** Get a pacs position and an orientation from a matrix
	  */
	static	void getPACSCoordsFromMatrix(NLMISC::CVector &pos, float &angle, const NLMISC::CMatrix &mat);
};


} // NLPACS


#endif // NL_U_MOVE_CONTAINER_H

/* End of u_move_container.h */
