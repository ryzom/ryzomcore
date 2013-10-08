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



#include "stdpch.h"


#include "dbgroup_list_sheet.h"
#include "nel/gui/group_container.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/interface_property.h"
#include "interface_manager.h"
#include "nel/gui/action_handler.h"
#include "../sheet_manager.h"
#include "game_share/animal_status.h"


extern CSheetManager SheetMngr;


using namespace std;
using namespace NL3D;
using namespace NLMISC;


// ***************************************************************************
bool CDBGroupListSheet::CSheetChild::isSheetValid(CDBGroupListSheet * /* pFather */)
{
	if(!Ctrl)	return false;
	return Ctrl->isSheetValid();
}

NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheet, std::string, "list_sheet");

// ***************************************************************************
CDBGroupListSheet::CDBGroupListSheet(const TCtorParam &param)
:	IListSheetBase(param)
{
	_CurWantedRows= 1;
	_MinRows= 1;
	_MaxRows= INT_MAX;
	_MaxItems= INT_MAX;
	_NbColumns= 2;
	_WSlot= 24;
	_WSpace= 0;
	_HSpace= 0;
	_ButtonAddRow= NULL;
	_ButtonSubRow= NULL;
	_ScrollBar= NULL;
	_DummyGroup= NULL;
	_Setuped= false;
	_DbBranch= NULL;
	_Array= true;
	_LeftMargin= 0;
	_RightMargin= 0;
	_TopMargin= 0;
	_BottomMargin= 0;
	_StartDbIdx= 0;
	_ColumnMax= std::numeric_limits<uint>::max();
	_ColumnFactor= 1;
	_ColumnCenter= false;
	_NeedToSort = true;
	_DisplayEmptySlot = false;
	_Squarify = false;

	_DbBranchObs.Owner= this;
	_BranchModified= true;

	_AnimalStatus= NULL;
	_CacheAnimalStatus= -1;

	_ListLeaveSpace= false;
}

// ***************************************************************************
CDBGroupListSheet::~CDBGroupListSheet()
{
	// delete the dummy group (not added to the group list because system)
	if(_DummyGroup)
		delete _DummyGroup;
	// delete my children
	for(uint i=0;i<_SheetChildren.size();i++)
		delete _SheetChildren[i];
}


