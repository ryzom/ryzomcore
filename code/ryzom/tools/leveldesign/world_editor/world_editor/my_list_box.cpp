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

// my_list_box.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "my_list_box.h"

#define COMBO_REAL_HEIGHT 300

// ***************************************************************************
// CMyListBox
// ***************************************************************************

CMyListBox::CMyListBox()
{
}

// ***************************************************************************

CMyListBox::~CMyListBox()
{
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CMyListBox, CListBox)
	//{{AFX_MSG_MAP(CMyListBox)
	ON_CONTROL_REFLECT(LBN_DBLCLK, OnDblclk)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CMyListBox message handlers
// ***************************************************************************

BOOL CMyListBox::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD wNotifyCode = HIWORD(wParam); // notification code 

	if (_EditingItem != LB_ERR)
	{
		switch (wNotifyCode)
		{
		case CBN_SELENDOK:
			{
				CString str;
				getWindowTextUTF8 (StringSelectComboBox, str);
				DeleteString(_EditingItem);
				InsertString(_EditingItem, str);
				SetCurSel (_SelectAfter);
				_EditingItem = LB_ERR;
				notifyParent ();
				StringSelectComboBox.ShowWindow(SW_HIDE);
			}
			break;
		case CBN_SELENDCANCEL:
			{
				if (_DeleteItIfCancel)
					DeleteString (_EditingItem);
				SetCurSel (_SelectAfter);
				_EditingItem = LB_ERR;
				StringSelectComboBox.ShowWindow(SW_HIDE);
			}
			break;
		}
	}
	
	return CListBox::OnCommand(wParam, lParam);
}

// ***************************************************************************

void CMyListBox::OnDblclk() 
{
}

// ***************************************************************************

BOOL CMyListBox::CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPVOID lpParam)
{
	if (CWnd::CreateEx(dwExStyle, lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, lpParam))
	{
		CRect itemRect (0, 0, 100, 100);
		nlverify (StringSelectComboBox.Create (CBS_DISABLENOSCROLL|WS_VSCROLL|CBS_DROPDOWNLIST|WS_TABSTOP, itemRect, 
			this, 0));		
		return TRUE;
	}
	return FALSE;
}

// ***************************************************************************

void CMyListBox::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CListBox::OnLButtonDblClk(nFlags, point);

	BOOL bOutside;
	_EditingItem = ItemFromPoint( point, bOutside );

	// Get the selected item
	_SelectAfter = GetCurSel();
	if ((_EditingItem == LB_ERR) || bOutside)
	{
		_EditingItem = InsertString (-1, "");
		_DeleteItIfCancel = true;
	}
	else
	{
		_DeleteItIfCancel = false;
		_SelectAfter = _EditingItem;
	}

	// Get the item rect
	RECT itemRect;
	GetItemRect(_EditingItem, &itemRect);

	// Show the combo box
	itemRect.bottom = itemRect.top + COMBO_REAL_HEIGHT;
	CString itemText;
	GetText (_EditingItem, itemText);
	StringSelectComboBox.SelectString (-1, itemText);
	StringSelectComboBox.SetWindowPos (&wndTop, itemRect.left, itemRect.top, itemRect.right-itemRect.left, COMBO_REAL_HEIGHT, SWP_SHOWWINDOW);
	StringSelectComboBox.ShowDropDown (TRUE);

	StringSelectComboBox.SetFocus();
	
}

// ***************************************************************************

void CMyListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	int item = GetCurSel();
	CListBox::OnKeyDown(nChar, nRepCnt, nFlags);

	// Suppr ?
	bool modified = false;
	if (nChar == VK_DELETE)
	{
		if (item != LB_ERR)
		{
			DeleteString (item);
			if ((item >= GetCount()) && (item != 0))
				item--;
			if (item < GetCount())
				SetCurSel (item);
			modified = true;
		}
	}
	else if (nChar == VK_INSERT)
	{
		if (item != LB_ERR)
		{
			CString text;
			GetText (item, text);
			InsertString (item, text);
			SetCurSel (item);
			modified = true;
		}
	}
	else if ( (nChar == VK_UP) && (GetAsyncKeyState (VK_CONTROL) & 0x8000)) 
	{
		if ( (item != LB_ERR) && (item != 0))
		{			
			CString itemText;
			GetText (item, itemText);
			InsertString (item-1, itemText);
			DeleteString (item+1);
			SetCurSel (item-1);
			modified = true;
		}
	}
	else if ( (nChar == VK_DOWN) && (GetAsyncKeyState (VK_CONTROL) & 0x8000)) 
	{
		if ( (item != LB_ERR) && (item != GetCount()-1))
		{			
			CString itemText;
			GetText (item, itemText);
			InsertString (item+2, itemText);
			DeleteString (item);
			SetCurSel (item+1);
			modified = true;
		}
	}

	// Notify parent ?
	if (modified)
		notifyParent ();
}

// ***************************************************************************

void CMyListBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	BOOL bOutside;
	_EditingItem = ItemFromPoint( point, bOutside );

	// Get the selected item
	if ((_EditingItem == LB_ERR) || bOutside)
		SetCurSel (-1);
	
	CListBox::OnLButtonDown(nFlags, point);
}

// ***************************************************************************

void CMyListBox::notifyParent ()
{
	if (GetParent())
		GetParent()->SendMessage (LBN_CHANGE, GetDlgCtrlID(), 0);
}

// ***************************************************************************
