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

#if !defined(AFX_PAGECOMPLEX_H__6EB5AE57_5E13_4028_9985_04EF86FB7AE6__INCLUDED_)
#define AFX_PAGECOMPLEX_H__6EB5AE57_5E13_4028_9985_04EF86FB7AE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageComplex.h : header file
//
#include "PageBase.h"


/////////////////////////////////////////////////////////////////////////////
// CPageComplex dialog

class CPageComplex : public CPageBase
{
	DECLARE_DYNCREATE(CPageComplex)

	void onDocChanged();

// Construction
public:
	CPageComplex(){}
	CPageComplex(NLGEORGES::CSoundDialog *soundDialog);
	~CPageComplex();

// Dialog Data
	//{{AFX_DATA(CPageComplex)
	enum { IDD = IDD_PAGE_COMPLEX };
	CButton	_BtnRandomSound;
	CButton	_BtnRandomDelay;
	int		_SequenceSize;
	int		_MaxDelay;
	int		_MinDelay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageComplex)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageComplex)
	afx_msg void OnBtnRandomDelay();
	afx_msg void OnBtnRandomSound();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGECOMPLEX_H__6EB5AE57_5E13_4028_9985_04EF86FB7AE6__INCLUDED_)
