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


//

#if !defined(AFX_EDIT_USER_PARAM_H__DF77F873_E367_4BCE_9E13_D8706ADCFD67__INCLUDED_)
#define AFX_EDIT_USER_PARAM_H__DF77F873_E367_4BCE_9E13_D8706ADCFD67__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// edit_user_param.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditUserParam dialog

class CEditUserParam : public CDialog
{
// Construction
public:
	CEditUserParam(uint32 userParamIndex, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditUserParam)
	enum { IDD = IDD_EDIT_USER_PARAM };
	CComboBox	m_UserParamIndex;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditUserParam)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	public:
		uint32 getUserParamIndex(void) const { return _UserParamIndex ; }
// Implementation
protected:
	uint32 _UserParamIndex ;
	// Generated message map functions
	//{{AFX_MSG(CEditUserParam)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeUserParamIndex();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDIT_USER_PARAM_H__DF77F873_E367_4BCE_9E13_D8706ADCFD67__INCLUDED_)
