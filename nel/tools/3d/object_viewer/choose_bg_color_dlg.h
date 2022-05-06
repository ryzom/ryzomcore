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

#if !defined(AFX_CHOOSE_BG_COLOR_DLG_H__F340B75A_1593_42D0_9D11_C569053B03D0__INCLUDED_)
#define AFX_CHOOSE_BG_COLOR_DLG_H__F340B75A_1593_42D0_9D11_C569053B03D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// choose_bg_color_dlg.h : header file
//

#include "ps_wrapper.h"


class CColorEdit;

/////////////////////////////////////////////////////////////////////////////
// CChooseBGColorDlg dialog

class CChooseBGColorDlg : public CDialog
{
// Construction
public:
	CChooseBGColorDlg(CObjectViewer *objectViewer, CWnd* pParent);   // standard constructor
	~CChooseBGColorDlg();

// Dialog Data
	//{{AFX_DATA(CChooseBGColorDlg)
	enum { IDD = IDD_CHOOSE_BG_COLOR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseBGColorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CColorEdit *_ColorEdit;
	struct CBGColorWrapper : public IPSWrapperRGBA
	{
		CObjectViewer *OV;
		void set(const NLMISC::CRGBA &col) { OV->setBackGroundColor(col); }
		NLMISC::CRGBA get() const  { return OV->getBackGroundColor(); }
	} _BGColorWrapper;
	// Generated message map functions
	//{{AFX_MSG(CChooseBGColorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSE_BG_COLOR_DLG_H__F340B75A_1593_42D0_9D11_C569053B03D0__INCLUDED_)
