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

// MasterTree.cpp : implementation file
//

#include "stdafx.h"
#include "master.h"
#include "MasterTree.h"
#include "Mainfrm.h"
#include "nel/misc/types_nl.h"
#ifdef NL_NEW
	#undef new
#endif
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ---------------------------------------------------------------------------

#define IDC_TREE					0x1000

// Top level menus
#define ID_MENU_CONTINENT_NEW			0x0044

#define ID_MENU_SORT_NAME_INC			0x0047
#define ID_MENU_SORT_NAME_DEC			0x0048
#define ID_MENU_SORT_DATE_INC			0x0049
#define ID_MENU_SORT_DATE_DEC			0x0050

// Continent menus (a level under the top)

#define ID_MENU_CONTINENT_DELETE		0x0064
#define ID_MENU_CONTINENT_OPEN			0x0066
#define ID_MENU_CONTINENT_PROPERTIES	0x0067
#define ID_MENU_CONTINENT_NEW_REGION	0x0068

#define ID_MENU_REGION_OPEN				0x0070
#define ID_MENU_REGION_RENAME			0x0071
#define ID_MENU_REGION_NEW_SUBREGION	0x0072
#define ID_MENU_REGION_NEW_PRIM			0x0073
#define ID_MENU_REGION_NEW_GEORGES		0x0074
#define ID_MENU_REGION_DELETE			0x0075

#define	ID_MENU_FILE_OPEN				0x0080
#define	ID_MENU_FILE_RENAME				0x0081
#define	ID_MENU_FILE_DELETE				0x0082

// ---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CMasterTreeDlg dialog
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP (CMasterTree, CTreeCtrl)

 	ON_WM_TIMER()

	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnLBeginDrag)

	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()

	ON_COMMAND(ID_MENU_CONTINENT_NEW,			OnMenuNewContinent)

	ON_COMMAND(ID_MENU_CONTINENT_DELETE,		OnMenuContinentDelete)
	ON_COMMAND(ID_MENU_CONTINENT_OPEN,			OnMenuContinentOpen)
	ON_COMMAND(ID_MENU_CONTINENT_PROPERTIES,	OnMenuContinentProperties)
	ON_COMMAND(ID_MENU_CONTINENT_NEW_REGION,	OnMenuContinentNewRegion)

	ON_COMMAND(ID_MENU_REGION_OPEN,				OnMenuRegionOpen)
	ON_COMMAND(ID_MENU_REGION_RENAME,			OnMenuRegionRename)
	ON_COMMAND(ID_MENU_REGION_NEW_PRIM,			OnMenuRegionNewPrim)
	ON_COMMAND(ID_MENU_REGION_NEW_GEORGES,		OnMenuRegionNewGeorges)
	ON_COMMAND(ID_MENU_REGION_DELETE,			OnMenuRegionDelete)
	ON_COMMAND(ID_MENU_REGION_NEW_SUBREGION,	OnMenuRegionNewSubRegion)

	ON_COMMAND(ID_MENU_FILE_OPEN,				OnMenuFileOpen)
	ON_COMMAND(ID_MENU_FILE_RENAME,				OnMenuFileRename)
	ON_COMMAND(ID_MENU_FILE_DELETE,				OnMenuFileDelete)
	
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
CMasterTree::CMasterTree ()
{
	_LDrag = false;
	_LastItemSelected = NULL;
	_LastActiveContinent = NULL;
}

// ---------------------------------------------------------------------------
void CMasterTree::OnTimer (UINT nIDEvent)
{
	if( nIDEvent != m_nTimerID )
	{
		CTreeCtrl::OnTimer(nIDEvent);
		return;
	}

	// Doesn't matter that we didn't initialize m_timerticks
	m_timerticks++;

	POINT pt;
	GetCursorPos (&pt);
	RECT rect;
	GetClientRect (&rect);
	ClientToScreen (&rect);

	if( pt.y < rect.top + 10 )
	{
		CImageList::DragShowNolock (FALSE);
		SendMessage (WM_VSCROLL, SB_LINEUP);
		CImageList::DragShowNolock (TRUE);
	} 
	else if( pt.y > rect.bottom - 10 )
	{
		CImageList::DragShowNolock (FALSE);
		SendMessage (WM_VSCROLL, SB_LINEDOWN);
		CImageList::DragShowNolock (TRUE);
	}
}

