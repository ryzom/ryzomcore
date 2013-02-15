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

#include "stdafx.h"

#include "tools_logic.h"
#include "world_editor.h"
#include "world_editor_doc.h"
#include "type_sel_dlg.h"
#include "main_frm.h"
#include "display.h"
#include "action.h"
#include "editor_primitive.h"
#include "dialog_properties.h"

#include <string>

using namespace std;
using namespace NLLIGO;
using namespace NLMISC;

// ***************************************************************************

#define ID_MENU_REGION_CREATE_GROUP	0x10018
#define ID_MENU_REGION_PROPERTIES	0x10019
#define ID_MENU_REGION_HIDE_ALL		0x10020
#define ID_MENU_REGION_UNHIDE_ALL	0x10021
#define ID_MENU_REGION_HIDE_TYPE	0x10022
#define ID_MENU_REGION_UNHIDE_TYPE	0x10023

#define ID_MENU_GROUP_CREATE_POINT	0x10024
#define ID_MENU_GROUP_CREATE_PATH	0x10025
#define ID_MENU_GROUP_CREATE_ZONE	0x10026
#define ID_MENU_GROUP_DELETE		0x10027
#define ID_MENU_GROUP_PROPERTIES	0x10028
#define ID_MENU_GROUP_HIDE			0x10029
#define ID_MENU_GROUP_UNHIDE		0x1002A
#define ID_MENU_GROUP_TRANSFERT_REPLACE		0x1002B
#define ID_MENU_GROUP_TRANSFERT_APPEND		0x1002C

#define ID_MENU_PRIM_DELETE			0x10010
#define ID_MENU_PRIM_PROPERTIES		0x10011
#define ID_MENU_PRIM_HIDE			0x10012

// ***************************************************************************

BEGIN_MESSAGE_MAP(CToolsLogicTree, CTreeCtrl)
	ON_WM_LBUTTONDOWN ()
	ON_WM_LBUTTONUP ()
	ON_WM_LBUTTONDBLCLK ()
	ON_WM_RBUTTONDOWN ()
/*
	ON_COMMAND (ID_MENU_REGION_CREATE_GROUP,OnMenuRegionCreateGroup)
	ON_COMMAND (ID_MENU_REGION_PROPERTIES,	OnMenuRegionProperties)
	ON_COMMAND (ID_MENU_REGION_HIDE_ALL,	OnMenuRegionHideAll)
	ON_COMMAND (ID_MENU_REGION_UNHIDE_ALL,	OnMenuRegionUnhideAll)
	ON_COMMAND (ID_MENU_REGION_HIDE_TYPE,	OnMenuRegionHideType)
	ON_COMMAND (ID_MENU_REGION_UNHIDE_TYPE,	OnMenuRegionUnhideType)

	ON_COMMAND (ID_MENU_GROUP_CREATE_POINT,	OnMenuGroupCreatePoint)
	ON_COMMAND (ID_MENU_GROUP_CREATE_PATH,	OnMenuGroupCreatePath)
	ON_COMMAND (ID_MENU_GROUP_CREATE_ZONE,	OnMenuGroupCreateZone)
	ON_COMMAND (ID_MENU_GROUP_DELETE,		OnMenuGroupDelete)
	ON_COMMAND (ID_MENU_GROUP_PROPERTIES,	OnMenuGroupProperties)
	ON_COMMAND (ID_MENU_GROUP_HIDE,			OnMenuGroupHide)
	ON_COMMAND (ID_MENU_GROUP_UNHIDE,		OnMenuGroupUnhide)
	ON_COMMAND (ID_MENU_GROUP_TRANSFERT_APPEND,		OnMenuGroupTransfertAppend)
	ON_COMMAND (ID_MENU_GROUP_TRANSFERT_REPLACE,	OnMenuGroupTransfertReplace)

	ON_COMMAND (ID_MENU_PRIM_DELETE,		OnMenuPrimDelete)
	ON_COMMAND (ID_MENU_PRIM_PROPERTIES,	OnMenuPrimProperties)
	ON_COMMAND (ID_MENU_PRIM_HIDE,			OnMenuPrimHide)
*/
	ON_COMMAND (ID_EDIT_DELETE,				OnMenuPrimDelete)
	ON_COMMAND (ID_EDIT_PROPERTIES,			OnMenuPrimProperties)
	ON_COMMAND (ID_PROJECT_NEWPRIMITIVE,	OnProjectNewPrimitive)
	ON_COMMAND (ID_VIEW_SHOW,				OnViewShow)
	ON_COMMAND (ID_VIEW_HIDE,				OnViewHide)
	ON_COMMAND (ID_EDIT_SELECT_CHILDREN,	OnEditSelectChildren)
	ON_COMMAND_RANGE (ID_EDIT_CREATE_BEGIN, ID_EDIT_CREATE_END, OnAddPrimitive)
	ON_COMMAND_RANGE (ID_EDIT_GENERATE_BEGIN, ID_EDIT_GENERATE_END, OnGeneratePrimitive)
	ON_COMMAND_RANGE (ID_EDIT_OPEN_FILE_BEGIN, ID_EDIT_OPEN_FILE_END, OnOpenFile)
	ON_COMMAND(ID_EDIT_EXPAND,				OnEditExpand)
	ON_COMMAND(ID_EDIT_COLLAPSE,			OnEditCollapse)
	ON_COMMAND(ID_RENAME_SELECTED, OnRenameSelected)
	ON_COMMAND(ID_REPAIR_SELECTED, OnRepairSelected)
	ON_COMMAND(ID_HELP_FINDER, OnTreeHelp)
	
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE ()
	
	//ON_NOTIFY_REFLECT(TVN_SELCHANGED,		OnSelChanged)
END_MESSAGE_MAP()

// ***************************************************************************

class CPrimitiveTreeData
{
public:
	CPrimitiveTreeData (const NLLIGO::CPrimitives *primitives, const IPrimitive *primitive)
	{
		Primitive = primitive;
		Primitives = primitives;
	}

	// The primitive
	const NLLIGO::CPrimitives		*Primitives;
	const IPrimitive				*Primitive;
};

// ***************************************************************************

CToolsLogicTree::CToolsLogicTree ()
{
	_MainFrame = NULL;
	SelectMark = NULL;
	m_boDragging = false;
}

// ***************************************************************************
void CToolsLogicTree::init (CMainFrame *pMF)
{
	_MainFrame = pMF;
//	pMF->_PRegionBuilder.setToolsLogic (this);
	_RegionsInfo.resize (0);
	// pMF->_PRegionBuilder.updateToolsLogic ();

	/*CImageList *pImgList = new CImageList;
	pImgList->Create (16, 16, ILC_MASK, 0, 5);
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_PRIM));		// 0
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_GROUP));		// 1
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_POINT));		// 2
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_POINT_HIDE));	// 3
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_LINE));		// 4
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_LINE_HIDE));	// 5
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_ZONE));		// 6
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_ZONE_HIDE));	// 7*/

	// Add the image list
	SetImageList (&(theApp.ImageList.ImageList), TVSIL_NORMAL);
}

// ***************************************************************************
void CToolsLogicTree::reset ()
{
//	GetTreeCtrl().DeleteAllItems();
	DeleteAllItems();
	_RegionsInfo.clear ();
}

// ***************************************************************************
void CToolsLogicTree::uninit ()
{
	_MainFrame = NULL;
}

// ***************************************************************************
void CToolsLogicTree::setTool (CToolsLogic *pTools)
{
	_ToolLogic = pTools;
}

// ***************************************************************************
uint32 CToolsLogicTree::createNewRegion (const string &name)
{
	SRegionInfo ri;

	if (_MainFrame == NULL)
		return 0;
	
	ri.Name = name;
	ri.RegionItem = insertItemUTF8 (*this, ri.Name.c_str(), 0, 0);
	//ri.PointItem = insertItemUTF8 (*this, "Points", ri.RegionItem);
	//ri.PathItem = insertItemUTF8 (*this, "Splines", ri.RegionItem);
	//ri.ZoneItem = insertItemUTF8 (*this, "Patatoids", ri.RegionItem);
	_RegionsInfo.push_back (ri);
	
	return _RegionsInfo.size()-1;
}

