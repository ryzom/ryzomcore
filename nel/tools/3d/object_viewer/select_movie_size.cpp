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

// SelectMovieSize.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "select_movie_size.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectMovieSize dialog


CSelectMovieSize::CSelectMovieSize(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectMovieSize::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectMovieSize)
	Width = 800;
	Height = 600;
	//}}AFX_DATA_INIT
}


void CSelectMovieSize::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectMovieSize)
	DDX_Text(pDX, IDC_WIDTH, Width);
	DDX_Text(pDX, IDC_HEIGHT, Height);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectMovieSize, CDialog)
	//{{AFX_MSG_MAP(CSelectMovieSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectMovieSize message handlers

void CSelectMovieSize::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}
