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

// select_by_location.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "select_by_location.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectByLocation dialog


CSelectByLocation::CSelectByLocation(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectByLocation::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectByLocation)
	X = 0.0f;
	Y = 0.0f;
	Threshold = 10.0f;
	//}}AFX_DATA_INIT
}


void CSelectByLocation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectByLocation)
	DDX_Text(pDX, IDC_X, X);
	DDX_Text(pDX, IDC_Y, Y);
	DDX_Text(pDX, IDC_THRESHOLD, Threshold);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectByLocation, CDialog)
	//{{AFX_MSG_MAP(CSelectByLocation)
	ON_BN_CLICKED(IDMORE, OnMore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectByLocation message handlers

void CSelectByLocation::OnOK() 
{
	SelectMore = false;
	
	CDialog::OnOK();
}

void CSelectByLocation::OnMore() 
{
	SelectMore = true;
	
	CDialog::OnOK();
}

BOOL CSelectByLocation::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
