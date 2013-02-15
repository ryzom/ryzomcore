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



#ifndef NL_DBGROUP_LIST_SHEET_H
#define NL_DBGROUP_LIST_SHEET_H

#include "nel/misc/types_nl.h"
#include "list_sheet_base.h"
#include "dbctrl_sheet.h"

namespace NLGUI
{
	class CCtrlBaseButton;
	class CCtrlScroll;
	class CGroupContainer;
}

// ***************************************************************************
/**
 * A List of item (ctrl sheet)
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CDBGroupListSheet : public IListSheetBase
{
public:

	/// Constructor
	CDBGroupListSheet(const TCtorParam &param);
	virtual ~CDBGroupListSheet();	// AJM: make base class destructors virtual to avoid memory leaks!

	/// CInterfaceGroup Interface
	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
	virtual void updateCoords ();
	virtual void checkCoords ();
	virtual void draw ();
	virtual void clearViews ();
	virtual bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);

	// mod interface
	void	changeNbRow(sint delta);

	/// \name IListSheetBase implementation
	// @{
	// Get the index of a sheet inserted in this list. Returns -1 if it is not an index of that list
	virtual	sint			getIndexOf(const CDBCtrlSheet	*sheet) const;
	// get number of sheets
	virtual uint			getNbSheet() const { return (uint)_SheetChildren.size(); }
	// get a sheet by its index
	virtual	CDBCtrlSheet	*getSheet(uint index) const;
	// Get the number of active elements
	virtual	sint32			getNbElt () const;
	// Get the scroll bar
	virtual	CCtrlScroll		*getScrollBar() const { return	_ScrollBar; }
	// get Db Branch name
	virtual const std::string &getDbBranchName() const {return _DbBranchName;}
	// @}

	// Called when the list changed for a reason or another and should be reconstructed to possibly sort items
	virtual void			sort() { }
	void					needToSort() { _NeedToSort = true; invalidateCoords(); }

	/** (useful for list only) Force the validity of an element, even if its sheetId==0
	 *	(empty slot displayed instead)
	 *	NB: invalidateCoords() called if state is changed
	 */
	void					forceValidity(uint element, bool forceValid);

public:

	// A child node
	struct	CSheetChild
	{
		// the ctrl added to the group
		CDBCtrlSheet	*Ctrl;
		// a cache, updated in updateCoords, and checked in draw() if !_Array
		sint32			SheetId;
		// Place in the item list (NB: before scrolling)
		uint			XItem;
		uint			YItem;
		// true if the item is valid (NB: but maybe not displayed)
		bool			Valid;
		// true if the item is forced to be valid
		bool			ForceValid;

		virtual void init(CDBGroupListSheet * /* pFather */, uint /* index */) {}
		virtual bool isInvalidated(CDBGroupListSheet * /* pFather */) { return false; }
		virtual void update(CDBGroupListSheet * /* pFather */) { }
		virtual sint getSectionId() const {return 0;}
		// test is that sheet is valid (is it to be displayed)
		virtual bool isSheetValid(CDBGroupListSheet *pFather);

		CSheetChild()
		{
			Ctrl= NULL;
			SheetId= 0;
			ForceValid= false;
		}
		virtual ~CSheetChild() { }

	};

	// Override if you want different child mangament
	virtual CSheetChild		*createSheetChild() { return new CSheetChild; }

	void setCanDrop(bool b) { _CanDrop = b; }
	bool getCanDrop() const { return _CanDrop; }

	const CCtrlSheetInfo &getCtrlSheetInfo() { return _CtrlInfo; }

protected:

	// Number of max elements displayed.
	sint						_MinRows;
	sint						_MaxRows;
	uint						_MaxItems;
	uint						_NbColumns;
	uint						_ColumnMax;
	uint						_ColumnFactor;
	sint						_LeftMargin;
	sint						_RightMargin;
	sint						_TopMargin;
	sint						_BottomMargin;

	sint						_CurWantedRows;
	sint						_LastScrollY;


	// branch of the DB
	NLMISC::CCDBNodeBranch				*_DbBranch;
	std::string					_DbBranchName;
	// Branch observer
	class CDBObs : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		CDBGroupListSheet		*Owner;
		virtual void update(NLMISC::ICDBNode* /* node */)	{Owner->_BranchModified= true;}
	};
	friend class CDBObs;
	CDBObs						_DbBranchObs;
	bool						_BranchModified;


	// Common Info for ctrl and group
	CCtrlSheetInfo				_CtrlInfo;
	// size of an item + extra space
	sint32						_WSlot, _HSlot;
	// size of the space only
	sint32						_WSpace, _HSpace;
	// True if must center the sheets
	bool						_ColumnCenter		: 1;
	bool						_NeedToSort         : 1;
	bool						_DisplayEmptySlot   : 1;
	bool						_Array              : 1;
	bool						_Squarify           : 1;

	bool						_CanDrop			: 1;

	// Children
	bool						_Setuped;
	CCtrlBaseButton				*_ButtonAddRow;
	CCtrlBaseButton				*_ButtonSubRow;
	CCtrlScroll					*_ScrollBar;
	CInterfaceGroup				*_DummyGroup;
	// The Start index in the database
	sint						_StartDbIdx;
	std::vector<CSheetChild*>	_SheetChildren;

	void		setup();
	sint		getCurScrollValue();
	CGroupContainer *getContainer();

	bool		isChildValid(uint i) const
	{
		if(i>=_SheetChildren.size())
			return false;
		return _SheetChildren[i]->ForceValid || _SheetChildren[i]->isSheetValid(const_cast<CDBGroupListSheet*>(this));
	}

	// For animals only
	NLMISC::CCDBNodeLeaf				*_AnimalStatus;
	sint32						_CacheAnimalStatus;

	// For sectionnable purpose
	std::vector<uint>			_SectionYItems;

	// for CDBGroupListSheetBonusMalus: list, but leave space between empty ctrls
	bool						_ListLeaveSpace;

	// section
	void	insertSectionGroupList(sint firstSectionId, sint lastSectionId, uint &groupSectionIndex, sint &yItem, sint xStart, sint &xItem);
};


#endif // NL_DBGROUP_LIST_SHEET_H

/* End of dbgroup_list_sheet.h */
