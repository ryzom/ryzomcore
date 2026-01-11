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

#if !defined(AFX_PRIMITIVE_CONFIGURATION_DLG_H__91B0FD73_97BA_4A00_A664_2F0BE6D52573__INCLUDED_)
#define AFX_PRIMITIVE_CONFIGURATION_DLG_H__91B0FD73_97BA_4A00_A664_2F0BE6D52573__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// primitive_configuration_dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrimitiveConfigurationDlg dialog

class CPrimitiveConfigurationDlg : public CDialog
{
// Construction
public:
	CPrimitiveConfigurationDlg(CWnd* pParent = NULL);   // standard constructor

	// Destroy
	void destroy ();

// Dialog Data
	//{{AFX_DATA(CPrimitiveConfigurationDlg)
	enum { IDD = IDD_PRIMITIVE_CONFIGURATION };
	CListCtrl	ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrimitiveConfigurationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// The id of the last item selected for popup menu callbacks
	static uint	LastId;

// Implementation
protected:

	// Backup list position for resize
	uint _WidthMargin, _HeightMargin;

	// Iterate all the primitive
	enum TIterateAction
	{
		Show = 0,
		Hide,
		Select
	};

	// Iterate each primitive and do the action. Update main frame at the end.
	void iteratePrimitives(TIterateAction iterateAction);

	// Generated message map functions
	//{{AFX_MSG(CPrimitiveConfigurationDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPrimitiveconfigurationHide();
	afx_msg void OnPrimitiveconfigurationSelect();
	afx_msg void OnPrimitiveconfigurationShow();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CPrimitiveConfigurationDlg	PrimitiveConfigurationDlg;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRIMITIVE_CONFIGURATION_DLG_H__91B0FD73_97BA_4A00_A664_2F0BE6D52573__INCLUDED_)
