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


// attrib_list_box.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "attrib_list_box.h"
#include "value_gradient_dlg.h"


/////////////////////////////////////////////////////////////////////////////
// CAttribListBox

CAttribListBox::CAttribListBox() : _DrawerInterface(NULL)
{
}

CAttribListBox::~CAttribListBox()
{
}


BEGIN_MESSAGE_MAP(CAttribListBox, CListBox)
	//{{AFX_MSG_MAP(CAttribListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttribListBox message handlers

void CAttribListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{	

	// TODO: Add your code to draw the specified item
	nlassert(_DrawerInterface) ; // setDrawer not called
	CDC *dc = CDC::FromHandle(lpDrawItemStruct->hDC) ;

	sint x = lpDrawItemStruct->rcItem.left, y = lpDrawItemStruct->rcItem.top ; 
	_DrawerInterface->displayValue(dc, lpDrawItemStruct->itemID, x, y) ;

	
					
		CBrush b ;
		if (lpDrawItemStruct->itemState & ODS_SELECTED)
		{
			b.CreateSolidBrush(RGB(0,0,0)) ;
		}
		else
		{
			b.CreateSolidBrush(RGB(255,255,255)) ;
		}
		CGdiObject *oldObj = dc->SelectObject(&b) ;				
		RECT r ;
		r.top = y + 3 ; r.bottom = y + 36 ; r.left = x + 3 ; r.right = x + 60 ;
		dc->FrameRect(&r, &b) ;		
		dc->SelectObject(oldObj) ;
		b.DeleteObject() ;
	
}

void CAttribListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	lpMeasureItemStruct->CtlType = ODT_LISTBOX  ;
	lpMeasureItemStruct->CtlID = _Id ;
	lpMeasureItemStruct->itemWidth = 64 ;
	lpMeasureItemStruct->itemHeight = 40 ;		
}

int CAttribListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct) 
{
	// TODO: Add your code to determine the sorting order of the specified items
	// return -1 = item 1 sorts before item 2
	// return 0 = item 1 and item 2 sort the same
	// return 1 = item 1 sorts after item 2
	
	return 0;
}
