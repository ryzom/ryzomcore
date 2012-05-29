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

#if !defined(AFX_TUNE_MRM_DLG_H__909B0EF0_8158_431F_91DC_B77D380DA7A0__INCLUDED_)
#define AFX_TUNE_MRM_DLG_H__909B0EF0_8158_431F_91DC_B77D380DA7A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// tune_mrm_dlg.h : header file
//

#include "nel/3d/scene.h"

/////////////////////////////////////////////////////////////////////////////
// CTuneMrmDlg dialog

class CTuneMrmDlg : public CDialog
{
// Construction
public:
	CTuneMrmDlg(CObjectViewer *viewer, NL3D::CScene *scene, CWnd* pParent = NULL);   // standard constructor
	~CTuneMrmDlg();

// Dialog Data
	//{{AFX_DATA(CTuneMrmDlg)
	enum { IDD = IDD_TUNE_MRM_DLG };
	CSliderCtrl	MaxValueSlider;
	CSliderCtrl	FaceSlider;
	CString	ViewCurrentMaxPoly;
	CString	ViewMaxValue;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTuneMrmDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTuneMrmDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnReleasedcaptureTmdSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnReleasedcaptureTmdSliderMax(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void	applySlider(UINT sliderCur, UINT sliderMax);

	CObjectViewer	*_ObjViewer;
	NL3D::CScene	*_Scene;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TUNE_MRM_DLG_H__909B0EF0_8158_431F_91DC_B77D380DA7A0__INCLUDED_)