// ---------------------------------------------------------------------------
void CMasterTree::OnLBeginDrag (NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTREEVIEW *pNMTV = (NMTREEVIEW*)pNMHDR;
	_DragItem = pNMTV->itemNew.hItem;
	HTREEITEM hParent = GetParentItem (_DragItem);
	if (hParent == NULL)
	{
		*pResult = true;
		return;
	}
	_LDrag = true;
	m_nTimerID = SetTimer (1, 50, NULL);
	_DragImg = CreateDragImage (_DragItem);
	_DragImg->BeginDrag (0, CPoint (8, 8));
	_DragImg->DragEnter (this, ((NM_TREEVIEW *)pNMHDR)->ptDrag);
	SetCapture ();
	Invalidate ();
	*pResult = false;
}

// ---------------------------------------------------------------------------
void CMasterTree::OnLButtonDown (UINT nFlags, CPoint point)
{
	/*HTREEITEM NewItem = HitTest (point);

	if (NewItem == NULL)
		return;
	*/
	CTreeCtrl::OnLButtonDown (nFlags, point);
/*
	HTREEITEM LastContinent, LastRegion;
	HTREEITEM NewContinent, NewRegion;
	LastContinent = getContinent (_LastItemSelected);
	LastRegion = getRegion (_LastItemSelected);

	NewContinent = getContinent (NewItem);
	NewRegion = getRegion (NewItem);

	if (LastRegion != NewRegion)
	{
		Expand (LastRegion, TVE_COLLAPSE);
		Expand (NewRegion, TVE_EXPAND);
	}

	if (LastContinent != NewContinent)
	{
		Expand (LastContinent, TVE_COLLAPSE);
		Expand (NewContinent, TVE_EXPAND);
	}

	_LastItemSelected = NewItem;
	Expand (NewItem, TVE_EXPAND);
	Select (NewItem, TVGN_CARET);*/
}
// ---------------------------------------------------------------------------
void CMasterTree::OnLButtonDblClk (UINT nFlags, CPoint point)
{
	CTreeCtrl::OnLButtonDblClk (nFlags, point);

	HTREEITEM hItem = HitTest (point);
	
	if (hItem != NULL)
	{
		CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();

		if (isFile(hItem))
		{
			pDlg->openFile (getPath(hItem));
		}
	}
}

