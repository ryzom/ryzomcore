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

#if !defined(AFX_WATCHLIST_H__4FA481A7_E054_4238_9DA3_7C729FAFC3B3__INCLUDED_)
#define AFX_WATCHLIST_H__4FA481A7_E054_4238_9DA3_7C729FAFC3B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WatchList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWatchList window

class CWatchList : public CCJListCtrl
{
// Construction
public:
	CWatchList();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWatchList)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	void Redraw();
	void UpdateRow(int iItem);
	void AddEditItem(LVITEM& item);
	void AddEmptyRow();
	virtual ~CWatchList();

	// Generated message map functions
protected:
	CStringArray m_exps;

	//{{AFX_MSG(CWatchList)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WATCHLIST_H__4FA481A7_E054_4238_9DA3_7C729FAFC3B3__INCLUDED_)
