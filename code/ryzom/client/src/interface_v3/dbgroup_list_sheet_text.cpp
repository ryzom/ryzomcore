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
#include "dbgroup_list_sheet_text.h"
#include "nel/gui/group_container.h"
#include "interface_manager.h"
#include "nel/misc/xml_auto_ptr.h"
#include "../sheet_manager.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/view_text.h"
#include "nel/gui/action_handler.h"
#include "../time_client.h"
#include "game_share/animal_status.h"


using namespace NLMISC;
using namespace std;

NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetText, std::string, "list_sheet_text");

// ***************************************************************************
CDBGroupListSheetText::CDBGroupListSheetText(const TCtorParam &param)
:	IListSheetBase(param),
	_TextTemplate(TCtorParam())
{
	_MaxItems = INT_MAX;
	_WSlot= 24;
	_HSlot= 24;
	_HSpace = 0;
	_ScrollBar = NULL;
	_List= NULL;
	_Setuped = false;
	_DbBranch = NULL;
	_Array = true;
	_StartDbIdx = 0;
	_XItem = _YItem = 0;
	_XText = _YText = 0;
	_OverColorNormal = CRGBA(255,255,255,0);
	_OverColorOver = CRGBA(255,255,255,64);
	_OverColorPushed = CRGBA(255,255,255,128);
	_SelectionEnabled = true;
	_Scrolling = 0;
	_LastTimeScrolled = 0;
	_ClickWhenPushed = false;
	_NeedToSort = true;
	_NeedUpdateAllText = true;

	_DbBranchObs.Owner = this;
	_BranchModified = true;
	_CheckCoordAccelerated = true;
	_GrayTextWithCtrlState= false;

	_AnimalStatus= NULL;
	_CacheAnimalStatus= -1;
	_CanDrop= false;

	for(uint i=0;i<CHARACTERISTICS::NUM_CHARACTERISTICS;i++)
		_LastPlayerCharac[i]= 0;
}
// ***************************************************************************
CDBGroupListSheetText::~CDBGroupListSheetText()
{
	uint i;
	for(i=0;i<_SheetChildren.size();i++)
		delete _SheetChildren[i];
}


// ***************************************************************************
bool CDBGroupListSheetText::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
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
		NLGUI::CDBManager::getInstance()->addBranchObserver( branch, &_DbBranchObs );
	}

	// parse the common ctrl info
	if(!_CtrlInfo.parseCtrlInfo(cur, parentGroup))
		return false;

	// If no value -> error
	if (_CtrlInfo._Type !=  CCtrlSheetInfo::SheetType_Macro)
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
		case CCtrlSheetInfo::SheetType_Mission:
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
		case CCtrlSheetInfo::SheetType_GuildFlag: dispSlotBmpId = rVR.getTextureIdFromName ("w_slot_blason.tga"); break;
		case CCtrlSheetInfo::SheetType_ElevatorDestination: dispSlotBmpId = rVR.getTextureIdFromName ("w_slot_blason.tga"); break;
		default: break;
	}
	rVR.getTextureSizeFromId (dispSlotBmpId, _WSlot, _HSlot);

	// add extra space
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

	// read row setup
	prop = (char*) xmlGetProp( cur, (xmlChar*)"maxitem" );
	if (prop)	fromString((const char*)prop, _MaxItems);


	// read start index
	prop = (char*) xmlGetProp( cur, (xmlChar*)"startitem" );
	if (prop)	fromString((const char*)prop, _StartDbIdx);
	_StartDbIdx= max(_StartDbIdx, 0);

	// read pos setup
	prop = (char*) xmlGetProp( cur, (xmlChar*)"xitem" );
	if(prop)	fromString((const char*)prop, _XItem);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"yitem" );
	if(prop)	fromString((const char*)prop, _YItem);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"xtext" );
	if(prop)	fromString((const char*)prop, _XText);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"ytext" );
	if(prop)	fromString((const char*)prop, _YText);

	// read button colors
	prop= (char*) xmlGetProp( cur, (xmlChar*)"over_color" );
	if (prop)	_OverColorNormal = convertColor (prop);
	prop= (char*) xmlGetProp( cur, (xmlChar*)"over_col_pushed" );
	if (prop)	_OverColorPushed = convertColor (prop);
	prop= (char*) xmlGetProp( cur, (xmlChar*)"over_col_over" );
	if (prop)	_OverColorOver = convertColor (prop);

	prop= (char*) xmlGetProp( cur, (xmlChar*)"selection" );
	if (prop)	_SelectionEnabled = convertBool(prop);

	prop= (char*) xmlGetProp( cur, (xmlChar*)"click_when_pushed" );
	if (prop)	_ClickWhenPushed = convertBool(prop);

	prop= (char*) xmlGetProp( cur, (xmlChar*)"gray_text_with_ctrl" );
	if (prop)	_GrayTextWithCtrlState = convertBool(prop);


	if (!_SelectionEnabled)
		_OverColorPushed = _OverColorNormal;

	// read text aspect.
	_TextTemplate.parseTextOptions(cur);

	// For items in Animals only
	prop = (char*) xmlGetProp( cur, (xmlChar*)"db_animal_status" );
	if (prop)
	{
		_AnimalStatus= NLGUI::CDBManager::getInstance()->getDbProp((const char*)prop, false);
	}

	return true;
}

