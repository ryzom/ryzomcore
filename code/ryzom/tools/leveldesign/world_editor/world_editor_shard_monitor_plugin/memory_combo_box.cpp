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

// memory_combo_box.cpp : implementation file
//

#include "stdafx.h"
#include "memory_combo_box.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************
// CMemoryComboBox
// ***************************************************************************

CMemoryComboBox::CMemoryComboBox()
{
	_EditChanged = false;
	_ComboOpen = false;
	_ComboCurSel = -1;
	_AutoCompleteExtension = false;
}

// ***************************************************************************

CMemoryComboBox::~CMemoryComboBox()
{
}

// ***************************************************************************

void CMemoryComboBox::create (DWORD style, const RECT &rect, CWnd *parent, UINT nId, const char *registerAdress, int memoryCount)
{
	// Register a window
	Id = nId;
	RegisterAdress = registerAdress;
	MemoryCount = memoryCount;
	LPCTSTR clas = AfxRegisterWndClass (0);
	if (clas)
	{
		if (Create (clas, "MemoryComboBox", style, rect, parent, nId))
		{
			// Create the combo box
			RECT comboRect;
			comboRect.left = 0;
			comboRect.top = 0;
			comboRect.right = rect.right-rect.left;
			comboRect.bottom = 200; //rect.bottom-rect.top;
			_ComboBox.Create (WS_CHILD|WS_VSCROLL|WS_VISIBLE|CBS_DROPDOWN|CBS_HASSTRINGS|CBS_AUTOHSCROLL, comboRect, this, 0);
		}
	}
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CMemoryComboBox, CWnd)
	//{{AFX_MSG_MAP(CMemoryComboBox)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************

void CMemoryComboBox::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	if (IsWindow (_ComboBox))
	{
		uint pos = _ComboBox.GetEditSel ();
		_ComboBox.SetWindowPos (NULL, 0, 0, cx, 200, SWP_NOZORDER);
		_ComboBox.SetEditSel (pos&0xffff, pos>>16);
	}
}

// ***************************************************************************

bool CMemoryComboBox::getMemory (int slot, std::string &ret)
{
	// Open the key
	HKEY hKey;
	if (RegOpenKey (HKEY_CURRENT_USER, RegisterAdress.c_str (), &hKey) == ERROR_SUCCESS)
	{
		// Get the value
		char strSrc[512];
		smprintf (strSrc, 512, "%d", slot);
		char str[512];
		long size = 512;
		if (RegQueryValue (hKey, strSrc, str, &size) == ERROR_SUCCESS)
		{
			ret = str;

			// Close
			RegCloseKey (hKey);

			return true;
		} 

		// Close
		RegCloseKey (hKey);
	}
	return false;
}

// ***************************************************************************

void CMemoryComboBox::scrollDown (int start, int end)
{
	// Open the key
	HKEY hKey;
	if (RegCreateKey (HKEY_CURRENT_USER, RegisterAdress.c_str (), &hKey) == ERROR_SUCCESS)
	{
		// Scroll down the list
		for (int i=end-1; i>start; i--)
		{
			// Get the old value
			char strSrc[512];
			smprintf (strSrc, 512, "%d", i-1);
			char str[512];
			long size = 512;
			if (RegQueryValue (hKey, strSrc, str, &size) == ERROR_SUCCESS)
			{
				// Set the value
				char strDst[512];
				smprintf (strDst, 512, "%d", i);
				RegSetValue (hKey, strDst, REG_SZ, str, size);
			} 
		}

		// Close
		RegCloseKey (hKey);
	}
}

// ***************************************************************************

void CMemoryComboBox::writeStringInRegistry (const std::string &str)
{
	// Open the key
	HKEY hKey;
	if (RegCreateKey (HKEY_CURRENT_USER, RegisterAdress.c_str (), &hKey) == ERROR_SUCCESS)
	{
		// Set the value
		RegSetValue (hKey, "0", REG_SZ, str.c_str (), str.size ());

		// Close
		RegCloseKey (hKey);
	}
}

// ***************************************************************************

