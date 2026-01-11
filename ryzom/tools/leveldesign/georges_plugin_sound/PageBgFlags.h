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

#if !defined(AFX_PAGEBGFLAGS_H__302480C3_66F1_460C_BC59_B468ACBD55B2__INCLUDED_)
#define AFX_PAGEBGFLAGS_H__302480C3_66F1_460C_BC59_B468ACBD55B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageBgFlags.h : header file
//
#include "nel/misc/types_nl.h"
#include "PageBase.h"

/////////////////////////////////////////////////////////////////////////////
// CPageBgFlags dialog

class CPageBgFlags : public CPageBase
{
	DECLARE_DYNCREATE(CPageBgFlags)

	// called by the master dialog when doc change
	void onDocChanged();

	// called by this dialog when data must be updated
	void updateData(bool updateEditFilter);


	/// Index of the current edited sound the the background
	sint	_Index;
	/// Flag to block updating will modifying
	bool	_recurse;

// Construction
public:
	CPageBgFlags() {}
	CPageBgFlags(NLGEORGES::CSoundDialog *soundDialog);
	~CPageBgFlags();

// Dialog Data
	//{{AFX_DATA(CPageBgFlags)
	enum { IDD = IDD_PAGE_BG_FLAGS };
	CString	_SubSoundName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageBgFlags)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageBgFlags)
	afx_msg void OnBtnEditAllOn();
	afx_msg void OnBtnEditAllOff();
	afx_msg void OnBtnEnvAllOff();
	afx_msg void OnBtnEnvAllOn();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGEBGFLAGS_H__302480C3_66F1_460C_BC59_B468ACBD55B2__INCLUDED_)
