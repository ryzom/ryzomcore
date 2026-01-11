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

// DayNightDlg.cpp : implementation file
//

#include "std_afx.h"

#include "object_viewer.h"
#include "day_night_dlg.h"
#include "nel/3d/water_pool_manager.h"
#include "nel/3d/nelu.h"




/////////////////////////////////////////////////////////////////////////////
// CDayNightDlg dialog


CDayNightDlg::CDayNightDlg(CObjectViewer *viewer, CWnd* pParent /*=NULL*/)
	: CDialog(CDayNightDlg::IDD, pParent), _ObjView(viewer)
{
	nlassert(viewer);
	//{{AFX_DATA_INIT(CDayNightDlg)
	//}}AFX_DATA_INIT
}


void CDayNightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDayNightDlg)
	DDX_Control(pDX, IDC_SCROLLBAR1, m_DayNightCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDayNightDlg, CDialog)
	//{{AFX_MSG_MAP(CDayNightDlg)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDayNightDlg message handlers

void CDayNightDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (nSBCode == SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK)
	{
		UpdateData(TRUE);
		if (pScrollBar == &m_DayNightCtrl)
		{
			_ObjView->getWaterPoolManager().setBlendFactor(NL3D::CNELU::Driver, nPos * (1.f / 255.f) );
			m_DayNightCtrl.SetScrollPos(nPos) ;
		}	
	}
	UpdateData(FALSE) ;
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CDayNightDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	m_DayNightCtrl.SetScrollRange(0, 255);	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
