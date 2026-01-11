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

// global_wind_dlg.cpp : implementation file
//

#include "std_afx.h"

#include "object_viewer.h"
#include "global_wind_dlg.h"


#define	NL_GLOBAL_WIND_SLIDER_RANGE		1000

/////////////////////////////////////////////////////////////////////////////
// CGlobalWindDlg dialog


CGlobalWindDlg::CGlobalWindDlg(CObjectViewer *objViewer, CWnd* pParent /*=NULL*/)
	: CDialog(CGlobalWindDlg::IDD, pParent), _ObjViewer(objViewer)
{
	//{{AFX_DATA_INIT(CGlobalWindDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGlobalWindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalWindDlg)
	DDX_Control(pDX, IDC_STATIC_GLOBAL_WIND_POWER, StaticPower);
	DDX_Control(pDX, IDC_SLIDER_GLOBAL_WIND_POWER, SliderPower);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalWindDlg, CDialog)
	//{{AFX_MSG_MAP(CGlobalWindDlg)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_GLOBAL_WIND_POWER, OnReleasedcaptureSliderGlobalWindPower)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ***************************************************************************
void	CGlobalWindDlg::updateView()
{
	float	a;
	TCHAR	str[256];

	// update Power.
	a= _ObjViewer->getGlobalWindPower();
	_stprintf(str, _T("%.2f"), a);
	StaticPower.SetWindowText(str);
	NLMISC::clamp(a, 0.f, 1.f);
	SliderPower.SetPos((sint)(a*NL_GLOBAL_WIND_SLIDER_RANGE));
}

// ***************************************************************************
// ***************************************************************************
// CVegetableWindDlg message handlers
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
BOOL CGlobalWindDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SliderPower.SetRange(0, NL_GLOBAL_WIND_SLIDER_RANGE);
	
	// Init them and static.
	updateView();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************
void CGlobalWindDlg::OnReleasedcaptureSliderGlobalWindPower(NMHDR* pNMHDR, LRESULT* pResult) 
{
	float	a;
	a= (float)SliderPower.GetPos() / NL_GLOBAL_WIND_SLIDER_RANGE;
	_ObjViewer->setGlobalWindPower(a);

	// refersh.
	updateView();
	
	*pResult = 0;
}

// ***************************************************************************
void CGlobalWindDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// test if one of my sliders.
	CSliderCtrl	*sliderCtrl= (CSliderCtrl*)pScrollBar;
	if( sliderCtrl==&SliderPower && nSBCode==SB_THUMBPOSITION || nSBCode==SB_THUMBTRACK)
	{
		float	a;
		TCHAR	str[256];

		a= (float)nPos / NL_GLOBAL_WIND_SLIDER_RANGE;
		_ObjViewer->setGlobalWindPower(a);
		_stprintf(str, _T("%.2f"), a);
		StaticPower.SetWindowText(str);
	}
	else
	{
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	}
}

void CGlobalWindDlg::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_OBJ_GLOBAL_WIND_DLG);

	CDialog::OnDestroy();
}
