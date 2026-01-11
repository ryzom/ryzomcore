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

#ifndef NL_PRIMITIVE_WORLD_IMAGE_H
#define NL_PRIMITIVE_WORLD_IMAGE_H

#include "nel/misc/types_nl.h"

#include "move_container.h"


namespace NLPACS {


/**
 * Data for the primitive duplicated for each world image it is linked
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CPrimitiveWorldImage
{
public:

	// Constructor
	CPrimitiveWorldImage ();

	// To call before destruction
	void deleteIt (CMoveContainer &container, uint8 worldImage);

	// Copy
	void copy (const CPrimitiveWorldImage& source);

	/**
	  * Set the global position of the move primitive.
	  *
	  * \param pos is the new global position of the primitive.
	  */
	void	setGlobalPosition (const UGlobalPosition& pos, CMoveContainer& container, CMovePrimitive &primitive, uint8 worldImage);

	/**
	  * Set the global position of the move primitive. Setting the global position
	  * can take a long time if you use a UGlobalRetriever. Set the position with
	  * this method only the first time or for teleporting.
	  *
	  * \param pos is the new global position of the primitive.
	  */
	void	setGlobalPosition (const NLMISC::CVectorD& pos, CMoveContainer& container, CMovePrimitive &primitive,
								uint8 worldImage, bool keepZ = false, UGlobalPosition::TType type = UGlobalPosition::Unspecified);

	/**
	  * Get the position of the move primitive at the end of the movement.
	  *
	  * \return the new position of the primitive.
	  */
	NLMISC::CVectorD	getFinalPosition () const
	{
		// Get the position
		return _Position.getPos();
	}

	/**
	  * Set the new orientation of the move primitive. Only for the box primitives.
	  *
	  * \param rot is the new OZ rotation in radian.
	  */
	void	setOrientation (double rot, CMoveContainer *container, CMovePrimitive *primitive, uint8 worldImage)
	{
		// New position
		_OBData.Orientation=rot;

		// Position has changed
		dirtPos (container, primitive, worldImage);
	}

	/**
	  * Set the speed vector for this primitive. Only for movable primitives.
	  *
	  * \param speed is the speed of the primitive.
	  */
	void	move (const NLMISC::CVectorD& speed, CMoveContainer& container, CMovePrimitive &primitive,
					uint8 worldImage);

	/**
	  * Set the speed vector for this primitive.
	  *
	  * \param speed is the new speed vector.
	  */
	void	setSpeed (const NLMISC::CVectorD& speed, CMoveContainer *container, CMovePrimitive *primitive, uint8 worldImage)
	{
		// New time
		_Speed=speed;

		// Speed has changed
		dirtPos (container, primitive, worldImage);
	}

	/**
	  * Get the speed vector for this primitive.
	  *
	  * \Return the new speed vector.
	  */
	const NLMISC::CVectorD&	getSpeed () const
	{
		// New time
		return _Speed;
	}

	/// Is in modified list ?
	bool	isInModifiedListFlag ()
	{
		return (_DynamicFlags&InModifiedListFlag) != 0;
	}

	/// Clear the inModifiedList flag.
	void	setInModifiedListFlag (bool itis)
	{
		if (itis)
			_DynamicFlags|=InModifiedListFlag;
		else
			_DynamicFlags&=~InModifiedListFlag;
	}

	// Link into modified list
	void	linkInModifiedList (CMovePrimitive* next)
	{
		_NextModified=next;
	}

	/// Is in modified list ?
	bool	isInWorldImageFlag ()
	{
		return (_DynamicFlags&InWorldImageFlag) != 0;
	}

	/// Clear the inModifiedList flag.
	void	setInWorldImageFlag (bool itis)
	{
		if (itis)
			_DynamicFlags|=InWorldImageFlag;
		else
			_DynamicFlags&=~InWorldImageFlag;
	}

	/// Get next modified primitive
	CMovePrimitive	*getNextModified () const
	{
		return _NextModified;
	}

	/**
	  * Return min of the bounding box in X.
	  */
	double	getBBXMin () const
	{
		return _BBXMin;
	}

	/**
	  * Return min of the bounding box in Y.
	  */
	double	getBBYMin () const
	{
		return _BBYMin;
	}

	/**
	  * Return max of the bounding box in X.
	  */
	double	getBBXMax () const
	{
		return _BBXMax;
	}

	/**
	  * Return max of the bounding box in Y.
	  */
	double	getBBYMax () const
	{
		return _BBYMax;
	}

	/**
	  * Return the delta position
	  */
	const NLMISC::CVectorD&	getDeltaPosition () const
	{
		return _DeltaPosition;
	}

	/// Return the nieme MoveElement. The primitive can have 4 move elements. Can be NULL if the ineme elment is not in a list
	CMoveElement	*getMoveElement (uint i)
	{
		return _MoveElement[i];
	}

	/// Remove the nieme MoveElement.
	void removeMoveElement (uint i, CMoveContainer &container, uint8 worldImage);