BOOL CMemoryComboBox::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// Branch the message
	switch (HIWORD(wParam))
	{
	case CBN_CLOSEUP:
		{
			// Closed 
			_ComboOpen = false;
			if (_ComboCurSel != _ComboBox.GetCurSel ())
			{
				notifyParent ();
			}

			CString str;
			_ComboBox.GetWindowText (str);
			// nlinfo ("CBN_CLOSEUP: %s", str);

			if ((uint)_ComboBox.GetCurSel () < Commands.size())
			{
				GetParent ()->SendMessage (MC_COMMAND, Commands[_ComboBox.GetCurSel ()].Id, Id);
			}
		}
		break;
	case CBN_SELCHANGE:
		{
			CString str;
			_ComboBox.GetWindowText (str);
			// nlinfo ("CBN_SELCHANGE: %s", str);

			// Edit not changed since select a string
			_EditChanged = false;

			// Open ?
			if (!_ComboOpen)
			{
				// Notify parent for string change
				notifyParent ();
			}
		}
		break;
	case CBN_DBLCLK:
		{
			CString str;
			_ComboBox.GetWindowText (str);
			// nlinfo ("CBN_DBLCLK: %s", str);

			if ((uint)_ComboBox.GetCurSel () < Commands.size())
			{
				break;
			}
		}
		break;
	case CBN_EDITCHANGE:
		{
			_EditChanged = true;
			CString str;
			_ComboBox.GetWindowText (str);
			// nlinfo ("CBN_EDITCHANGE: %s", str);

			if ((uint)_ComboBox.GetCurSel () < Commands.size())
			{
				break;
			}
		};
		break;
	case CBN_EDITUPDATE:
		{
			CString str;
			_ComboBox.GetWindowText (str);
			// nlinfo ("CBN_EDITUPDATE: %s", str);

			if ((uint)_ComboBox.GetCurSel () < Commands.size())
			{
				break;
			}
		};
		break;
	case CBN_DROPDOWN:
		{
			// Edit change ?
			if (_EditChanged)
			{
				notifyParent ();
			}

			// Closed 
			_ComboOpen = true;

			// Get current text
			CString strText;
			_ComboBox.GetWindowText (strText);

			// Look for the string in the combo
			uint i;
			uint count = _ComboBox.GetCount ();
			_ComboCurSel = -1;
			for (i=0; i<count; i++)
			{
				CString str;
				_ComboBox.GetLBText( i, str);
				if (str == strText)
				{
					_ComboCurSel = i;
					break;
				}
			}
			// nlinfo ("CBN_DROPDOWN: %s", str);
		}
		break;
	case CBN_ERRSPACE:
		{
			CString str;
			_ComboBox.GetWindowText (str);
			// nlinfo ("CBN_ERRSPACE: %s", str);

			if ((uint)_ComboBox.GetCurSel () < Commands.size())
			{
				break;
			}
		}
		break;
	case CBN_KILLFOCUS:
		{
			CString str;
			_ComboBox.GetWindowText (str);
			// nlinfo ("CBN_KILLFOCUS: %s", str);

			// Edit changed ?
			if (_EditChanged)
			{
				notifyParent ();
			}

			// Send the notify message to the parent
			NMHDR strNotify;
			strNotify.hwndFrom = *this;
			strNotify.idFrom = Id;
			strNotify.code = NM_KILLFOCUS;
			GetParent ()->SendMessage (WM_NOTIFY, Id, (LPARAM)&strNotify);
		}
		break;
	case CBN_SELENDCANCEL:
		{
			CString str;
			_ComboBox.GetWindowText (str);
			// nlinfo ("CBN_SELENDCANCEL: %s", str);

			if ((uint)_ComboBox.GetCurSel () < Commands.size())
			{
			}
		}
		break;
	case CBN_SELENDOK:
		{
			CString str;
			_ComboBox.GetWindowText (str);
			// nlinfo ("CBN_SELENDOK: %s", str);

			if ((uint)_ComboBox.GetCurSel () < Commands.size())
			{
				break;
			}
		}
		break;
	case CBN_SETFOCUS:
		{
			_EditChanged = false;
			CString str;
			_ComboBox.GetWindowText (str);
			refreshStrings ();
			_ComboBox.SetWindowText (str);
			// nlinfo ("CBN_SETFOCUS: %s", str);

			if ((uint)_ComboBox.GetCurSel () < Commands.size())
			{
				break;
			}

			// Send the notify message to the parent
			NMHDR strNotify;
			strNotify.hwndFrom = *this;
			strNotify.idFrom = Id;
			strNotify.code = NM_SETFOCUS;
			GetParent ()->SendMessage (WM_NOTIFY, Id, (LPARAM)&strNotify);
		}
		break;
	}
	
	return CWnd::OnCommand(wParam, lParam);
}

// ***************************************************************************

void CMemoryComboBox::pushString ()
{
	CString str;
	GetWindowText (str);
	pushString((LPCTSTR) str);
}


// ***************************************************************************