// ***************************************************************************
void CDBGroupListSheetText::updateCoords ()
{
	H_AUTO ( RZ_CDBGroupListSheetText_updateCoords )

	setup();

	// first, just update WReal. must update even if not active
	CInterfaceElement::updateCoords();

	// if the element (or one of its parent) is not active, avoid huge CPU usage
	if (!isActiveThroughParents())
		return;

	// no scrollbar/list? => abort
	if(!_ScrollBar || !_List)
		return;

	uint	i;
//	sint	wMax= _List->getWReal();


	if (_NeedToSort)
	{
		sort();
		_NeedToSort = false;
		for(i=0;i<_SheetChildren.size();i++)
		{
			string sTmp = toString(i);
			_SheetChildren[i]->Ctrl->setParamsOnLeftClick(sTmp);
			_SheetChildren[i]->Ctrl->setParamsOnRightClick(sTmp);
			_SheetChildren[i]->Button->setParamsOnLeftClick(sTmp);
			_SheetChildren[i]->Button->setParamsOnRightClick(sTmp);
		}
	}

	// **** update sheet id cache, and nameId (for special items)
	string sTmp;
	// yoyo: why macro list text???
	if (_CtrlInfo._Type != CCtrlSheetInfo::SheetType_Macro)
	{
		// for all children, test the ones that really need text updates, and update cache
		for(i=0;i<_SheetChildren.size();i++)
		{
			CDBCtrlSheet	*ctrl= _SheetChildren[i]->Ctrl;
			sint32	curId= ctrl->getSheetId();
			sint32	curNameId= ctrl->getItemNameId();
			sint32	curRMClassType = ctrl->getItemRMClassType();
			sint32  curRMFaberStatType = ctrl->getItemRMFaberStatType();

			// test if must update the text, before update
			if(_NeedUpdateAllText)
			{
				_SheetChildren[i]->NeedUpdateText= true;
			}
			else
			{
				// if cache is different from cur
				if( curId != _SheetChildren[i]->SheetId ||
					curNameId != _SheetChildren[i]->NameId ||
					curRMClassType != _SheetChildren[i]->RMClassType ||
					curRMFaberStatType != _SheetChildren[i]->RMFaberStatType ||
					_SheetChildren[i]->isInvalidated(this)
					)
				{
					_SheetChildren[i]->NeedUpdateText= true;
				}
			}

			// bkup the old name id cached
			sint32		oldNameId= _SheetChildren[i]->NameId;

			// update
			_SheetChildren[i]->SheetId = curId;
			_SheetChildren[i]->NameId = curNameId;
			_SheetChildren[i]->RMClassType = curRMClassType;
			_SheetChildren[i]->RMFaberStatType = curRMFaberStatType;
			_SheetChildren[i]->update(this);

			// Name id management (wait for dyn string coming from IOS)
			if( oldNameId != curNameId )
			{
				if( curNameId == 0 )
					_NameIdToUpdate.erase( _SheetChildren[i] );
				else
				{
					// if not received, must insert in list of pending dynstring to check each frame
					ucstring result;
					if( !STRING_MANAGER::CStringManagerClient::instance()->getDynString ( curNameId, result) )
						_NameIdToUpdate.insert( _SheetChildren[i] );
				}
			}
		}
	}
	else
	{
		for(i=0;i<_SheetChildren.size();i++)
			_SheetChildren[i]->NeedUpdateText= true;
	}
	// next pass: no more need to update all text
	_NeedUpdateAllText= false;

	// **** set visiblity and place in array for each controls
	sint	yItem= 0;
	sint	curSectionId, maxSectionId;
	getCurrentBoundSectionId(curSectionId, maxSectionId);
	uint	groupSectionIndex= 0;
	_NumValidSheets = 0;
	for(i=0;i<_SheetChildren.size();i++)
	{
		bool validSheet = _SheetChildren[i]->isSheetValid(this);
		_SheetChildren[i]->Valid = validSheet;

		// validate the control if in array mode or if in list mode and valid
		if(_Array || validSheet)
			_SheetChildren[i]->Valid = true;

		// compute Item coordinates if must display it
		if(_SheetChildren[i]->Valid)
		{
			++ _NumValidSheets;
			// if sectionable, maybe insert a group header
			if(_Sectionable && !_SectionableError)
			{
				sint	ctrlSectionId= _SheetChildren[i]->getSectionId();
				if(ctrlSectionId!=curSectionId)
				{
					// insert all section between curSectionId excluded and ctrlSectionId included
					insertSectionGroupList(curSectionId+1, ctrlSectionId+1, groupSectionIndex, yItem);
					// new active section
					curSectionId= ctrlSectionId;
				}
			}

			// get its position
			_SheetChildren[i]->YItem= yItem;
			yItem++;
		}
	}

	// if sectionable and show empty section, may show the last empty sections
	if(_Sectionable && !_SectionableError && _SectionEmptyScheme!=NoEmptySection)
	{
		// if there is empty section left
		if(curSectionId+1<maxSectionId)
		{
			// insert the empty sections
			insertSectionGroupList(curSectionId+1, maxSectionId, groupSectionIndex, yItem);
			// Add a last space for clearness, if not already done
			if(_SectionEmptyScheme==AllowEmptySectionWithNoSpace)
				yItem++;
		}
	}

	// max yitem
	sint	yItemMax= yItem;

	// release any unused group
	releaseSectionGroups(groupSectionIndex);

	// **** replace all controls
	for(i=0;i<_SheetChildren.size();i++)
	{
		CSheetChild		&child= *_SheetChildren[i];

		// compute coordinates if must display it
		if(child.Valid)
		{
			sint	deltaX= child.getDeltaX(this);

			// setup the sheet
			child.Ctrl->setActive(true);
			child.Ctrl->setX( _XItem + deltaX );
			child.Ctrl->setY( _YItem - child.YItem*_HSlot);

			// setup text
			child.Text->setActive(true);
			child.Text->setX( _XText + deltaX );
			child.Text->setY( _YText - child.YItem*_HSlot );
			child.Text->setLineMaxW( min(	(sint32)(_List->getWReal()-(_XText+deltaX)),
											(sint32)_TextTemplate.getLineMaxW()), false	);

			// build the text if really needed
			if(child.NeedUpdateText)
			{
				child.updateViewText(this);
				child.NeedUpdateText= false;
			}

			// setup button
			child.Button->setActive(true);
			child.Button->setX(0);
			child.Button->setY(-child.YItem*_HSlot);
			child.Button->setW(_List->getWReal());
			child.Button->setH(_HSlot);
		}
		else
		{
			child.hide(this);
		}
	}


	// update the coordinates of the List. take the max yItem used
	_List->setH(yItemMax*_HSlot);
	// Keep the same MaxH
	// must update it before _ScrollBar update (done in CInterfaceGroup::updateCoords())
	_List->updateCoords();
	// Display the scroll bar only if needed
	if(_List->getHReal() > _List->getMaxHReal())
		_ScrollBar->setActive( true );
	else
	{
		_ScrollBar->setActive( false );
		// reset ofsY
		_List->setOfsY(0);
	}

	// call base method
	CInterfaceGroup::updateCoords();
}