// ***************************************************************************
uint32 CToolsLogicTree::createNewGroup (uint32 nRegion, const string &sGroupName)
{
	SRegionInfo &rRI = _RegionsInfo[nRegion];

	if (_MainFrame == NULL)
		return 0;

	HTREEITEM newItem = insertItemUTF8 (*this, sGroupName.c_str(), 1, 1, rRI.RegionItem);
	rRI.Groups.push_back (newItem);
	
	return rRI.Groups.size()-1;
}

// ***************************************************************************
HTREEITEM CToolsLogicTree::addPoint (uint32 nRegion, uint32 nGroup, const string &name, bool bHide)
{
	if (bHide)
		return insertItemUTF8 (*this, name.c_str(), 3, 3, _RegionsInfo[nRegion].Groups[nGroup]);
	return insertItemUTF8 (*this, name.c_str(), 2, 2, _RegionsInfo[nRegion].Groups[nGroup]);
}

// ***************************************************************************
HTREEITEM CToolsLogicTree::addPath (uint32 nRegion, uint32 nGroup, const string &name, bool bHide)
{
	if (bHide)
		return insertItemUTF8 (*this, name.c_str(), 5, 5, _RegionsInfo[nRegion].Groups[nGroup]);
	return insertItemUTF8 (*this, name.c_str(), 4, 4, _RegionsInfo[nRegion].Groups[nGroup]);
}

// ***************************************************************************
HTREEITEM CToolsLogicTree::addZone (uint32 nRegion, uint32 nGroup, const string &name, bool bHide)
{
	if (bHide)
		return insertItemUTF8 (*this, name.c_str(), 7, 7, _RegionsInfo[nRegion].Groups[nGroup]);
	return insertItemUTF8 (*this, name.c_str(), 6, 6, _RegionsInfo[nRegion].Groups[nGroup]);
}

// ***************************************************************************
void CToolsLogicTree::expandAll (uint32 nPos)
{
	Expand (_RegionsInfo[nPos].RegionItem, TVE_EXPAND);
	for(uint32 i = 0; i < _RegionsInfo[nPos].Groups.size(); ++i)
		Expand (_RegionsInfo[nPos].Groups[i], TVE_EXPAND);
}

// ***************************************************************************
BOOL CToolsLogicTree::PreCreateWindow (CREATESTRUCT& cs)
{
	cs.style |= TVS_SHOWSELALWAYS;
	if (!CTreeCtrl::PreCreateWindow(cs))
		return FALSE;
	return TRUE;
}

// ***************************************************************************

void CToolsLogicTree::OnLButtonDown (UINT nFlags, CPoint point)
{
	// Hit item test
	UINT uFlags;
	HTREEITEM hItem = HitTest(point, &uFlags);

	// Item found ?
	if (hItem)
	{
		// Remove lock mode
		getMainFrame ()->setSelectionLocked (false);

		// Select		
		SelectItem (hItem);
		CPrimitiveTreeData *data = (CPrimitiveTreeData*)GetItemData (hItem);
		CDatabaseLocatorPointer locator;

		// The document
		CWorldEditorDoc *doc = getDocument ();

		doc->getLocator (locator, data->Primitive);
		if (data)
		{

			// Begin
			doc->beginModification ();

			// Unselect
			if (((nFlags & MK_CONTROL) == 0) && ((nFlags & MK_SHIFT) == 0))
				doc->addModification (new CActionUnselectAll ());

			// Shift select
			if ((nFlags & MK_SHIFT) != 0)
			{
				// Not the same node
				if (SelectMark && (SelectMark != hItem))
				{
					// Same parent ?
					if (GetParentItem (SelectMark) == GetParentItem (hItem))
					{
						// Find the hItem
						HTREEITEM current = SelectMark;
						do
						{
							current = GetNextSiblingItem (current);
						}
						while (current && (current != hItem));

						// Found ?
						if (current)
						{
							// Select all items
							current = SelectMark;
							current = GetNextSiblingItem (current);
							nlassert (current);
							while (current != hItem)
							{							
								// Get current data
								CPrimitiveTreeData *dataCurrent = (CPrimitiveTreeData*)GetItemData (current);
								CDatabaseLocatorPointer locatorCurrent;
								doc->getLocator (locatorCurrent, dataCurrent->Primitive);

								// Select
//								IProperty *prop;
								if ((getPrimitiveEditor(dataCurrent->Primitive)->getSelected() && (nFlags & MK_CONTROL) != 0))
//								if ((dataCurrent->Primitive->getPropertyByName ("selected", prop) && (nFlags & MK_CONTROL) != 0))
									doc->addModification (new CActionUnselect (locatorCurrent));
								else
									doc->addModification (new CActionSelect (locatorCurrent));

								// Next item
								current = GetNextSiblingItem (current);
								nlassert (current);
							}
						}
						else
						{
							// Select all items
							current = SelectMark;
							current = GetPrevSiblingItem (current);
							nlassert (current);
							while (current != hItem)
							{							
								// Get current data
								CPrimitiveTreeData *dataCurrent = (CPrimitiveTreeData*)GetItemData (current);
								CDatabaseLocatorPointer locatorCurrent;
								doc->getLocator (locatorCurrent, dataCurrent->Primitive);

								// Select
//								IProperty *prop;
								if ((getPrimitiveEditor(dataCurrent->Primitive)->getSelected() && (nFlags & MK_CONTROL) != 0))
//								if ((dataCurrent->Primitive->getPropertyByName ("selected", prop) && (nFlags & MK_CONTROL) != 0))
									doc->addModification (new CActionUnselect (locatorCurrent));
								else
									doc->addModification (new CActionSelect (locatorCurrent));

								// Next item
								current = GetPrevSiblingItem (current);
								nlassert (current);
							}
						}
					}
				}
			}

			// Select
//			IProperty *prop;
			if ((getPrimitiveEditor(data->Primitive)->getSelected() && (nFlags & MK_CONTROL) != 0))
//			if ((data->Primitive->getPropertyByName ("selected", prop) && (nFlags & MK_CONTROL) != 0))
				doc->addModification (new CActionUnselect (locator));
			else
			{
				// Get the path of selected primitive and store it
				std::string path;
				doc->getFilePath(locator.getDatabaseIndex(), path);
				doc->setPathOfSelectedPrimitive(path);
				doc->addModification (new CActionSelect (locator));
			}

			// End
			doc->endModification ();

		
			// we save the property dialog which was holding the old selection
			CDialogProperties* pDlg = NULL;
			list<CDialogProperties*>::iterator it = PropertiesDialogs.begin();
			bool ok = false;

			while ( !ok && ( it != PropertiesDialogs.end() ) )
			{
				if ( (*it)->equalsSelection( Selection ) )
				{
					pDlg = (*it);
					ok = true;
				}

				it++;
			}

			// Update data
			getMainFrame ()->updateData ();

			if ( ( pDlg ) && ! ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) ) 
			{
				// is there already a property dialog with the new selection ?
				CDialogProperties *pNewSelectionDlg = NULL;
				it = PropertiesDialogs.begin();
				bool ok = false;

				while ( !ok && ( it != PropertiesDialogs.end() ) )
				{
					if ( (*it)->equalsSelection( Selection ) )
					{
						pNewSelectionDlg = (*it);
						ok = true;
					}

					it++;
				}
					
				if ( pNewSelectionDlg )
					pNewSelectionDlg->SetFocus();
				else
					pDlg->changeSelection (Selection);

			}
		}

		// No shift select
		if ((nFlags & MK_SHIFT) == 0)
			// New mark
			SelectMark = hItem;
	}

	// Call previous method
	CTreeCtrl::OnLButtonDown (nFlags, point);
}

// ***************************************************************************

void CToolsLogicTree::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// Hit item test
	UINT uFlags;
	HTREEITEM hItem = HitTest(point, &uFlags);

	// Item found ?
	if (hItem)
	{
		// Remove lock mode
		getMainFrame ()->setSelectionLocked (false);

		// The docmuent
		CWorldEditorDoc *doc = getDocument ();

		// Select		
		SelectItem (hItem);
		CPrimitiveTreeData *data = (CPrimitiveTreeData*)GetItemData (hItem);
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, data->Primitive);
		if (data)
		{
			// The docmuent
			CWorldEditorDoc *doc = getDocument ();

			// Begin
			doc->beginModification ();

			// Unselect
			if ((nFlags & MK_CONTROL) == 0)
				doc->addModification (new CActionUnselectAll ());

			// Select
			doc->addModification (new CActionSelect (locator));

			// End
			doc->endModification ();

			// Update data
			getMainFrame ()->updateData ();

			// Call properties
			_MainFrame->OnEditProperties();
		}
	}

	// New mark
	SelectMark = hItem;
}

