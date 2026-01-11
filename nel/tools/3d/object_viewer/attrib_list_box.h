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

#if !defined(AFX_ATTRIB_LIST_BOX_H__0927B20B_FD26_433B_856B_6EF2871C233E__INCLUDED_)
#define AFX_ATTRIB_LIST_BOX_H__0927B20B_FD26_433B_856B_6EF2871C233E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// attrib_list_box.h : header file
//

class CValueGradientDlg ;
struct IValueGradientDlgClient ;

/////////////////////////////////////////////////////////////////////////////
// CAttribListBox window

class CAttribListBox : public CListBox
{
public:	
	/// a function that can display a value in a gradient, with the given offset
	typedef void (* TDisplayValue)(CDC *dc, uint index, sint x, sint y, void *lParam) ;


// Construction
public:
	CAttribListBox();

// Attributes
public:

// Operations
public:

	void setDrawer(IValueGradientDlgClient *drawer)
	{
		_DrawerInterface = drawer ;
	}

	void setCtrlID(uint id) { _Id = id ; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttribListBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAttribListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAttribListBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()


	IValueGradientDlgClient *_DrawerInterface ;	
	uint _Id ;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTRIB_LIST_BOX_H__0927B20B_FD26_433B_856B_6EF2871C233E__INCLUDED_)