// ***************************************************************************
void CDBGroupListSheetText::insertSectionGroupList(sint firstSectionId, sint lastSectionId, uint &groupSectionIndex, sint &yItem)
{
	// display at least the new section
	if(firstSectionId>=lastSectionId || _SectionEmptyScheme==NoEmptySection)
		firstSectionId= lastSectionId-1;
	// else display also the empty sections (if firstSectionId<ctrlSectionId)

	// for all sections to add
	for(;firstSectionId<lastSectionId;firstSectionId++)
	{
		if(insertSectionGroup(groupSectionIndex, firstSectionId, _XItem, _YItem - yItem*_HSlot))
		{
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
void CDBGroupListSheetText::checkCoords ()
{
	nlassert(_Setuped);
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// test if must check all this pass
	bool	mustUpdateAll= needCheckAllItems();
	bool	forceCheck= _CheckCoordAccelerated == false || mustUpdateAll;

	// test each child to update each frame, if need to update text id
	for( set< CSheetChild* >::iterator it = _NameIdToUpdate.begin(); it != _NameIdToUpdate.end(); )
	{
		CSheetChild * cst = (*it);
		// String result
		ucstring result;
		if( STRING_MANAGER::CStringManagerClient::instance()->getDynString ( cst->NameId, result) )
		{
			set< CSheetChild *>::iterator itTmp = it;
			++it;
			_NameIdToUpdate.erase(itTmp);
			cst->NeedUpdateText= true;
			forceCheck= true;
		}
		else
		{
			++it;
		}
	}

	// If forceCheck or the database has changed somewhere in the branch
	if( forceCheck || _BranchModified == true )
	{
		// if the branch is modified, then may check the gray state of each control => gray the text too!
		if (_BranchModified && _GrayTextWithCtrlState)
		{
			// The gray color
			CRGBA	normalColor= _TextTemplate.getColor();
			CRGBA	grayColor= CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlSheetGrayColor).getValColor();
			CRGBA	redifyColor= CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlTextRedifyColor).getValColor();
			grayColor.modulateFromColor(grayColor, normalColor);
			redifyColor.modulateFromColor(redifyColor, normalColor);

			// for all ctrl sheets
			for(uint i=0;i<_SheetChildren.size();i++)
			{
				if(_SheetChildren[i]->Text)
				{
					// don't use getGrayed() method cause update at next draw only!!
					// mus retrieve the lock ptr, and read value directly
					CCDBNodeLeaf	*lockPtr= _SheetChildren[i]->Ctrl->getLockValuePtr();
					bool	grayed= lockPtr && lockPtr->getValue32()!=0;
					bool	redifiyed;
					if( _SheetChildren[i]->Ctrl->getType()!=CCtrlSheetInfo::SheetType_Item )
						redifiyed= lockPtr && lockPtr->getValue32()==2;
					else
						redifiyed= _SheetChildren[i]->Ctrl->getItemPrerequisitValidPtr()->getValueBool();

					if(redifiyed)
						_SheetChildren[i]->Text->setColor(redifyColor);
					else if(grayed)
						_SheetChildren[i]->Text->setColor(grayColor);
					else
						_SheetChildren[i]->Text->setColor(normalColor);
				}
			}
		}

		// reset
		_BranchModified= false;

		// if must update all
		if(mustUpdateAll)
		{
			_NeedUpdateAllText= true;
			_NeedToSort = true;
			invalidateCoords();
		}
		else
		// Must test all SheetId change (even if array), because sheetId change means ViewText name change
		// Must test also the NameId change (if ctrl is item)
		{
			// must check each frame if the sheet ids have changed
			bool	mustUpdateCoord= false;
			for(uint i=0;i<_SheetChildren.size();i++)
			{
				sint32	curId= _SheetChildren[i]->Ctrl->getSheetId();
				sint32	curNameId= _SheetChildren[i]->Ctrl->getItemNameId();
				sint32	curRMClassType = _SheetChildren[i]->Ctrl->getItemRMClassType();
				sint32  curRMFaberStatType = _SheetChildren[i]->Ctrl->getItemRMFaberStatType();
				if( curId != _SheetChildren[i]->SheetId ||
					curNameId != _SheetChildren[i]->NameId ||
					curRMClassType != _SheetChildren[i]->RMClassType ||
					curRMFaberStatType != _SheetChildren[i]->RMFaberStatType
				  )
				{
					_NeedToSort = true;
					mustUpdateCoord= true;
					break;
				}
				if (_SheetChildren[i]->isInvalidated(this))
				{
					_NeedToSort = true;
					mustUpdateCoord= true;
					break;
				}
				// if need update text (typically if dynstring for nameid received)
				if (_SheetChildren[i]->NeedUpdateText)
				{
					_NeedToSort = true;
					mustUpdateCoord= true;
					break;
				}
			}
			// if changed
			if(mustUpdateCoord)
			{
				invalidateCoords();
			}
		}
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

	// must check other content (important for list!)
	CInterfaceGroup::checkCoords();
}

// ***************************************************************************
void CDBGroupListSheetText::draw ()
{
	// Drag'N'Drop : display the selected slot bitmap if this slot accept the currently dragged element
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	if (_CanDrop)
	{
		CGroupContainer *pGC = getContainer();
		if(pGC)
			pGC->setHighLighted(false);
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

	if (_EmptyListNotifier)
	{
		_EmptyListNotifier->setActive(_DownloadComplete && (_NumValidSheets == 0));		
	}
}

// ***************************************************************************
void CDBGroupListSheetText::clearViews ()
{
	CInterfaceGroup::clearViews();
}

// ***************************************************************************
bool CDBGroupListSheetText::handleEvent (const NLGUI::CEventDescriptor &event)
{
	if (event.getType() == NLGUI::CEventDescriptor::mouse)
	{
		_Scrolling = 0;
		const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;
		if (isIn(eventDesc.getX(), eventDesc.getY()))
		{
			// Drag'n'drop from a ctrl sheet that belongs to this list
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			if ((CWidgetManager::getInstance()->getCapturePointerLeft() != NULL) && (CWidgetManager::getInstance()->getCapturePointerLeft()->getParent() == _List))
			{
				CDBCtrlSheet *pDraggedSheet = NULL;
				CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(CWidgetManager::getInstance()->getCapturePointerLeft());
				if (pCB != NULL)
				{
					// A button has been captured -> Transform the capture to the corresponding ctrlsheet
					sint pos = getIndexOf(pCB);
					if ((pos >= 0) &&
						_SheetChildren[pos]->Ctrl->isDraggable() && (!_SheetChildren[pos]->Ctrl->getGrayed()))
					{
						pDraggedSheet = _SheetChildren[pos]->Ctrl;
						CWidgetManager::getInstance()->setCapturePointerLeft(pDraggedSheet);
						NLGUI::CEventDescriptorMouse newEv = eventDesc;
						// Send this because not send (the captured button has processed the event mouseleftdown)
						newEv.setEventTypeExtended(NLGUI::CEventDescriptorMouse::mouseleftdown);
						pDraggedSheet->handleEvent(newEv);
					}
				}
				else
				{
					pDraggedSheet = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCapturePointerLeft());
					// auto scroll only if swapable
					if(swapable())
					{
						// If a ctrlsheet of this list is being dragged and we are on top or bottom : scroll
						if ((eventDesc.getY()-_YReal) < 10)
							_Scrolling = -1;
						if ((eventDesc.getY()-_YReal) > _HReal-10)
							_Scrolling = 1;
					}
				}

				// If we were dragging a sheet from this list so we want to drop it somewhere
				if(swapable())
				{
					if (pDraggedSheet != NULL)
					if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
					{
						sint posdst = -1,possrc = -1;
						const vector<CCtrlBase*> &rV = CWidgetManager::getInstance()->getCtrlsUnderPointer();
						for (uint i = 0; i < rV.size(); ++i)
						{
							CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(rV[i]);
							if (pCB != NULL)
							{
								posdst = getIndexOf(pCB);
								if (posdst >= 0)
									break;
							}
						}
						possrc = getIndexOf(pDraggedSheet);
						if ((posdst != -1) && (possrc != -1))
							onSwap(possrc, posdst);
					}
				}
			}


			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousewheel)
			{
				// If scroll ok, and if scroll possible
				if (_ScrollBar != NULL && _List != NULL && _List->getH()>_List->getMaxH())
				{
					// The listSheetTrade can be very big. So to get better precision, scroll only 1 item for 1 mouseWheel
					// In this case, also ensure that we get no Items clipped by the group.
					sint32	ofsY= _List->getOfsY();
					ofsY= (ofsY/_HSlot) * _HSlot;
					_List->setOfsY(ofsY);
					// and scroll for 1+ item.
					_ScrollBar->moveTargetY (-eventDesc.getWheel() * _HSlot);
					return true;
				}
				else
					return false;
			}
		}
	}
	else if (event.getType() == NLGUI::CEventDescriptor::system)
	{
		const NLGUI::CEventDescriptorSystem &systemEvent = (const NLGUI::CEventDescriptorSystem &) event;
		if (systemEvent.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::clocktick)
		{
			if (_Scrolling != 0)
				if (_ScrollBar != NULL && _List != NULL)
				{
					if ((T1 - _LastTimeScrolled) > 250)
					{
						_LastTimeScrolled = T1;
						sint32	ofsY= _List->getOfsY();
						ofsY= (ofsY/_HSlot) * _HSlot;
						_List->setOfsY(ofsY);
						_ScrollBar->moveTargetY (-_Scrolling * _HSlot);
					}
				}
		}
	}

	if (CInterfaceGroup::handleEvent(event)) return true;
	return false;
}


// ***************************************************************************
void CDBGroupListSheetText::setup()
{
	H_AUTO ( RZ_CDBGroupListSheetText_setup )

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if (_Setuped)
		return;
	_Setuped= true;

	// bind to the controls.
	_ScrollBar = dynamic_cast<CCtrlScroll*>(CInterfaceGroup::getCtrl("scroll_row"));
	_List = dynamic_cast<CInterfaceGroup*>(CInterfaceGroup::getGroup("list"));
	_EmptyListNotifier = findFromShortId("empty_list_notify");

	// Set ScrollMode if button setuped
	if(_ScrollBar)
	{
		// Link ScrollBar
		_ScrollBar->setTarget(_List);
		_ScrollBar->setAlign (3); // Top
	}

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

	// Remove childs if any
	_List->clearGroups();
	uint i;
	for(i=0;i<_SheetChildren.size();i++)
		delete _SheetChildren[i];

	// Create all ctrl childs
	_SheetChildren.resize(nbNodes);

	// determine the inventory slot from the database branch id
	int slotNum = CDBCtrlSheet::getInventorySlot( _DbBranchName );

	for (i=0; i < (uint)nbNodes; i++)
	{
		H_AUTO ( RZ_CDBGroupListSheetText_child )

		_SheetChildren[i] = createSheetChild();
		// **** Create a ctrl.
		CDBCtrlSheet	*ctrl= new CDBCtrlSheet(CViewBase::TCtorParam());
		// Manual setup.
		ctrl->setId(_List->getId()+":sheet"+toString(i));
		// NB: unlike list_sheet, parent to _List
		ctrl->setParent (_List);
		ctrl->setParentPos (_List);
		ctrl->setParentPosRef (Hotspot_TL);
		ctrl->setPosRef (Hotspot_TL);
		ctrl->setActive(true);
		ctrl->setType(_CtrlInfo._Type);
		// Copy Info for tooltip
		ctrl->setInstantContextHelp(_ToolTipInstant);
		ctrl->setDefaultContextHelp(_ContextHelp);
		ctrl->setOnContextHelp(_OnContextHelp);
		// link on the element i+_StartDbIdx
//		ctrl->initSheet( _DbBranchName + ":" + toString(i+_StartDbIdx), _CtrlInfo );
		ctrl->initSheetFast( _DbBranchName, i+_StartDbIdx, slotNum, _CtrlInfo );
		// Change the ActionHandler for selection and info. Because must also select the button....
		ctrl->setActionOnLeftClick("lst_select");
		ctrl->setParamsOnLeftClick(toString(i));
		ctrl->setActionOnRightClick("lst_rclick");
		ctrl->setParamsOnRightClick(toString(i));

		// Add it to the list.
		_List->addCtrl(ctrl);

		// Add it to us, for faster lookup
		_SheetChildren[i]->Ctrl= ctrl;

		// **** Create the text
		CViewText		*text= _SheetChildren[i]->createViewText();
		text->setId(_List->getId()+":text"+toString(i));
		text->setParent (_List);
		text->setParentPos (_List);
		text->setParentPosRef (Hotspot_TL);
		text->setPosRef (Hotspot_TL);
		text->setActive(true);
		// set text aspect
		text->setFontSize(_TextTemplate.getFontSize());
		text->setColor(_TextTemplate.getColor());
		text->setShadow(_TextTemplate.getShadow());
		text->setLineMaxW(_TextTemplate.getLineMaxW());
		text->setMultiLine(_TextTemplate.getMultiLine());
		if(text->getMultiLine())
		{
			text->setMultiLineSpace(_TextTemplate.getMultiLineSpace());
			text->setMultiLineMaxWOnly (_TextTemplate.getMultiLineMaxWOnly());
			text->setMultiMaxLine(_TextTemplate.getMultiMaxLine());
		}
		text->setTextMode(_TextTemplate.getTextMode());
		text->setModulateGlobalColor(_TextTemplate.getModulateGlobalColor());

		// append
		_List->addView(text);
		_SheetChildren[i]->Text= text;

		// **** Create the button
		CCtrlButton		*button= new CCtrlButton(CViewBase::TCtorParam());
		button->setId(_List->getId()+":but"+toString(i));
		button->setParent (_List);
		button->setParentPos (_List);
		button->setParentPosRef (Hotspot_TL);
		button->setPosRef (Hotspot_TL);
		button->setActive(true);
		// setup aspect
		button->setTexture("blank.tga");
		button->setTexturePushed("blank.tga");
		button->setTextureOver("blank.tga");
		button->setColor(_OverColorNormal);
		button->setColorPushed(_OverColorPushed);
		button->setColorOver(_OverColorOver);
		button->setScale(true);
		// setup handle
		button->setType(CCtrlButton::RadioButton);
		button->setActionOnLeftClick("lst_select");
		button->setParamsOnLeftClick(toString(i));
		button->setActionOnRightClick("lst_rclick");
		button->setParamsOnRightClick(toString(i));
		button->setClickWhenPushed(_ClickWhenPushed);
		// init radio button
		button->initRBRef();
		// append
		_List->addCtrl(button);
		_SheetChildren[i]->Button = button;

		_SheetChildren[i]->init(this, i);
	}
	CWidgetManager::getInstance()->registerClockMsgTarget(this);
}


// ***************************************************************************
sint CDBGroupListSheetText::getIndexOf(const CDBCtrlSheet	*sheet) const
{
	for(uint k = 0; k < _SheetChildren.size(); ++k)
	{
		if (_SheetChildren[k]->Ctrl == sheet) return k;
	}
	return -1;
}

// ***************************************************************************
sint CDBGroupListSheetText::getIndexOf(const CCtrlButton *button) const
{
	for(uint k = 0; k < _SheetChildren.size(); ++k)
	{
		if (_SheetChildren[k]->Button == button) return k;
	}
	return -1;
}


// ***************************************************************************
CDBCtrlSheet *CDBGroupListSheetText::getSheet(uint index) const
{
	if (index >= _SheetChildren.size()) return NULL;
	return _SheetChildren[index]->Ctrl;
}

// ***************************************************************************
sint32 CDBGroupListSheetText::getNbElt () const
{
	sint32 NumValidChildren= 0;
	for(uint i=0;i<_SheetChildren.size();i++)
	{
		// is the control a correct sheetId?
		CDBCtrlSheet *ctrl= _SheetChildren[i]->Ctrl;
		bool validSheet= ctrl->isSheetValid();

		// default
		bool bValid= false;

		// validate the control if in array mode
		if(_Array)
			bValid= true;
		// validate the control only if valid sheet and filter OK
		else if(validSheet)
		{
			// if no filter, OK
			bValid= true;
		}

		if(bValid)
			NumValidChildren++;
	}

	return NumValidChildren;
}

// ***************************************************************************
void CDBGroupListSheetText::unselect()
{
	// unselect the first radio button
	if (_SheetChildren.empty()) return;
	_SheetChildren[0]->Button->unselect();
}

// ***************************************************************************
bool CDBGroupListSheetText::CSheetChild::isSheetValid(CDBGroupListSheetText * /* pFather */)
{
	if (!Ctrl) return false;
	return Ctrl->isSheetValid();
}

// ***************************************************************************
void CDBGroupListSheetText::CSheetChild::hide(CDBGroupListSheetText * /* pFather */)
{
	Ctrl->setActive(false);
	Text->setActive(false);
	Button->setActive(false);
}

// ***************************************************************************
CGroupContainer *CDBGroupListSheetText::getContainer()
{
	return dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getWindow(this));
}

