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

#ifndef __SHELLTREE_H__
#define __SHELLTREE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ShellTree.h : header file
//

#include <shlobj.h>
#include "ShellPidl.h"

/////////////////////////////////////////////////////////////////////////////
// CShellTree window
//
// This source is part of CShellTree - Selom Ofori
// 
// Version: 1.02 (any previously unversioned copies are older/inferior
//
// This code is free for all to use. Mutatilate it as much as you want
// See MFCENUM sample from microsoft

class AFX_EXT_CLASS CShellTree : public CTreeCtrl, public CShellPidl
{
public:
	enum FindAttribs{type_drive,type_folder};

// Construction
public:
	CShellTree();

// Attributes
public:
    BOOL m_bOutaHere;

// Operations
public:
	void	PopulateTree();
	void	PopulateTree(int nFolder);
	void	OnFolderExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	void	GetContextMenu(NMHDR* pNMHDR, LRESULT* pResult);
	BOOL	OnFolderSelected(NMHDR* pNMHDR, LRESULT* pResult, CString &szFolderPath);
	void	OnDeleteShellItem(NMHDR* pNMHDR, LRESULT* pResult);
	void	EnableImages();
	BOOL	GetSelectedFolderPath(CString &szFolderPath);
	bool	SearchTree(HTREEITEM treeNode, CString szSearchName, FindAttribs attr);
	void	TunnelTree(CString szFindPath);
	LPSHELLFOLDER	GetParentShellFolder(HTREEITEM folderNode);
	LPITEMIDLIST	GetRelativeIDLIST(HTREEITEM folderNode);
	LPITEMIDLIST	GetFullyQualifiedID(HTREEITEM folderNode);
	void	FindTreePidl(	HTREEITEM nextNode,
							HTREEITEM& folderNode, 
							LPLVITEMDATA lplvid, 
							bool& valid);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShellTree)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CShellTree();

	// Generated message map functions
protected:
	void	FillTreeView(LPSHELLFOLDER lpsf, LPITEMIDLIST  lpifq, HTREEITEM     hParent);
	void	GetNormalAndSelectedIcons(LPITEMIDLIST lpifq, LPTV_ITEM lptvitem);
 	static int CALLBACK TreeViewCompareProc(LPARAM, LPARAM, LPARAM);
	
	//{{AFX_MSG(CShellTree)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // __SHELLTREE_H__
