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

#if !defined(AFX_BREAKPOINTWND_H__3BB3442D_2252_41F9_AC78_998E605AF9CF__INCLUDED_)
#define AFX_BREAKPOINTWND_H__3BB3442D_2252_41F9_AC78_998E605AF9CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BreakPointWnd.h : header file
//
#include "ProjectFile.h"
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CBreakPointWnd dialog

class CBreakPointWnd : public CDialog
{
// Construction
public:
	class CFileBreakPoint
	{
	public:
		CProjectFile				*m_File;
		CProjectFile::CBreakPoint	 m_BP;
	};



	CBreakPointWnd(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBreakPointWnd)
	enum { IDD = IDD_BREAKPOINTS };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBreakPointWnd)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL	

public:
	std::vector<CFileBreakPoint> m_BP;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBreakPointWnd)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeBpList();
	afx_msg void OnCondition();
	afx_msg void OnCheckBP();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRemove();
	afx_msg void OnDeleteAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void UpdateContent();
	void UpdateButtons();
	void EraseSel();	
	CCheckListBox &getBPListCtrl();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BREAKPOINTWND_H__3BB3442D_2252_41F9_AC78_998E605AF9CF__INCLUDED_)
