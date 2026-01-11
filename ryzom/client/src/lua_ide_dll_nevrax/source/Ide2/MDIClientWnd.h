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

#if !defined(AFX_MDICLIENTWND_H__A19DDD02_2C26_4675_A107_262E4B931144__INCLUDED_)
#define AFX_MDICLIENTWND_H__A19DDD02_2C26_4675_A107_262E4B931144__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MDIClientWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMDIClientWnd window

class CMDIClientWnd : public CWnd
{
// Construction
public:
	CMDIClientWnd();

// Attributes
public:
protected:
	CMenu m_menuBuild, m_menuDebug, m_menuProject;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMDIClientWnd)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	void ResetMenu();
	virtual ~CMDIClientWnd();

	// Generated message map functions
protected:
	afx_msg LRESULT OnMDISetMenu(WPARAM wParam, LPARAM lParam);
	//{{AFX_MSG(CMDIClientWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MDICLIENTWND_H__A19DDD02_2C26_4675_A107_262E4B931144__INCLUDED_)
