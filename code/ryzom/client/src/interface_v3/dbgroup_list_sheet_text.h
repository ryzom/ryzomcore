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



#ifndef NL_DBGROUP_LIST_SHEET_TEXT_H
#define NL_DBGROUP_LIST_SHEET_TEXT_H

#include "nel/misc/types_nl.h"


#include "nel/misc/types_nl.h"
#include "list_sheet_base.h"
#include "dbctrl_sheet.h"
#include "nel/gui/view_text.h"

namespace NLGUI
{
	class CCtrlButton;
	class CCtrlScroll;
	class CGroupContainer;
}


// ***************************************************************************
class	CHandlerListSheetTradeSelect;
class	CHandlerListSheetTradeRightClick;

// ***************************************************************************
/**
 * List of sheet with a text. Selectable by line.
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003
 */
class CDBGroupListSheetText : public IListSheetBase
{
public:

	/// Constructor
	CDBGroupListSheetText(const TCtorParam &param);
	virtual	~CDBGroupListSheetText();	// AJM: make base class destructors virtual to avoid memory leaks!

	virtual uint getNbSheet() const { return (uint)_SheetChildren.size(); }

	/// CInterfaceGroup Interface
	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
	virtual void updateCoords ();
	virtual void checkCoords ();
	virtual void draw ();
	virtual void clearViews ();
	virtual bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);

	/// \name IListSheetBase implementation
	// @{
	// Get the index of a sheet inserted in this list. Returns -1 if it is not an index of that list
	virtual	sint			getIndexOf(const CDBCtrlSheet *sheet) const;

	// get a sheet by its index
	virtual	CDBCtrlSheet	*getSheet(uint index) const;
	// Get the number of active elements
	virtual	sint32			getNbElt () const;
	// Get the scroll bar
	virtual	CCtrlScroll		*getScrollBar() const { return	_ScrollBar; }
	// get Db Branch name
	virtual const std::string &getDbBranchName() const {return _DbBranchName;}
	// @}

	// unselect any pushed button
	void					 unselect();

	// Helper : if this sheet db list is updated from the server, then it can know when the download is finished
	// If, because of a filter, the list is empty, a replacement message can be shown to inform the player
	void					  notifyDownloadComplete(bool downloadComplete);

	//////////////////////////////////////////////////////////////////////////

	friend class	CHandlerListSheetTextSelect;
	friend class	CHandlerListSheetTextRightClick;

	// A child node
	struct	CSheetChild
	{
		// the owner of this sheet child
		CDBGroupListSheetText	*Owner;
		// the ctrl added to the group
		CDBCtrlSheet		*Ctrl;
		// The view text for infos
		CViewText			*Text;
		// The global button to select this item.
		CCtrlButton			*Button;
		// a cache, updated in updateCoords, and checked in checkCoords()
		sint32				SheetId;
		// For Item only, cache for NameId, updated in updateCoords, and checked in checkCoords()
		sint32				NameId;
		// For Item  only, cache for raw material class & faber stat
		sint32				RMClassType;
		sint32				RMFaberStatType;
		// true if the item is valid (NB: but maybe not displayed)
		bool				Valid;
		// true if need to validate the text at next updateCoord
		bool				NeedUpdateText;
		// Position of the item in the list.
		sint32				YItem;

		// Called at setup to permit to initialize derived data
		virtual void init(CDBGroupListSheetText *pFather, uint index);

		// Called at checkCoord to know if we have to rebuild the child
		// if _CheckCoordAccelerated is true (default), called at each DB modif, else called each frame
		// NB: if CDBGroupListSheetText::needCheckAll() return true, all are tested too
		virtual bool isInvalidated(CDBGroupListSheetText * /* pFather */) { return false; }

		// Called at updateCoords to update cache ...
		virtual void update(CDBGroupListSheetText * /* pFather */) { }

		// Called at updateCoords to remake the text
		virtual void updateViewText(CDBGroupListSheetText * /* pFather */)
		{
			ucstring text;
			Ctrl->getContextHelp(text);
			Text->setText(text);
		}

		// create a CViewText object that is displayed next to the item. This is the opportunnity to create
		// a derived class such as CViewTextID
		virtual CViewText *createViewText() const { return new CViewText(CViewBase::TCtorParam()); }

		// test is that sheet is valid (is it displayed)
		virtual bool isSheetValid(CDBGroupListSheetText *pFather);

		// Hide the child (when not valid). Opptortunity to hide other elements
		virtual void hide(CDBGroupListSheetText *pFather);

		// return the Child X delta (does not affect button, just CtrlSheet and Text)
		virtual	sint getDeltaX(CDBGroupListSheetText * /* pFather */) const {return 0;}

		// for listsheet sectionable, return the sectionId associated with this sheet
		virtual	sint getSectionId() const {return 0;}


		CSheetChild()
		{
			Owner= NULL;
			Ctrl= NULL;
			Text= NULL;
			Button= NULL;
			SheetId= 0;
			NameId= 0;
			NeedUpdateText= true;
		}
		virtual ~CSheetChild();

	protected:
		// tool method to setup an common item text. used by ListSheetBag and ListSheetTextShare for instance
		void		updateViewTextAsItem();
	};

	// Override if you want different child mangament
	virtual CSheetChild		*createSheetChild() { return new CSheetChild; }

	// true if this list is able to swap(). if yes, then the list support "scroll when drag"
	virtual bool			swapable() const {return false;}
	// Called when we drag'n'drop a control sheet of this list on another of this list. Default is no op
	virtual void			onSwap (sint /* nDraggedSheet */, sint /* nDroppedSheet */) {}

	// Called when the list changed for a reason or another and should be reconstructed to possibly sort items
	virtual void			sort() { }
	void					needToSort() { _NeedToSort = true; invalidateCoords(); }

	/// Gets.
	sint32					getWSlot() const {return _WSlot;}
	sint32					getHSlot() const {return _HSlot;}
	sint32					getHSpace() const {return _HSpace;}
	sint32					getXItem() const {return _XItem;}
	sint32					getYItem() const {return _YItem;}
	sint32					getXText() const {return _XText;}
	sint32					getYText() const {return _YText;}


	void setCanDrop(bool b) { _CanDrop = b; }
	bool getCanDrop() const { return _CanDrop; }

	const CCtrlSheetInfo &getCtrlSheetInfo() { return _CtrlInfo; }

	CGroupContainer *getContainer();

