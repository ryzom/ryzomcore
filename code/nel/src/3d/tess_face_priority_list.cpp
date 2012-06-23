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

#include "std3d.h"

#include "nel/3d/tess_face_priority_list.h"
#include "nel/misc/debug.h"
#include <cmath>
#include "nel/3d/tessellation.h"
#include "nel/misc/fast_floor.h"


using	namespace NLMISC;
using	namespace std;

namespace NL3D
{

// ***************************************************************************
// ***************************************************************************
//	CTessFacePListNode
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CTessFacePListNode::linkInPList(CTessFacePListNode	&root)
{
	// unlink old list from me.
	_PrecTessFaceInPList->_NextTessFaceInPList= _NextTessFaceInPList;
	_NextTessFaceInPList->_PrecTessFaceInPList= _PrecTessFaceInPList;

	// link me to the list.
	_PrecTessFaceInPList= &root;
	_NextTessFaceInPList= root._NextTessFaceInPList;
	// link the list to me.
	root._NextTessFaceInPList->_PrecTessFaceInPList= this;
	root._NextTessFaceInPList= this;
	/*
		NB if list was empty (this, this), then
			_PrecTessFaceInPList= &root
			_NextTessFaceInPList= root._NextTessFaceInPList= &root !
			root._NextTessFaceInPList->_PrecTessFaceInPList= this;	=> root._PrecTessFaceInPList= this
			root._NextTessFaceInPList= this
	*/
}

// ***************************************************************************
void		CTessFacePListNode::unlinkInPList()
{
	/*
		NB: if this node was empty (this, this), this is a No-Op.
		If this node was the last of a list, then the root correctly get (&root, &root) after this.
	*/
	// unlink old list from me.
	_PrecTessFaceInPList->_NextTessFaceInPList= _NextTessFaceInPList;
	_NextTessFaceInPList->_PrecTessFaceInPList= _PrecTessFaceInPList;

	// reset to empty node.
	_PrecTessFaceInPList= this;
	_NextTessFaceInPList= this;
}


// ***************************************************************************
void		CTessFacePListNode::appendPList(CTessFacePListNode	&root)
{
	// If list to append is not empty.
	if( root._NextTessFaceInPList != &root )
	{
		// If we are empty.
		if( _NextTessFaceInPList==this )
		{
			// link the appendList to the root.
			_PrecTessFaceInPList= root._PrecTessFaceInPList;
			_NextTessFaceInPList= root._NextTessFaceInPList;
			// link the root to the appendList.
			_PrecTessFaceInPList->_NextTessFaceInPList= this;
			_NextTessFaceInPList->_PrecTessFaceInPList= this;
		}
		// else bind first-last in the interval prec-next.
		else
		{
			CTessFacePListNode		*first= root._NextTessFaceInPList;
			CTessFacePListNode		*last= root._PrecTessFaceInPList;
			CTessFacePListNode		*prec= this;
			CTessFacePListNode		*next= _NextTessFaceInPList;
			// insert the appendList in our list.
			next->_PrecTessFaceInPList= last;
			prec->_NextTessFaceInPList= first;
			// insert our list in the appendList.
			last->_NextTessFaceInPList= next;
			first->_PrecTessFaceInPList= prec;
		}

		// clear input list.
		root._PrecTessFaceInPList= &root;
		root._NextTessFaceInPList= &root;
	}
}



// ***************************************************************************
// ***************************************************************************
//	CTessFacePriorityList
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTessFacePriorityList::CTessFacePriorityList()
{
	_OODistStep= 1;
	_NEntries= 0;
	_MaskEntries= 0;
	_EntryModStart= 0;
	_NumQuadrant= 0;
}

// ***************************************************************************
CTessFacePriorityList::~CTessFacePriorityList()
{
	clear();
}

// ***************************************************************************
void		CTessFacePriorityList::init(float distStep, uint numEntries, float distMaxMod, uint numQuadrant)
{
	uint i;
	nlassert(distStep>0);
	nlassert(numEntries>0 && isPowerOf2(numEntries));
	nlassert(distMaxMod<1);
	nlassert(numQuadrant==0 || (numQuadrant>=4 && isPowerOf2(numQuadrant)) );

	// clear the prioriy list before.
	clear();

	// setup
	_OODistStep= 1.0f / distStep;
	_NEntries= numEntries;
	_MaskEntries= _NEntries-1;
	_EntryModStart= (uint)ceil(distMaxMod * _NEntries);
	NLMISC::clamp(_EntryModStart, 0U, _NEntries-1);
	_NumQuadrant= numQuadrant;

	// Build the Rolling tables.
	_RollingTables.resize(1+_NumQuadrant);
	for(i=0;i<_RollingTables.size();i++)
	{
		_RollingTables[i].init(_NEntries);
		// setup the quadrant direction. NB: 0 not used since "Direction less rolling table"
		if(i>=1)
		{
			uint	idx= i-1;
			// split evenly the plane with direction.
			float	angle= float(2*Pi*idx)/_NumQuadrant;
			CMatrix	mat;
			mat.rotateZ(angle);
			// setup the vector
			_RollingTables[i].QuadrantDirection= mat.getJ();
		}
	}

	// Build Quarter Selection. In is in CCW order. Out: see selectQuadrant()
	const	uint	quarterLut[4]= {1,3,2,0};
	_MaskQuadrant= _NumQuadrant-1;
	for(i=0;i<4;i++)
	{
		// Re-Order for faster ASM acces. see selectQuadrant()
		uint	idx= quarterLut[i];
		_QuarterQuadrantStart[idx]= i*_NumQuadrant/4;
		_QuarterQuadrantEnd[idx]= 1+ (i+1)*_NumQuadrant/4;
	}

}

// ***************************************************************************
void		CTessFacePriorityList::clear()
{
	// just clear all the rolling tables.
	_RollingTables.clear();
}


// ***************************************************************************
uint		CTessFacePriorityList::selectQuadrant(const CVector &direction)
{
	// if numQuadrants=0, ret 0.
	if(_NumQuadrant==0)
		return 0;


	// select the quarter.
	uint	quarterId;
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	__asm
	{
		mov		esi, direction
		mov		eax, [esi]direction.y
		mov		ebx, [esi]direction.x
		// read the sign
		and		eax, 0x80000000
		and		ebx, 0x80000000
		// set the bit
		shr		eax, 30
		shr		ebx, 31
		// assemble
		or		eax, ebx
		mov		quarterId, eax
	}
#else
	if(direction.y>0)
	{
		if(direction.x>0)	quarterId= 0;
		else				quarterId= 1;
	}
	else
	{
		if(direction.x>0)	quarterId= 2;
		else				quarterId= 3;
	}
#endif

	// Run only the quadrants of the associated quarter.
	uint	quadrantStart= _QuarterQuadrantStart[quarterId];
	uint	quadrantEnd= _QuarterQuadrantEnd[quarterId];


	// For all quadrants of the quarter, return the best direction
	float	bestDirPs= -FLT_MAX;
	uint	bestQuadrant= 0;
	for(uint i=quadrantStart;i<quadrantEnd;i++)
	{
		// get the quadrant Id. Must mask for last (0). And Start at 1 in the _RollingTables.
//		uint	quadrantId= (i&_MaskQuadrant)+1;
		// Test with this quadrant
		float	ps= direction*_RollingTables[i].QuadrantDirection;
		if(ps>bestDirPs)
		{
			bestDirPs= ps;
			bestQuadrant= i;
		}
	}

	return bestQuadrant;
}


// ***************************************************************************
void		CTessFacePriorityList::insert(uint quadrantId, float distance, CTessFace *value)
{
	// plist must be inited.
	nlassert(_NEntries>0 && quadrantId<_RollingTables.size());

	// First, setup in our basis.
	distance*= _OODistStep;

	// Insert int the good quadrant
	CRollingTable	&rollTable= _RollingTables[quadrantId];

	// Then, look where we must insert it.
	sint	idInsert;
	if(distance<=0)
		idInsert= 0;
	else
	{
		// Must insert so we can't miss it when a shift occurs (=> floor).
		idInsert= NLMISC::OptFastFloor(distance + rollTable.Remainder);
	}
	idInsert= std::max(0, idInsert);

	// Manage Mod.
	// If the element to insert must be inserted at  distance > distMax.
	if(idInsert>(sint)_MaskEntries)
	{
		// Compute number of entries to apply the mod.
		uint	nMod= _NEntries - _EntryModStart;
		// Of how many entries are we too far.
		idInsert= idInsert - _MaskEntries;
		// Then loop in the interval [_EntryModStart, _NEntries[.
		idInsert= idInsert % nMod;
		idInsert= _EntryModStart + idInsert;
	}

	// insert in the Roll Table.
	rollTable.insertInRollTable(idInsert, value);
}


// ***************************************************************************
void		CTessFacePriorityList::shift(const CVector &direction, CTessFacePListNode	&pulledElements)
{
	// plist must be inited.
	nlassert(_NEntries>0);

	pulledElements.unlinkInPList();

	// Shift all the rolling tables
	for(uint i=0;i<_RollingTables.size();i++)
	{
		CRollingTable	&rollTable= _RollingTables[i];
		float	shiftDistance;

		// if quadrant 0, Direction less => get distance with Euler norm.
		if(i==0)
		{
			shiftDistance= direction.norm();
		}
		// Else compute the effective distance we run to the plane.
		else
		{
			shiftDistance= direction*rollTable.QuadrantDirection;
			// For now, just clamp, but may be interesting to shift Back the rolling table !!
			shiftDistance= max(0.f, shiftDistance);
		}

		// First, setup in our basis.
		shiftDistance*= _OODistStep;

		// at least, fill OUT with elements of entry 0.
		// For all elements of the entry 0 of , pull them, and insert in result list.
		pulledElements.appendPList(rollTable.getRollTableEntry(0));

		// shift.
		rollTable.Remainder+= shiftDistance;
		// If Remainder>=1, it means that we must shift the rolling table, and get elements deleted.
		uint	entryShift= (uint)floor(rollTable.Remainder);
		rollTable.Remainder= rollTable.Remainder - entryShift;

		// shift full array??
		if( entryShift >= _NEntries)
		{
			entryShift= _NEntries;
			// The entire array is pulled, _Remainder should get a good value.
			rollTable.Remainder= 0;
		}

		// If some real shift, do it.
		rollTable.shiftEntries(entryShift, pulledElements);
	}
}


// ***************************************************************************
void		CTessFacePriorityList::shiftAll(CTessFacePListNode	&pulledElements)
{
	// plist must be inited.
	nlassert(_NEntries>0);

	pulledElements.unlinkInPList();

	// Do it for all rolling tables.
	for(uint i=0;i<_RollingTables.size();i++)
	{
		// The entire array is pulled, _Remainder should get a good value.
		uint entryShift= _NEntries;
		_RollingTables[i].Remainder= 0;

		// shift the entire array.
		_RollingTables[i].shiftEntries(entryShift, pulledElements);
	}
}


// ***************************************************************************
// ***************************************************************************
// Rolling table.
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
CTessFacePriorityList::CRollingTable::CRollingTable()
{
	_EntryStart= 0;
	_NEntries= 0;
	_MaskEntries= 0;
	Remainder= 0;
}

// ***************************************************************************
CTessFacePriorityList::CRollingTable::~CRollingTable()
{
	clearRollTable();
}


// ***************************************************************************
void		CTessFacePriorityList::CRollingTable::init(uint numEntries)
{
	_EntryStart= 0;
	_NEntries= numEntries;
	_MaskEntries= _NEntries-1;
	Remainder= 0;
	_Entries.resize(numEntries);
}


// ***************************************************************************
void		CTessFacePriorityList::CRollingTable::insertInRollTable(uint entry, CTessFace *value)
{
	CTessFacePListNode	&root= _Entries[ (entry + _EntryStart) & _MaskEntries ];

	// Insert into list.
	value->linkInPList(root);
}

// ***************************************************************************
CTessFacePListNode	&CTessFacePriorityList::CRollingTable::getRollTableEntry(uint entry)
{
	CTessFacePListNode	&root= _Entries[ (entry + _EntryStart) & _MaskEntries ];
	return root;
}

// ***************************************************************************
void		CTessFacePriorityList::CRollingTable::clearRollTableEntry(uint entry)
{
	CTessFacePListNode	&root= _Entries[ (entry + _EntryStart) & _MaskEntries ];

	// clear all the list.
	while( root.nextInPList() != &root )
	{
		// unlink from list
		CTessFacePListNode	*node= root.nextInPList();
		node->unlinkInPList();
	}
}

// ***************************************************************************
void		CTessFacePriorityList::CRollingTable::shiftRollTable(uint shiftEntry)
{
	// delete all elements shifted.
	for(uint i=0; i<shiftEntry; i++)
	{
		clearRollTableEntry(i);
	}
	// shift to right the ptr of entries.
	_EntryStart+= shiftEntry;
	_EntryStart= _EntryStart & _MaskEntries;
}

// ***************************************************************************
void		CTessFacePriorityList::CRollingTable::clearRollTable()
{
	for(uint i=0; i<_NEntries; i++)
	{
		clearRollTableEntry(i);
	}
	_EntryStart= 0;
	// For convenience only (not really useful).
	Remainder= 0;
}

// ***************************************************************************
void		CTessFacePriorityList::CRollingTable::shiftEntries(uint entryShift, CTessFacePListNode	&pulledElements)
{
	if(entryShift>0)
	{
		// before shifting the roll Table, fill pulledElements.
		for(uint i=0; i<entryShift; i++)
		{
			// For all elements of the ith entry, pull them and isnert in result list.
			pulledElements.appendPList(getRollTableEntry(i));
		}

		// shift the roll Table. lists are already empty.
		shiftRollTable(entryShift);
	}
}




} // NL3D