void CMemoryComboBox::pushString (const std::string &str)
{
	if (str != "")
	{
		// Look for an existing static 
		uint i;
		for (i=0; i<StaticStrings.size(); i++)
		{
			// Get the value
			if (StaticStrings[i] == str)
				break;
		}

		// Not found ?
		if (i == StaticStrings.size())
		{
			// Look for an existing value
			int i;
			for (i=0; i<MemoryCount; i++)
			{
				// Get the value
				std::string value;
				if (getMemory (i, value))
				{
					if (value == str)
					{
						i++;
						break;
					}
				}
			}

			// Something change ?
			if (i != 0)
			{
				// String found ?
				scrollDown (0, i);
				writeStringInRegistry (str);
			}

			// Look for an existing value
			int itemCount = _ComboBox.GetCount ();
			for (i=Commands.size() + StaticStrings.size(); i<(int)(itemCount+Commands.size()+StaticStrings.size()); i++)
			{
				// Get the value
				CString value;
				if(_ComboBox.GetLBTextLen(i)>0)
				{
					_ComboBox.GetLBText( i, value);
					if (value == str.c_str())
						break;
				}
			}

			// Something change ?
			if (i == (int)(itemCount+Commands.size()+ StaticStrings.size()))
			{
				// Insert the sting
				_ComboBox.InsertString (Commands.size()+ StaticStrings.size(), str.c_str());
			}
		}
	}
}

// ***************************************************************************

void CMemoryComboBox::refreshStrings ()
{
	_ComboBox.ResetContent ();

	uint i;
	for (i=0; i<Commands.size(); i++)
	{
		addLabelCommands (i);
	}

	int count = Commands.size();
	for (i=0; i<StaticStrings.size(); i++)
	{
		_ComboBox.InsertString (count, StaticStrings[i].c_str ());
		count++;
	}

	for (i=0; i<(uint)MemoryCount; i++)
	{
		std::string ret;
		if (getMemory (i, ret))
		{
			_ComboBox.InsertString (count, ret.c_str ());
			count++;
		}
	}
}

// ***************************************************************************

void CMemoryComboBox::setRegisterAdress (const char *registerAdress)
{
	RegisterAdress = registerAdress;
	refreshStrings ();
}

// ***************************************************************************

void CMemoryComboBox::addCommand (const char* commandLabel, uint commandId)
{
	Commands.push_back (CCommand ());
	Commands[Commands.size()-1].Label = commandLabel;
	Commands[Commands.size()-1].Id = commandId;
	addLabelCommands (Commands.size()-1);
}

// ***************************************************************************

void CMemoryComboBox::clearCommand ()
{
	for (uint i=0; i<Commands.size(); i++)
	{
		_ComboBox.DeleteString (0);
	}
	Commands.clear ();
}

// ***************************************************************************

void CMemoryComboBox::addLabelCommands (uint i)
{
	_ComboBox.InsertString (i, Commands[i].Label.c_str ());
}

// ***************************************************************************

uint CMemoryComboBox::getCommandCount () const
{
	return Commands.size();
}

// ***************************************************************************

void CMemoryComboBox::OnSetFocus(CWnd* pOldWnd) 
{
	if (pOldWnd != this)
	{
		if (IsWindow (_ComboBox))
		{
			CString str;
			_ComboBox.GetWindowText (str);
			_ComboBox.SetFocus ();
			refreshStrings ();
			_ComboBox.SetWindowText (str);
			_ComboBox.SetEditSel (0, -1);
		}
	}
}

// ***************************************************************************

void CMemoryComboBox::onOK ()
{
	_ComboBox.SetEditSel (0, -1);

	// Send the message to the parent
	DWORD wParam = (CBN_SELENDOK<<16) + Id;
	GetParent ()->SendMessage (WM_COMMAND, wParam, 0);
}

// ***************************************************************************

void CMemoryComboBox::onCancel ()
{
	_ComboBox.SetEditSel (0, -1);

	// Send the message to the parent
	DWORD wParam = (CBN_SELENDCANCEL<<16) + Id;
	GetParent ()->SendMessage (WM_COMMAND, wParam, 0);
}

// ***************************************************************************

void CMemoryComboBox::clearStaticStrings ()
{
	StaticStrings.clear ();
}

// ***************************************************************************

void CMemoryComboBox::addStaticStrings (const char* strings)
{
	StaticStrings.push_back (strings);
}

// ***************************************************************************

