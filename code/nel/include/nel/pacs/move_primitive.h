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

#ifndef NL_MOVE_PRIMITIVE_H
#define NL_MOVE_PRIMITIVE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

#include "nel/pacs/u_move_primitive.h"

#include "move_container.h"
#include "primitive_world_image.h"
#include "global_retriever.h"

#define NELPACS_DIST_BACK 0.01

namespace NLPACS
{

/**
 * Description of movables primitives.
 *
 * This primitive can be a 2d oriented box or a 2d oriented cylinder.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CMovePrimitive: public UMovePrimitive
{
private:
	friend class CMoveContainer;

	// Some flags
	enum TStaticFlags
	{
		// Mask for the primitive type
		PrimitiveMask	=0x000f,

		// Mask for the primitive type
		ReactionMask	=0x00f0,

		// Mask for the trigger type
		TriggerMask		=0x0f00,

		// Obstacle flag. This flag tells that this object is an obstacle for others objects.
		ObstacleFlag	=0x1000,

		// Ghost flag. This flag tells that this object is a ghost object or not.
		NonCollisionableFlag	=0x2000,

		/// The primitive shoudln't be snapped to ground
		DontSnapToGroundFlag = 0x4000,

		// Force the size to uint16.
		ForceSize		=0xffff
	};

public:

	/// Constructor
	CMovePrimitive (CMoveContainer* container, uint8 firstWorldImage, uint8 numWorldImage);

	/// Destructor
	virtual ~CMovePrimitive ();
	//
	virtual	void	setDontSnapToGround(bool dont)
	{
		if (dont) _StaticFlags |= DontSnapToGroundFlag;
		else _StaticFlags &= ~DontSnapToGroundFlag;
	}
	//
	bool	getDontSnapToGround() const
	{
		return (_StaticFlags & DontSnapToGroundFlag) != 0;
	}
	// Return true if this primitive is noncollisionable
	bool isNonCollisionable () const
	{
		return (_StaticFlags&NonCollisionableFlag)!=0;
	}

	// Set noncollisionable
	void setNonCollisionable (bool nonCollisionable)
	{
		if (nonCollisionable)
			_StaticFlags|=NonCollisionableFlag;
		else
			_StaticFlags&=~NonCollisionableFlag;
	}

	// Get the nieme world image
	CPrimitiveWorldImage		*getWorldImage (uint i) const
	{
		// Checks this primitive belong of the requested world image.
		nlassert ((i>=(uint)_FirstWorldImage)&&(i<(uint)_FirstWorldImage+(uint)_NumWorldImage));

		// Return the good one
		return _WorldImages[i-_FirstWorldImage];
	}

	// is the primitive inserted in the world image
	bool isInserted (uint i) const
	{
		// Checks
		if ((i>=(uint)_FirstWorldImage)&&(i<(uint)_FirstWorldImage+(uint)_NumWorldImage))
		{
			// Get world image
			return getWorldImage (i)->isInWorldImageFlag ();
		}
		else
			return false;
	}

	virtual uint8				getFirstWorldImageV () const;
	virtual uint8				getNumWorldImageV () const;

	// Get first world image used
	uint8	getFirstWorldImage () const
	{
		return _FirstWorldImage;
	}

	// Get count of world image used
	uint8	getNumWorldImage () const
	{
		return _NumWorldImage;
	}

	// Dirt positions of the primitive in all the world images
	void	dirtAllPos ()
	{
		for (uint i=0; i<_NumWorldImage; i++)
		{
			// Get world image and check if it exist
			CPrimitiveWorldImage	*worldImage=_WorldImages[i];

			// Dirt its pos
			worldImage->dirtPos (_Container, this, uint8(i+_FirstWorldImage));
		}
	}

	/**
	  * Set the primitive type.
	  *
	  * \param type is the new primitive type.
	  */
	void	setPrimitiveType (TType type)
	{
		// New position
		_StaticFlags&=~(uint32)PrimitiveMask;
		_StaticFlags|=type;
	}

