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

#if !defined(AFX_ANIMATION_SET_DLG_H__AA6100E2_19FA_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_ANIMATION_SET_DLG_H__AA6100E2_19FA_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// animation_set_dlg.h : header file
//

#include <nel/3d/animation_set.h>

/////////////////////////////////////////////////////////////////////////////
// CAnimationSetDlg dialog

class CAnimationSetDlg : public CDialog
{
	friend class CObjectViewer;
// Construction
public:
	CAnimationSetDlg (class CObjectViewer* objView, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAnimationSetDlg)
	enum { IDD = IDD_ANIMATION_SET };
	CComboBox	EditedObject;
	CListBox	PlayList;
	CTreeCtrl	SkelTree;
	CTreeCtrl	Tree;
	int			UseMixer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimationSetDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Load an animation
	virtual void refresh (BOOL update);

	// Pointer on an animation set
	CObjectViewer*		_ObjView;

	// Generated message map functions
	//{{AFX_MSG(CAnimationSetDlg)
	afx_msg void OnAddAnimation();
	afx_msg void OnReset();
	afx_msg void OnAddSkelWt();
	afx_msg void OnDestroy();
	afx_msg void OnListInsert();
	afx_msg void OnListUp();
	afx_msg void OnListDown();
	afx_msg void OnListDelete();
	afx_msg void OnSetAnimLength();
	afx_msg void OnUseList();
	afx_msg void OnUseMixer();
	afx_msg void OnSelchangeEditedObject();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMATION_SET_DLG_H__AA6100E2_19FA_11D5_9CD4_0050DAC3A412__INCLUDED_)
