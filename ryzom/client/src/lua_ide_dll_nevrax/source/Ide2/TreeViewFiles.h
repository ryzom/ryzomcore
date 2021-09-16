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

#if !defined(AFX_TREEVIEWFILES_H__D5303282_718E_4AB7_A667_017E34E216BB__INCLUDED_)
#define AFX_TREEVIEWFILES_H__D5303282_718E_4AB7_A667_017E34E216BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TreeViewFiles.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTreeViewFiles view

class CTreeViewFiles : public CTreeView
{
protected:
	CTreeViewFiles();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTreeViewFiles)

// Attributes
public:

// Operations
public:
	void RemoveAll();
	HTREEITEM AddProjectFile(CString strName, long lParam);
	void AddRoot(CString strProject);
	void Expand();
	void Sort();
	CTreeCtrl *getTreeCtrl() { return m_pTree; }
protected:
	CTreeCtrl* m_pTree;
	HTREEITEM m_hRoot, m_hFilesFolder;
	CImageList m_images;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTreeViewFiles)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnProjectAddFiles();
	virtual void OnProjectProperties();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTreeViewFiles();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CTreeViewFiles)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TREEVIEWFILES_H__D5303282_718E_4AB7_A667_017E34E216BB__INCLUDED_)
