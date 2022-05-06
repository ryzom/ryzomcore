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

// system_information_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "client_config.h"
#include "system_information_dlg.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************
// CSystemInformationDlg dialog
// ***************************************************************************

CSystemInformationDlg::CSystemInformationDlg(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CSystemInformationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSystemInformationDlg)
	//}}AFX_DATA_INIT
}

// ***************************************************************************

void CSystemInformationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSystemInformationDlg)
	DDX_Control(pDX, IDC_SYS_INFO_VIDEO, VideoCtrl);
	DDX_Control(pDX, IDC_SYS_INFO_PHY_MEM, PhysicalMemoryCtrl);
	DDX_Control(pDX, IDC_SYS_INFO_VIDEO_DRIVER, VideoDriverCtrl);
	DDX_Control(pDX, IDC_SYS_INFO_OS, OSCtrl);
	DDX_Control(pDX, IDC_SYS_INFO_CPU, CPUCtrl);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CSystemInformationDlg, CDialog)
	//{{AFX_MSG_MAP(CSystemInformationDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CSystemInformationDlg message handlers
// ***************************************************************************

BOOL CSystemInformationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	string deviceName;
	uint64 driverVersion;
	if (CSystemInfo::getVideoInfo (deviceName, driverVersion))
	{
		VideoCtrl.SetWindowText (deviceName.c_str ());
		uint32 version = (uint32)(driverVersion&0xffff);
		VideoDriverCtrl.SetWindowText (toString ("%d.%d.%d.%d", version/1000%10, version/100%10, version/10%10, version%10).c_str ());
	}
	PhysicalMemoryCtrl.SetWindowText (bytesToHumanReadable(CSystemInfo::totalPhysicalMemory()).c_str ());
	OSCtrl.SetWindowText (CSystemInfo::getOS().c_str ());
	CPUCtrl.SetWindowText (CSystemInfo::getProc().c_str ());
	
	UpdateData (TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
