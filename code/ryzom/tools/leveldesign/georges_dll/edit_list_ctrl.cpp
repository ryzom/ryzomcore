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

// edit_list_ctrl.cpp : implementation file
//

#include "stdafx.h"
#include "georges_edit.h"
#include "edit_list_ctrl.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CEditListCtrl

CEditListCtrl::CEditListCtrl()
{
	ListCtrl.Ctrl = this;
	ColumnCount = 0;
	OnBrowse = false;
}

CEditListCtrl::~CEditListCtrl()
{
}

bool CEditListCtrl::create (DWORD wStyle, RECT &rect, CWnd *parent, uint dialog_index)
{
	DlgIndex = dialog_index;

	// Register window class
	LPCTSTR className = AfxRegisterWndClass( 0 ); 

	// Create this window
	if (CWnd::Create (className, "empty", WS_CHILD|wStyle, rect, parent, dialog_index))
	{
		RECT subRect;
		subRect.left = 0;
		subRect.top = 0;
		subRect.right = rect.right-rect.left;
		subRect.bottom = rect.bottom-rect.top;
#if defined(NL_COMP_VC) && NL_COMP_VC_VERSION >= 80
		if (ListCtrl.CreateEx ( WS_EX_CLIENTEDGE, /*WC_LISTVIEW, "",*/ WS_CHILD|LVS_REPORT, subRect, this, 0))
#else
		if (ListCtrl.CreateEx ( WS_EX_CLIENTEDGE, WC_LISTVIEW, "", WS_CHILD|LVS_REPORT, subRect, this, 0))
#endif
		{
			ListCtrl.ShowWindow ( SW_SHOW );

			RECT rect;
			rect.left = 0;
			rect.top = 0;
			rect.right = 100;
			rect.bottom = 100;
			Edit.Create (WS_BORDER, rect, &ListCtrl, 0);
			Edit.SetFont (ListCtrl.GetFont());
			Combo.Create (WS_BORDER|CBS_DROPDOWNLIST, rect, &ListCtrl, IdCombo);
			Combo.SetFont (ListCtrl.GetFont());
			MemCombo.create (WS_CHILD, rect, &ListCtrl, IdMemCombo, "", theApp.RememberListSize);
			MemCombo.SetFont (ListCtrl.GetFont());
			return true;
		}
	}
	
	return false;
}

BEGIN_MESSAGE_MAP(CEditListCtrl, CWnd)
	//{{AFX_MSG_MAP(CEditListCtrl)
	ON_WM_SHOWWINDOW()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEditListCtrl message handlers


