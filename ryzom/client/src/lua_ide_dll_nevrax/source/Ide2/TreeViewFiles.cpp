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

// TreeViewFiles.cpp : implementation file
//

#include "stdafx.h"
#include "ide2.h"
#include "TreeViewFiles.h"

#include "MainFrame.h"
#include "LuaDoc.h"
#include "LuaView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeViewFiles

IMPLEMENT_DYNCREATE(CTreeViewFiles, CTreeView)

CTreeViewFiles::CTreeViewFiles()
{
}

CTreeViewFiles::~CTreeViewFiles()
{
}


BEGIN_MESSAGE_MAP(CTreeViewFiles, CTreeView)
	//{{AFX_MSG_MAP(CTreeViewFiles)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemexpanded)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_PROJECT_ADD_FILES, OnProjectAddFiles)
	ON_COMMAND(ID_PROJECT_PROPERTIES, OnProjectProperties)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTreeViewFiles drawing

void CTreeViewFiles::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CTreeViewFiles diagnostics

#ifdef _DEBUG
void CTreeViewFiles::AssertValid() const
{
	CTreeView::AssertValid();
}

void CTreeViewFiles::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTreeViewFiles message handlers

int CTreeViewFiles::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pTree = &GetTreeCtrl();

	m_images.Create (IDB_IL_FILE, 16, 1, RGB(0,255,0));
	m_pTree->SetImageList (&m_images, TVSIL_NORMAL);

	return 0;
}

void CTreeViewFiles::RemoveAll()
{
	m_pTree->DeleteAllItems();
}

void CTreeViewFiles::AddRoot(CString strProject)
{
	CString strLabel;
	strLabel.Format("Project '%s'", strProject);

	TV_INSERTSTRUCT root;
	root.hParent = NULL;
	root.hInsertAfter = TVI_SORT;
	root.item.iImage = 1;
	root.item.iSelectedImage = 1;
	root.item.pszText = strLabel.GetBuffer(0);
	root.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	m_hRoot = m_pTree->InsertItem(&root);

	TV_INSERTSTRUCT files;
	files.hParent = m_hRoot;
	files.hInsertAfter = TVI_SORT;
	files.item.iImage = 2;
	files.item.iSelectedImage = 2;
	files.item.pszText = "Lua scripts";
	files.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	m_hFilesFolder = m_pTree->InsertItem(&files);
}

HTREEITEM CTreeViewFiles::AddProjectFile(CString strName, long lParam)
{
	TV_INSERTSTRUCT file;
	file.hParent = m_hFilesFolder;
	file.hInsertAfter = TVI_LAST;
	file.item.iImage = 4;
	file.item.iSelectedImage = 4;
	file.item.pszText = strName.GetBuffer(0);
	file.item.lParam = lParam;
	file.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	
	return m_pTree->InsertItem(&file);
}

BOOL CTreeViewFiles::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	
	return CTreeView::PreCreateWindow(cs);
}

void CTreeViewFiles::OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if ( pNMTreeView->itemNew.hItem == m_hFilesFolder )
	{
		if ( pNMTreeView->action==TVE_EXPAND )
			m_pTree->SetItemImage(pNMTreeView->itemNew.hItem, 3, 3);
		else if ( pNMTreeView->action==TVE_COLLAPSE )
			m_pTree->SetItemImage(pNMTreeView->itemNew.hItem, 2, 2);
	}

	*pResult = 0;
}


void CTreeViewFiles::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CPoint pt;
	GetCursorPos(&pt);
	m_pTree->ScreenToClient(&pt);
	UINT nFlags;
	HTREEITEM item = m_pTree->HitTest(pt, &nFlags);
	
	HMENU hMenu = NULL;
	if ( item == m_hRoot )
		hMenu = LoadMenu(theApp.m_hInstance, MAKEINTRESOURCE(IDR_PROJECT_MENU));

	if ( !hMenu )
		return;

	HMENU hSubMenu = GetSubMenu(hMenu, 0);
	if (!hSubMenu) 
	{
		DestroyMenu(hMenu);
		return;
	}

	POINT mouse;
	GetCursorPos(&mouse);
	::SetForegroundWindow(m_hWnd);	
	::TrackPopupMenu(hSubMenu, 0, mouse.x, mouse.y, 0, m_hWnd, NULL);

	DestroyMenu(hMenu);
	
	*pResult = 0;
}

void CTreeViewFiles::OnProjectAddFiles() 
{
	CProject* pProject = ((CMainFrame*)AfxGetMainWnd())->GetProject();
	pProject->AddFiles();
}

void CTreeViewFiles::OnProjectProperties() 
{
	CProject* pProject = ((CMainFrame*)AfxGetMainWnd())->GetProject();
	pProject->Properties();
}

void CTreeViewFiles::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CPoint pt;
	GetCursorPos(&pt);
	m_pTree->ScreenToClient(&pt);
	UINT nFlags;
	HTREEITEM hItem = m_pTree->HitTest(pt, &nFlags);

	CProjectFile* pPF = (CProjectFile*)m_pTree->GetItemData(hItem);
	if ( pPF )
	{
		CLuaView* pView = theApp.OpenProjectFilesView(pPF);
		pView->Activate();
	}

	*pResult = 0;
}

void CTreeViewFiles::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	HTREEITEM hItem = m_pTree->GetSelectedItem();
	if ( hItem && pTVKeyDown->wVKey == VK_DELETE )
	{
		CProjectFile* pPF = (CProjectFile*)m_pTree->GetItemData(hItem);
		if ( pPF )
		{
			m_pTree->DeleteItem(hItem);
			CProject* pProject = ((CMainFrame*)AfxGetMainWnd())->GetProject();
			pProject->RemoveFile(pPF);
		}
	}

	*pResult = 0;
}

void CTreeViewFiles::Expand()
{
	m_pTree->Expand(m_hRoot, TVE_EXPAND);
	m_pTree->Expand(m_hFilesFolder, TVE_EXPAND);	
}

void CTreeViewFiles::Sort()
{
	m_pTree->SortChildren(m_hFilesFolder);
}

