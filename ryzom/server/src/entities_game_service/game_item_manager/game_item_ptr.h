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

#ifndef GAME_ITEM_PTR_H
#define GAME_ITEM_PTR_H

#define ITEM_DEBUG


class CGameItem;

//------------------------------------------------------------------------
// ptr class added by sadge
// inline implementations are located at bottom of this file

class CGameItemPtr
{
public:
	// ctor	- default
	CGameItemPtr();

	// ctor	- copy
	CGameItemPtr(const CGameItemPtr &other);

	// ctor	- initialise from a CGameItem*
	CGameItemPtr(const CGameItem *item);

	~CGameItemPtr();

	// equivalent to: new CGameItem
	CGameItem *newItem(bool destroyable=true,bool dropable=true);

	// equivalent to: if (this==NULL) {return new CGameItem;} else {return this;}
	CGameItem *newItemIfNull(bool destroyable=true,bool dropable=true);

	// equivalent to: new CGameItem(...)
//	CGameItem *newItem( const NLMISC::CEntityId& id, const NLMISC::CSheetId& sheetId, uint32 recommended, sint16 slotCount, bool destroyable, bool dropable );
	CGameItem *newItem( const NLMISC::CSheetId& sheetId, uint32 recommended, bool destroyable, bool dropable );

	// equivalent to: delete (for a CGameItem*)
	void deleteItem();

	// * operator - returning the item referenced by this pointer
	CGameItem * operator*() const;

	// -> operator - returning the item referenced by this pointer
	CGameItem *operator->() const;

	// () operator - returning the item referenced by this pointer
	CGameItem *operator()() const;

	// assignment
	const CGameItemPtr &operator=(const CGameItemPtr &other);
	const CGameItemPtr &operator=(const CGameItem *item);

	// compare 2 CGameItemPtrs
	bool operator==(const CGameItemPtr &other) const;
	bool operator!=(const CGameItemPtr &other) const;

	// compare CGameItemPtrs to CGameItem*
	bool operator==(const CGameItem *item) const;
	bool operator!=(const CGameItem *item) const;

	uint32 getUniqueIndex() { return _idx; }

	// For set and map
	bool operator < (const CGameItemPtr &other) const;
private:
	// link to item
	void linkToItem();
	// unlink from item
	void unlinkFromItem();
	// init
	void reset();

	// index into the CGameItem singleton's _Items vector
	uint32 _idx;

	#ifdef ITEM_DEBUG
		// used for making sure that referenced items haven't been freed or reallocated
		// contains a copy of a CGameItem's allocation index (which changes at every alloaction and free)
		sint32 _debug;
		CGameItem* _debugPtr;
	#endif
};




#endif //GAME_ITEM_PTR_H