// ***************************************************************************
bool CDBGroupListSheet::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if(!CInterfaceGroup::parse(cur, parentGroup))
		return false;

	// read params
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CXMLAutoPtr prop;

	// value
	prop = xmlGetProp (cur, (xmlChar*)"value");
	if (prop)
	{
		// get a branch in the database.
		CCDBNodeBranch *branch= NLGUI::CDBManager::getInstance()->getDbBranch(prop);
		if(!branch)
		{
			nlinfo ("Branch not found in the database %s", (const char*)prop);
			return false;
		}
		// store
		_DbBranch= branch;
		_DbBranchName= (const char*)prop;
		// add observer
		NLGUI::CDBManager::getInstance()->addBranchObserver(branch, &_DbBranchObs);
	}

	// parse the common ctrl info
	if(!_CtrlInfo.parseCtrlInfo(cur, parentGroup))
		return false;

	// If macro has no value -> error
	if (_CtrlInfo._Type != CCtrlSheetInfo::SheetType_Macro)
	{
		if (!prop)
		{
			nlinfo ("no value in %s", _Id.c_str());
			return false;
		}
	}

	// get item size.
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	sint32	dispSlotBmpId = 0;
	switch(_CtrlInfo._Type)
	{
		case CCtrlSheetInfo::SheetType_Auto:
		case CCtrlSheetInfo::SheetType_Pact:
		case CCtrlSheetInfo::SheetType_Item:  dispSlotBmpId = rVR.getTextureIdFromName ("w_slot_item.tga"); break;
		case CCtrlSheetInfo::SheetType_SBrick:
		case CCtrlSheetInfo::SheetType_SPhraseId:
		case CCtrlSheetInfo::SheetType_SPhrase:
			if(_CtrlInfo._BrickOverable)
				dispSlotBmpId = rVR.getTextureIdFromName ("w_slot_spell.tga");
			else
				dispSlotBmpId = rVR.getTextureIdFromName ("w_slot_brick.tga");
			break;
		case CCtrlSheetInfo::SheetType_Macro: dispSlotBmpId = rVR.getTextureIdFromName ("w_slot_brick.tga"); break;
		default: break;
	}
	rVR.getTextureSizeFromId (dispSlotBmpId, _WSlot, _HSlot);

	// add extra space
	prop = (char*) xmlGetProp( cur, (xmlChar*)"wspace" );
	if (prop)
	{
		fromString((const char*)prop, _WSpace);
		_WSlot+= _WSpace;
	}
	prop = (char*) xmlGetProp( cur, (xmlChar*)"hspace" );
	if (prop)
	{
		fromString((const char*)prop, _HSpace);
		_HSlot+= _HSpace;
	}

	// read array. if true, empty slots are displayed, else only correct sheet id are displayed
	prop = (char*) xmlGetProp( cur, (xmlChar*)"array" );
	if(prop)
	{
		_Array= convertBool(prop);
	}

	// read margin
	prop = (char*) xmlGetProp( cur, (xmlChar*)"lmargin" );
	if (prop)	fromString((const char*)prop, _LeftMargin);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"rmargin" );
	if (prop)	fromString((const char*)prop, _RightMargin);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"tmargin" );
	if (prop)	fromString((const char*)prop, _TopMargin);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"bmargin" );
	if (prop)	fromString((const char*)prop, _BottomMargin);

	// read row setup
	prop = (char*) xmlGetProp( cur, (xmlChar*)"rowmin" );
	if (prop)	fromString((const char*)prop, _MinRows);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"rowmax" );
	if (prop)	fromString((const char*)prop, _MaxRows);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"maxitem" );
	if (prop)	fromString((const char*)prop, _MaxItems);


	// read start index
	prop = (char*) xmlGetProp( cur, (xmlChar*)"startitem" );
	if (prop)	fromString((const char*) prop, _StartDbIdx);
	_StartDbIdx= max(_StartDbIdx, 0);

	// Column options
	prop = (char*) xmlGetProp( cur, (xmlChar*)"column_max" );
	if (prop)	fromString((const char*) prop, _ColumnMax);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"column_factor" );
	if (prop)	fromString((const char*) prop, _ColumnFactor);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"column_center" );
	if (prop)	_ColumnCenter= convertBool(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"display_empty_slot" );
	if (prop)	_DisplayEmptySlot = convertBool(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"squarify" );
	if (prop)	_Squarify = convertBool(prop);

	// For items in Animals only
	prop = (char*) xmlGetProp( cur, (xmlChar*)"db_animal_status" );
	if (prop)
	{
		_AnimalStatus= NLGUI::CDBManager::getInstance()->getDbProp((const char*)prop, false);
	}

	return true;
}

// ***************************************************************************
sint CDBGroupListSheet::getCurScrollValue()
{
	// if have some scroll to do
	if(_CurWantedRows>_MaxRows)
	{
		// strange value
		const float	bias= 0.35f;
		// Must remove one entry to get correct result
		sint sv= (sint)floor(bias + _CurWantedRows*(float)_DummyGroup->getOfsY() / favoid0((float)(_DummyGroup->getH()-_HSlot)));
		// Must always have _MaxRows displayed
		clamp(sv,0,_CurWantedRows-_MaxRows);
		return sv;
	}
	else
		// no scroll because all fit in the required space
		return 0;
}

// ***************************************************************************
CGroupContainer *CDBGroupListSheet::getContainer()
{
	return dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getWindow(this));