// ---------------------------------------------------------------------------
void CMasterTree::OnLButtonUp (UINT nFlags, CPoint point)
{
	if (_LDrag)
	{
		SelectDropTarget (NULL);
		_DragImg->DragLeave (this);
		_DragImg->EndDrag ();
		ReleaseCapture ();
		KillTimer (m_nTimerID);
		_LDrag = false;
		delete _DragImg;
		Invalidate ();

		HTREEITEM dragEndItem = HitTest (point);
		if ((_DragItem == NULL) || (dragEndItem == NULL))
			return;

		if (isFile(dragEndItem))
			return;

		Select (dragEndItem, TVGN_CARET);
		
		CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
		pDlg->copy (getPath(_DragItem), getPath(dragEndItem));
	}
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMouseMove (UINT nFlags, CPoint point)
{
	if (_LDrag)
	{
		_DragImg->DragMove (point);
		_DragImg->DragShowNolock (FALSE);

		UINT Flags;
		HTREEITEM hItem = HitTest (point, &Flags);
		HTREEITEM hParent = GetParentItem (hItem) ;

		if ((_DragItem == NULL) || (hItem == NULL))
			return;

		if (!isFile(hItem))
			SelectDropTarget (hItem);
		
		_DragImg->DragShowNolock (TRUE);			
	}
}

// ---------------------------------------------------------------------------
void CMasterTree::OnRButtonDown (UINT nFlags, CPoint point)
{
	UINT uFlags;
	HTREEITEM hItem = HitTest (point, &uFlags);
	HTREEITEM hParent = GetParentItem (hItem);
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();

	CRect r;
	this->GetWindowRect (r);
	
	CMenu *pMenu = new CMenu;
	pMenu->CreatePopupMenu ();

	if (hItem == NULL)
	{
		pMenu->AppendMenu (MF_STRING, ID_MENU_CONTINENT_NEW, "&New Continent");
		pMenu->AppendMenu (MF_SEPARATOR);
		pMenu->AppendMenu (MF_STRING, ID_MENU_SORT_NAME_DEC, "Sort By Name (A-Z)");
		pMenu->AppendMenu (MF_STRING, ID_MENU_SORT_NAME_INC, "Sort By Name (Z-A)");
		pMenu->AppendMenu (MF_STRING, ID_MENU_SORT_DATE_INC, "Sort By Date (New-Old)");
		pMenu->AppendMenu (MF_STRING, ID_MENU_SORT_DATE_DEC, "Sort By Date (Old-New)");
	}
	else if (TVHT_ONITEM & uFlags)
	{
		Select (hItem, TVGN_CARET);

		// System roots == continents
		if (hParent == NULL)
		{
			pMenu->AppendMenu (MF_STRING, ID_MENU_CONTINENT_OPEN, "&Open");
			pMenu->AppendMenu (MF_STRING, ID_MENU_CONTINENT_PROPERTIES, "&Properties");
			pMenu->AppendMenu (MF_STRING, ID_MENU_CONTINENT_NEW_REGION, "New &Region");
			pMenu->AppendMenu (MF_STRING, ID_MENU_CONTINENT_DELETE, "&Delete");
		}
		// Under the continents == regions and sub-regions
		else
		{
			if (!isFile (hItem))
			{
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_OPEN, "&Open");
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_RENAME, "&Rename");
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_NEW_SUBREGION, "New &SubRegion");
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_NEW_GEORGES, "New &Form");
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_NEW_PRIM, "New &Patatoid");
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_DELETE, "&Delete");
			}
			else
			{
				pMenu->AppendMenu (MF_STRING, ID_MENU_FILE_OPEN, "&Open");
				pMenu->AppendMenu (MF_STRING, ID_MENU_FILE_RENAME, "&Rename");
				pMenu->AppendMenu (MF_STRING, ID_MENU_FILE_DELETE, "&Delete");
			}
		}
	}
	pMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON, 
							r.left+point.x, r.top+point.y, this);
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuNewContinent ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->continentNew ();
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuContinentDelete ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->continentDelete ((LPCSTR)GetItemText(GetSelectedItem()));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuContinentOpen ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent ();
	HTREEITEM hItem = GetSelectedItem ();
	HTREEITEM hParent = GetParentItem (hItem);

	if (_LastActiveContinent != NULL)
		SetItem (_LastActiveContinent, TVIF_STATE, NULL, 0, 0, 0, TVIS_BOLD, 0);
	SetItem (hItem, TVIF_STATE, NULL, 0, 0, TVIS_BOLD, TVIS_BOLD, 0);
	_LastActiveContinent = hItem;
	pDlg->continentOpen (string((LPCSTR)GetItemText(hItem)));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuContinentProperties ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->continentProperties ((LPCSTR)GetItemText(GetSelectedItem()));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuContinentNewRegion ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->continentNewRegion ((LPCSTR)GetItemText(GetSelectedItem()));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionOpen ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	HTREEITEM ht = GetSelectedItem();
	pDlg->regionOpen (getPath(ht));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionRename ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	HTREEITEM ht = GetSelectedItem();
	pDlg->regionRename (getPath(ht));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionNewSubRegion ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	HTREEITEM ht = GetSelectedItem();
	pDlg->regionNewSubRegion (getPath(ht));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionNewPrim ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	HTREEITEM ht = GetSelectedItem();
	pDlg->regionNewPrim (getPath(ht));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionNewGeorges ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	HTREEITEM ht = GetSelectedItem();
	pDlg->regionNewGeorges (getPath(ht));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionDelete ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	HTREEITEM ht = GetSelectedItem();
	pDlg->regionDelete (getPath(ht));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuFileOpen ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	HTREEITEM hItem = GetSelectedItem();	
	bool bFile = isFile (hItem);
	if (!bFile) return;
	pDlg->fileOpen (getPath(hItem));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuFileRename ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	HTREEITEM hItem = GetSelectedItem();	
	bool bFile = isFile (hItem);
	if (!bFile) return;
	pDlg->fileRename (getPath(hItem));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuFileDelete ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	HTREEITEM hItem = GetSelectedItem();
	bool bFile = isFile (hItem);
	if (!bFile) return;
	pDlg->fileDelete (getPath(hItem));
}

// ---------------------------------------------------------------------------
string CMasterTree::getCurrentPath ()
{
	return getPath(GetSelectedItem());
}

// TOOLS

// ---------------------------------------------------------------------------
string CMasterTree::getPath (HTREEITEM ht)
{
	string sTmp = "";

	while (ht != NULL)
	{
		sTmp = string("\\") + (LPCSTR)GetItemText(ht) + sTmp;
		ht = GetParentItem(ht);
	}
	
	return sTmp;
}