// ***************************************************************************
bool		CDBGroupListSheetText::needCheckAllItems()
{
	if(_CtrlInfo._Type==CCtrlSheetInfo::SheetType_Item || _CtrlInfo._Type==CCtrlSheetInfo::SheetType_Auto)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		// Must test each frame if Player characteristics have changed, because in this case, ItemRequirement
		// and thus TextColor may change....
		bool	mustTest= false;
		for(uint i=0;i<CHARACTERISTICS::NUM_CHARACTERISTICS;i++)
		{
			sint32	newVal= pIM->getCurrentPlayerCarac((CHARACTERISTICS::TCharacteristics)i);
			if(newVal!=_LastPlayerCharac[i])
				mustTest= true;
			// update cache
			_LastPlayerCharac[i]= newVal;
		}
		return mustTest;
	}

	return false;
}

// ***************************************************************************
void		CDBGroupListSheetText::CSheetChild::updateViewTextAsItem()
{
	if(Ctrl && Text && Ctrl->getSheetCategory() == CDBCtrlSheet::Item)
	{
		// get the text
		ucstring text;
		Ctrl->getContextHelp(text);

		// Text color red if requirement not met
		if(Ctrl->getType() == CCtrlSheetInfo::SheetType_Item)
		{
			if(!Ctrl->checkItemRequirement())
				text= CI18N::get("uiItemCannotUseColor") + text;
		}

		// For item, add some information
		if(Ctrl->getType() == CCtrlSheetInfo::SheetType_Item)
		{
			const CItemSheet *pIS = Ctrl->asItemSheet();
			if(pIS)
			{
				// Add craft info for MP
				if(pIS->Family==ITEMFAMILY::RAW_MATERIAL)
				{
					ucstring	ipList;
					pIS->getItemPartListAsText(ipList);
					if(ipList.empty())
					{
						if(pIS->isUsedAsCraftRequirement())
							text+= "\n" + CI18N::get("uiItemMpCraftRequirement");
						else
							text+= "\n" + CI18N::get("uiItemMpNoCraft");
					}
					else
						text+= "\n" + CI18N::get("uiItemMpCanCraft") + ipList;
				}
			}
		}

		// set text
		Text->setTextFormatTaged(text);
	}
}