BOOL CEditListCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// Item double click ?
	UpdateData ();

	NMHDR *gPtr = (NMHDR*)lParam;
	switch (gPtr->code)
	{
	case NM_DBLCLK:
		{
			LPNMLISTVIEW ptr = (LPNMLISTVIEW)lParam;
			// Get the item
			SubItem = ptr->iSubItem;
			for (Item =0; Item<(uint)ListCtrl.GetItemCount(); Item++)
			{
				CRect ref;
				ListCtrl.GetItemRect( Item, ref, LVIR_BOUNDS );
				if ((ref.top <= ptr->ptAction.y) && (ref.bottom >= ptr->ptAction.y))
					break;
			}
			if (Item != (uint)ListCtrl.GetItemCount ())
			{
				editItem (Item, SubItem);
			}
			else
			{
				// Insert an item at the end
				string text;
				getNewItemText (ListCtrl.GetItemCount (), 0, text);
				ListCtrl.InsertItem (ListCtrl.GetItemCount (), text.c_str ());
				for (uint i=1; i<ColumnCount; i++)
				{
					getNewItemText (ListCtrl.GetItemCount ()-1, i, text);
					ListCtrl.SetItemText (ListCtrl.GetItemCount ()-1, i, text.c_str ());
				}
				ListCtrl.SetItemState (ListCtrl.GetItemCount ()-1, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

				// Edit it
				editItem (ListCtrl.GetItemCount ()-1, SubItem);

				notifyParentChange ();
			}
		}
		break;
	case LVN_KEYDOWN:
		{
			NMLVKEYDOWN *ptr = (NMLVKEYDOWN*)lParam;
			switch (ptr->wVKey)
			{
			case VK_INSERT:
				{
					POSITION pos = ListCtrl.GetFirstSelectedItemPosition();
					int item = 0;
					if (pos)
					{
						item = ListCtrl.GetNextSelectedItem(pos);
						string text;
						getNewItemText (item, 0, text);
						ListCtrl.InsertItem (item, text.c_str ());
						for (uint i=1; i<ColumnCount; i++)
						{
							getNewItemText (item, i, text);
							ListCtrl.SetItemText (item, i, text.c_str ());
						}
					}
					else
					{
						string text;
						getNewItemText (0, 0, text);
						ListCtrl.InsertItem (0, text.c_str ());
						for (uint i=1; i<ColumnCount; i++)
						{
							getNewItemText (0, i, text);
							ListCtrl.SetItemText (0, i, text.c_str ());
						}
						ListCtrl.SetItemState (0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
					}
					notifyParentChange ();
				}
				break;
			case VK_DELETE:
				{
					int firstItem = -1;
					POSITION pos = ListCtrl.GetFirstSelectedItemPosition();
					if (pos)
						firstItem = ListCtrl.GetNextSelectedItem(pos);
					while (pos = ListCtrl.GetFirstSelectedItemPosition())
					{
						// Get selected item
						int nItem = ListCtrl.GetNextSelectedItem(pos);
						ListCtrl.DeleteItem( nItem );
					}
					if (firstItem>=ListCtrl.GetItemCount())
						firstItem--;
					if ((firstItem != -1) && (firstItem>=0))
					{
						// Select an item
						ListCtrl.SetItemState (firstItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
					}
					notifyParentChange ();
				}
				break;
			case VK_F2:
				{
					int firstItem = -1;
					POSITION pos = ListCtrl.GetFirstSelectedItemPosition();
					if (pos)
						firstItem = ListCtrl.GetNextSelectedItem(pos);
					if (firstItem != -1)
						editItem (firstItem, 0);
				}
				break;
			}
		}
		break;
	case NM_CLICK:
		{
			LPNMLISTVIEW ptr = (LPNMLISTVIEW)lParam;
			// Get the item
			SubItem = ptr->iSubItem;
			for (Item =0; Item<(uint)ListCtrl.GetItemCount(); Item++)
			{
				CRect ref;
				ListCtrl.GetItemRect( Item, ref, LVIR_BOUNDS );
				if ((ref.top <= ptr->ptAction.y) && (ref.bottom >= ptr->ptAction.y))
					break;
			}
			if ((Item != (uint)ListCtrl.GetItemCount ()) && (SubItem>0))
			{
				editItem (Item, SubItem);
			}
		}
		break;
	}
	
	UpdateData (FALSE);

	return CWnd::OnNotify(wParam, lParam, pResult);
}

BOOL CMyListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (!Ctrl->OnBrowse)
	{
		Ctrl->ListCtrl.UpdateData ();
		switch (HIWORD(wParam))
		{
		case EN_KILLFOCUS:
			{
				if (Ctrl->Edit.IsWindowVisible ())
				{
					CString str;
					Ctrl->Edit.GetWindowText (str);
					Ctrl->ListCtrl.SetItemText (Ctrl->Item, Ctrl->SubItem, str);
					Ctrl->Edit.ShowWindow (SW_HIDE);	
					Ctrl->notifyParentChange ();
					Ctrl->onItemChanged (Ctrl->Item, Ctrl->SubItem);
				}
			}
			break;
		case CBN_SELENDOK:
			{
				switch (LOWORD(wParam))
				{
				case CEditListCtrl::IdCombo:
					{
						if (Ctrl->Combo.IsWindowVisible ())
						{
							CString str;
							Ctrl->Combo.GetWindowText (str);
							Ctrl->ListCtrl.SetItemText (Ctrl->Item, Ctrl->SubItem, str);
							Ctrl->Combo.ShowWindow (SW_HIDE);	
							Ctrl->notifyParentChange ();
							Ctrl->onItemChanged (Ctrl->Item, Ctrl->SubItem);
							Ctrl->ListCtrl.SetFocus ();
						}
					}
					break;
				}
			}
		case CBN_SELENDCANCEL:
			{
				switch (LOWORD(wParam))
				{
				case CEditListCtrl::IdCombo:
					{
						if (Ctrl->Combo.IsWindowVisible ())
						{
							Ctrl->Combo.ShowWindow (SW_HIDE);	
							Ctrl->ListCtrl.SetFocus ();
						}
					}
					break;
				}
			}
			break;
 		}
		
		Ctrl->ListCtrl.UpdateData (FALSE);
	}

	return CWnd::OnCommand(wParam, lParam);
}

LRESULT CMyListCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case MC_COMMAND:
		{
			if (wParam == CEditListCtrl::CmdBrowse)
			{
				std::string		defExt;
				std::string		defFilename;
				std::string		filter;
				std::string		defDir;
				Ctrl->getBrowseInfo (Ctrl->Item, Ctrl->SubItem, defExt, defFilename, defDir, filter);

				CFileDialog dlgFile (TRUE, defExt.c_str (), defFilename.c_str (), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter.c_str (), theApp.m_pMainWnd);
				dlgFile.m_ofn.lpstrInitialDir = defDir.c_str ();
				Ctrl->OnBrowse = true;
				if (dlgFile.DoModal () == IDOK)
				{
					Ctrl->MemCombo.UpdateData ();
					Ctrl->MemCombo.SetWindowText (dlgFile.GetFileName ());
					Ctrl->MemCombo.UpdateData (FALSE);
					Ctrl->ListCtrl.SetItemText (Ctrl->Item, Ctrl->SubItem, dlgFile.GetFileName ());
					Ctrl->notifyParentChange ();
					Ctrl->onItemChanged (Ctrl->Item, Ctrl->SubItem);
					Ctrl->MemCombo.ShowWindow (SW_HIDE);	
					Ctrl->ListCtrl.SetFocus ();
				}
				else
				{
					Ctrl->MemCombo.ShowWindow (SW_HIDE);	
					Ctrl->ListCtrl.SetFocus ();
				}
				Ctrl->OnBrowse = false;
			}
		}
		break;
	case MC_STRINGCHANGE:
		{
			if (Ctrl->MemCombo.IsWindowVisible ())
			{
				// nlinfo ("closeMemComboBox");
				Ctrl->closeMemComboBox (true);
			}
		}
		break;
	case MC_ESCAPE:
		{
			Ctrl->MemCombo.ShowWindow (SW_HIDE);
		}
		break;
	}

	return CListCtrl::WindowProc(message, wParam, lParam);
}