/*	CGroupContainer *pGC = NULL;
	CInterfaceGroup *pParent = getParent();
	while (pParent != NULL)
	{
		pGC = dynamic_cast<CGroupContainer*>(pParent);
		if (pGC != NULL) break;
		pParent = pParent->getParent();
	}
	return pGC;*/
}

// ***************************************************************************
void CDBGroupListSheet::updateCoords ()
{
	H_AUTO ( RZ_CDBGroupListSheet_updateCoords )

	setup();
	if (!_Active) return;
	if (_Parent != NULL)
	{
		if (!_Parent->getActive()) return;
		if ((_Parent->getParent() != NULL) && (!_Parent->getParent()->getActive())) return;
	}

	// NB: yStart<=0 because top down list.
	uint	i;
	sint	xStart= _LeftMargin;
	sint	yStart= -_TopMargin;
	sint	wMax;


	// **** Compute layout
	if (!_Squarify)
	{
		// first, just update WReal
		CInterfaceElement::updateCoords();
		wMax = getWReal()-_RightMargin;
	}
	else
	{
		// we assume that the list impose the size
		// eval number of valid sheet
		uint numValidSheets;
		if (_Array)
		{
			numValidSheets = (uint)_SheetChildren.size();
		}
		else
		{
			// count valid sheets
			// **** update sheet id cache if needed
			if (_CtrlInfo._Type != CCtrlSheetInfo::SheetType_Macro)
			{
				if (_CtrlInfo._Type != CCtrlSheetInfo::SheetType_Macro)
				{
					for(i=0;i<_SheetChildren.size();i++)
					{
						CDBCtrlSheet	*ctrl= _SheetChildren[i]->Ctrl;
						_SheetChildren[i]->SheetId= ctrl->getSheetId();
					}
				}
			}
			numValidSheets = 0;
			for(i=0;i<_SheetChildren.size();i++)
			{
				// if valid and filtered
				if (isChildValid(i))
					++numValidSheets;
			}
		}
		if (_DisplayEmptySlot)
		{
			// increment num sheet if the empty slot is to be displayed
			CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
			if (pCS != NULL && pCS->isSheetValid()) ++numValidSheets;
		}
		_ColumnMax = (uint) (ceil(sqrtf((float) numValidSheets)));
		wMax = _ColumnMax * _WSlot - _WSpace + xStart;
	}
	// display at least one item per row. skip an extra space
	_NbColumns= (_WSpace+wMax-xStart)/_WSlot;
	// max user limit
	_NbColumns= min(_NbColumns, _ColumnMax);
	// Respect factor
	if(_ColumnFactor>1)
	{
		_NbColumns= _NbColumns/_ColumnFactor;
		// don't let 0 sheet
		_NbColumns= max(_NbColumns, 1U);
		_NbColumns*= _ColumnFactor;
	}
	else
	{
		// Allow at least one column.
		_NbColumns= max(_NbColumns, 1U);
	}

	if (_NeedToSort)
	{
		sort();
		_NeedToSort = false;
	}

	// **** update sheet id cache
	if (_CtrlInfo._Type != CCtrlSheetInfo::SheetType_Macro)
	for(i=0;i<_SheetChildren.size();i++)
	{
		CDBCtrlSheet	*ctrl= _SheetChildren[i]->Ctrl;
		_SheetChildren[i]->SheetId= ctrl->getSheetId();

		_SheetChildren[i]->update(this);
	}

	// **** set visiblity and place in array for each controls
	sint	xItem= 0;
	sint	yItem= 0;
	sint	xItemValid= 0;
	sint	yItemValid= 0;
	sint	curSectionId, maxSectionId;
	getCurrentBoundSectionId(curSectionId, maxSectionId);
	uint	groupSectionIndex= 0;
	_SectionYItems.clear();
	for(i=0;i<_SheetChildren.size();i++)
	{
		// is the control a correct sheetId?
		CDBCtrlSheet *ctrl= _SheetChildren[i]->Ctrl;
		bool validSheet= isChildValid(i);

		// default
		_SheetChildren[i]->Valid= false;

		// special display empty slot
		if ( (_DisplayEmptySlot) && (i == 0) )
		{
			_SheetChildren[i]->Valid= true;
			CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
			if (pCS != NULL)
			{
				ctrl->setTextureNoItem (pCS->getTextureNoItem());
				_SheetChildren[i]->Valid = pCS->isSheetValid();
			}
		}

		// validate the control if in array mode, or if valid sheet (and filtered)
		if(_Array || validSheet)
			_SheetChildren[i]->Valid= true;

		// compute Item coordinates if must display it
		if(_SheetChildren[i]->Valid)
		{
			// if sectionable, maybe insert a group header
			if(_Sectionable && !_SectionableError)
			{
				sint	ctrlSectionId= _SheetChildren[i]->getSectionId();
				if(ctrlSectionId!=curSectionId)
				{
					// insert all section between curSectionId excluded and ctrlSectionId included
					insertSectionGroupList(curSectionId+1, ctrlSectionId+1, groupSectionIndex, yItem, xStart, xItem);
					// new active section
					curSectionId= ctrlSectionId;
				}
			}

			// get item position
			_SheetChildren[i]->XItem= xItem;
			_SheetChildren[i]->YItem= yItem;
			// next place
			xItem++;
			if(xItem >= (sint)_NbColumns)
			{
				xItem= 0;
				yItem++;
			}

			// this item is valid
			xItemValid= xItem;
			yItemValid= yItem;
		}
		else if (_ListLeaveSpace)
		{
			// next place
			xItem++;
			if(xItem >= (sint)_NbColumns)
			{
				xItem= 0;
				yItem++;
			}

			// this item is not valid, don't copy to xItemValid/yItemValid
		}
	}

	// if sectionable and show empty section, may show the last empty sections
	if(_Sectionable && !_SectionableError && _SectionEmptyScheme!=NoEmptySection)
	{
		// if there is empty section left
		if(curSectionId+1<maxSectionId)
		{
			// insert the empty sections
			insertSectionGroupList(curSectionId+1, maxSectionId, groupSectionIndex, yItem, xStart, xItem);
			// Add a last space for clearness, if not already done
			if(_SectionEmptyScheme==AllowEmptySectionWithNoSpace)
				yItem++;
			// valid the x/y (important for scroll bar)
			xItemValid= xItem;
			yItemValid= yItem;
		}
	}

	// release any unused section group
	releaseSectionGroups(groupSectionIndex);

	// How many rows we want to display
	if(xItemValid == 0)
		// the last row is empty => don't add it
		_CurWantedRows= yItemValid;
	else
		_CurWantedRows= yItemValid+1;

	// Clamp _MaxRows
	if(_MaxRows<_MinRows)
		_MaxRows= _MinRows;

	// compute the scroll from the dummy group
	sint	curScrollY= 0;
	if(_ScrollBar)
	{
		// get the scroll value, and round to the next square.
		curScrollY= getCurScrollValue();
		_LastScrollY= curScrollY;
	}

	// **** replace all controls
	for(i=0;i<_SheetChildren.size();i++)
	{
		CSheetChild		&child= *_SheetChildren[i];

		// compute coordinates if must display it
		if(child.Valid)
		{
			sint	yRel= child.YItem-curScrollY;
			// Must display it if in the row window
			if(yRel>=0 && yRel<_MaxRows)
			{
				sint	xLine= xStart;
				// Center?
				if( _ColumnCenter )
				{
					// get the number of column for this line
					sint	nCol= _NbColumns;
					if( (sint)child.YItem==_CurWantedRows-1 && xItemValid>0)
					{
						nCol= xItemValid;
					}
					sint	wItems= nCol*_WSlot-_WSpace;
					// Center
					xLine+= ((wMax-xStart) - wItems)/2;
				}

				// setup
				child.Ctrl->setActive(true);
				child.Ctrl->setX(xLine+child.XItem*_WSlot);
				child.Ctrl->setY(yStart-yRel*_HSlot);
			}
			else
			{
				child.Ctrl->setActive(false);
			}
		}
		else
			child.Ctrl->setActive(false);
	}

	// **** replace all section group
	if(_Sectionable && !_SectionableError)
	{
		nlassert(_SectionGroups.size()==_SectionYItems.size());
		for(i=0;i<_SectionGroups.size();i++)
		{
			sint	yRel= _SectionYItems[i]-curScrollY;
			// Must display it if in the row window
			if(yRel>=0 && yRel<_MaxRows)
			{
				_SectionGroups[i]->setActive(true);
				_SectionGroups[i]->setY(yStart-yRel*_HSlot);
			}
			else
				_SectionGroups[i]->setActive(false);
		}
	}

	// **** re-compute Scroll setup
	if (_ResizeFromChildH)
	{
		// NB: yStart<=0 because top down list
		_H= -yStart;
		// Add row size
		if(_CurWantedRows<_MaxRows)
			// display all, add empty space if _MinRows > _CurWantedRows
			_H+= max(_MinRows, _CurWantedRows)*_HSlot;
		else
			// display row wanted
			_H+= _MaxRows*_HSlot;
		// Add bottom margin
		_H+= _BottomMargin;
	}
	else
	{
		CInterfaceElement::updateCoords();
		sint32 hMax = getHReal() - _TopMargin - _BottomMargin;
		_MaxRows = hMax / _HSlot;
		_MaxRows = max(1, _MaxRows);
	}



	// update the coordinates of the dummy group
	if(_ScrollBar)
	{
		_DummyGroup->setH(_CurWantedRows*_HSlot);
		_DummyGroup->setMaxH(_MaxRows*_HSlot);
		// must update it before _ScrollBar update (done in CInterfaceGroup::updateCoords())
		_DummyGroup->updateCoords();
		// Display the scroll bar only if needed
		_ScrollBar->setActive(_CurWantedRows>_MaxRows && _MaxRows>0);
	}

	bool bTmp = _ResizeFromChildH;
	_ResizeFromChildH = false;
	// call base method
	CInterfaceGroup::updateCoords();
	_ResizeFromChildH = bTmp;
}

