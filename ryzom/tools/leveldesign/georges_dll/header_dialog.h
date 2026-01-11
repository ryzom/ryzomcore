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

#if !defined(HEADER_TYPE_H)
#define HEADER_TYPE_H

#include "base_dialog.h"
#include "edit_list_ctrl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// type_dialog.h : header file
//

namespace NLGEORGES
{
class CFileHeader;
};

class CGeorgesEditView;

/////////////////////////////////////////////////////////////////////////////
// CHeaderDialog dialog

class CHeaderDialog : public CBaseDialog
{
// Construction
public:
	CHeaderDialog ();   // standard constructor

	enum
	{
		TypeCombo= 100,
	};

	enum
	{
		CbState = 1,
		BtIncrement,
		EdComments,
		EdLog,
	};

	// The widgets
	CStatic			LabelVersion;
	CButton			IncrementVersion;
	CStatic			LabelState;
	CComboBox		ComboState;
	CStatic			LabelComments;
	CEdit		 	Comments;
	CStatic			LabelLog;
	CEdit		 	Log;

	// From CBaseDialog
	void onLastFocus ();
	void onFirstFocus ();

	// Get from document, update rightview UI
	void getFromDocument (const NLGEORGES::CFileHeader &header);

	// Set to document, update document with rightview UI
	void setStateToDocument ();
	void setVersionToDocument ();
	void setCommentsToDocument ();

	// Resize widget callback
	void resizeWidgets ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHeaderDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CHeaderDialog)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(HEADER_TYPE_H)
