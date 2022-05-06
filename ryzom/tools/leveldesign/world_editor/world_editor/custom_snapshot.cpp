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

// custom_snapshot.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "custom_snapshot.h"

/////////////////////////////////////////////////////////////////////////////
// CCustomSnapshot dialog


CCustomSnapshot::CCustomSnapshot(CWnd* pParent /*=NULL*/)
	: CDialog(CCustomSnapshot::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCustomSnapshot)
	Height = 512;
	Width = 512;
	KeepRatio = TRUE;
	FixedSize = 0;
	OutputRGB = 0;
	//}}AFX_DATA_INIT
}


void CCustomSnapshot::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomSnapshot)
	DDX_Control(pDX, IDC_KEEPRATIO, KeepRatioCtrl);
	DDX_Control(pDX, IDC_HEIGHT, HeightCtrl);
	DDX_Control(pDX, IDC_WIDTH, WidthCtrl);
	DDX_Text(pDX, IDC_HEIGHT, Height);
	DDV_MinMaxUInt(pDX, Height, 1, 65535);
	DDX_Text(pDX, IDC_WIDTH, Width);
	DDV_MinMaxUInt(pDX, Width, 1, 65535);
	DDX_Check(pDX, IDC_KEEPRATIO, KeepRatio);
	DDX_Radio(pDX, IDC_FIXED_SIZE, FixedSize);
	DDX_Radio(pDX, IDC_RGBA, OutputRGB);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomSnapshot, CDialog)
	//{{AFX_MSG_MAP(CCustomSnapshot)
	ON_BN_CLICKED(IDC_KEEPRATIO, OnKeepratio)
	ON_BN_CLICKED(IDC_FIXED_SIZE, OnFixedSize)
	ON_BN_CLICKED(IDC_GRAY_SCALE2, OnGrayScale2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomSnapshot message handlers

// ***************************************************************************

void CCustomSnapshot::OnKeepratio() 
{
	updateData ();
}

// ***************************************************************************

void CCustomSnapshot::OnFixedSize() 
{
	updateData ();
}

// ***************************************************************************

void CCustomSnapshot::OnGrayScale2() 
{
	updateData ();
}

// ***************************************************************************

BOOL CCustomSnapshot::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	updateData ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void CCustomSnapshot::updateData ()
{
	UpdateData ();
	BOOL enable = FixedSize != 0;
	WidthCtrl.EnableWindow (enable);
	HeightCtrl.EnableWindow (enable && !KeepRatio);
	KeepRatioCtrl.EnableWindow (enable);
	UpdateData (FALSE);
}

// ***************************************************************************