void CEditListCtrl::onOK ()
{
	if (Edit.IsWindowVisible ())
	{
		CString str;
		Edit.GetWindowText (str);
		ListCtrl.SetItemText (Item, SubItem, str);
		Edit.ShowWindow (SW_HIDE);
		notifyParentChange ();
		onItemChanged (Item, SubItem);
	}
	else if (Combo.IsWindowVisible ())
	{
		CString str;
		Combo.GetWindowText (str);
		ListCtrl.SetItemText (Item, SubItem, str);
		Combo.ShowWindow (SW_HIDE);
		notifyParentChange ();
		onItemChanged (Item, SubItem);
	}
	else if (MemCombo.IsWindowVisible ())
	{
		memComboBoxAsChange (false);
		CString str;
		MemCombo.GetWindowText (str);
		ListCtrl.SetItemText (Item, SubItem, str);
		MemCombo.ShowWindow (SW_HIDE);
		notifyParentChange ();
		onItemChanged (Item, SubItem);
	}
	else
	{
		if (ListCtrl.GetSelectedCount() == 1)
		{
			POSITION pos = ListCtrl.GetFirstSelectedItemPosition();
			int item = ListCtrl.GetNextSelectedItem (pos);
			editItem (item, 0);
		}
	}
}

void CEditListCtrl::onCancel  ()
{
	if (Edit.IsWindowVisible ())
	{
		Edit.ShowWindow (SW_HIDE);
	}
	else if (Combo.IsWindowVisible ())
	{
		Combo.ShowWindow (SW_HIDE);
	}
	else if (MemCombo.IsWindowVisible ())
	{
		MemCombo.ShowWindow (SW_HIDE);
	}
}