// ***************************************************************************
void CToolsLogicTree::OnRButtonDown (UINT nFlags, CPoint point)
{
	// Hit item test
	UINT uFlags;
	HTREEITEM hItem = HitTest(point, &uFlags);

	// The docmuent
	CWorldEditorDoc *doc = getDocument ();

	// Item found ?
	if (hItem)
	{
		SelectItem (hItem);
		CPrimitiveTreeData *data = (CPrimitiveTreeData*)GetItemData (hItem);
		CDatabaseLocatorPointer locator;
		doc->getLocator (locator, data->Primitive);
		if (data)
		{
			// The docmuent
			CWorldEditorDoc *doc = getDocument ();

			// Begin
			doc->beginModification ();

			// Unselect
			nlassert (data->Primitive);
//			IProperty *prop;
			if (((nFlags & MK_CONTROL) == 0) && (!getPrimitiveEditor(data->Primitive)->getSelected()))
//			if (((nFlags & MK_CONTROL) == 0) && (!data->Primitive->getPropertyByName ("selected", prop)))
				doc->addModification (new CActionUnselectAll ());

			// Select
			doc->addModification (new CActionSelect (locator));

			// End
			doc->endModification ();

			// Update data
			getMainFrame ()->updateData ();
		}
	}

	// New mark
	SelectMark = hItem;

	// Menu creation
	ClientToScreen (&point);
	getMainFrame ()->createContextMenu (this, point, false);
}

// ***************************************************************************
void CToolsLogicTree::OnSelChanged ()
{
	HTREEITEM hItem = GetSelectedItem();
	HTREEITEM hParent = GetParentItem (hItem);

	if (hItem != NULL)
	{
		// Select the item
		Select(hItem, TVGN_CARET);
	}
}

// ***************************************************************************
// Contextual Menu : Region
// ***************************************************************************

// ***************************************************************************
void CToolsLogicTree::OnMenuRegionCreateGroup ()
{
	HTREEITEM item = GetSelectedItem ();
	HTREEITEM parent = GetParentItem (item);

	// Go to the Region Item
	while (parent != NULL)
	{
		item = parent;
		parent = GetParentItem (item);
	}

	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == item)
	{
		HTREEITEM newGroup = insertItemUTF8 (*this, "New Group", 1, 1, item);
		Expand (item, TVE_EXPAND);
		Expand (newGroup, TVE_EXPAND);
	}
	Invalidate ();
}

// ***************************************************************************

void CToolsLogicTree::OnMenuRegionHideAll ()
{
	HTREEITEM itemRegion = GetSelectedItem ();
	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == itemRegion)
	{
		HTREEITEM itemGroup = GetChildItem (itemRegion);
		while (itemGroup != NULL)
		{
			HTREEITEM itemPrim = GetChildItem (itemGroup);
			while (itemPrim != NULL)
			{
				int nImg, nSelImg;
				GetItemImage (itemPrim, nImg, nSelImg);
				if ((nImg == 2) || (nImg == 4) || (nImg == 6))
					nImg = nImg + 1;
				SetItemImage (itemPrim, nImg, nImg);

				itemPrim = GetNextItem (itemPrim, TVGN_NEXT);
			}
			itemGroup = GetNextItem (itemGroup, TVGN_NEXT);
		}

		break;
	}
}

// ***************************************************************************
void CToolsLogicTree::OnMenuRegionUnhideAll ()
{
	HTREEITEM itemRegion = GetSelectedItem ();
	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == itemRegion)
	{

		HTREEITEM itemGroup = GetChildItem (itemRegion);
		while (itemGroup != NULL)
		{
			HTREEITEM itemPrim = GetChildItem (itemGroup);
			while (itemPrim != NULL)
			{
				int nImg, nSelImg;
				GetItemImage (itemPrim, nImg, nSelImg);
				if ((nImg == 3) || (nImg == 5) || (nImg == 7))
					nImg = nImg - 1;
				SetItemImage (itemPrim, nImg, nImg);

				itemPrim = GetNextItem (itemPrim, TVGN_NEXT);
			}
			itemGroup = GetNextItem (itemGroup, TVGN_NEXT);
		}
		break;
	}
}

// ***************************************************************************
void CToolsLogicTree::OnMenuRegionHideType ()
{
	HTREEITEM item = GetSelectedItem ();
	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == item)
	{
		CTypeSelDlg dial(this);
		dial._TypesInit = &(_MainFrame->_Environnement.Types);
		if (dial.DoModal() == IDOK)
		{
		}
		break;
	}
}

// ***************************************************************************
void CToolsLogicTree::OnMenuRegionUnhideType ()
{
	HTREEITEM item = GetSelectedItem ();
	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == item)
	{
		CTypeSelDlg dial(this);
		dial._TypesInit = &(_MainFrame->_Environnement.Types);
		if (dial.DoModal() == IDOK)
		{
		}
		break;
	}
}

// ***************************************************************************
// Contextual Menu : Group
// ***************************************************************************

// ***************************************************************************
void CToolsLogicTree::OnMenuGroupCreatePoint()
{
	HTREEITEM itemGroup = GetSelectedItem ();
	HTREEITEM itemRegion = GetParentItem (itemGroup);
	uint32 i;
	CCreateDialog dialog (this);
	dialog.TypesForInit = &_MainFrame->_Environnement.Types;

	for (i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == itemRegion)
	{
		dialog.setRegionName (_RegionsInfo[i].Name);
		break;
	}

	dialog.MainFrame = _MainFrame;

	if (dialog.DoModal () == IDOK)
	if (strlen(dialog.Name) > 0)
	{
		HTREEITEM newItem = insertItemUTF8 (*this, dialog.Name, 2, 2, itemGroup);
		Expand (itemGroup, TVE_EXPAND);
		// Create the newItem
		for (i = 0; i < _RegionsInfo.size(); ++i)
		if (_RegionsInfo[i].RegionItem == itemRegion)
		{
		}
		// Callback handling
		_MainFrame->primZoneModified();
	}
	Invalidate ();
}

// ***************************************************************************
void CToolsLogicTree::OnMenuGroupCreatePath()
{
	HTREEITEM itemGroup = GetSelectedItem ();
	HTREEITEM itemRegion = GetParentItem (itemGroup);
	uint32 i;
	CCreateDialog dialog (this);
	dialog.TypesForInit = &_MainFrame->_Environnement.Types;

	for (i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == itemRegion)
	{
		dialog.setRegionName (_RegionsInfo[i].Name);
		break;
	}

	dialog.MainFrame = _MainFrame;

	if (dialog.DoModal () == IDOK)
	if (strlen(dialog.Name) > 0)
	{
		HTREEITEM newItem = insertItemUTF8 (*this, dialog.Name, 4, 4, itemGroup);
		Expand (itemGroup, TVE_EXPAND);
		// Create the newItem
		for (i = 0; i < _RegionsInfo.size(); ++i)
		if (_RegionsInfo[i].RegionItem == itemRegion)
		{
		}
		// Callback handling
		_MainFrame->primZoneModified();
	}
	Invalidate ();
}

// ***************************************************************************
void CToolsLogicTree::OnMenuGroupCreateZone()
{
	HTREEITEM itemGroup = GetSelectedItem ();
	HTREEITEM itemRegion = GetParentItem (itemGroup);
	uint32 i;
	CCreateDialog dialog (this);
	dialog.TypesForInit = &_MainFrame->_Environnement.Types;

	for (i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == itemRegion)
	{
		dialog.setRegionName (_RegionsInfo[i].Name);
		break;
	}

	dialog.MainFrame = _MainFrame;

	if (dialog.DoModal () == IDOK)
	if (strlen(dialog.Name) > 0)
	{
		HTREEITEM newItem = insertItemUTF8 (*this, dialog.Name, 6, 6, itemGroup);
		Expand (itemGroup, TVE_EXPAND);
		// Create the newItem
		for (i = 0; i < _RegionsInfo.size(); ++i)
		if (_RegionsInfo[i].RegionItem == itemRegion)
		{
		}
		// Callback handling
		_MainFrame->primZoneModified();
	}
	Invalidate ();
}