// ---------------------------------------------------------------------------
HTREEITEM CMasterTree::getContinent (HTREEITEM ht)
{
	if (ht == NULL) return NULL;
	if (GetParentItem(ht) == NULL) return NULL;
	while (GetParentItem(GetParentItem(ht)) != NULL) 
		ht = GetParentItem(ht);
	if (isFile(ht)) return NULL;
	return ht;
}

// ---------------------------------------------------------------------------
HTREEITEM CMasterTree::getRegion (HTREEITEM ht)
{
	if (ht == NULL) return NULL;
	if (GetParentItem(ht) == NULL) return NULL;
	if (GetParentItem(GetParentItem(ht)) == NULL) return NULL;
	while (GetParentItem(GetParentItem(GetParentItem(ht))) != NULL) 
		ht = GetParentItem(ht);
	if (isFile(ht)) return NULL;
	return ht;
}

// ---------------------------------------------------------------------------
bool CMasterTree::isFile (HTREEITEM ht)
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	map<HTREEITEM,string>::iterator it = pDlg->_Files.find (ht);
	if (it != pDlg->_Files.end())
		return true;
	else
		return false;
}


/////////////////////////////////////////////////////////////////////////////
// CMasterTreeDlg dialog
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMasterTreeDlg, CDialog)
	//{{AFX_MSG_MAP(CMasterTreeDlg)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE, OnExpanding)
	//ON_NOTIFY(NM_DBLCLK, IDC_TREE, OnLDblClkTree)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



// ---------------------------------------------------------------------------
CMasterTreeDlg::CMasterTreeDlg ()
{
	_Tree = NULL;
	ContinentSortBy = 1;	// 0/1 - by name
	TrashSortBy = 1;		// 2/3 - by date
	BackupSortBy = 2;
}