protected:
	friend struct CSheetChild;

	// Number of max elements displayed.
	bool						_Array;
	uint						_MaxItems;
	bool						_SelectionEnabled;


	// branch of the DB
	NLMISC::CCDBNodeBranch				*_DbBranch;
	std::string					_DbBranchName;
	// Branch observer
	class CDBObs : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		CDBGroupListSheetText	*Owner;
		virtual void update(NLMISC::ICDBNode* /* node */)	{Owner->_BranchModified= true;}
	};
	friend class CDBObs;
	CDBObs						_DbBranchObs;
	bool						_BranchModified;
	bool						_CheckCoordAccelerated;
	bool						_GrayTextWithCtrlState;

	bool						_CanDrop;

	// Common Info for ctrl and group
	CCtrlSheetInfo				_CtrlInfo;
	// size of an item + extra space
	sint32						_WSlot, _HSlot;
	// size of the space only
	sint32						_HSpace;
	// Placement of the ctrl in the row
	sint32						_XItem, _YItem;
	// Placement of the viewtext in the row
	sint32						_XText, _YText;

	bool						_Setuped;
	CCtrlScroll					*_ScrollBar;
	CInterfaceGroup				*_List;
	NLMISC::CRefPtr<CInterfaceElement> _EmptyListNotifier;
	bool						_DownloadComplete;

	// The Start index in the database
	sint						_StartDbIdx;
	// All the children
	std::vector<CSheetChild*>	_SheetChildren;

	bool						_NeedToSort;
	// Drag'n'Drop
	sint32						_Scrolling;
	sint64						_LastTimeScrolled;

	// Button Colors
	NLMISC::CRGBA				_OverColorNormal;
	NLMISC::CRGBA				_OverColorOver;
	NLMISC::CRGBA				_OverColorPushed;

	// Text template Aspect
	CViewText					_TextTemplate;

	// generate a click on pushed item ?
	bool						_ClickWhenPushed;


	uint						_NumValidSheets; 

	// List of sheet child to update when dynstring received
	std::set< CSheetChild* >	_NameIdToUpdate;


	void		setup();
	sint		getIndexOf(const CCtrlButton *button) const;

	// For animals only
	NLMISC::CCDBNodeLeaf				*_AnimalStatus;
	sint32						_CacheAnimalStatus;

	// For items only (requirement color)
	sint32			_LastPlayerCharac[CHARACTERISTICS::NUM_CHARACTERISTICS];
	virtual bool	needCheckAllItems();
	bool			_NeedUpdateAllText;

	// section
	void	insertSectionGroupList(sint firstSectionId, sint lastSectionId, uint &groupSectionIndex, sint &yItem);

};


#endif // NL_DBGROUP_LIST_SHEET_TEXT_H

/* End of dbgroup_list_sheet_trade.h */