	/// Add the primitive in the cell and resort the list.
	void addMoveElement (CMoveCell& cell, uint16 x, uint16 y, double centerX, double centerY, CMovePrimitive *primitive,
						CMoveContainer &container, uint8 worldImage);

	/// Add the primitive in the cell at the end of the list and don't sort
	void addMoveElementendOfList (CMoveCell& cell, uint16 x, uint16 y, CMovePrimitive *primitive,
														CMoveContainer &container);

	/// Update precalculated data
	void update (double beginTime, double endTime, CMovePrimitive &primitive)
	{
		// Pos dirt ?
		if (_DynamicFlags&DirtPosFlag)
		{
			// Compute precalculated data
			precalcPos (primitive);

			// Clean
			_DynamicFlags&=~DirtPosFlag;
		}

		// Bounding box dirt ?
		if (_DynamicFlags&DirtBBFlag)
		{
			// Compute precalculated data
			precalcBB (beginTime, endTime, primitive);

			// Clean
			_DynamicFlags&=~DirtBBFlag;
		}
	}

	/// Check sorted lists
	void checkSortedList (uint8 worldImage);

	// Return the global position of the primitive
	const UGlobalPosition& getGlobalPosition()
	{
		return _Position.getGlobalPos();
	}

	// Return the orientation
	double getOrientation () const
	{
		return _OBData.Orientation;
	}

	// Dirt the position flag. Position has changed.
	void	dirtPos (CMoveContainer *container, CMovePrimitive *primitive, uint8 worldImage)
	{
		_DynamicFlags|=DirtPosFlag;
		dirtBB (container, primitive, worldImage);
	}

	// Dirt the bounding box flag.
	void	dirtBB (CMoveContainer *container, CMovePrimitive *primitive, uint8 worldImage)
	{
		// Warn container that BB has changed
		container->changed (primitive, worldImage);

		_DynamicFlags|=DirtBBFlag;
	}

	// Return the init time
	double	getInitTime() const
	{
		return _InitTime;
	}

	// Compute precalculated data for the position
	void precalcPos (CMovePrimitive &primitive);

	// Compute precalculated data for the speed
	void precalcSpeed ();

	// Compute precalculated bounding box
	void precalcBB (double beginTime, double endTime, CMovePrimitive &primitive);

	// *** Some methods to eval collisions
	/* * NOTES:		BB for bounding box
					OB for oriented Box
					OC for oriented cylinder
					P for box point
					S for box segment
	*/

	/**
	  * Eval collisions with the other primitive.
	  *
	  * \param other is another move primitive to test collisions with.
	  * \param desc is a collision descriptor filled with information
	  * about the collision context if the method return true.
	  * \param timeMin is the time you want to clip collision result in the past.
	  * \param timeMax is the time you want to clip collision result in the futur.
	  *
	  * \return true if a collision has been detected in the time range, else false.
	  */
	bool	evalCollision (CPrimitiveWorldImage& other, class CCollisionDesc& desc, double timeMin,
									double timeMax, uint32 testTime, uint32 maxTestIteration,
									double &firstContactTime, double &lastContactTime,
									CMovePrimitive& primitive, CMovePrimitive& otherPrimitive);

	// Box over box
	bool	evalCollisionOBoverOB (CPrimitiveWorldImage& other, CCollisionDesc& desc, double timeMin,
									double timeMax, double &firstContactTime, double &lastContactTime,
									CMovePrimitive& primitive, CMovePrimitive& otherPrimitive);

	// Box over cylinder
	bool	evalCollisionOBoverOC (CPrimitiveWorldImage& other, CCollisionDesc& desc, double timeMin,
									double timeMax, double &firstContactTime, double &lastContactTime,
									CMovePrimitive& primitive, CMovePrimitive& otherPrimitive);

	// Cylinder over cylinder
	bool	evalCollisionOCoverOC (CPrimitiveWorldImage& other, CCollisionDesc& desc, double timeMin,
									double timeMax, double &firstContactTime, double &lastContactTime,
									CMovePrimitive& primitive, CMovePrimitive& otherPrimitive);

	// *** Subprimitive over subprimitive

	// Point over segment in OB/OB test
	bool	evalCollisionPoverS (CPrimitiveWorldImage& other, CCollisionDesc& desc, uint numPoint, uint numSeg,
									CMovePrimitive& primitive, CMovePrimitive& otherPrimitive);

	// Point over cylinder
	bool	evalCollisionPoverOC (CPrimitiveWorldImage& other, CCollisionDesc& desc, uint numPoint,
									double &firstContactTime, double &lastContactTime, CMovePrimitive& primitive,
									CMovePrimitive& otherPrimitive);

	// Segment over cylinder
	bool	evalCollisionSoverOC (CPrimitiveWorldImage& other, CCollisionDesc& desc, uint numPoint,
									CMovePrimitive& primitive, CMovePrimitive& otherPrimitive);