	/**
	  * Set the reaction type.
	  *
	  * \param type is the new reaction type.
	  */
	void	setReactionType (TReaction type)
	{
		// New position
		_StaticFlags&=~(uint32)ReactionMask;
		_StaticFlags|=type;
	}

	/**
	  * Set the trigger type. Default type is NotATrigger.
	  *
	  * \param type is the new trigger type.
	  */
	void	setTriggerType (TTrigger type)
	{
		// New position
		_StaticFlags&=~(uint32)TriggerMask;
		_StaticFlags|=type;
	}

	/**
	  * Set the collision mask for this primitive. Default mask is 0xffffffff.
	  *
	  * \param mask is the new collision mask.
	  */
	void	setCollisionMask (TCollisionMask mask)
	{
		_CollisionMask=mask;
	}

	/**
	  * Set the occlusion mask for this primitive. Default mask is 0xffffffff.
	  *
	  * \param mask is the new occlusion mask.
	  */
	void	setOcclusionMask (TCollisionMask mask)
	{
		_OcclusionMask=mask;
	}

	/**
	  * Set the obstacle flag.
	  *
	  * \param obstacle is true if this primitive is an obstacle, else false.
	  */
	void	setObstacle (bool obstacle)
	{
		// New flag
		if (obstacle)
			_StaticFlags|=ObstacleFlag;
		else
			_StaticFlags&=~(uint32)ObstacleFlag;
	}

	/**
	  * Set the box size. Only for boxes.
	  *
	  * \param width is the new width size of the box. It the size of the sides aligned on OX.
	  * \param depth is the new depth size of the box. It the size of the sides aligned on OY.
	  */
	void	setSize (float width, float depth)
	{
		// Checks
		nlassert ((((uint32)_StaticFlags)&PrimitiveMask)==_2DOrientedBox);

		// New position
		_Length[0]=width;
		_Length[1]=depth;
	}

	/**
	  * Set the height. For boxes or cylinder.
	  *
	  * \param height is the new height size of the box. It the size of the sides aligned on OZ.
	  */
	void	setHeight (float height)
	{
		// New size
		_Height=height;
	}

	/**
	  * Set the cylinder size. Only for cylinder.
	  *
	  * \param radius is the new radius size of the cylinder.
	  */
	void	setRadius (float radius)
	{
		// Checks
		nlassert ((((uint32)_StaticFlags)&PrimitiveMask)==_2DOrientedCylinder);

		// New position
		_Length[0]=radius;
	}

	/// Get primitive type
	TType	getPrimitiveTypeInternal () const
	{
		// New position
		return (TType)(_StaticFlags&(uint32)PrimitiveMask);
	}

	/// Get reaction type
	TReaction	getReactionTypeInternal () const
	{
		// New position
		return (TReaction)(_StaticFlags&(uint32)ReactionMask);
	}

	/// Get reaction type
	TTrigger	getTriggerTypeInternal () const
	{
		// New position
		return (TTrigger)(_StaticFlags&(uint32)TriggerMask);
	}

	/// Get collision mask
	TCollisionMask	getCollisionMaskInternal () const
	{
		// New position
		return _CollisionMask;
	}

	/// Get occlusion mask
	TCollisionMask	getOcclusionMaskInternal () const
	{
		// New position
		return _OcclusionMask;
	}

	/// Get attenuation
	float getAttenuation() const
	{
		return _Attenuation;
	}

	/// Get length
	float getLength (uint where) const
	{
		return _Length[where];
	}

	/// Get height
	float getHeightInternal () const
	{
		return _Height;
	}

	/// Get length
	float getRadiusInternal () const
	{
		return _Length[0];
	}

	/// Is an obstacle ?
	bool isObstacle () const
	{
		return (_StaticFlags&ObstacleFlag)!=0;
	}

	/// Is collisionable
	bool	isCollisionable() const
	{
		return !isNonCollisionable();
	}

	/// Add a collision time ordered table element
	void addCollisionOTInfo (CCollisionOTInfo *info)
	{
		// Link to the list
		info->primitiveLink (this, _RootOTInfo);
		_RootOTInfo=info;
	}

