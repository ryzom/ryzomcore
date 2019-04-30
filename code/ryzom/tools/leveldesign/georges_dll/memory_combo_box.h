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

#if !defined(AFX_MEMORY_COMBO_BOX_H__A0A59637_63E4_40A6_AA7D_B57B4707ABF2__INCLUDED_)
#define AFX_MEMORY_COMBO_BOX_H__A0A59637_63E4_40A6_AA7D_B57B4707ABF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// memory_combo_box.h : header file
//

#define MC_COMMAND (WM_APP+0x29)
#define MC_STRINGCHANGE (WM_APP+0x30)
#define MC_ESCAPE (WM_APP+0x45)

#include <string>
#include "color_wnd.h"

/////////////////////////////////////////////////////////////////////////////
// CMemoryComboBox window

class CMemoryComboBox : public CWnd
{
// Construction
public:
	CMemoryComboBox();

	enum 
	{
		HeightMin = 100,
	};

	void onOK ();
	void onCancel ();
	void create (DWORD style, const RECT &rect, CWnd *parent, UINT nId, const char *registerAdress, int memoryCount);
	void create (DWORD style, const RECT &rect, CWnd *parent, UINT nId);
	void setRegisterAdress (const char *registerAdress);
	void clearCommand ();
	void addCommand (const char* commandLabel, uint commandId);
	void clearStaticStrings ();
	void addStaticStrings (const char* strings);
	uint getCommandCount () const;
	void showDropDown ();
	bool haveFocus ();
	bool isWnd (const CWnd *wnd) const;
	void enableAutoCompleteExtension (bool enable, const char * ext);

	std::string			RegisterAdress;
	int					MemoryCount;
	UINT				Id;

	class CCommand
	{
	public:
		std::string		Label;
		uint			Id;
	};

	std::vector<std::string>	StaticStrings;
	std::vector<CCommand>	Commands;

private:

	// Auto complete extension
	bool				_AutoCompleteExtension;
	std::string			_Extension;

	// Send the string changed to the parent
	void notifyParent ();

	// Add the current string in the registry
	void pushString ();

	// Get strings from the registry
	void refreshStrings ();
	
	// The internal combobox
	CComboBox			_ComboBox;

	// Edit changed
	bool				_EditChanged;
	bool				_ComboOpen;
	int					_ComboCurSel;

	// Use the memory
	bool				_Memory;

	void scrollDown (int start, int end);
	bool getMemory (int slot, std::string &ret);
	void pushString (const std::string &str);
	void addLabelCommands (uint i);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMemoryComboBox)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMemoryComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMemoryComboBox)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEMORY_COMBO_BOX_H__A0A59637_63E4_40A6_AA7D_B57B4707ABF2__INCLUDED_)