// ***************************************************************************
void CToolsLogicTree::OnMenuGroupDelete ()
{
	HTREEITEM itemGroup = GetSelectedItem ();
	HTREEITEM itemRegion;
	HTREEITEM itemRoot;

	// Go to the Region Item
	itemRegion = itemRoot = itemGroup;
	while (itemRoot != NULL)
	{
		itemRegion = itemRoot;
		itemRoot = GetParentItem (itemRegion);
	}

	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == itemRegion)
	{
		HTREEITEM hChildItem = GetChildItem (itemGroup);
		while (hChildItem != NULL)
		{
			HTREEITEM hNextItem = GetNextItem (hChildItem, TVGN_NEXT);

			DeleteItem (hChildItem);
			OnSelChanged ();

			hChildItem = hNextItem;
		}
		for (uint32 j = 0; j < _RegionsInfo[i].Groups.size(); ++j)
		if (_RegionsInfo[i].Groups[j] == itemGroup)
		{
			for (uint32 k = j+1; k < _RegionsInfo[i].Groups.size(); ++k)
				_RegionsInfo[i].Groups[k-1] = _RegionsInfo[i].Groups[k];
			_RegionsInfo[i].Groups.resize (_RegionsInfo[i].Groups.size()-1);
		}
		DeleteItem (itemGroup);
		// Callback handling
		_MainFrame->primZoneModified();
	}

}

// ***************************************************************************
void CToolsLogicTree::OnMenuGroupProperties()
{
	CEditStringDlg es(this);
	HTREEITEM hItem = GetSelectedItem ();
	es.Name = GetItemText (hItem);
	if (es.DoModal() == IDOK)
	{
		setItemTextUTF8 (*this, hItem, es.Name);

		HTREEITEM hChildItem = GetChildItem(hItem);
		while (hChildItem != NULL)
		{
			HTREEITEM hNextItem = GetNextItem (hChildItem, TVGN_NEXT);
			hChildItem = hNextItem;
		}
	}
}

// ***************************************************************************
void CToolsLogicTree::OnMenuGroupHide()
{
	HTREEITEM itemGroup = GetSelectedItem ();
	HTREEITEM itemRegion = GetParentItem (itemGroup);

	HTREEITEM hChildItem = GetChildItem(itemGroup);
	while (hChildItem != NULL)
	{
		HTREEITEM hNextItem = GetNextItem (hChildItem, TVGN_NEXT);

		int nImg, nSelImg;
		GetItemImage (hChildItem, nImg, nSelImg);
		if ((nImg == 2) || (nImg == 4) || (nImg == 6))
			nImg = nImg + 1;
		SetItemImage (hChildItem, nImg, nImg);

		hChildItem = hNextItem;
	}
}

// ***************************************************************************
void CToolsLogicTree::OnMenuGroupUnhide()
{
	HTREEITEM itemGroup = GetSelectedItem ();
	HTREEITEM itemRegion = GetParentItem (itemGroup);

	HTREEITEM hChildItem = GetChildItem(itemGroup);
	while (hChildItem != NULL)
	{
		HTREEITEM hNextItem = GetNextItem (hChildItem, TVGN_NEXT);

		int nImg, nSelImg;
		GetItemImage (hChildItem, nImg, nSelImg);
		if ((nImg == 3) || (nImg == 5) || (nImg == 7))
			nImg = nImg - 1;
		SetItemImage (hChildItem, nImg, nImg);

		hChildItem = hNextItem;
	}
}

// ***************************************************************************
void CToolsLogicTree::OnMenuGroupTransfertAppend ()
{
	vector<string> vTemp;
	
	HTREEITEM itemGroup = GetSelectedItem ();
	HTREEITEM hChildItem = GetChildItem(itemGroup);
	while (hChildItem != NULL)
	{
		HTREEITEM hNextItem = GetNextItem (hChildItem, TVGN_NEXT);

		vTemp.push_back((LPCSTR)GetItemText(hChildItem));
		
		hChildItem = hNextItem;
	}

	/* if (_MainFrame->_MasterCB != NULL)
		_MainFrame->multiTransfert(vTemp, true); */
}

// ***************************************************************************
void CToolsLogicTree::OnMenuGroupTransfertReplace ()
{
	vector<string> vTemp;
	
	HTREEITEM itemGroup = GetSelectedItem ();
	HTREEITEM hChildItem = GetChildItem(itemGroup);
	while (hChildItem != NULL)
	{
		HTREEITEM hNextItem = GetNextItem (hChildItem, TVGN_NEXT);

		vTemp.push_back((LPCSTR)GetItemText(hChildItem));
		
		hChildItem = hNextItem;
	}

	/* if (_MainFrame->_MasterCB != NULL)
		_MainFrame->multiTransfert(vTemp, false); */
}

// ***************************************************************************
// Contextual Menu : Prim
// ***************************************************************************

void CToolsLogicTree::OnMenuPrimDelete()
{
	_MainFrame->deletePrimitive (false, "Delete");
}

// ***************************************************************************

void CToolsLogicTree::OnProjectNewPrimitive ()
{
	_MainFrame->OnProjectNewPrimitive ();
}

// ***************************************************************************

void CToolsLogicTree::OnViewShow ()
{
	_MainFrame->OnViewShow ();
}

// ***************************************************************************

void CToolsLogicTree::OnViewHide ()
{
	_MainFrame->OnViewHide ();
}

// ***************************************************************************

void CToolsLogicTree::OnEditExpand ()
{
	_MainFrame->OnEditExpand ();
}

// ***************************************************************************

void CToolsLogicTree::OnEditCollapse ()
{
	_MainFrame->OnEditCollapse ();
}

// ***************************************************************************

void CToolsLogicTree::OnEditSelectChildren ()
{
	_MainFrame->OnEditSelectChildren ();
}

// ***************************************************************************

void CToolsLogicTree::OnMenuPrimProperties()
{
	// Open the property dialog
	_MainFrame->OnEditProperties ();
}

// ***************************************************************************

void CToolsLogicTree::OnAddPrimitive (UINT nID)
{
	// Get the document
	CWorldEditorDoc *doc = getDocument ();

	// Begin modification
	doc->beginModification ();

	// Only one selection ?
	nlassert (Selection.size () == 1);
	nlassert (Selection.front ());

	// What class is it ?
	const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*(Selection.front ()));
	nlassert (primClass);
	uint childId = nID - ID_EDIT_CREATE_BEGIN;

	// Get the child class
	CDatabaseLocatorPointer insert;
	doc->getLocator (insert, Selection.front ());
	CDatabaseLocator dest;
	insert.appendChild (dest);

	// Position on the Display
	CDisplay *display = getDisplay ();
	float delta = (float)DELTA_POS_ADD_PRIMITIVE * (display->_CurViewMax.x - display->_CurViewMin.x) / (float)display->getWidth ();
	CVector pos = (display->_CurViewMin + display->_CurViewMax)/2;
	if (doc->addModification (new CActionAddPrimitiveByClass (dest, primClass->DynamicChildren[childId].ClassName.c_str (), pos, delta,
		primClass->DynamicChildren[childId].Parameters)))
	{
		// Unselect all
		doc->addModification (new CActionUnselectAll ());

		// Select it
		doc->addModification (new CActionSelect (dest));
	}

	// End modification
	doc->endModification ();

	// Update the data
	_MainFrame->updateData ();
}

// ***************************************************************************

void CToolsLogicTree::OnGeneratePrimitive (UINT nID)
{
	_MainFrame->OnGeneratePrimitive (nID);
}

// ***************************************************************************

void CToolsLogicTree::OnOpenFile (UINT nID)
{
	_MainFrame->OnOpenFile (nID);
}

// ***************************************************************************
// CToolsLogic
// ***************************************************************************

// ***************************************************************************

BEGIN_MESSAGE_MAP (CToolsLogic, CFormView)
	ON_WM_CREATE()
	ON_WM_SIZE ()
	ON_WM_PAINT ()
	ON_BN_CLICKED (IDC_NEW_PATAT, OnNewPatat)
	ON_BN_CLICKED (IDC_NEW_GROUP, OnNewGroup)
END_MESSAGE_MAP()

// ***************************************************************************

IMPLEMENT_DYNCREATE(CToolsLogic, CFormView)
	
// ***************************************************************************
CToolsLogic::CToolsLogic() : CFormView(IDD_TOOLS_LOGIC)
{
	_Tree = NULL;
	_TreeCreated = false;
}

// ***************************************************************************
CToolsLogicTree *CToolsLogic::GetTreeCtrl()
{
	if (_TreeCreated)
		return _Tree;
	return NULL;
}
	