// ***************************************************************************
void	CDBGroupListSheet::insertSectionGroupList(sint firstSectionId, sint lastSectionId, uint &groupSectionIndex, sint &yItem, sint xStart, sint &xItem)
{
	// display at least the new section
	if(firstSectionId>=lastSectionId || _SectionEmptyScheme==NoEmptySection)
		firstSectionId= lastSectionId-1;
	// else display also the empty sections (if firstSectionId<ctrlSectionId)

	// for all sections to add
	for(;firstSectionId<lastSectionId;firstSectionId++)
	{
		// NB: Y position recomputed below
		if(insertSectionGroup(groupSectionIndex, firstSectionId, xStart, 0))
		{
			// not empty line?
			if(xItem!=0)
			{
				xItem= 0;
				yItem++;
			}
			_SectionYItems.resize(groupSectionIndex+1);
			_SectionYItems[groupSectionIndex]= yItem;

			// next
			groupSectionIndex++;
			yItem++;

			// if it is an empty section, may also add a white space
			if(firstSectionId<lastSectionId-1 && _SectionEmptyScheme==AllowEmptySectionWithExtraSpace)
				yItem++;
		}
		else
		{
			// never retry
			_SectionableError= true;
			break;
		}
	}
}

// ***************************************************************************
void CDBGroupListSheet::checkCoords ()
{
	nlassert(_Setuped);

	// If the database has changed somewhere in the branch
	if(_BranchModified)
	{
		_BranchModified= false;

		bool	mustUpdateCoord= false;

		// If the list_sheet is a list
		if(!_Array)
		{
			// must check each frame if the sheet ids have changed
			for(uint i=0;i<_SheetChildren.size();i++)
			{
				sint32	curId= _SheetChildren[i]->Ctrl->getSheetId();
				if(curId!=_SheetChildren[i]->SheetId)
				{
					_NeedToSort = true;
					mustUpdateCoord= true;
					break;
				}
			}
		}

		// deriver test
		if(!mustUpdateCoord)
		{
			for(uint i=0;i<_SheetChildren.size();i++)
			{
				if (_SheetChildren[i]->isInvalidated(this))
				{
					_NeedToSort = true;
					mustUpdateCoord= true;
					break;
				}
			}
		}

		// if changed
		if(mustUpdateCoord)
		{
			invalidateCoords();
		}
	}

	// If the list_sheet has scroll
	if(_ScrollBar)
	{
		sint	scrollVal= getCurScrollValue();
		if(scrollVal!=_LastScrollY)
			invalidateCoords();
	}

	// Check each frame if must change the ctrl gray state of all items, according to animal status
	if(_AnimalStatus)
	{
		if(_CacheAnimalStatus != _AnimalStatus->getValue32())
		{
			_CacheAnimalStatus= _AnimalStatus->getValue32();
			bool	mustGrayAll= !ANIMAL_STATUS::isInventoryAvailable((ANIMAL_STATUS::EAnimalStatus)_CacheAnimalStatus);
			for(uint j=0;j<getNbSheet();j++)
				getSheet(j)->setItemBeastGrayed(mustGrayAll);
		}
	}
}

