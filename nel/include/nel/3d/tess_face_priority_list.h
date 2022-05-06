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

#ifndef NL_TESS_FACE_PRIORITY_LIST_H
#define NL_TESS_FACE_PRIORITY_LIST_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include <vector>


namespace NL3D
{


class	CTessFace;


// ***************************************************************************
/** A chain link node for PriorityList. NB: It is a circular list <=> (this,this) if list is empty
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTessFacePListNode
{
public:

	// init to empty list.
	CTessFacePListNode()
	{
		_PrecTessFaceInPList= this;
		_NextTessFaceInPList= this;
	}
	~CTessFacePListNode()
	{
		// if not done, unlink.
		unlinkInPList();
	}

	// Copy cons don't copy Link stuff
	CTessFacePListNode(const CTessFacePListNode &)
	{
		_PrecTessFaceInPList= this;
		_NextTessFaceInPList= this;
	}
	// Operator= don't copy Link stuff
	CTessFacePListNode	&operator= (const CTessFacePListNode &) {return *this;}

	/// unlinkInPList, then link this node to the root of a list.
	void		linkInPList(CTessFacePListNode	&root);
	/// if linked, unlink this node from his list.
	void		unlinkInPList();

	/** append a list just after this node. root is the root of the list. It is not inserted in the result.
	 *	After this, the list pointed by "root" is empty.
	 */
	void		appendPList(CTessFacePListNode	&root);

	/// get next ptr. next==this if list empty.
	CTessFacePListNode		*precInPList() const {return _PrecTessFaceInPList;}
	CTessFacePListNode		*nextInPList() const {return _NextTessFaceInPList;}

private:
	CTessFacePListNode		*_PrecTessFaceInPList;
	CTessFacePListNode		*_NextTessFaceInPList;

};


// ***************************************************************************
/** This class manage a Priority list of elements, inserted with a "distance". The priority list can be shifted, so
 *	elements with new distance <=0 are pulled from the priority list.
 *
 *	NB: it works quite well if you have (as example) a distStep of 1, and you shift only with value of 0.01 etc..,
 *	because it manages a "Remainder system", which do the good stuff.
 *	How does it works? in essence, it is a Rolling table, with 1+ shift back when necessary (maybe not at each shift()).
 *
 *	Additionally, it includes a "2D quadrant" notion. If you use this class to know when the "camera" enters
 *	a specific area, then Quadrants help you because the notion of direction is kept.
 *	Only the XY plane is used here.
 *
 *	Instead, if you use this class to know when the "camera" leaves a specific area, then the quadrant notion
 *	is not interesting since the camera can leave the area in any direction...
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTessFacePriorityList
{
public:

	/// Constructor
	CTessFacePriorityList();
	~CTessFacePriorityList();

	/** Clear and Init the priority list. It reserve (numQuadrants+1) Rolling table of numEntries entries.
	 *
	 *	\param numEntries gives the number of entries and MUST be powerOf2. distMax= numEntries*distStep
	 *	\parm distMaxMod is important for performance and MUST be < 1. eg: distMaxMod= 0.8.
	 *	It is a trick to avoid the "Too Far Priority problem". Imagine you have a setup such distMax= 1000,
	 *	and you insert(1100, an element). If we clamp to the max, it may be a bad thing, because ALL elements
	 *	inserted after 1000 will be poped in one shift(), after 1000 shift(1) for example.
	 *	To avoid this, if distMaxMod==0.8, then insert(1050) will really insert at 850, so elements will be poped
	 *	not as the same time (for the extra cost of some elements get poped too early...).
	 *
	 *	\param numQuadrant set 0 if don't want to support quadrant notion. else set >=4 and a power of 2 (else nlassert)
	 */
	void			init(float distStep, uint numEntries, float distMaxMod, uint numQuadrant);
	/** Clear the priority list. All elements are removed. NB: for convenience, the remainder is reset.
	 */
	void			clear();

	/**	prior to insert an element with a direction notion, select which quadrant according to the direction
	 *	Camera -> center of interest.
	 *	\return 0 if no quadrant available, else the quadrant id
	 */
	uint			selectQuadrant(const NLMISC::CVector &direction);

	/**	get the quadrant dir associated to the Id. Undefined if 0.
	 */
	const NLMISC::CVector	&getQuadrantDirection(uint quadrantId) const {return _RollingTables[quadrantId].QuadrantDirection;}

	/** Insert an element at a given distance, and in a given quadrant.
	 *	Insert at the closest step. eg insert(1.2, elt) will insert elt at entry 1 (assuming distStep==1 here).
	 *	Special case: if distance<=0, it is ensured that at each shift(), the element will be pulled.
	 *	NB: manage correctly the entry where it is inserted, according to the Remainder system.
	 *
	 *	USE OptFastFloor if distance>0 => MUST be enclosed in OptFastFloorBegin/OptFastFloorEnd()
	 *
	 *	\param quadrantId set 0 if you want to insert in the "direction less" rolling table. else set the value
	 *	returned by selectQuadrant()
	 *	\param distance if quadrantId==0, distance should be the norm()-SphereSize, else it shoudl be
	 *	direction*quadrantDirection-SphereSize.
	 */
	void			insert(uint quadrantId, float distance, CTessFace *value);

	/** Shift the Priority list in a given direction.
	 *	NB: for each rolling table, even if shiftDistance==0, all elements in the entry 0 are pulled out.
	 *	\parm pulledElements is the root of the list which will contains elements pulled from the list.
	 */
	void			shift(const NLMISC::CVector &direction, CTessFacePListNode	&pulledElements);

	/** Same as shift(), but shift all the array.
	 */
	void			shiftAll(CTessFacePListNode	&pulledElements);