// ***************************************************************************
void CToolsLogic::init (CMainFrame *pMF)
{
	_MainFrame = pMF;
	if (GetTreeCtrl() != NULL)
		GetTreeCtrl()->init (pMF);
}

// ***************************************************************************
void CToolsLogic::reset ()
{
	if (GetTreeCtrl() != NULL)
		GetTreeCtrl()->reset ();
}

// ***************************************************************************
void CToolsLogic::uninit ()
{
	if (GetTreeCtrl() != NULL)
		GetTreeCtrl()->uninit ();
}

// ***************************************************************************
uint32 CToolsLogic::createNewRegion (const string &sRegionName)
{
	if (_TreeCreated)
		return GetTreeCtrl()->createNewRegion (sRegionName);
	return 0;
}

// ***************************************************************************
uint32 CToolsLogic::createNewGroup (uint32 nRegion, const string &sGroupName)
{
	if (_TreeCreated)
		return GetTreeCtrl()->createNewGroup (nRegion, sGroupName);
	return 0;
}

// ***************************************************************************
HTREEITEM CToolsLogic::addPoint (uint32 nRegion, uint32 nGroup, const string &name, bool bHide)
{
	if (_TreeCreated)
		return GetTreeCtrl()->addPoint (nRegion, nGroup, name, bHide);
	return NULL;
}

// ***************************************************************************
HTREEITEM CToolsLogic::addPath  (uint32 nRegion, uint32 nGroup, const string &name, bool bHide)
{
	if (_TreeCreated)
		return GetTreeCtrl()->addPath (nRegion, nGroup, name, bHide);
	return NULL;
}

// ***************************************************************************
HTREEITEM CToolsLogic::addZone  (uint32 nRegion, uint32 nGroup, const string &name, bool bHide)
{
	if (_TreeCreated)
		return GetTreeCtrl()->addZone (nRegion, nGroup, name, bHide);
	return NULL;
}

// ***************************************************************************
void CToolsLogic::expandAll (uint32 nRegion)
{
	if (_TreeCreated)
		GetTreeCtrl()->expandAll (nRegion);
}

#define TREE_TOP 0
#define TREE_LEFT 0
#define TREE_RIGHT 0
#define TREE_BOTTOM 0

// ***************************************************************************
int CToolsLogic::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
//	if (CFormView::OnCreate (lpCreateStruct) == -1)
//		return -1;
	if (CFormView::OnCreate (lpCreateStruct) == -1)
		return -1;

	::CRect iniRect;
	GetClientRect(&iniRect);
	iniRect.top = TREE_TOP; iniRect.left = TREE_LEFT;
	iniRect.right += TREE_RIGHT; iniRect.bottom += TREE_BOTTOM;
	_Tree = new CToolsLogicTree;
	_Tree->Create	( WS_VISIBLE | WS_TABSTOP | WS_CHILD
					  | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS, 
					  iniRect, this, 0x1) ;

	BOOL tIsUnicode; 
#ifdef UNICODE 
tIsUnicode = TreeView_SetUnicodeFormat(_Tree->m_hWnd, 1); 
#else 
tIsUnicode = TreeView_SetUnicodeFormat(_Tree->m_hWnd, 0); 
#endif // UNICODE 

	_Tree->setTool (this);
	_TreeCreated = true;

	return 0;
}

// ***************************************************************************
void CToolsLogic::OnSize (UINT nType, int cx, int cy)
{
	CFormView::OnSize (nType, cx, cy);
	// Resize list ctrl to fill the whole view.
	if (_TreeCreated)
	{
		::CRect iniRect;
		GetClientRect(&iniRect);
		
		iniRect.top += TREE_TOP; iniRect.left += TREE_LEFT;
		iniRect.right += TREE_RIGHT; iniRect.bottom += TREE_BOTTOM;
		
		GetTreeCtrl()->MoveWindow (&iniRect);
	}

}

// ***************************************************************************
void CToolsLogic::OnPaint ()
{
	GetTreeCtrl()->ShowScrollBar(SB_VERT);
	CFormView::OnPaint();
}

// ***************************************************************************
void CToolsLogic::OnNewGroup ()
{
	GetTreeCtrl()->OnMenuRegionCreateGroup ();
}

// ***************************************************************************
void CToolsLogic::OnNewPatat ()
{
	CComboBox *pCB = (CComboBox*)GetDlgItem(IDC_TYPE);
	CString str;
	getWindowTextUTF8 (*pCB, str);
	string sStr = (LPCSTR)str;
}

// ***************************************************************************
// CCreateDialog
// ***************************************************************************