	/// Remove one collision time ordered table element.
	void removeCollisionOTInfo (CCollisionOTInfo *toRemove);

	/// Remove all collision time ordered table element.
	void removeCollisionOTInfo ();

	/// Check sorted lists
	void checkSortedList ();

	/// Check trigger flag
	bool isTriggered (CMovePrimitive& second, bool enter, bool exit);

	/// \name From UMovePrimitive

	void					setAbsorbtion (float attenuation);
	void					setOrientation (double rot, uint8 worldImage);
	void					setGlobalPosition (const NLMISC::CVectorD& pos, uint8 worldImage, UGlobalPosition::TType type = UGlobalPosition::Unspecified);
	void					setGlobalPosition (const UGlobalPosition& pos, uint8 worldImage);
	void					move (const NLMISC::CVectorD& speed, uint8 worldImage);
	NLMISC::CVectorD		getFinalPosition (uint8 worldImage)  const;
	const NLMISC::CVectorD&	getSpeed (uint8 worldImage) const;
	void					insertInWorldImage (uint8 worldImage);
	void					removeFromWorldImage (uint8 worldImage);
	TType					getPrimitiveType () const;
	TReaction				getReactionType () const;
	TTrigger				getTriggerType () const;
	TCollisionMask			getCollisionMask () const;
	TCollisionMask			getOcclusionMask () const;
	bool					getObstacle () const;
	float					getAbsorbtion () const;
	void					getSize (float& width, float& depth) const;
	float					getHeight () const;
	float					getRadius () const;
	double					getOrientation (uint8 worldImage) const;
	void					getGlobalPosition (UGlobalPosition& pos, uint8 worldImage) const;

	// Test time. Return true if tetst can be perform, false if too many test have been computed for this primitive
	bool checkTestTime (uint32 testTime, uint32 maxTestIteration)
	{
		// Already checked for this test time ?
		if (testTime!=_LastTestTime)
		{
			// First time this primitive is visited at this test
			_LastTestTime=testTime;

			// Test counter
			_IterationCount=maxTestIteration;
		}
		else
		{
			// Too many test this primitive ?
			if (_IterationCount<=0)
			{
//				nlwarning("in checkTestTime(): Get to the max iteration count");
				return false;
			}
			else
				_IterationCount--;
		}
		// Ok, test can be performed
		return true;
	}

	// *** Get mass

	float getMass () const
	{
		// Box ?
		if ( (_StaticFlags&PrimitiveMask) == _2DOrientedBox )
			return _Length[0]*_Length[1]*_Height;
		// Cylinder ?
		else
		{
			nlassert ( (_StaticFlags&PrimitiveMask) == _2DOrientedCylinder );
			return _Length[0]*(float)NLMISC::Pi*_Height;
		}
	}

	// *** Primitive over primitive

	bool	isInCollision (CMovePrimitive *primitive);

private:
	// The length of the 4 edges. The first is the width, the second is the depth
	// For cylinder, the first is the radius
	float				_Length[2];

	// This is the height of the box or of the cylinder.
	float				_Height;

	// Attenuation
	float				_Attenuation;

	// Flags
	uint16				_StaticFlags;

	// Num world images
	uint8				_NumWorldImage;

	// First world images
	uint8				_FirstWorldImage;

	// Occlusion mask
	TCollisionMask		_OcclusionMask;

	// Collision mask
	TCollisionMask		_CollisionMask;

	// Container of this primitive
	CMoveContainer		*_Container;

	// List of time ordered table element using this primitive
	CCollisionOTInfo	*_RootOTInfo;

	// Pointer table of world images for this primitive
	CPrimitiveWorldImage	**_WorldImages;

	// Last primitive test time
	uint32				_LastTestTime;

	// Iteration count
	sint32				_IterationCount;
};

} // NLPACS

#include "move_container_inline.h"

#endif // NL_MOVE_PRIMITIVE_H

/* End of move_primitive.h */
