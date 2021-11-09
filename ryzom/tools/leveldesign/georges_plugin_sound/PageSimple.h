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

#if !defined(AFX_PAGESIMPLE_H__21AC2079_9B99_4F63_AFC8_4DFF4930E31D__INCLUDED_)
#define AFX_PAGESIMPLE_H__21AC2079_9B99_4F63_AFC8_4DFF4930E31D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageSimple.h : header file
//

#include "PageBase.h"

/////////////////////////////////////////////////////////////////////////////
// CPageSimple dialog

class CPageSimple : public CPageBase
{
	DECLARE_DYNCREATE(CPageSimple)

	void onDocChanged();

// Construction
public:
	CPageSimple() {}
	CPageSimple(NLGEORGES::CSoundDialog *soundDialog);
	~CPageSimple();

// Dialog Data
	//{{AFX_DATA(CPageSimple)
	enum { IDD = IDD_PAGE_SIMPLE };
	CStatic	_FilesizeCtrl;
	CStatic	_FilenameCtrl;
	CStatic	_AudioFormatCtrl;
	CString	_AudioFormat;
	CString	_Filename;
	CString	_Filesize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageSimple)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageSimple)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGESIMPLE_H__21AC2079_9B99_4F63_AFC8_4DFF4930E31D__INCLUDED_)
