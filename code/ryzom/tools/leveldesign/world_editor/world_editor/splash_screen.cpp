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

// splash_screen.cpp : implementation file
//

#include "stdafx.h"
#include "world_editor.h"
#include "splash_screen.h"
#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CSplashScreen dialog


CSplashScreen::CSplashScreen(CWnd* pParent /*=NULL*/)
	: CDialog(CSplashScreen::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSplashScreen)
	m_load_list = _T("");
	//}}AFX_DATA_INIT
}


void CSplashScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSplashScreen)
	DDX_Text(pDX, IDC_LOAD_WORLD_EDIT, m_load_list);
	//}}AFX_DATA_MAP
}

void CSplashScreen::addLine(std::string newLine)
{
	m_load_list=m_load_list+"\n"+newLine.c_str();
	UpdateData(false);
	ShowWindow(true);
	RedrawWindow();
}
void CSplashScreen::close()
{
	OnClose();
}


void CSplashScreen::progress (float progressValue)
{
	MSG	msg;
	while ( PeekMessage(&msg,NULL,0,0,PM_REMOVE) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
}

BEGIN_MESSAGE_MAP(CSplashScreen, CDialog)
	//{{AFX_MSG_MAP(CSplashScreen)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSplashScreen message handlers
