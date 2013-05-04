// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#if !defined(AFX_SELECT_STRING_H__A9ECE120_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_SELECT_STRING_H__A9ECE120_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// select_string.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectString dialog

class CSelectString : public CDialog
{
// Construction
public:
	CSelectString(const std::vector<std::string>& vectString, const char* title, CWnd* pParent, bool empty);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectString)
	enum { IDD = IDD_SELECT_STRING };
	CButton		EmptyCtrl;
	CListBox	ListCtrl;
	//}}AFX_DATA

	std::string							Title;
	std::vector<std::string>			Strings;
	int									Selection;
	bool								Empty;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectString)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectString)
	virtual void OnOK();
	afx_msg void OnDblclkList();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList();
	afx_msg void OnEmpty();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECT_STRING_H__A9ECE120_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_)