// ***************************************************************************
void CDBGroupListSheet::draw ()
{
	// Drag'N'Drop : display the selected slot bitmap if this slot accept the currently dragged element
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	if (_CanDrop)
	{
		CGroupContainer *pGC = getContainer();
		if (pGC != NULL) pGC->setHighLighted(false);
	}

	_CanDrop = false;
	if (_CtrlInfo._AHOnCanDrop != NULL)
	if (CWidgetManager::getInstance()->getCapturePointerLeft())
	{
		CGroupContainer *pGC = getContainer();
		if (CWidgetManager::getInstance()->getCurrentWindowUnder() == pGC)
		{
			if ((CWidgetManager::getInstance()->getPointer()->getX() >= _XReal) &&
				(CWidgetManager::getInstance()->getPointer()->getX() < (_XReal + _WReal))&&
				(CWidgetManager::getInstance()->getPointer()->getY() > _YReal) &&
				(CWidgetManager::getInstance()->getPointer()->getY() <= (_YReal+ _HReal)))
			{
				CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCapturePointerLeft());
				if ((pCSSrc != NULL) && pCSSrc->isDragged())
				{
					string params = string("src=") + pCSSrc->getId();
					if (!_CtrlInfo._AHCanDropParams.empty())
					{
						string sTmp = _CtrlInfo._AHCanDropParams;
						params = sTmp + "|" + params;
					}
					CAHManager::getInstance()->runActionHandler (_CtrlInfo._AHOnCanDrop, this, params);
				}
			}
		}

		// Set the container highlighted
		if (pGC != NULL) pGC->setHighLighted(_CanDrop);
	}

	CInterfaceGroup::draw();
}