// ***************************************************************************
CDBGroupListSheetText::CSheetChild::~CSheetChild()
{
	if(Owner)
		Owner->_NameIdToUpdate.erase(this);
}


// ***************************************************************************
void CDBGroupListSheetText::CSheetChild::init(CDBGroupListSheetText *pFather, uint /* index */)
{
	Owner= pFather;
}


// ***************************************************************************
void CDBGroupListSheetText::notifyDownloadComplete(bool downloadComplete)
{
	_DownloadComplete = downloadComplete;
	if (_EmptyListNotifier && !_DownloadComplete)
	{
		_EmptyListNotifier->setActive(false);
	}
}


// ***************************************************************************
// ***************************************************************************
// Actions Handlers
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// Selection of the trade
class	CHandlerListSheetTextSelect : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// The grandparent of the button/ctrlSheet should be the list_sheet_text
		if(!pCaller->getParent())
			return;
		CDBGroupListSheetText	*listSheetTrade= dynamic_cast<CDBGroupListSheetText*>(pCaller->getParent()->getParent());
		if(!listSheetTrade)
			return;

		// select?
		uint	selected;
		fromString(Params, selected);

		// get button and ctrlSheet selected
		nlassert(selected < listSheetTrade->_SheetChildren.size());
		CDBCtrlSheet	*ctrlSheet=  listSheetTrade->_SheetChildren[selected]->Ctrl;
		CCtrlButton		*ctrlButton= listSheetTrade->_SheetChildren[selected]->Button;

		// run it, but take the wanted action handler
		CAHManager::getInstance()->runActionHandler(
			listSheetTrade->_CtrlInfo._AHOnLeftClick, ctrlSheet,
			listSheetTrade->_CtrlInfo._AHLeftClickParams);

		// get the button and set it the selected one
		if (!listSheetTrade->_SelectionEnabled)
			ctrlButton->setPushed(false);
	}

};
REGISTER_ACTION_HANDLER( CHandlerListSheetTextSelect, "lst_select" );