	/**
	  * Eval collisions with the global retriever.
	  *
	  * \param retriever is the global retriever used to test collision
	  *
	  * \return true if a collision has been detected in the time range, else false.
	  */
	const TCollisionSurfaceDescVector *evalCollision (CGlobalRetriever &retriever, CCollisionSurfaceTemp& surfaceTemp,
													uint32 testTime, uint32 maxTestIteration, CMovePrimitive& primitive);

	// Make a move with globalRetriever. Must be call after a free collision evalCollision call.
	void	doMove (CGlobalRetriever &retriever, CCollisionSurfaceTemp& surfaceTemp, double originalMax, double finalMax, bool keepZ = false);

	// Make a move wihtout globalRetriever.
	void	doMove (double timeMax);

	// Reaction between two primitives. Return true if one object has been modified.
	void reaction (CPrimitiveWorldImage& second, const CCollisionDesc& desc, CGlobalRetriever* retriver,
					CCollisionSurfaceTemp& surfaceTemp, bool collision, CMovePrimitive &primitive,
					CMovePrimitive &otherPrimitive, CMoveContainer *container, uint8 worldImage,
					uint8 secondWorldImage, bool secondConst);

	// Reaction with a static collision. Return true if one object has been modified.
	void reaction (	const CCollisionSurfaceDesc&	surfaceDesc, const UGlobalPosition& globalPosition,
					CGlobalRetriever& retriever, double ratio, double dt, CMovePrimitive &primitive, CMoveContainer &container,
					uint8 worldImage);

private:
	// Some flags
	enum TDynamicFlags
	{
		// The dirt flag. Precalculated data for the position must be recomputed.
		DirtPosFlag			=0x0001,

		// The dirt bounding box. Precalculated data for the bounding box must be recomputed.
		DirtBBFlag			=0x0002,

		// In modified list.
		InModifiedListFlag	=0x0004,

		// Inserted in the world image.
		InWorldImageFlag	=0x0008,

		// Force the size to uint16.
		ForceSize			=0xffff
	};

	// Flags
	uint16				_DynamicFlags;

	// This position is the central bottom position for the box or for the cylinder
	class CPosition
	{
	private:
		// 3d position
		NLMISC::CVectorD					_3dPosition;

		// Global position
		UGlobalPosition						_GlobalPosition;
	public:
		// Return the 3d position
		const NLMISC::CVectorD&						getPos () const
		{
			return _3dPosition;
		}

		// Set the 3d position
		void										setPos (const NLMISC::CVectorD& newPos)
		{
			_3dPosition=newPos;
		}

		// Return the global position
		const UGlobalPosition&						getGlobalPos () const
		{
			return _GlobalPosition;
		}

		// Set the global position
		void										setGlobalPos (const UGlobalPosition& globalPosition,
																  CGlobalRetriever& globalRetriver)
		{
			// Get position with global position
			_GlobalPosition=globalPosition;
			_GlobalPosition.LocalPosition.Estimation.z = globalRetriver.getMeanHeight(globalPosition);
			_3dPosition=globalRetriver.getDoubleGlobalPosition (_GlobalPosition);
//			_3dPosition.z=(double)globalRetriver.getMeanHeight(globalPosition);
		}

		void										setGlobalPosKeepZ(const UGlobalPosition& globalPosition,
																	  CGlobalRetriever& globalRetriver)
		{
			_GlobalPosition.setKeepZ(globalPosition);
			NLMISC::CVectorD dPos = globalRetriver.getDoubleGlobalPosition (globalPosition);
			_3dPosition.x = dPos.x;
			_3dPosition.y = dPos.y;
		}

	};

	// Global position of the primitive in the world image.
	CPosition			_Position;

	// Time for valid getPos () position.  _Position.getPos () == _3dInitPosition + _Speed * _InitTime;
	double				_InitTime;

	// 3d position at t=0
	NLMISC::CVectorD	_3dInitPosition;

	// Delta position is : _Speed*Deltatime
	NLMISC::CVectorD	_DeltaPosition;

	// Current speed
	NLMISC::CVectorD	_Speed;

	// Movable bounding box
	double				_BBXMin;
	double				_BBYMin;
	double				_BBXMax;
	double				_BBYMax;

	// Union for data space shared by primitive type (cylinder or box)
	union
	{
		// Data for Boxes
		struct
		{
			// 2d position of the 4 points of the box at initial time.
			double PointPosX[4];
			double PointPosY[4];

			// The normalized direction vector of the 4 edges of the box.
			double EdgeDirectionX[4];
			double EdgeDirectionY[4];

			// The box orientation
			double Orientation;
		} _OBData;
	};

	// Sorted list of modified primitives. This pointer is managed by CMoveContainer.
	CMovePrimitive		*_NextModified;

	// Pointer into the 4 possibles sorted lists of movable primitives. Can be NULL if not in the list
	CMoveElement		*_MoveElement[4];
};


} // NLPACS


#endif // NL_PRIMITIVE_WORLD_IMAGE_H

/* End of primitive_world_image.h */
