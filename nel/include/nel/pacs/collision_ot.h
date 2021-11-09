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

#ifndef NL_COLLISION_OT_H
#define NL_COLLISION_OT_H

#include "nel/misc/types_nl.h"
#include "collision_desc.h"
#include "global_retriever.h"


namespace NLPACS
{

class CMovePrimitive;
class CCollisionOTInfo;

/**
 * Odered table element
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionOT
{
public:
	CCollisionOT ()
	{
		_Info=false;
		clear ();
	}

	virtual ~CCollisionOT () { }

	/// Clear the element
	void			clear ()
	{
		_Next=NULL;
		_Previous=NULL;
	}

	/// Link in the ot
	void			link (CCollisionOT* newElement)
	{
		if (newElement)
		{
			newElement->_Previous=this;
			newElement->_Next=_Next;
		}
		if (_Next)
			_Next->_Previous=newElement;
		_Next=newElement;
	}

	/// Remove from OT
	void			unlink ()
	{
		if (_Previous)
			_Previous->_Next=_Next;
		if (_Next)
			_Next->_Previous=_Previous;
		_Next=NULL;
		_Previous=NULL;
	}

	/// Return true if it an info collision else false;
	bool			isInfo () const
	{
		return _Info;
	}

	/// Get next element in the ot
	CCollisionOT		*getPrevious () const
	{
		return _Previous;
	}

	/// Get next element in the ot
	CCollisionOT		*getNext () const
	{
		return _Next;
	}

	/// Get next element in the ot
	CCollisionOTInfo	*getNextInfo () const;

private:
	// The next cell
	CCollisionOT	*_Next;

	// The previous cell
	CCollisionOT	*_Previous;

protected:
	// Is a CCollisionOTInfo
	bool			_Info;
};

/**
 * Collision descriptor for time odered table
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionOTInfo : public CCollisionOT
{
public:
	CCollisionOTInfo ()
	{
		_Info=true;
	}

	// Link in the primitive
	virtual void primitiveLink (CMovePrimitive *primitive, CCollisionOTInfo *other) =0;

	// Link in the primitive
	virtual CCollisionOTInfo	*getNext (CMovePrimitive *primitive) const =0;

	// Return false for dynamic collision, true for static collision
	virtual bool				isCollisionAgainstStatic () const=0;

	// Return collision time
	virtual double				getCollisionTime () const=0;

	// Remove the collision from the primitives
	virtual void				removeFromPrimitives ()=0;

	// Get second primitive
	virtual CMovePrimitive		*getOtherPrimitive (CMovePrimitive *primitive) const=0;
};

/**
 * Dynamic collisions descriptor for time odered table
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionOTDynamicInfo : public CCollisionOTInfo
{
public:
	CCollisionOTDynamicInfo ()
	{
		_Info=true;
	}

	// Get the first primitive in the collision
	CMovePrimitive		*getFirstPrimitive () const
	{
		return _FirstPrimitive;
	}

	// Get the second primitive in the collision
	CMovePrimitive		*getSecondPrimitive () const
	{
		return _SecondPrimitive;
	}

	// Get the collision descriptor
	const CCollisionDesc	&getCollisionDesc () const
	{
		return _Desc;
	}

	// Is a collision
	bool				isCollision() const
	{
		return _Collision;
	}

	// Is enter
	bool				isEnter() const
	{
		return _Enter;
	}

	// Is exit
	bool				isExit() const
	{
		return _Exit;
	}

	// Is exit
	bool				isInside() const
	{
		return _Inside;
	}

	// Is second primitive is static ?
	bool				isSecondStatic () const
	{
		return _SecondStatic;
	}

	// Get the world image
	uint8 getFirstWorldImage () const
	{
		return _FirstWorldImage;
	}

	// Get the world image
	uint8 getSecondWorldImage () const
	{
		return _SecondWorldImage;
	}

	// Init the info
	void				init (CMovePrimitive *firstPrimitive, CMovePrimitive *secondPrimitive, const CCollisionDesc& desc, bool collision,
								bool enter, bool exit, bool inside, uint8 firstWorldImage, uint8 secondWorldImage, bool secondStatic)
	{
		_FirstPrimitive=firstPrimitive;
		_SecondPrimitive=secondPrimitive;
		_Desc=desc;
		_Collision=collision;
		_Enter=enter;
		_Exit=exit;
		_Inside=inside;
		_FirstWorldImage=firstWorldImage;
		_SecondWorldImage=secondWorldImage;
		_SecondStatic=secondStatic;
	}

	// Link in the primitive
	void				primitiveLink (CMovePrimitive *primitive, CCollisionOTInfo *other)
	{
		// First primitive ?
		if (primitive==_FirstPrimitive)
		{
			// Check
			nlassert  (primitive!=_SecondPrimitive);

			// Link
			_FirstNext=other;
		}
		else // second
		{
			// Check
			nlassert  (primitive==_SecondPrimitive);

			// Link
			_SecondNext=other;
		}
	}

	// Link in the primitive
	CCollisionOTInfo	*getNext (CMovePrimitive *primitive) const
	{
		// First primitive ?
		if (primitive==_FirstPrimitive)
		{
			// Check
			nlassert  (primitive!=_SecondPrimitive);

			// return next
			return _FirstNext;
		}
		else // second
		{
			// Check
			nlassert  (primitive==_SecondPrimitive);

			// Link
			return _SecondNext;
		}
	}

	// Return false for dynamic collision, true for static collision
	bool				isCollisionAgainstStatic () const
	{
		return false;
	}

	// Return collision time
	double				getCollisionTime () const
	{
		return _Desc.ContactTime;
	}

	// Remove the collision from the primitives
	void				removeFromPrimitives ();

	// Get second primitive
	CMovePrimitive		*getOtherPrimitive (CMovePrimitive *primitive) const
	{
		if (_FirstPrimitive==primitive)
			return _SecondPrimitive;
		else
		{
			nlassert (_SecondPrimitive==primitive);
			return _FirstPrimitive;
		}
	}

private:

	// The first primitive
	CMovePrimitive		*_FirstPrimitive;

	// The second primitive
	CMovePrimitive		*_SecondPrimitive;

	// Descriptor
	CCollisionDesc		_Desc;

	// Collision or only trigger ?
	bool				_Collision;
	bool				_Enter;
	bool				_Exit;
	bool				_Inside;
	bool				_SecondStatic;

	// World images
	uint8 _FirstWorldImage;
	uint8 _SecondWorldImage;

	// First primitive linked list
	CCollisionOTInfo	*_FirstNext;

	// Second primitive linked list
	CCollisionOTInfo	*_SecondNext;
};

/**
 * Static collisions descriptor for time odered table
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionOTStaticInfo : public CCollisionOTInfo
{
public:
	CCollisionOTStaticInfo ()
	{
		_Info=true;
	}

	// Get the first primitive in the collision
	CMovePrimitive		*getPrimitive () const
	{
		return _Primitive;
	}

	// Get the collision descriptor
	const CCollisionSurfaceDesc	&getCollisionDesc () const
	{
		return _StaticDesc;
	}

	// Get the collision descriptor
	const UGlobalPosition &getGlobalPosition () const
	{
		return _GlobalPosition;
	}

	// Get delta time
	double getDeltaTime () const
	{
		return _DeltaTime;
	}

	// Get the world image
	uint8 getWorldImage () const
	{
		return _WorldImage;
	}

	// Init the info
	void				init (CMovePrimitive *primitive, const CCollisionSurfaceDesc& desc,
								const UGlobalPosition& nextGlobalPosition, double delta, uint8 worldImage)
	{
		_Primitive=primitive;
		_StaticDesc=desc;
		_GlobalPosition=nextGlobalPosition;
		_DeltaTime=delta;
		_WorldImage=worldImage;
	}

	// Link in the primitive
	void				primitiveLink (CMovePrimitive * /* primitive */, CCollisionOTInfo *other)
	{
		// Link
		_Next=other;
	}

	// Link in the primitive
	CCollisionOTInfo	*getNext (CMovePrimitive * /* primitive */) const
	{
		// return next
		return _Next;
	}

	// Return false for dynamic collision, true for static collision
	bool				isCollisionAgainstStatic () const
	{
		return true;
	}

	// Return collision time
	double				getCollisionTime () const
	{
		return _StaticDesc.ContactTime;
	}

	// Remove the collision from the primitives
	void				removeFromPrimitives ();

	CMovePrimitive		*getOtherPrimitive (CMovePrimitive * /* primitive */) const
	{
		return NULL;
	}
private:
	// The first primitive
	CMovePrimitive		*_Primitive;

	// The static descriptor
	CCollisionSurfaceDesc	_StaticDesc;

	// The next global position
	UGlobalPosition	_GlobalPosition;

	// Delta time for this collid
	double				_DeltaTime;

	// World image
	uint8				_WorldImage;

	// Next primitive in the linked list
	CCollisionOTInfo	*_Next;
};

// ***************************************************************************

inline CCollisionOTInfo	*CCollisionOT::getNextInfo () const
{
	// Get next
	CCollisionOT *next=_Next;

	// Is an info ?
	while ( next && (!next->isInfo ()) )
		next=next->getNextInfo ();

	// Return an info
	return (CCollisionOTInfo*)next;
}



} // NLPACS


#endif // NL_COLLISION_OT_H

/* End of collision_ot.h */
