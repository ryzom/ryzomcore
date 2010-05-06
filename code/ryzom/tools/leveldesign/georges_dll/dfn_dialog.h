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

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#if !defined(DFN_TYPE_H_INCLUDED)
#define DFN_TYPE_H_INCLUDED

#include "base_dialog.h"
#include "edit_list_ctrl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dfn_dialog.h : header file
//

class CGeorgesEditView;

/////////////////////////////////////////////////////////////////////////////
// CDfnDialog dialog

class CDfnParentEditListCtrl : public CEditListCtrl
{
	CEditListCtrl::TItemEdit	getItemEditMode (uint item, uint subItem);
	void						getMemComboBoxProp (uint item, uint subItem, std::string &regAdr, bool &browse);
	void						getNewItemText (uint item, uint subItem, std::string &ret);
	void						getBrowseInfo (uint item, uint subItem, std::string &defExt, std::string &defFilename, std::string &defDir, std::string &filter);
public:
	class CDfnDialog			*Dialog;
};

class CDfnEditListCtrl : public CEditListCtrl
{
	CEditListCtrl::TItemEdit	getItemEditMode (uint item, uint subItem);
	void						getComboBoxStrings (uint item, uint subItem, std::vector<std::string> &retStrings);
	void						getMemComboBoxProp (uint item, uint subItem, std::string &regAdr, bool &browse);
	void						getNewItemText (uint item, uint subItem, std::string &ret);
	void						getBrowseInfo (uint item, uint subItem, std::string &defExt, std::string &defFilename, std::string &defDir, std::string &filter);
	void						onItemChanged (uint item, uint subItem);
public:
	class CDfnDialog			*Dialog;
};

/**
  *  The dialog class for a DFN edition right window.
  */
class CDfnDialog : public CBaseDialog
{
// Construction
public:
	CDfnDialog ();   // standard constructor

	enum
	{
		TypeCombo= 100,
	};

	enum
	{
		LtParents,
		LtStruct,
	};

	enum
	{
		ParentHeight = 80,
		DfnHeight = 80,
	};

	// The widgets
	CStatic					LabelParents;
	CDfnParentEditListCtrl	Parents;
	CStatic					LabelStruct;
	CDfnEditListCtrl		Struct;

	// From CDialog
	virtual void OnOK ();
	virtual void OnCancel ();
	
	// From CBaseDialog
	void onOpenSelected ();
	void onFirstFocus ();
	void onLastFocus ();

	// Get from document, update rightview UI
	void getFromDocument (const NLGEORGES::CFormDfn &type);

	// Set to document, update document with rightview UI
	void setParentsToDocument ();
	void setStructToDocument ();

	// Resize widget callback
	void resizeWidgets ();

	CWnd* GetNextDlgTabItem( CWnd* pWndCtl, BOOL bPrevious = FALSE ) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDfnDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDfnDialog)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnInitDialog();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(DFN_TYPE_H_INCLUDED)
