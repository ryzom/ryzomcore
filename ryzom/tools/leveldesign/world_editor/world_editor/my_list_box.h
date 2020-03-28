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

#if !defined(AFX_MY_LIST_BOX_H__24C857D7_CA96_42FE_86BC_38BBF134EA79__INCLUDED_)
#define AFX_MY_LIST_BOX_H__24C857D7_CA96_42FE_86BC_38BBF134EA79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// my_list_box.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMyListBox window

#define LBN_CHANGE (WM_USER+10)

class CMyListBox : public CListBox
{
// Construction
public:
	CMyListBox();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyListBox)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyListBox();

	// The string selection combo
	CComboBox	StringSelectComboBox;

	// Create
	BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPVOID lpParam = NULL);

	// Generated message map functions
protected:

	void notifyParent ();

	// Item editing
	int			_EditingItem;
	int			_SelectAfter;
	bool		_DeleteItIfCancel;

	//{{AFX_MSG(CMyListBox)
	afx_msg void OnDblclk();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MY_LIST_BOX_H__24C857D7_CA96_42FE_86BC_38BBF134EA79__INCLUDED_)