// ---------------------------------------------------------------------------
FILETIME getFileTime (const string &fname)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	FILETIME ret = { 0xFFFFFFFF, 0xFFFFFFFF };

	hFind = FindFirstFile (fname.c_str(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE) 
	{
		ret = FindFileData.ftLastWriteTime;
		FindClose (hFind);
	}
	return ret;
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::parseAdd (HTREEITEM itRoot, const string &path, char SortType, int DirDepth)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_TREE);
	vector<string> SortTable;
	sint32 i, j;

	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	if (!SetCurrentDirectory (path.c_str()))
	{
		SetCurrentDirectory (sCurDir);
		return;
	}
	
	hFind = FindFirstFile ("*.*", &findData);	
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// Look if the name is a system directory
			bool bFound = false;
			for (uint32 i = 0; i < MAX_SYS_DIR; ++i)
				if (stricmp (findData.cFileName, gSysDir[i]) == 0)
				{
					bFound = true;
					break;
				}
			if (!bFound) // No ok lets add it
			{
				SortTable.push_back (findData.cFileName);
			}
		}
		while (FindNextFile(hFind, &findData) != 0);
		FindClose (hFind);
	}

	if (SortTable.size() == 0)
	{
		SetCurrentDirectory (sCurDir);
		return;
	}

	if (SortType == 0) // Sort By Name increasing (z-a)
	{
		for (i = 0; i < (sint32)SortTable.size()-1; ++i)
		for (j = i+1; j < (sint32)SortTable.size(); ++j)
			if (strcmp(SortTable[i].c_str(), SortTable[j].c_str()) < 0)
			{
				string tmp = SortTable[i];
				SortTable[i] = SortTable[j];
				SortTable[j] = tmp;
			}
	}
	if (SortType == 1) // Sort By Name decreasing (a-z)
	{
		for (i = 0; i < (sint32)SortTable.size()-1; ++i)
		for (j = i+1; j < (sint32)SortTable.size(); ++j)
			if (strcmp(SortTable[i].c_str(), SortTable[j].c_str()) > 0)
			{
				string tmp = SortTable[i];
				SortTable[i] = SortTable[j];
				SortTable[j] = tmp;
			}
	}
	if (SortType == 2) // Sort By Date increasing
	{
		for (i = 0; i < (sint32)SortTable.size()-1; ++i)
		{
			FILETIME timeI = getFileTime (SortTable[i]);
			for (j = i+1; j < (sint32)SortTable.size(); ++j)
			{
				FILETIME timeJ = getFileTime (SortTable[j]);
				if ((timeI.dwHighDateTime < timeJ.dwHighDateTime) ||
					(
						(timeI.dwHighDateTime == timeJ.dwHighDateTime) &&
						(timeI.dwLowDateTime < timeJ.dwLowDateTime)
					))
				{
					string tmp = SortTable[i];
					SortTable[i] = SortTable[j];
					SortTable[j] = tmp;
					timeI = timeJ;
				}
			}
		}
	}

	if (SortType == 3) // Sort By Date decreasing
	{
		for (i = 0; i < (sint32)SortTable.size()-1; ++i)
		{
			FILETIME timeI = getFileTime (SortTable[i]);
			for (j = i+1; j < (sint32)SortTable.size(); ++j)
			{
				FILETIME timeJ = getFileTime (SortTable[j]);
				if ((timeI.dwHighDateTime > timeJ.dwHighDateTime) ||
					(
						(timeI.dwHighDateTime == timeJ.dwHighDateTime) &&
						(timeI.dwLowDateTime > timeJ.dwLowDateTime)
					))
				{
					string tmp = SortTable[i];
					SortTable[i] = SortTable[j];
					SortTable[j] = tmp;
					timeI = timeJ;
				}
			}
		}
	}

	// Put directories first !
	vector<string> SortTable2;
	for (i = 0; i < (sint32)SortTable.size(); ++i)
	{
		if (GetFileAttributes(SortTable[i].c_str())&FILE_ATTRIBUTE_DIRECTORY)
			SortTable2.push_back(SortTable[i]);
	}
	for (i = 0; i < (sint32)SortTable.size(); ++i)
	{
		if (!(GetFileAttributes(SortTable[i].c_str())&FILE_ATTRIBUTE_DIRECTORY))
			SortTable2.push_back(SortTable[i]);
	}
	SortTable = SortTable2;

	// Recurse
	for (i = 0; i < (sint32)SortTable.size(); ++i)
	{
		if (GetFileAttributes(SortTable[i].c_str())&FILE_ATTRIBUTE_DIRECTORY)
		{
			HTREEITEM item;

			if (DirDepth == 0)
				item = pTree->InsertItem (SortTable[i].c_str(), 2, 2, itRoot);
			else
				item = pTree->InsertItem (SortTable[i].c_str(), 0, 0, itRoot);
			string newPath = path;
			if (newPath[newPath.size()-1] != '\\')
				newPath += "\\";
			newPath += SortTable[i].c_str();
			parseAdd (item, newPath, SortType, DirDepth+1);
		}
		else
		{
			// Look if the name is a valid filename (look at the extension)
			bool bFound = false;
			for (uint32 j = 0; j < MAX_INVALID_EXT; ++j)
				if (strlen(SortTable[i].c_str()) > strlen(gInvalidExt[j]))
				if (stricmp(&SortTable[i].c_str()[strlen(SortTable[i].c_str())-strlen(gInvalidExt[j])], gInvalidExt[j]) == 0)
				{
					bFound = true;
					break;
				}
			// If the extension is an invalid one -> Do not display file
			if (!bFound)
			{
				HTREEITEM item = pTree->InsertItem (SortTable[i].c_str(), 1, 1, itRoot);
				string sTemp = path;
				if (sTemp[sTemp.size()-1] != '\\')
					sTemp += "\\";
				sTemp += SortTable[i].c_str();
				_Files.insert(map<HTREEITEM,string>::value_type(item, sTemp));
			}

		}
	}
	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
