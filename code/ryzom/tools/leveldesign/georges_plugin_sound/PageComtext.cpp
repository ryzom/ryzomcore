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

// PageComtext.cpp : implementation file
//

#include "std_sound_plugin.h"
#include "georges_plugin_sound.h"
#include <map>
#include "PageComtext.h"
#include "resource.h"

using namespace std;

uint EDIT_ARG_ID [] = 
{
	IDC_EDIT_ARG_0,
	IDC_EDIT_ARG_1,
	IDC_EDIT_ARG_2,
	IDC_EDIT_ARG_3,
	IDC_EDIT_ARG_4,
	IDC_EDIT_ARG_5,
	IDC_EDIT_ARG_6,
	IDC_EDIT_ARG_7,
	IDC_EDIT_ARG_8,
	IDC_EDIT_ARG_9
};

map<uint, uint>	EDIT_ARG_IDX;

/////////////////////////////////////////////////////////////////////////////
// CPageComtext property page

#undef new
IMPLEMENT_DYNCREATE(CPageComtext, CPageBase)
#define new NL_NEW

CPageComtext::CPageComtext(NLGEORGES::CSoundDialog *soundDialog) : CPageBase(soundDialog, CPageComtext::IDD)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//{{AFX_DATA_INIT(CPageComtext)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	for (uint i =0; i<10; ++i)
	{
		EDIT_ARG_IDX.insert(make_pair(EDIT_ARG_ID[i], i));

		// init the context sound vars
		SoundContext.Args[i] = 0;
	}
}

CPageComtext::~CPageComtext()
{
}

void CPageComtext::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageComtext)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageComtext, CPropertyPage)
	//{{AFX_MSG_MAP(CPageComtext)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageComtext message handlers

BOOL CPageComtext::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (lParam != 0 && HIWORD(wParam) == EN_CHANGE)
	{
		int id = ::GetDlgCtrlID(HWND(lParam));
		// command come from a control
		if (EDIT_ARG_IDX.find(id) != EDIT_ARG_IDX.end())
		{
			// need to update the var.
			char tmp[1024];
			GetDlgItem(id)->GetWindowText(tmp, 1024);
			SoundContext.Args[EDIT_ARG_IDX[id]] = atoi(tmp);
		}
	}
	
	return CPropertyPage::OnCommand(wParam, lParam);
}