LRESULT CMemoryComboBox::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// Get window text
	switch (message)
	{
	case WM_GETTEXTLENGTH:
		{
			int curSel = _ComboBox.GetCurSel ();
			if ((curSel == -1) || ((uint)curSel<Commands.size()))
				return _ComboBox.GetWindowTextLength ();
			else
				return _ComboBox.GetLBTextLen (curSel);
		}
		break;
	case WM_GETTEXT:
		{
			int curSel = _ComboBox.GetCurSel ();
			if ((curSel == -1) || ((uint)curSel<Commands.size()))
			{
				_ComboBox.GetWindowText ((char*)lParam, wParam);
				return _ComboBox.GetWindowTextLength ();
			}
			else
			{
				_ComboBox.GetLBText (curSel, (char*)lParam);
				return _ComboBox.GetLBTextLen (curSel);
			}
		}
		break;
	case WM_SETTEXT:
		{
			_ComboBox.SetWindowText ((LPCTSTR)lParam);

			// Push the string
			pushString ();
		}
		break;
	case WM_SETFONT:
		{
			_ComboBox.SendMessage (WM_SETFONT, wParam, lParam);
		}
		break;
	}
	
	return CWnd::WindowProc(message, wParam, lParam);
}

// ***************************************************************************

void CMemoryComboBox::showDropDown ()
{
	_ComboBox.ShowDropDown ();
}

// ***************************************************************************

bool CMemoryComboBox::haveFocus ()
{
	CWnd *focus = GetFocus ();
	CWnd *parent = focus ? focus->GetParent () : NULL;
	return focus && 
		( (focus->m_hWnd == this->m_hWnd) || (focus->m_hWnd == _ComboBox.m_hWnd ) || 
		( parent && (parent->m_hWnd == this->m_hWnd) || (parent->m_hWnd == _ComboBox.m_hWnd ) ) );
}

// ***************************************************************************

bool CMemoryComboBox::isWnd (const CWnd *wnd) const
{
	if (wnd)
	{
		return (this == wnd) || (&_ComboBox == wnd) || (&_ComboBox == (const CWnd*)wnd->GetParent ());
	}
	return false;
}

// ***************************************************************************

void CMemoryComboBox::notifyParent ()
{
	// nlinfo ("notify parent");

	// Push the string
	pushString ();

	// Edit not changed
	_EditChanged = false;

	// Notify parent
	CWnd *parent = GetParent ();
	if (parent)
	{
		parent->SendMessage (MC_STRINGCHANGE, Id, 0);
	}
}

// ***************************************************************************

BOOL CMemoryComboBox::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		{
			if (pMsg->wParam==VK_ESCAPE)
			{
				CWnd *parent = GetParent ();
				if (parent)
				{
					parent->SendMessage (MC_ESCAPE, Id, 0);
				}
			}
			if (pMsg->wParam==VK_RETURN)
			{
				if (!_ComboBox.GetDroppedState ())
				{
					// Auto compelete ?
					if (_AutoCompleteExtension)
					{
						CString str;
						_ComboBox.GetWindowText (str);
						string str2 = str;
						if ((!str2.empty ()) && (str2.find ('.') == string::npos))
						{
							str2 += "." + _Extension;
							_ComboBox.SetWindowText (str2.c_str ());
						}
					}

					// Select the text
					_ComboBox.SetEditSel (0, -1);					

					// Notify parent change
					notifyParent ();
					return TRUE;
				}
			}
		}
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

// ***************************************************************************

void CMemoryComboBox::OnEnable(BOOL bEnable) 
{
	CWnd::OnEnable(bEnable);
	
	_ComboBox.EnableWindow (bEnable);
}

// ***************************************************************************

void CMemoryComboBox::enableAutoCompleteExtension (bool enable, const char * ext)
{
	_AutoCompleteExtension = enable;
	const char *point = ext;
	while ((*point == '.') || (*point == '*'))
		point++;
	_Extension = point;
}

// ***************************************************************************

CString CMemoryComboBox::getCurrString() const
{
	CString str;
	_ComboBox.GetWindowText(str);
	return str;
}

// ***************************************************************************
void CMemoryComboBox::setCurSel(const std::string &value)
{
	int index = _ComboBox.FindStringExact(0, value.c_str());
	if (index != CB_ERR)
	{
		_ComboBox.SetCurSel(index);
	}
}

// ***************************************************************************
void CMemoryComboBox::setCurSel(int index)
{
	_ComboBox.SetCurSel(index);
}

// ***************************************************************************
int CMemoryComboBox::getCount() const
{
	return _ComboBox.GetCount();
}

// ***************************************************************************
BOOL CMemoryComboBox::setEditSel(int startChar, int endChar)
{
	return _ComboBox.SetEditSel(startChar, endChar);
}

// ***************************************************************************
void CMemoryComboBox::setFocus()
{
	_ComboBox.SetFocus();
}


// ***************************************************************************