// ***************************************************************************
void CDBGroupListSheet::clearViews ()
{
	CInterfaceGroup::clearViews();
}

// ***************************************************************************
bool CDBGroupListSheet::handleEvent (const NLGUI::CEventDescriptor &event)
{
	if (event.getType() == NLGUI::CEventDescriptor::mouse)
	{
		const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
		if (isIn(eventDesc.getX(), eventDesc.getY()))
		{
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousewheel)
			{
				if (_ScrollBar != NULL)
				{
					// and scroll for 1+ item.
					_ScrollBar->moveTargetY (-eventDesc.getWheel() * _HSlot);
					return true;
				}
			}
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
			{
				CGroupContainer *pGC = getContainer();
				if (pGC != NULL) pGC->setHighLighted(false);
			}
		}
	}
	return CInterfaceGroup::handleEvent(event);
}


// ***************************************************************************
void CDBGroupListSheet::setup()
{
	if (_Setuped)
		return;
	_Setuped= true;

	H_AUTO ( RZ_CDBGroupListSheet_setup )

	// bind to the controls.
	_ButtonAddRow= dynamic_cast<CCtrlBaseButton*>(CInterfaceGroup::getCtrl("add_row"));
	_ButtonSubRow= dynamic_cast<CCtrlBaseButton*>(CInterfaceGroup::getCtrl("sub_row"));
	_ScrollBar = dynamic_cast<CCtrlScroll*>(CInterfaceGroup::getCtrl("scroll_row"));;

	// Set ScrollMode if button setuped
	if(_ScrollBar)
	{
		// Create a dummy group JUST to be the target of the scrollBar. Not linked to this, not displayed etc...
		_DummyGroup= new CInterfaceGroup(CViewBase::TCtorParam());
		_DummyGroup->setId(_Id+":sys_dummy_group");
		_DummyGroup->setParent (this);
		_DummyGroup->setParentPos (this);
		_DummyGroup->setParentPosRef (Hotspot_TL);
		_DummyGroup->setPosRef (Hotspot_TL);
		_DummyGroup->setW (0);
		_DummyGroup->setActive(true);

		// Link ScrollBar
		_ScrollBar->setTarget(_DummyGroup);
		_ScrollBar->setAlign (3); // Top
	}

	// actions
	if(_ButtonAddRow)
		_ButtonAddRow->setActionOnLeftClick("list_sheet_add_row");
	if(_ButtonSubRow)
		_ButtonSubRow->setActionOnLeftClick("list_sheet_sub_row");

	// Create the ctrl sheet. One for each possible entry in the database
	sint	nbNodes= 0;
	if (_CtrlInfo._Type != CCtrlSheetInfo::SheetType_Macro)
	{
		nbNodes = _DbBranch->getNbNodes();
		// Maximize the number of item displayed (total)
		nbNodes= min((sint)_MaxItems, nbNodes-_StartDbIdx);
		nbNodes= max(0, nbNodes);
	}
	else
	{
		nbNodes= _MaxItems;
	}

	// clear old if needed
	sint i;
	for(i=0;i<(sint)_SheetChildren.size();i++)
		delete _SheetChildren[i];
	_SheetChildren.clear();

	// Create all ctrl childs
	nbNodes += (_DisplayEmptySlot?1:0);
	_SheetChildren.resize(nbNodes, NULL);

	// determine the inventory slot from the database branch id
	int slotNum = CDBCtrlSheet::getInventorySlot( _DbBranchName );

	for (i = 0; i < nbNodes; i++)
	{
		H_AUTO ( RZ_CDBGroupListSheet_child )

		_SheetChildren[i] = createSheetChild();

		// Create a ctrl.
		CDBCtrlSheet	*ctrl= new CDBCtrlSheet(CViewBase::TCtorParam());
		// Manual setup.
		ctrl->setId(_Id+":"+toString(i));
		ctrl->setParent (this);
		ctrl->setParentPos (this);
		ctrl->setParentPosRef (Hotspot_TL);
		ctrl->setPosRef (Hotspot_TL);
		ctrl->setActive(true);
		ctrl->setType(_CtrlInfo._Type);
		// Copy Info for tooltip
		ctrl->setInstantContextHelp(_ToolTipInstant);
		ctrl->setDefaultContextHelp(_ContextHelp);
		ctrl->setOnContextHelp(_OnContextHelp);
		ctrl->setToolTipParent(getToolTipParent());
		ctrl->setToolTipParentPosRef(getToolTipParentPosRef());
		ctrl->setToolTipPosRef(getToolTipPosRef());
		// link on the element i+_StartDbIdx
		if (_DisplayEmptySlot)
		{
			if (i == 0)
				ctrl->initSheet("UI:EMPTY", _CtrlInfo);
			else
//				ctrl->initSheet(_DbBranchName+":"+toString(i-1+_StartDbIdx), _CtrlInfo);
				ctrl->initSheetFast(_DbBranchName, (i-1+_StartDbIdx), slotNum, _CtrlInfo);
		}
		else
//			ctrl->initSheet(_DbBranchName+":"+toString(i+_StartDbIdx), _CtrlInfo);
			ctrl->initSheetFast(_DbBranchName, i+_StartDbIdx, slotNum, _CtrlInfo);

		// Add it to the group.
		addCtrl(ctrl);

		// Add it to us, for faster lookup
		_SheetChildren[i]->Ctrl= ctrl;

		// deriver init
		_SheetChildren[i]->init(this, i);
	}

}


