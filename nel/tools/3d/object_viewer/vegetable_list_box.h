// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#if !defined(AFX_VEGETABLE_LIST_BOX_H__2F5C38C3_1FB5_4B8F_86C5_DE9B194BD9F6__INCLUDED_)
#define AFX_VEGETABLE_LIST_BOX_H__2F5C38C3_1FB5_4B8F_86C5_DE9B194BD9F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// vegetable_list_box.h : header file
//


class	CVegetableDlg;

/////////////////////////////////////////////////////////////////////////////
// CVegetableListBox window

class CVegetableListBox : public CListBox
{
// Construction
public:
	CVegetableListBox();

// Attributes
public:
	// Owner. Must be init
	CVegetableDlg	*VegetableDlg;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVegetableListBox)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CVegetableListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CVegetableListBox)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGETABLE_LIST_BOX_H__2F5C38C3_1FB5_4B8F_86C5_DE9B194BD9F6__INCLUDED_)
