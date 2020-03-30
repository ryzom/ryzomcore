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

#include "nel/pacs/move_cell.h"
#include "nel/pacs/move_element.h"
#include "nel/pacs/move_primitive.h"

namespace NLPACS
{

// ***************************************************************************

CMoveCell::CMoveCell()
{
	_FirstX=NULL;
	_LastX=NULL;
	/*_FirstY=NULL;
	_LastY=NULL;*/
}

// ***************************************************************************

void CMoveCell::unlinkX (CMoveElement *element)
{
	// Check first last
	if (_FirstX==element)
		_FirstX=element->NextX;
	if (_LastX==element)
		_LastX=element->PreviousX;

	// Relink to others
	if (element->NextX)
		element->NextX->PreviousX=element->PreviousX;
	if (element->PreviousX)
		element->PreviousX->NextX=element->NextX;
}

// ***************************************************************************

/*void CMoveCell::unlinkY (CMoveElement *element)
{
	// Linked in list ?
	// Check first / last
	if (_FirstY==element)
		_FirstY=element->NextY;
	if (_LastY==element)
		_LastY=element->PreviousY;

	// Relink to others
	if (element->NextY)
		element->NextY->PreviousY=element->PreviousY;
	if (element->PreviousY)
		element->PreviousY->NextY=element->NextY;
}*/

// ***************************************************************************

void CMoveCell::linkX (CMoveElement *previous, CMoveElement *element, CMoveElement *next)
{
	// Link the element
	element->NextX=next;
	element->PreviousX=previous;

	// Link to others
	if (previous)
		previous->NextX=element;
	if (next)
		next->PreviousX=element;

	// Check first / last
	if (previous==NULL)
		_FirstX=element;
	if (next==NULL)
		_LastX=element;
}

// ***************************************************************************

/*void CMoveCell::linkY (CMoveElement *previous, CMoveElement *element, CMoveElement *next)
{
	// Link the element
	element->NextY=next;
	element->PreviousY=previous;

	// Link to others
	if (previous)
		previous->NextY=element;
	if (next)
		next->PreviousY=element;

	// Check first / last
	if (previous==NULL)
		_FirstY=element;
	if (next==NULL)
		_LastY=element;
}*/

// ***************************************************************************

void CMoveCell::updateSortedLists (CMoveElement *element, uint8 worldImage)
{
	// ** Update sorted list on X

	// Primitive pointer
	CMovePrimitive *primitive=element->Primitive;

	// Get the world image
	CPrimitiveWorldImage *wI=primitive->getWorldImage (worldImage);

	// Test if we will go to the right
	CMoveElement *ptr=element->NextX;
	if (ptr && (wI->getBBXMin() > ptr->Primitive->getWorldImage (worldImage)->getBBXMin()) )
	{
		// Unlink
		unlinkX (element);

		// Adjust the list localisation
		while (ptr->NextX && (wI->getBBXMin() > ptr->NextX->Primitive->getWorldImage (worldImage)->getBBXMin()) )
		{
			// Next ptr
			ptr=ptr->NextX;
		}

		// Here we go
		linkX (ptr, element, ptr->NextX);
	}
	else
	{
		// Test if we will go to the left
		ptr=element->PreviousX;
		if (ptr && (ptr->Primitive->getWorldImage (worldImage)->getBBXMin() > wI->getBBXMin()) )
		{
			// Unlink
			unlinkX (element);

			// Adjust the list localisation
			while (ptr->PreviousX && (ptr->PreviousX->Primitive->getWorldImage (worldImage)->getBBXMin() > wI->getBBXMin()) )
			{
				// Next ptr
				ptr=ptr->PreviousX;
			}

			// Here we go
			linkX (ptr->PreviousX, element, ptr);
		}
	}

/*	// ** Update sorted list on Y

	// Test if we will go to the right
	ptr=element->NextY;
	if (ptr && (primitive->getBBYMin() > ptr->Primitive->getBBYMin()) )
	{
		// Unlink
		unlinkY (element);

		// Adjust the list localisation
		while (ptr->NextY && (primitive->getBBYMin() > ptr->NextY->Primitive->getBBYMin()) )
		{
			// Next ptr
			ptr=ptr->NextY;
		}

		// Here we go
		linkY (ptr, element, ptr->NextY);
	}
	else
	{
		// Test if we will go to the left
		ptr=element->PreviousY;
		if (ptr && (ptr->Primitive->getBBYMin() > primitive->getBBYMin()) )
		{
			// Unlink
			unlinkY (element);

			// Adjust the list localisation
			while (ptr->PreviousY && (ptr->PreviousY->Primitive->getBBYMin() > primitive->getBBYMin()) )
			{
				// Next ptr
				ptr=ptr->PreviousY;
			}

			// Here we go
			linkY (ptr->PreviousY, element, ptr);
		}
	}*/
}

// ***************************************************************************

} // NLPACS
