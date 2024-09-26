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

#if !defined(AFX_PAGEPOSITION_H__234F9FB4_3803_4973_BC3D_8D398A9C438A__INCLUDED_)
#define AFX_PAGEPOSITION_H__234F9FB4_3803_4973_BC3D_8D398A9C438A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PagePosition.h : header file
//
#include "PageBase.h"

namespace NLGEORGES
{
	class CListenerView;
}


/////////////////////////////////////////////////////////////////////////////
// CPagePosition dialog

class CPagePosition : public CPageBase
{
	DECLARE_DYNCREATE(CPagePosition)

	void onDocChanged();

// Construction
public:
	CPagePosition(){}
	CPagePosition(NLGEORGES::CSoundDialog *soundDialog);
	~CPagePosition();

// Dialog Data
	//{{AFX_DATA(CPagePosition)
	enum { IDD = IDD_PAGE_POSITION };
	CStatic	_Picture;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPagePosition)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPagePosition)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	NLGEORGES::CListenerView	*_ListenerView;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGEPOSITION_H__234F9FB4_3803_4973_BC3D_8D398A9C438A__INCLUDED_)
