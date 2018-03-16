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

#include "stdpacs.h"

#include "nel/pacs/move_primitive.h"
#include "nel/pacs/collision_desc.h"
#include "nel/pacs/move_element.h"
#include "nel/misc/hierarchical_timer.h"

using namespace NLMISC;

H_AUTO_DECL ( NLPACS_Get_Global_Position )
H_AUTO_DECL ( NLPACS_Set_Global_Position )
H_AUTO_DECL ( NLPACS_Set_UGlobal_Position )
H_AUTO_DECL ( NLPACS_Move )
#define	NLPACS_HAUTO_GET_GLOBAL_POSITION	H_AUTO_USE ( NLPACS_Get_Global_Position )
#define	NLPACS_HAUTO_SET_GLOBAL_POSITION	H_AUTO_USE ( NLPACS_Set_Global_Position )
#define	NLPACS_HAUTO_SET_UGLOBAL_POSITION	H_AUTO_USE ( NLPACS_Set_UGlobal_Position )
#define	NLPACS_HAUTO_MOVE					H_AUTO_USE ( NLPACS_Move )

namespace NLPACS
{

// ***************************************************************************

CMovePrimitive::CMovePrimitive (CMoveContainer* container, uint8 firstWorldImage, uint8 numWorldImage)
{
	_FirstWorldImage=firstWorldImage;
	_NumWorldImage=numWorldImage;

	_CollisionMask=0xffffffff;
	_OcclusionMask=0xffffffff;
	_Attenuation=1;
	_Container=container;
	_StaticFlags=0;
	_RootOTInfo=NULL;
	_LastTestTime=0xffffffff;

	// Ptr table alloc
	_WorldImages=_Container->allocateWorldImagesPtrs (numWorldImage);

	// Alloc world images
	for (uint j=0; j<numWorldImage; j++)
		_WorldImages[j]=_Container->allocateWorldImage ();
}

// ***************************************************************************

CMovePrimitive::~CMovePrimitive ()
{
	// Alloc world images
	for (uint j=0; j<(uint)_NumWorldImage; j++)
	{
		_WorldImages[j]->deleteIt (*_Container, (uint8)(_FirstWorldImage+j));
		_Container->freeWorldImage (_WorldImages[j]);
	}

	// Ptr table alloc
	_Container->freeWorldImagesPtrs (_WorldImages);
}

// ***************************************************************************

void CMovePrimitive::removeCollisionOTInfo (CCollisionOTInfo *toRemove)
{
	// Should be ok
	CCollisionOTInfo	*previousElement=NULL;
	CCollisionOTInfo	*element=_RootOTInfo;
	nlassert (element);

	// Look for it
	while (element)
	{
		// Good one ?
		if (element==toRemove)
		{
			// If previous element, just link
			if (previousElement)
				previousElement->primitiveLink (this, element->getNext (this));
			else
				_RootOTInfo=element->getNext (this);

			// End
			break;
		}

		// Look for next
		previousElement=element;
		element=element->getNext (this);
	}

	// Should be found
	nlassert (element);
}

// ***************************************************************************

void CMovePrimitive::removeCollisionOTInfo ()
{
	// For each element in the list
	CCollisionOTInfo	*element=_RootOTInfo;
	while (element)
	{
		// Unlink from ot
		element->unlink ();

		// Remove collision ot info from other primitive
		CMovePrimitive *other=element->getOtherPrimitive (this);
		if (other)
		{
			// Remove it in the other element
			other->removeCollisionOTInfo (element);
		}

		// Next element
		element=element->getNext (this);
	}

	// Relink element because we keep it
	_RootOTInfo=NULL;
}

// ***************************************************************************

void CMovePrimitive::checkSortedList ()
{
	// Check sorted list for ecah world image
	for (uint i=0; i<(uint)_NumWorldImage; i++)
		_WorldImages[i]->checkSortedList (uint8(i+_FirstWorldImage));
}

// ***************************************************************************

bool CMovePrimitive::isTriggered (CMovePrimitive& second, bool enter, bool exit)
{
	// Generate a trigger ?

	// Is the two are not triggers ?
	if ( ( (_StaticFlags&TriggerMask) == NotATrigger ) && ( (second._StaticFlags&TriggerMask) == NotATrigger ) )
		return false;

	// Is one of them is an enter trigger ?
	if ( enter && ( (_StaticFlags&EnterTrigger) || (second._StaticFlags&EnterTrigger) ) )
		return true;

	// Is one of them is an exit trigger ?
	if ( exit && ( (_StaticFlags&ExitTrigger) || (second._StaticFlags&ExitTrigger) ) )
		return true;

	// Is one of them is a trigger ?
	if ( (_StaticFlags&OverlapTrigger) || (second._StaticFlags&OverlapTrigger) )
		return true;

	return false;
}

// ***************************************************************************

void CMovePrimitive::insertInWorldImage (uint8 worldImage)
{
	// Check it is a collisionable primitive
	nlassert (!isNonCollisionable());

	// Check ad get the primitive world image
	CPrimitiveWorldImage *wI=getWorldImage (worldImage);

	// Set as inserted
	wI->setInWorldImageFlag (true);

	// Flag to update this wI
	_Container->changed (this, worldImage);
}

// ***************************************************************************

void CMovePrimitive::removeFromWorldImage (uint8 worldImage)
{
	// Check it is a collisionable primitive
	nlassert (!isNonCollisionable());

	// Check ad get the primitive world image
	CPrimitiveWorldImage *wI=getWorldImage (worldImage);

	// Remove from cells
	wI->deleteIt (*_Container, worldImage);

	// Set as non inserted
	wI->setInWorldImageFlag (false);
}

// ***************************************************************************

void CMovePrimitive::setAbsorbtion (float attenuation)
{
	_Attenuation=attenuation;
}

// ***************************************************************************

void CMovePrimitive::setOrientation (double rot, uint8 worldImage)
{
	if (isNonCollisionable())
		getWorldImage (0)->setOrientation (rot, _Container, this, worldImage);
	else
		getWorldImage (worldImage)->setOrientation (rot, _Container, this, worldImage);
}

// ***************************************************************************

void CMovePrimitive::setGlobalPosition (const UGlobalPosition& pos, uint8 worldImage)
{
	NLPACS_HAUTO_SET_GLOBAL_POSITION

	if (isNonCollisionable())
		getWorldImage (0)->setGlobalPosition (pos, *_Container, *this, worldImage);
	else
		getWorldImage (worldImage)->setGlobalPosition (pos, *_Container, *this, worldImage);
}

// ***************************************************************************

void CMovePrimitive::setGlobalPosition (const NLMISC::CVectorD& pos, uint8 worldImage, UGlobalPosition::TType /* type */)
{
	NLPACS_HAUTO_SET_UGLOBAL_POSITION

	if (isNonCollisionable())
		getWorldImage (0)->setGlobalPosition (pos, *_Container, *this, worldImage, (_StaticFlags & DontSnapToGroundFlag) != 0);
	else
		getWorldImage (worldImage)->setGlobalPosition (pos, *_Container, *this, worldImage, (_StaticFlags & DontSnapToGroundFlag) != 0);
}

// ***************************************************************************

void CMovePrimitive::move (const NLMISC::CVectorD& speed, uint8 worldImage)
{
	NLPACS_HAUTO_MOVE

	if (isNonCollisionable())
		getWorldImage (0)->move (speed, *_Container, *this, worldImage);
	else
		getWorldImage (worldImage)->move (speed, *_Container, *this, worldImage);
}

// ***************************************************************************

NLMISC::CVectorD CMovePrimitive::getFinalPosition (uint8 worldImage)  const
{
	if (isNonCollisionable())
		return getWorldImage (0)->getFinalPosition ();
	else
		return getWorldImage (worldImage)->getFinalPosition ();
}

// ***************************************************************************

const NLMISC::CVectorD&	CMovePrimitive::getSpeed (uint8 worldImage) const
{
	if (isNonCollisionable())
		return getWorldImage (0)->getSpeed ();
	else
		return getWorldImage (worldImage)->getSpeed ();
}

// ***************************************************************************

CMovePrimitive::TType CMovePrimitive::getPrimitiveType () const
{
	return getPrimitiveTypeInternal ();
}

// ***************************************************************************

CMovePrimitive::TReaction CMovePrimitive::getReactionType () const
{
	return getReactionTypeInternal ();
}

// ***************************************************************************

CMovePrimitive::TTrigger CMovePrimitive::getTriggerType () const
{
	return getTriggerTypeInternal ();
}

// ***************************************************************************

CMovePrimitive::TCollisionMask CMovePrimitive::getCollisionMask () const
{
	return getCollisionMaskInternal ();
}

// ***************************************************************************

CMovePrimitive::TCollisionMask CMovePrimitive::getOcclusionMask () const
{
	return getOcclusionMaskInternal ();
}

// ***************************************************************************

bool CMovePrimitive::getObstacle () const
{
	return isObstacle ();
}

// ***************************************************************************

float CMovePrimitive::getAbsorbtion () const
{
	return _Attenuation;
}

// ***************************************************************************

void CMovePrimitive::getSize (float& width, float& depth) const
{
	width=getLength(0);
	depth=getLength(1);
}

// ***************************************************************************

float CMovePrimitive::getHeight () const
{
	return getHeightInternal ();
}

// ***************************************************************************

float CMovePrimitive::getRadius () const
{
	return getRadiusInternal ();
}

// ***************************************************************************

double CMovePrimitive::getOrientation (uint8 worldImage) const
{
	if (isNonCollisionable())
		return getWorldImage (0)->getOrientation ();
	else
		return getWorldImage (worldImage)->getOrientation ();
}

// ***************************************************************************

void CMovePrimitive::getGlobalPosition (UGlobalPosition& pos, uint8 worldImage) const
{
	NLPACS_HAUTO_GET_GLOBAL_POSITION

	if (isNonCollisionable())
		pos=getWorldImage (0)->getGlobalPosition();
	else
		pos=getWorldImage (worldImage)->getGlobalPosition();
}

// ***************************************************************************

uint8 CMovePrimitive::getFirstWorldImageV () const
{
	return getFirstWorldImage ();
}

// ***************************************************************************

uint8 CMovePrimitive::getNumWorldImageV () const
{
	return getNumWorldImage ();
}

// ***************************************************************************

bool CMovePrimitive::isInCollision (CMovePrimitive *primitive)
{
	// Should be ok
	CCollisionOTInfo	*element=_RootOTInfo;

	// Look for it
	while (element)
	{
		// Dynamic collision ?
		if (!element->isCollisionAgainstStatic())
		{
			// Cast
			const CCollisionOTDynamicInfo *dynInfo=static_cast<const CCollisionOTDynamicInfo*> (element);

			// Check if the primitive is used
			if ((dynInfo->getFirstPrimitive()== primitive)||(dynInfo->getSecondPrimitive()== primitive))
				return true;
		}

		// Look for next
		element=element->getNext (this);
	}

	return false;
}

} // NLPACS