// ***************************************************************************
BEGIN_MESSAGE_MAP(CCreateDialog, CDialog)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
		ON_CBN_SELCHANGE(IDC_COMBOTYPE, OnSelChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ***************************************************************************
CCreateDialog::CCreateDialog (CWnd*pParent) : CDialog(IDD_CREATE_ELEMENT, pParent) 
{
	Name[0] = 0;
	LayerName[0] = 0;
}

// ***************************************************************************
BOOL CCreateDialog::OnInitDialog ()
{
	CDialog::OnInitDialog();

	for (uint32 i = 0; i < TypesForInit->size(); ++i)
	{
		ComboType.InsertString (-1, TypesForInit->operator[](i).Name.c_str());
	}

	if (TypesForInit->size()>0)
	{
		if (ComboType.SelectString (0, LayerName) == CB_ERR)
			ComboType.SetCurSel (0);
		UpdateData();
		if (strlen(Name) == 0)
			OnSelChange();
	}
	
	return true;
}

// ***************************************************************************
void CCreateDialog::DoDataExchange (CDataExchange* pDX )
{
	DDX_Control(pDX, IDC_COMBOTYPE, ComboType);

	DDX_Text(pDX, IDC_EDIT_NAME, (LPTSTR)Name, 128);
	DDV_MaxChars(pDX, Name, 128);

	DDX_Text(pDX, IDC_COMBOTYPE, (LPTSTR)LayerName, 128);
	DDV_MaxChars(pDX, LayerName, 128);
}

// ***************************************************************************
void CCreateDialog::OnOK()
{
	UpdateData ();

	// If the "region_" do not exist add it
	if (strncmp(RegionPost.c_str(), Name, strlen(RegionPost.c_str())) != 0)
	{
		char sTmp[128];
		strcpy (sTmp, RegionPost.c_str());
		strcat (sTmp, Name);
		strcpy (Name, sTmp);
		UpdateData (false);
	}

	if (strcmp(PropName.c_str(), Name) == 0)
		CDialog::OnOK();
	
	CDialog::OnOK();
}

// ***************************************************************************
void CCreateDialog::setRegionName (const string &rn)
{
	for (uint32 i = 0; i < rn.size(); ++i)
	{
		if (rn[i] == '.')
		{
			RegionPost += '-';
			return;
		}
		RegionPost += rn[i];
	}
	RegionPost += '-';
}

// ***************************************************************************
void CCreateDialog::OnSelChange ()
{
	int cs = ComboType.GetCurSel();
	CString sTmp;
	ComboType.GetLBText (cs, sTmp);

	if (PropType == (LPCSTR)sTmp)
	{
		strcpy (Name, PropName.c_str());
	}
	else
	{
		strcpy (Name, RegionPost.c_str());
		strcat (Name, (LPCSTR)sTmp);
		strcat (Name, "-");
	}

	UpdateData (false);
	Invalidate ();
}

// ***************************************************************************
// CEditStringDlg
// ***************************************************************************

// ***************************************************************************
CEditStringDlg::CEditStringDlg(CWnd*pParent) : CDialog(IDD_EDIT_STRING, pParent) 
{
	Name = _T("");
}

// ***************************************************************************
BOOL CEditStringDlg::OnInitDialog ()
{
	CDialog::OnInitDialog();

	UpdateData();

	return TRUE;
}

// ***************************************************************************
void CEditStringDlg::DoDataExchange (CDataExchange* pDX )
{
	DDX_Text(pDX, IDC_EDIT_NAME, Name);
}

// ***************************************************************************
void CEditStringDlg::OnOK()
{
	UpdateData ();
	CDialog::OnOK();
}

// ***************************************************************************

// Returns the index-th first level tree node
void selectItem (CTreeCtrl &tree, uint index)
{
	HTREEITEM item = tree.GetRootItem ();
	while (index--)
	{
		item = tree.GetNextSiblingItem (item);
		nlassert (item);
	}
	tree.SelectItem (item);
}

// ***************************************************************************

BOOL CToolsLogic::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
	int idCtrl = (int) wParam; 
    LPNMHDR pnmh = (LPNMHDR) lParam; 

	// The document
	CWorldEditorDoc *doc = getDocument ();

	switch (pnmh->code)
	{
	case TVN_SELCHANGED:
		{
			// Select the good item
			LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) lParam;

			// By keyboard ?

			if ( pnmtv->action == TVC_BYKEYBOARD )// || 
			    // ( ( GetAsyncKeyState( VK_L)) && !( GetAsyncKeyState( VK_MENU )  & 0x8000 ) ) )
			{
				// we save the current properties dialog
				CDialogProperties *pDlg = NULL;

				list<CDialogProperties*>::iterator it = PropertiesDialogs.begin();
				bool ok = false;

				while ( !ok && ( it != PropertiesDialogs.end() ) )
				{
					if ( (*it)->equalsSelection( Selection ) )
					{
						pDlg = (*it);
						ok = true;
					}

					it++;
				}

				// Remove lock mode
				getMainFrame ()->setSelectionLocked (false);

				// Get the item data
				CPrimitiveTreeData *data = (CPrimitiveTreeData*)_Tree->GetItemData (pnmtv->itemNew.hItem);
				CDatabaseLocatorPointer locator;
				doc->getLocator (locator, data->Primitive);

				// Begin
				doc->beginModification ();

				// Unselect
				doc->addModification (new CActionUnselectAll ());

				// Something to select ?
				if (data)
				{
					// Select
					doc->addModification (new CActionSelect (locator));
				}

				// End
				doc->endModification ();

				// Update data
				getMainFrame ()->updateData ();

				if ( pDlg )
					pDlg->changeSelection (Selection);

				// New mark
				_Tree->SelectMark = pnmtv->itemNew.hItem;


				SetFocus();
			}
		}
		break;
	case TVN_ITEMEXPANDED:
		{
			// Take the item
			LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) lParam;

			// Get the item data
			CPrimitiveTreeData *data = (CPrimitiveTreeData*)_Tree->GetItemData (pnmtv->itemNew.hItem);

			// Don't make an action (don't modify the document) when expand / collapse the list..

			// Unselect

			getPrimitiveEditor(data->Primitive)->setExpanded((pnmtv->action&TVE_EXPAND)!=0);
//			const_cast<IPrimitive*> (data->Primitive)->Expanded = (pnmtv->action&TVE_EXPAND)!=0;
		}
		break;
	case TVN_KEYDOWN:
		{
			// Get selected item
			HTREEITEM item = _Tree->GetSelectedItem ();
			if (item)
			{
				// Take the item
				LPNMTVKEYDOWN pnmtv = (LPNMTVKEYDOWN) lParam;

				// Key down  + CTRL ?
				if ( (pnmtv->wVKey == VK_UP) && (GetAsyncKeyState (VK_CONTROL) & 0x8000))
				{
					// Get the item data
					CPrimitiveTreeData *data = (CPrimitiveTreeData*)_Tree->GetItemData (item);
					CDatabaseLocatorPointer locator;
					CDatabaseLocatorPointer root;
					doc->getLocator (locator, data->Primitive);
					root.getRoot (locator.getDatabaseIndex());

					// Get the previous locator
					if (!(root == locator))
					{
						// Get the previous locator
						CDatabaseLocatorPointer previousLocator = locator;
						if (previousLocator.previousChild ())
						{
							// Begin
							doc->beginModification ();

							// Copy the primitive
							const IPrimitive *copy = data->Primitive->copy ();

							// Remove the primitive
							doc->addModification (new CActionDelete (locator));

							// Add the primitive at the previous position
							doc->addModification (new CActionAddPrimitive (*copy, previousLocator));
							doc->addModification (new CActionSelect (previousLocator));
							delete copy;



							// End
							doc->endModification ();

							// Update data
							getMainFrame ()->updateData ();

							// Get selected item
							doc->getLocator (previousLocator, locator);
							nlverify (previousLocator.previousChild ());
							IPrimitiveEditor *primitiveEditor = dynamic_cast<IPrimitiveEditor*> (const_cast<IPrimitive*> (previousLocator.Primitive));
							_Tree->SelectItem (primitiveEditor->_TreeItem);
						}
					}
					else
					{
						// Not the top primitive
						uint index = locator.getDatabaseIndex ();
						if (index > 0)
						{
							// Begin
							doc->beginModification ();

							// Exchange the primitives
							doc->addModification (new CActionXChgDatabaseElement (index-1));

							// End
							doc->endModification ();

							// Update data
							getMainFrame ()->updateData ();

							selectItem (*_Tree, index-1);
						}
					}
				}
				// Key up + CTRL ?
				else if ( (pnmtv->wVKey == VK_DOWN) && (GetAsyncKeyState (VK_CONTROL) & 0x8000))
				{
					// Get the item data
					CPrimitiveTreeData *data = (CPrimitiveTreeData*)_Tree->GetItemData (item);
					CDatabaseLocatorPointer locator;
					CDatabaseLocatorPointer root;
					doc->getLocator (locator, data->Primitive);
					root.getRoot (locator.getDatabaseIndex());

					// Get the previous locator
					if (!(root == locator))
					{
						CDatabaseLocatorPointer nextLocator = locator;
						if (nextLocator.nextChild ())
						{
							// Begin
							doc->beginModification ();

							// Copy the primitive
							const IPrimitive *copy = data->Primitive->copy ();

							// Remove the primitive
							doc->addModification (new CActionDelete (locator));

							// Add the primitive at the previous position
							doc->addModification (new CActionAddPrimitive (*copy, nextLocator));
							doc->addModification (new CActionSelect (nextLocator));
							delete copy;

							// End
							doc->endModification ();

							// Update data
							getMainFrame ()->updateData ();

							// Get selected item
							doc->getLocator (nextLocator, locator);
							nlverify (nextLocator.nextChild ());
							IPrimitiveEditor *primitiveEditor = dynamic_cast<IPrimitiveEditor*> (const_cast<IPrimitive*> (nextLocator.Primitive));
							_Tree->SelectItem (primitiveEditor->_TreeItem);
						}
					}
					else
					{
						// Not the top primitive
						uint index = locator.getDatabaseIndex ();
						if (index+1 < doc->getNumDatabaseElement ())
						{
							// Begin
							doc->beginModification ();

							// Exchange the primitives
							doc->addModification (new CActionXChgDatabaseElement (index));

							// End
							doc->endModification ();

							// Update data
							getMainFrame ()->updateData ();

							selectItem (*_Tree, index+1);
						}
					}
				}
			}
		}
		break;
 	}

	return CFormView::OnNotify( wParam, lParam, pResult );
}

// ***************************************************************************

HTREEITEM CToolsLogic::addPrimitive (HTREEITEM parentItem, HTREEITEM lastBrother, const CDatabaseLocatorPointer &locator)
{
	HTREEITEM treeItem = insertItemUTF8 (*_Tree, "", parentItem, lastBrother);

	// Get the primitives
	CWorldEditorDoc *doc = getDocument ();
	const NLLIGO::CPrimitives *primitives = &(doc->getDatabaseElements (locator.getDatabaseIndex ()));

	// Set the item data
	_Tree->SetItemData (treeItem, (DWORD)new CPrimitiveTreeData (primitives, locator.Primitive));

	// Expand parent ?
	if (parentItem != TVI_ROOT)
	{
		CPrimitiveTreeData *parentData = (CPrimitiveTreeData *)_Tree->GetItemData (parentItem);
		nlassert (parentData);
		_Tree->Expand (parentItem, getPrimitiveEditor(parentData->Primitive)->getExpanded()?TVE_EXPAND:TVE_COLLAPSE);
	}

	return treeItem;
}

// ***************************************************************************

