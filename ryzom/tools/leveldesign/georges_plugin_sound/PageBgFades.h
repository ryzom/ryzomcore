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

#if !defined(AFX_PAGEBGFADES_H__AED887E2_E8D1_4BDB_B180_B6ADF8C44D58__INCLUDED_)
#define AFX_PAGEBGFADES_H__AED887E2_E8D1_4BDB_B180_B6ADF8C44D58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageBgFades.h : header file
//
#include "PageBase.h"

/////////////////////////////////////////////////////////////////////////////
// CPageBgFades dialog

class CPageBgFades : public CPageBase
{
	DECLARE_DYNCREATE(CPageBgFades)

// Construction
public:
	CPageBgFades() {}
	CPageBgFades(NLGEORGES::CSoundDialog *soundDialog);
	~CPageBgFades();

// Dialog Data
	//{{AFX_DATA(CPageBgFades)
	enum { IDD = IDD_PAGE_BG_FADES };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageBgFades)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageBgFades)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGEBGFADES_H__AED887E2_E8D1_4BDB_B180_B6ADF8C44D58__INCLUDED_)
