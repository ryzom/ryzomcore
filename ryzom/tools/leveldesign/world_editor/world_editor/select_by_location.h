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

#if !defined(AFX_SELECT_BY_LOCATION_H__2DC6A9DF_4564_4572_B570_63A4CC9EFD51__INCLUDED_)
#define AFX_SELECT_BY_LOCATION_H__2DC6A9DF_4564_4572_B570_63A4CC9EFD51__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// select_by_location.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectByLocation dialog

class CSelectByLocation : public CDialog
{
// Construction
public:
	CSelectByLocation(CWnd* pParent = NULL);   // standard constructor

	bool SelectMore;

// Dialog Data
	//{{AFX_DATA(CSelectByLocation)
	enum { IDD = IDD_SELECT_PRIMITIVE_BY_LOCATION };
	float	X;
	float	Y;
	float	Threshold;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectByLocation)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectByLocation)
	virtual void OnOK();
	afx_msg void OnMore();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECT_BY_LOCATION_H__2DC6A9DF_4564_4572_B570_63A4CC9EFD51__INCLUDED_)
