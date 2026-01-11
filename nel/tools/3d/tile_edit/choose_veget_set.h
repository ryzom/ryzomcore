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

#if !defined(AFX_CHOOSE_VEGET_SET_H__EA817A1B_515D_4FD8_984F_35155195041C__INCLUDED_)
#define AFX_CHOOSE_VEGET_SET_H__EA817A1B_515D_4FD8_984F_35155195041C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// choose_veget_set.h : header file
//

#include <string>

class SelectionTerritoire;

/////////////////////////////////////////////////////////////////////////////
// CChooseVegetSet dialog

class CChooseVegetSet : public CDialog
{
// Construction
public:
	CChooseVegetSet(SelectionTerritoire* pParent, const std::string &oldFile);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChooseVegetSet)
	enum { IDD = IDD_CHOOSE_VEGET };
	CButton	Name;
	//}}AFX_DATA

	SelectionTerritoire		*Parent;
	std::string				FileName;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseVegetSet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseVegetSet)
	afx_msg void OnBrowse();
	virtual BOOL OnInitDialog();
	afx_msg void OnReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSE_VEGET_SET_H__EA817A1B_515D_4FD8_984F_35155195041C__INCLUDED_)