// ***************************************************************************
void CDBGroupListSheet::changeNbRow(sint delta)
{
	if(_CurWantedRows<_MinRows)
		_MaxRows= _MinRows;
	else
	{
		// preclamp is necessary if _MaxRows was too big before.
		clamp(_MaxRows, _MinRows, (sint)_CurWantedRows);
		// add the value
		_MaxRows+= delta;
		// clamp to range
		clamp(_MaxRows, _MinRows, (sint)_CurWantedRows);
	}
	// size change: we must update all coords (us and our father)
	invalidateCoords();
}


// ***************************************************************************
sint CDBGroupListSheet::getIndexOf(const CDBCtrlSheet	*sheet) const
{
	for(uint k = 0; k < _SheetChildren.size(); ++k)
	{
		if (_SheetChildren[k]->Ctrl == sheet) return k;
	}
	return -1;
}

// ***************************************************************************
CDBCtrlSheet *CDBGroupListSheet::getSheet(uint index) const
{
	if (index >= _SheetChildren.size()) return NULL;
	return _SheetChildren[index]->Ctrl;
}

// ***************************************************************************
sint32 CDBGroupListSheet::getNbElt () const
{
	sint32 NumValidChildren= 0;
	for(uint i=0;i<_SheetChildren.size();i++)
	{
		// is the control a correct sheetId?
		CDBCtrlSheet *ctrl= _SheetChildren[i]->Ctrl;
		bool validSheet= isChildValid(i);

		// default
		bool bValid= false;

		// special display empty slot
		if ( (_DisplayEmptySlot) && (i == 0) )
		{
			CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
			if (pCS != NULL)
			{
				ctrl->setTextureNoItem (pCS->getTextureNoItem());
				bValid = pCS->isSheetValid();
			}
		}

		// validate the control if in array mode, or if valid sheet (and filtered)
		if(_Array || validSheet)
			bValid= true;

		// if valid
		if(bValid)
			NumValidChildren++;
	}

	return NumValidChildren;
}


// ***************************************************************************
void	CDBGroupListSheet::forceValidity(uint element, bool forceValid)
{
	if(!_Array && element<_SheetChildren.size())
	{
		if(forceValid!=_SheetChildren[element]->ForceValid)
		{
			_SheetChildren[element]->ForceValid= forceValid;
			invalidateCoords();
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// Actions Handlers
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
class CListSheetAddRow : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
	{
		CDBGroupListSheet *pLS = dynamic_cast<CDBGroupListSheet*>(pCaller->getParent());
		if (pLS== NULL) return;
		pLS->changeNbRow(+1);
	}
};
REGISTER_ACTION_HANDLER (CListSheetAddRow, "list_sheet_add_row");

// ***************************************************************************
class CListSheetSubRow : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
	{
		CDBGroupListSheet *pLS = dynamic_cast<CDBGroupListSheet*>(pCaller->getParent());
		if (pLS== NULL) return;
		pLS->changeNbRow(-1);
	}
};
REGISTER_ACTION_HANDLER (CListSheetSubRow, "list_sheet_sub_row");

