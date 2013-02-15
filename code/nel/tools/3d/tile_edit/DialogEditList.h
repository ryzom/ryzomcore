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

#if !defined(AFX_DIALOGEDITLIST_H__69173FE0_A38B_11D4_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_DIALOGEDITLIST_H__69173FE0_A38B_11D4_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogEditList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogEditList dialog

class CDialogEditList : public CDialog
{
// Construction
public:
	CDialogEditList(CWnd* pParent = NULL);   // standard constructor

	virtual void OnInit ()=0;
	virtual void OnOk ()=0;

// Dialog Data
	//{{AFX_DATA(CDialogEditList)
	enum { IDD = IDD_EDIT_LIST };
	CListBox	m_ctrlList;
	CComboBox	m_ctrlCombo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogEditList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogEditList)
	afx_msg void OnAdd();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGEDITLIST_H__69173FE0_A38B_11D4_9CD4_0050DAC3A412__INCLUDED_)