void CToolsLogic::updatePrimitive (HTREEITEM item, const CDatabaseLocatorPointer &locator)
{
	if (locator.Primitive)
	{
		// Is selected ?
//		IProperty *prop;
		_Tree->SetItemState (item, getPrimitiveEditor(locator.Primitive)->getSelected()?TVIS_BOLD:0, TVIS_BOLD);
//		_Tree->SetItemState (item, locator.Primitive->getPropertyByName ("selected", prop)?TVIS_BOLD:0, TVIS_BOLD);

		// Root ?
		if (locator.Primitive->getParent ())
		{
			// Set the name
			string name;
			string className;
			locator.Primitive->getPropertyByName ("name", name);
			locator.Primitive->getPropertyByName ("class", className);
			const  CPrimitiveClass *pc = theApp.Config.getPrimitiveClass(className.c_str());
			if (pc && pc->Type == CPrimitiveClass::Alias)
			{
				const CPrimAlias *pa = dynamic_cast<const CPrimAlias*>(locator.Primitive);
				nlassert(pa != NULL);

				name += " "+toString(pa->getAlias());
			}
			setItemTextUTF8 (*_Tree, item, name.c_str ());
		}
		else
		{
			// Set the primitive name
			string name;
			getDocument ()->getPrimitiveDisplayName (name, locator.getDatabaseIndex ());
			setItemTextUTF8 (*_Tree, item, name.c_str ());
		}

		// Expand / collapse it
		_Tree->Expand (item, getPrimitiveEditor(locator.Primitive)->getExpanded()?TVE_EXPAND:TVE_COLLAPSE);

		// Hidden ?
//		string temp;
		bool hidden = getPrimitiveEditor(locator.Primitive)->getHidden();
//		bool hidden = locator.Primitive->getPropertyByName ("hidden", temp);

		// Class of primitive ?
		bool imagesSets = false;
		string className;

		if (locator.Primitive->getPropertyByName ("class", className))
		{
			// Look for open / close icones..
			int _hidden = theApp.ImageList.getImage ((className+"_hidden").c_str ());
			int opened = theApp.ImageList.getImage ((className+"_opened").c_str ());
			int closed = theApp.ImageList.getImage ((className+"_closed").c_str ());
			if ((hidden && (_hidden != -1)) || ((opened != -1) && (closed != -1)))
			{
				// Set the images
				_Tree->SetItemImage (item, hidden?_hidden:closed, hidden?_hidden:opened);
				imagesSets = true;
			}
			else
			{
				// Look for open / close icones..
				int image = theApp.ImageList.getImage (className.c_str ());
				if (image != -1)
				{
					// Set the images
					_Tree->SetItemImage (item, image, image);
					imagesSets = true;
				}
			}
		}
		
		// Icon sets or error ?
		// Set the image, If there is an error, force to set an icon
		const IPrimitiveEditor *node = dynamic_cast<const IPrimitiveEditor *> (locator.Primitive);
		if (!imagesSets || (node && node->_Error != IPrimitiveEditor::NoError))
		{
			// Root ?
			if (locator.Primitive->getParent () == NULL)
			{
				if (node && node->_Error != IPrimitiveEditor::NoError)
				{
					_Tree->SetItemImage (item, theApp.ImageList.getImage (IDI_ERROR_STRUCTURE), theApp.ImageList.getImage (IDI_ERROR_STRUCTURE));
				}
				else
				{
					_Tree->SetItemImage (item, theApp.ImageList.getImage (hidden?IDI_ROOT_HIDDEN:IDI_ROOT_CLOSED), theApp.ImageList.getImage (hidden?IDI_ROOT_HIDDEN:IDI_ROOT_OPENED));
				}
			}
			else
			{
				// What kind of class ?
				if (node)
				{
					if (node->_Error != IPrimitiveEditor::NoError)
					{
						_Tree->SetItemImage (item, theApp.ImageList.getImage (IDI_ERROR_STRUCTURE), theApp.ImageList.getImage (IDI_ERROR_STRUCTURE));
					}
					else
					{
						// Some children ?
						if (locator.Primitive->getNumChildren ())
							_Tree->SetItemImage (item, theApp.ImageList.getImage (hidden?IDI_FOLDER_HIDDEN:IDI_FOLDER_CLOSED), theApp.ImageList.getImage (hidden?IDI_FOLDER_HIDDEN:IDI_FOLDER_OPENED));
						else
							_Tree->SetItemImage (item, theApp.ImageList.getImage (hidden?IDI_PROPERTY_HIDDEN:IDI_PROPERTY_CLOSED), theApp.ImageList.getImage (hidden?IDI_PROPERTY_HIDDEN:IDI_PROPERTY_OPENED));
					}
				}
				else
				{
					const CPrimPointEditor *point = dynamic_cast<const CPrimPointEditor *> (locator.Primitive);
					if (point)
						_Tree->SetItemImage (item, theApp.ImageList.getImage (hidden?IDI_POINT_HIDDEN:IDI_POINT_CLOSED), theApp.ImageList.getImage (hidden?IDI_POINT_HIDDEN:IDI_POINT_OPENED));
					else
					{
						const CPrimPathEditor *path = dynamic_cast<const CPrimPathEditor *> (locator.Primitive);
						if (path)
							_Tree->SetItemImage (item, theApp.ImageList.getImage (hidden?IDI_LINE_HIDDEN:IDI_LINE_CLOSED), theApp.ImageList.getImage (hidden?IDI_LINE_HIDDEN:IDI_LINE_OPENED));
						else
						{
							const CPrimZoneEditor *node = dynamic_cast<const CPrimZoneEditor *> (locator.Primitive);
							if (node && node->_Error != IPrimitiveEditor::NoError)
							{
								_Tree->SetItemImage (item, theApp.ImageList.getImage (IDI_ERROR_STRUCTURE), theApp.ImageList.getImage (IDI_ERROR_STRUCTURE));
							}
							else
							{
								safe_cast<const CPrimZoneEditor *> (locator.Primitive);
								_Tree->SetItemImage (item, theApp.ImageList.getImage (hidden?IDI_ZONE_HIDDEN:IDI_ZONE_CLOSED), theApp.ImageList.getImage (hidden?IDI_ZONE_HIDDEN:IDI_ZONE_OPENED));
							}
						}
					}
				}
			}
		}
	}
	else
	{
		// Not bold
		_Tree->SetItemState (item, 0, TVIS_BOLD);
	}

	// Remove mark
	_Tree->SelectMark = NULL;
}

// ***************************************************************************

void CToolsLogic::ensureVisible (IPrimitiveEditor *primitive)
{
	if (primitive->_TreeItem)
		_Tree->EnsureVisible (primitive->_TreeItem);
}

// ***************************************************************************

void CToolsLogicTree::OnRenameSelected() 
{
	// todo rebranch rename selection
/*	// Not empty ?
	if (!Selection.empty ())
	{
		bool modified = false;
		CWorldEditorDoc *doc = getDocument ();

		// For each selected primitives
		std::list<NLLIGO::IPrimitive*>::iterator ite1 = Selection.begin ();
		while (ite1 != Selection.end ())
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, *ite1);
			
			// Primitive ?
			const IPrimitive *primitive = locator.Primitive;
			if (primitive)
			{
				// Get the class name
				const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*primitive);
				// Does the class uses Autoname ?
				if (primClass && primClass->Autoname != "")
				{
					// do we have a "name" field
					const IProperty *propName;
					if (primitive->getPropertyByName ("name", propName))
					{
						// we collect all properties of the class in a parameter list
						set<CPrimitiveClass::CParameter>	parameterList;
						for (uint p=0; p<primClass->Parameters.size (); p++)
						{
							// Is the parameter visible ?
							if (primClass->Parameters[p].Visible)
							{
								// we avoid the reserved keywords
								if (primClass->Parameters[p].Name != "name" &&
									primClass->Parameters[p].Name != "hidden" &&
									primClass->Parameters[p].Name != "selected" &&
									primClass->Parameters[p].Name != "class" &&
									primClass->Parameters[p].Name != "")
								{
									parameterList.insert (primClass->Parameters[p]);
								}
							}
						}
						// we rename the string in one pass
						CString Name = primClass->Autoname.c_str();
						
						set<CPrimitiveClass::CParameter>::iterator ite2 = parameterList.begin ();
						while (ite2 != parameterList.end ())
						{
							string result;
							CString OriginalName = ite2->Name.c_str();
							if (ite2->Type == CPrimitiveClass::CParameter::String ||
								ite2->Type == CPrimitiveClass::CParameter::ConstString)
							{
								primitive->getPropertyByName (OriginalName, result);
							}
							else if (ite2->Type == CPrimitiveClass::CParameter::StringArray)
							{
								const IProperty *propName;
								if (primitive->getPropertyByName (OriginalName, propName))
								{
									const CPropertyStringArray *propStringString = dynamic_cast<const CPropertyStringArray *> (propName);
									result = propStringString->StringArray[0];
								}
								else
								{
									result = "undefined";
								}
							}
							
							OriginalName = "$"+OriginalName+"$";
							Name.Replace(OriginalName, result.c_str());
							++ite2;
						}
						
						// now, we overwrite the original name
						// Create a default property
						string value;
						value = Name;
						if (!modified)
						{
							doc->beginModification ();
							modified = true;
						}
						doc->addModification (new CActionSetPrimitivePropertyString (locator, "name", value.c_str(), false));
					}
					
				}
			}
			ite1++;
		}
		if (modified)
		{
			doc->endModification ();
			getMainFrame ()->updateData ();
		}
	}*/
}