void CEditListCtrl::editItem (uint item, uint subitem)
{
	// Get the sub Item rect
	Item = item;
	SubItem = subitem;
	CRect subItemRect;
	ListCtrl.GetSubItemRect( Item, SubItem, LVIR_BOUNDS, subItemRect );

	if ( (SubItem == 0) && (ColumnCount>1) )
	{
		CRect subItemLeft;
		if (ListCtrl.GetSubItemRect( Item, 1, LVIR_BOUNDS, subItemLeft))
			subItemRect.right = subItemLeft.left;
	}

	// Get edit mode
	TItemEdit editMode = getItemEditMode (Item, SubItem);
	if (editMode == EditEdit)
	{
		// Move the editbox
		Edit.SetWindowPos (NULL, subItemRect.left, subItemRect.top, subItemRect.right-subItemRect.left, subItemRect.bottom-subItemRect.top, SWP_SHOWWINDOW);
		char tmp[512];
		ListCtrl.GetItemText (Item, SubItem, tmp, 512);
		Edit.SetWindowText (tmp);
		Edit.SetSel( 0, -1);
		Edit.SetFocus ();
	}
	else if (editMode == EditFixedCombo)
	{
		// Get item string
		char tmp[512];
		ListCtrl.GetItemText (Item, SubItem, tmp, 512);

		// Get the combo string
		Combo.UpdateData ();
		Combo.ResetContent ();
		std::vector<std::string> retStrings;
		getComboBoxStrings (Item, SubItem, retStrings);
		for (uint i=0; i<retStrings.size (); i++)
		{
			Combo.InsertString (-1, retStrings[i].c_str());
			if (retStrings[i] == tmp)
				Combo.SetCurSel (i);
		}

		// Move the editbox
		Combo.SetWindowPos (NULL, subItemRect.left, subItemRect.top, subItemRect.right-subItemRect.left, DropDown, SWP_SHOWWINDOW);
		Combo.SetWindowText (tmp);
		Combo.SetFocus ();
		Combo.ShowDropDown ();
		Combo.UpdateData (FALSE);
	}
	else if (editMode == EditMemCombo)
	{
		// Get item string
		char tmp[512];
		ListCtrl.GetItemText (Item, SubItem, tmp, 512);

		// Get the combo strings
		string retString;
		bool browse;
		getMemComboBoxProp (Item, SubItem, retString, browse);
		MemCombo.setRegisterAdress (retString.c_str ());
		MemCombo.clearCommand ();
		if (browse)
			MemCombo.addCommand (GEORGES_EDIT_BROWSE_LABEL, CmdBrowse);

		// Move the editbox
		MemCombo.SetWindowPos (NULL, subItemRect.left, subItemRect.top, subItemRect.right-subItemRect.left, 500, SWP_SHOWWINDOW);
		MemCombo.SetWindowText (tmp);
		MemCombo.SetFocus ();
	}
}

void CEditListCtrl::notifyParentChange ()
{
	CWnd *parent = GetParent ();
	if (parent)
		GetParent ()->PostMessage (LC_CHANGE, DlgIndex, 0);
}

void CEditListCtrl::OnSetFocus( CWnd* pOldWnd )
{
	if (pOldWnd != this)
	{
		// Set the focus
		ListCtrl.SetFocus ();

		// Something selected ?
		POSITION pos = ListCtrl.GetFirstSelectedItemPosition();
		int item = 0;
		if (!pos && ListCtrl.GetItemCount ())
		{
			ListCtrl.SetItemState (0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		}
	}
}

void CEditListCtrl::insertColumn (uint id, const char *name)
{
	ListCtrl.InsertColumn (id, name);
	ColumnCount++;
}

void CEditListCtrl::getComboBoxStrings (uint item, uint subItem, std::vector<std::string> &retStrings)
{
	retStrings.reserve (3);
	retStrings.push_back ("String0");
	retStrings.push_back ("String1");
	retStrings.push_back ("String2");
}


void	CEditListCtrl::closeMemComboBox (bool update)
{
	if (update)
		memComboBoxAsChange (false);
	MemCombo.ShowWindow (SW_HIDE);	
	ListCtrl.SetFocus ();
}

void	CEditListCtrl::memComboBoxAsChange (bool selChange)
{
	MemCombo.UpdateData ();
	
	CString str;
	MemCombo.GetWindowText (str);
	ListCtrl.SetItemText (Item, SubItem, str);
	notifyParentChange ();
	onItemChanged (Item, SubItem);
}

void CEditListCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	if (IsWindow (ListCtrl))
		ListCtrl.SetWindowPos (NULL, 0, 0, cx, cy, SWP_NOZORDER|SWP_NOOWNERZORDER);
}

void CEditListCtrl::recalcColumn () 
{
	if (ColumnCount>0)
	{
		RECT listRect;
		ListCtrl.GetClientRect (&listRect);
		int width = listRect.right/ColumnCount;
		int i;
		for (i=0; i<(int)ColumnCount-1; i++)
		{
			ListCtrl.SetColumnWidth( i, width );
			listRect.right -= width;
		}
		ListCtrl.SetColumnWidth( i, listRect.right);
	}
}