HTREEITEM CMasterTreeDlg::activate (const std::string &name, HTREEITEM parent)
{
	// Extract the eltName
	string eltName;
	string resName;
	uint32 i = 0;
	if (name.size() == 0) return NULL;
	if (name[i] == '\\') ++i;
	for (; i < name.size(); ++i)
	{
		if (name[i] == '\\') break;
		eltName += name[i];
	}
	++i;
	for (; i < name.size(); ++i)
	{
		resName += name[i];
	}

	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem (IDC_TREE);
	HTREEITEM hChildItem = pTree->GetChildItem (parent);
	bool bFound = false;
	while (hChildItem != NULL)
	{
		if (eltName == (LPCSTR)pTree->GetItemText(hChildItem))
		{
			pTree->SetItem (hChildItem, TVIF_STATE, NULL, 0, 0, TVIS_BOLD, TVIS_BOLD, 0);
			pTree->Expand (hChildItem, TVE_EXPAND);
			activate (resName, hChildItem);
			bFound = true;
			break;
		}

		hChildItem = pTree->GetNextItem (hChildItem, TVGN_NEXT);
	}
	if (bFound)
		return hChildItem;
	else
		return NULL;
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::update (const std::string& ContinentsPath)
{
	CMasterTree *pTree = (CMasterTree*)GetDlgItem (IDC_TREE);
	string sCurPath = pTree->getCurrentPath();
	_Files.clear();
	pTree->DeleteAllItems ();
	// Parse all the path
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	parseAdd (TVI_ROOT, ContinentsPath, ContinentSortBy);
	SetCurrentDirectory (sCurDir);
	// Hilight active Continent
	CMainFrame *pMF = (CMainFrame *)GetParent ();
	activate (sCurPath, TVI_ROOT);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange (pDX);
	//{{AFX_DATA_MAP(CMasterTreeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

// ---------------------------------------------------------------------------
BOOL CMasterTreeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_Tree = new CMasterTree;
	RECT r;
	r.left = r.top = 10;
	r.right = r.bottom = 40;
	_Tree->Create (WS_VISIBLE|WS_BORDER, r, this, IDC_TREE);
	// Load image list
	CImageList *pImgList = new CImageList;
	pImgList->Create (16, 16, ILC_MASK, 0, 5);
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_FOLDER));
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_FILE));
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_CONTINENTS));
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_BACKUP));
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_TRASH));

	_Tree->SetImageList (pImgList, TVSIL_NORMAL);
	return true;
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::OnSize (UINT nType, int cx, int cy)
{
	if (_Tree)
		_Tree->SetWindowPos (&wndTop, 0, 0, cx-20, cy-20, SWP_NOMOVE);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::OnExpanding (LPNMHDR pnmhdr, LRESULT *pLResult)
{
	/*CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem (IDC_TREE);
	NMTREEVIEW *pnmtv = (LPNMTREEVIEW) pnmhdr;

	if ((pnmtv->itemNew.state & TVIS_EXPANDED) != 0)
	{
		// We want to collapse this item -> forbid it
		*pLResult = true;
	}*/
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::openFile (const std::string &fname)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->openFile (fname.c_str());
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::openDir (const std::string &fname)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->openDir (fname.c_str());
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::continentNew ()
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->continentNew ();
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::continentDelete (const string &contName)
{
	CMainFrame *pMF = (CMainFrame*)GetParent();
	pMF->continentDelete (contName);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::continentProperties (const string &contName)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->continentProperties (contName);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::continentNewRegion (const string &contName)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->continentNewRegion (contName);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::continentOpen (const string &contName)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->continentOpen (contName);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionOpen (const string &path)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionOpen (path);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionRename (const string &path)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionRename (path);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionNewPrim (const string &path)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionNewPrim (path);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionNewGeorges (const string &path)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionNewGeorges (path);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionNewSubRegion (const std::string &path)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionNewSubRegion (path);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionDelete (const std::string &path)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionDelete (path);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::fileOpen (const std::string &sFileFullName)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->fileOpen (sFileFullName);	
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::fileRename (const std::string &sFileFullName)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->fileRename (sFileFullName);	
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::fileDelete (const std::string &sFileFullName)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->fileDelete (sFileFullName);	
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::copy (const std::string &pathSrc, const std::string &pathDst)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->copy (pathSrc, pathDst);
}

// ---------------------------------------------------------------------------
/*
void CMasterTreeDlg::OnLDblClkTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem (IDC_TREE);
	NMTREEVIEW *pnmtv = (LPNMTREEVIEW) pNMHDR;
	HTREEITEM hItem = pTree->GetSelectedItem();
	HTREEITEM hParent = pTree->GetParentItem (hItem);
	
	*pResult = 0;
	if (hItem != NULL)
	{
		map<HTREEITEM,string>::iterator it = _Files.find (hItem);

		if (it != _Files.end())
		{
			// Double click on a file open the corresponding editor
			openAnyFile (it->second.c_str());
			return;
		}

		if ((hParent != NULL) && (pTree->GetParentItem (hParent) == NULL))
		{
			// Double click on a Continent open all editors
			// Parse all files and open them in the good editor
			HTREEITEM hChildItem = pTree->GetChildItem (hItem);

			while (hChildItem != NULL)
			{
				it = _Files.find (hChildItem);
				if (it != _Files.end())
				{
					openAnyFile (it->second.c_str());
				}
				hChildItem = pTree->GetNextItem (hChildItem, TVGN_NEXT);
			}
			return;
		}
	}

}
*/
// ---------------------------------------------------------------------------
void CMasterTreeDlg::OnCancel ()
{
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::OnOK ()
{
}