// ***************************************************************************

void CToolsLogicTree::OnRepairSelected() 
{
	// Not empty ?
	if (!Selection.empty ())
	{
		bool modified = false;
		CWorldEditorDoc *doc = getDocument ();
		
		// For each selected primitives
		std::list<NLLIGO::IPrimitive*>::iterator ite1 = Selection.begin ();
		while (ite1 != Selection.end ())
		{
			CDatabaseLocatorPointer locator;
			doc->getLocator (locator, *ite1);
			
			const IPrimitive *primitive = locator.Primitive;
			if (primitive)
			{
				string className;
				bool Success;
				Success = primitive->getPropertyByName ("class", className);
				
				if (!Success)
				{
					// is this the root node ?
					const IPrimitive *parent = primitive->getParent();
					if (!parent)
					{
						// if no parent, we have a root node
						className = "root";
						Success = true;
					}
				}
				if (Success)
				{
					const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (className.c_str ());
					if (primClass)
					{
						uint CountStaticChildren = 0;
						CDatabaseLocatorPointer myLocator;

						// FIRST PASS:
						// we check if the Children belong to the original class
						// if it's not the case, we remove the unknown Children
restart:
						myLocator = locator;
						if (myLocator.firstChild ())
						{
							do
							{
								const IPrimitive *primitive2 = myLocator.Primitive;
								if (primitive2)
								{
									string ClassName2;
									nlverify (primitive2->getPropertyByName ("class", ClassName2));
									uint c;
									bool Found = false;
									for (c=0; c<primClass->StaticChildren.size (); c++)
									{
										if (primClass->StaticChildren[c].ClassName == ClassName2)
										{
											Found = true;
											++CountStaticChildren;
											break;
										}
									}
									
									for (c=0; c<primClass->GeneratedChildren.size (); c++)
									{
										if (primClass->GeneratedChildren[c].ClassName == ClassName2)
										{
											Found = true;
											break;
										}
									}
									
									for (c=0; c<primClass->DynamicChildren.size (); c++)
									{
										if (primClass->DynamicChildren[c].ClassName == ClassName2)
										{
											Found = true;
											break;
										}
									}
									if (!Found)
									{
										// a useless child is here, we need to delete it
										// Begin modification
										if (!modified)
										{
											doc->beginModification ();
											modified = true;
										}
										doc->deletePrimitive (myLocator);
										doc->modifyDatabase (myLocator.getDatabaseIndex ());
										// since we deleted the child, we cannot point to the next one, so we restart the loop
										// I don't think it's possible to use something better than this goto !
										goto restart;
									}
								}
							}
							while (myLocator.nextChild ());
						}
						if (CountStaticChildren != primClass->StaticChildren.size())
						{
							// SECOND PASS:
							// if the number of static children of the class is different from the number of static children of the original class
							// we have to add the missing class
							uint child;
							for (child=0; child<primClass->StaticChildren.size (); ++child)
							{
								myLocator = locator;
								bool Found = false;
								// we parse the tree recursively, collection errors, and propagating to the parent
								if (myLocator.firstChild ())
								{
									do
									{
										const IPrimitive *primitive2 = myLocator.Primitive;
										if (primitive2)
										{
											string ClassName2;
											string name;
											nlverify (primitive2->getPropertyByName ("class", ClassName2));
											if (primitive2->getPropertyByName ("name", name))
											{
												if (primClass->StaticChildren[child].ClassName == ClassName2 && 
													primClass->StaticChildren[child].Name == name)
												{
													Found = true;
													break;
												}
											}
										}
									}
									while (myLocator.nextChild ());
								}
								if (!Found)
								{
									// a child is missing, we need to create it
									// Begin modification
									if (!modified)
									{
										doc->beginModification ();
										modified = true;
									}
									// What class is it ?
									// Get the child class
									CDatabaseLocatorPointer locator2 = locator;
									locator2.firstChild();
									CDatabaseLocator dest = locator2;
									
									// Position on the Display
									CDisplay *display = getDisplay ();
									float delta = (float)DELTA_POS_ADD_PRIMITIVE * (display->_CurViewMax.x - display->_CurViewMin.x) / (float)display->getWidth ();
									if (doc->addModification (new CActionAddPrimitiveByClass (dest, primClass->StaticChildren[child].ClassName.c_str (), getDisplay ()->_CurPos, 
										delta, primClass->StaticChildren[child].Parameters)))
									{
										CDatabaseLocatorPointer dest2;
										doc->getLocator (dest2, dest);

										// Get a pointer on the new primitive
										doc->addModification (new CActionSetPrimitivePropertyString (dest2, "name", primClass->StaticChildren[child].Name.c_str(), false));
									}
								}
							}
						}
					}
				}
			}	
			ite1++;
		}
		if (modified)
		{
			doc->endModification ();
			// verify the structures, to get their status
			VerifyPrimitivesStructures();
			InvalidateAllPrimitives ();
			getMainFrame ()->updateData ();
		}
	}
}

// ***************************************************************************

void CToolsLogicTree::OnTreeHelp()
{
	getMainFrame ()->OnHelpFinder();
}

// ***************************************************************************

void CToolsLogicTree::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if ( GetAsyncKeyState( VK_MENU ) & 0x8000 )
	{
		NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
		HTREEITEM    hItem       = pNMTreeView->itemNew.hItem;
		if(!hItem) return;
		SelectItem(hItem);

		CWorldEditorDoc *doc = getDocument ();
		CDatabaseLocatorPointer locator;
		doc->getLocator ( locator, Selection.front() );

		if ( theApp.Config.isPrimitiveDeletable ( *( locator.Primitive ) ) )
		{
			m_hDragItem    = hItem;
			m_pDragImgList = CreateDragImage(hItem);
			if(!m_pDragImgList) return;

			m_pDragImgList->BeginDrag( 0, CPoint(0,0) );
			m_pDragImgList->DragEnter( this, pNMTreeView->ptDrag );
			m_boDragging = true;

			ShowCursor(false);
			SetCapture();
			m_hDragTarget = NULL;
			getMainFrame()->OnEditCut();
		}

	//	m_nTimer = SetTimer(1,25,NULL);
		*pResult = 0;
	}
}

// ***************************************************************************

void CToolsLogicTree::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_boDragging)
	{
		// highlight target

		TVHITTESTINFO tvHit;
		tvHit.pt = point;
		HTREEITEM hTarget = HitTest(&tvHit);

		if ( hTarget )
		{
			if(hTarget != m_hDragTarget)
			{                                                     
				// this test avoids flickering
				m_pDragImgList->DragShowNolock(false);
				SelectDropTarget(hTarget);
				m_pDragImgList->DragShowNolock(true);
				m_hDragTarget = hTarget;
			}
		}

		// move image being dragged

		m_pDragImgList->DragMove(point);
	}

	CTreeCtrl::OnMouseMove( nFlags, point );
}

// ***************************************************************************

void CToolsLogicTree::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ( m_boDragging )
	{
		KillTimer(1);

		ReleaseCapture();
		ShowCursor( true );
		m_boDragging = false;

		m_pDragImgList->DragLeave( this );
		m_pDragImgList->EndDrag();
		delete m_pDragImgList;
		m_pDragImgList = NULL;

		TVHITTESTINFO tvHit;
		tvHit.pt = point;
		HTREEITEM hTarget = HitTest(&tvHit);
			
		OnLButtonDown( nFlags, point );
		
		// Can be dropped ?
		if ( GetDropHilightItem() && getMainFrame()->CanDrop() )
		{		
			getMainFrame()->OnEditPaste();
		}
		else
		{
			// un-select
			getMainFrame()->onMenuModeUndo();
			// un-cut
			getMainFrame()->onMenuModeUndo();
		}

		OnLButtonDown( nFlags, point );
		SelectDropTarget( NULL );
	}   

	CTreeCtrl::OnLButtonUp(nFlags, point);
}