// **************************
private:

	/*
		NB: Remainder E [0,1[.
		Meaning: value to substract to entries, to get their actual value.
		eg: the entry 1 means "1 unit" (in the internal basis, ie independent of distStep). If Remainder==0.1,
			then, entry 1 means "0.9 unit".
			Then, if, as example, an element must be inserted at dist=0.95, it will be inserted in entry 1, and not entry 0!
		NB: the "meaning" of entry 0 is always <=0.
	*/
	float			_Remainder;
	float			_OODistStep;
	uint			_NEntries;
	uint			_MaskEntries;	// == _NEntries-1
	uint			_EntryModStart;
	uint			_NumQuadrant;
	// For Fast Selection of Quadrant. Split the list of quadrant into 4. NB: ids start at 0. and must AND the Ids.
	uint			_MaskQuadrant;
	uint			_QuarterQuadrantStart[4];
	uint			_QuarterQuadrantEnd[4];


	/// \name The rolling tables
	// @{

	/// A single rolling table  <=> HTable.
	class		CRollingTable
	{
	public:
		// The quadrant Direction for this rolling table.
		NLMISC::CVector		QuadrantDirection;

		// computed and decremented by CTessFacePriorityList
		float		Remainder;

	public:
		CRollingTable();
		~CRollingTable();

		// init the roll table
		void		init(uint numEntries);

		// entry is relative to _EntryStart.
		void		insertInRollTable(uint entry, CTessFace *value);

		// get the root of the list at entry. entry is relative to _EntryStart.
		CTessFacePListNode		&getRollTableEntry(uint entry);

		// shift the rollingTable of shiftEntry. elements in entries shifted are deleted. no-op if 0.
		void		shiftRollTable(uint shiftEntry);

		// clear all elements of the roll table.
		void		clearRollTable();

		/// append elements shifted and shift
		void		shiftEntries(uint entryShift, CTessFacePListNode	&pulledElements);

	private:
		// where is the entry 0 in _Entries.
		std::vector<CTessFacePListNode>		_Entries;
		uint					_EntryStart;
		uint					_NEntries;
		uint					_MaskEntries;	// == _NEntries-1

		// clear all element of an entry of the roll table. entry is relative to _EntryStart.
		void		clearRollTableEntry(uint entry);

	};

	// There is 1+NumQuadrant rollingTable.
	std::vector<CRollingTable>		_RollingTables;

	// @}


};



} // NL3D


#endif // NL_TESS_FACE_PRIORITY_LIST_H

/* End of tess_face_priority_list.h */