// ***************************************************************************
// RightClick on the trade
class	CHandlerListSheetTextRightClick : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// The grandparent of the button/ctrlSheet should be the list_sheet_text
		if(!pCaller->getParent())
			return;
		CDBGroupListSheetText	*listSheetTrade= dynamic_cast<CDBGroupListSheetText*>(pCaller->getParent()->getParent());
		if(!listSheetTrade)
			return;

		// select?
		uint	selected;
		fromString(Params, selected);

		// get button and ctrlSheet selected
		nlassert(selected < listSheetTrade->_SheetChildren.size());
		CDBCtrlSheet	*ctrlSheet=  listSheetTrade->_SheetChildren[selected]->Ctrl;
		CDBCtrlSheet::setCurrSelSheet(ctrlSheet);

		// run it, but take the wanted action handler
		CAHManager::getInstance()->runActionHandler(
			listSheetTrade->_CtrlInfo._AHOnRightClick, ctrlSheet,
			listSheetTrade->_CtrlInfo._AHRightClickParams);

		// Run Menu (if item is not being dragged)
		if (!listSheetTrade->_CtrlInfo._ListMenuRight.empty())
		{
			if ( CDBCtrlSheet::getDraggedSheet() == NULL)
			{
				CWidgetManager::getInstance()->enableModalWindow (ctrlSheet, listSheetTrade->_CtrlInfo._ListMenuRight);
			}
		}

	}

};
REGISTER_ACTION_HANDLER( CHandlerListSheetTextRightClick, "lst_rclick" );

// ***************************************************************************
// RightClick on the trade
class	CHandlerListSheetTextResetSelection : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		CDBGroupListSheetText *listSheetTrade = dynamic_cast<CDBGroupListSheetText*>(CWidgetManager::getInstance()->getElementFromId(Params));
		if (listSheetTrade == NULL)
			return;

		listSheetTrade->unselect();
	}
};
REGISTER_ACTION_HANDLER( CHandlerListSheetTextResetSelection, "list_sheet_text_reset_selection" );
