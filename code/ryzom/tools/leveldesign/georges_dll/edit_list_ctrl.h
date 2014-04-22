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

#if !defined(AFX_EDIT_LIST_CTRL_H__49812C0E_3696_49D3_92A9_6AF18E4FD689__INCLUDED_)
#define AFX_EDIT_LIST_CTRL_H__49812C0E_3696_49D3_92A9_6AF18E4FD689__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// edit_list_ctrl.h : header file
//

#include "memory_combo_box.h"

/////////////////////////////////////////////////////////////////////////////
// CEditListCtrl window

class CEditListCtrl;

#define LC_CHANGE (WM_APP+0x28)

// Derived class of CListCtrl used for CEditListCtrl
class CMyListCtrl : public CListCtrl
{
public:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	CEditListCtrl	*Ctrl;
};

class CEditListCtrl : public CWnd
{
// Construction
public:
	CEditListCtrl();

	enum TItemEdit
	{
		EditEdit,
		EditFixedCombo,
		EditMemCombo,
		NoEdit,
	};

	enum
	{
		DropDown = 200	
	};

	enum
	{
		IdCombo = 0,
		IdMemCombo,
	};

	enum
	{
		CmdBrowse = 0,
	};

	// Create method
	bool create (DWORD wStyle, RECT &rect, CWnd *parent, uint dialog_index);
	
	// The CListCtrl
	CMyListCtrl			ListCtrl;
	CEdit				Edit;
	CComboBox			Combo;
	CMemoryComboBox		MemCombo;
	bool				OnBrowse;
	uint		Item, SubItem, ColumnCount;
	uint		DlgIndex;

	virtual		TItemEdit getItemEditMode (uint item, uint subItem) {return EditFixedCombo;}
	virtual		void getComboBoxStrings (uint item, uint subItem, std::vector<std::string> &retStrings);
	virtual		void getMemComboBoxProp (uint item, uint subItem, std::string &regAdr, bool &browse) { regAdr = ""; browse = false; }
	virtual		void getNewItemText (uint item, uint subItem, std::string &ret) { ret = "new"; }
	virtual		void getBrowseInfo (uint item, uint subItem, std::string &defExt, std::string &defFilename, std::string &defDir, std::string &filter) { defExt=""; defFilename=""; filter=""; defDir="";}
	virtual		void onItemChanged (uint item, uint subItem) {}

	void	memComboBoxAsChange (bool selChange);
	void	closeMemComboBox (bool update);
	void	recalcColumn ();

// Attributes
public:

// Operations
public:

	void	onOK ();
	void	onCancel ();
	void	editItem (uint item, uint subitem);
	void	notifyParentChange ();
	void	insertColumn (uint id, const char*name);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditListCtrl)
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void OnSetFocus( CWnd* pOldWnd );
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditListCtrl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDIT_LIST_CTRL_H__49812C0E_3696_49D3_92A9_6AF18E4FD689__INCLUDED_)
