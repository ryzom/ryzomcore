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

// PageBgFades.cpp : implementation file
//

#include "std_sound_plugin.h"
#include "georges_plugin_sound.h"
#include "nel/misc/types_nl.h"
#include "sound_dialog.h"
#include "sound_plugin.h"
#include "nel/sound/u_audio_mixer.h"

#include "PageBgFades.h"
#include "resource.h"

using namespace std;

uint FILTER_NAMES[] =
{
	IDC_FILTER_NAME_00,
	IDC_FILTER_NAME_01,
	IDC_FILTER_NAME_02,
	IDC_FILTER_NAME_03,
	IDC_FILTER_NAME_04,
	IDC_FILTER_NAME_05,
	IDC_FILTER_NAME_06,
	IDC_FILTER_NAME_07,
	IDC_FILTER_NAME_08,
	IDC_FILTER_NAME_09,
	IDC_FILTER_NAME_10,
	IDC_FILTER_NAME_11,
	IDC_FILTER_NAME_12,
	IDC_FILTER_NAME_13,
	IDC_FILTER_NAME_14,
	IDC_FILTER_NAME_15,
	IDC_FILTER_NAME_16,
	IDC_FILTER_NAME_17,
	IDC_FILTER_NAME_18,
	IDC_FILTER_NAME_19,
	IDC_FILTER_NAME_20,
	IDC_FILTER_NAME_21,
	IDC_FILTER_NAME_22,
	IDC_FILTER_NAME_23,
	IDC_FILTER_NAME_24,
	IDC_FILTER_NAME_25,
	IDC_FILTER_NAME_26,
	IDC_FILTER_NAME_27,
	IDC_FILTER_NAME_28,
	IDC_FILTER_NAME_29,
	IDC_FILTER_NAME_30,
	IDC_FILTER_NAME_31
};

uint FILTER_FADE_IN[] = 
{
	IDC_FADE_IN_00,
	IDC_FADE_IN_01,
	IDC_FADE_IN_02,
	IDC_FADE_IN_03,
	IDC_FADE_IN_04,
	IDC_FADE_IN_05,
	IDC_FADE_IN_06,
	IDC_FADE_IN_07,
	IDC_FADE_IN_08,
	IDC_FADE_IN_09,
	IDC_FADE_IN_10,
	IDC_FADE_IN_11,
	IDC_FADE_IN_12,
	IDC_FADE_IN_13,
	IDC_FADE_IN_14,
	IDC_FADE_IN_15,
	IDC_FADE_IN_16,
	IDC_FADE_IN_17,
	IDC_FADE_IN_18,
	IDC_FADE_IN_19,
	IDC_FADE_IN_20,
	IDC_FADE_IN_21,
	IDC_FADE_IN_22,
	IDC_FADE_IN_23,
	IDC_FADE_IN_24,
	IDC_FADE_IN_25,
	IDC_FADE_IN_26,
	IDC_FADE_IN_27,
	IDC_FADE_IN_28,
	IDC_FADE_IN_29,
	IDC_FADE_IN_30,
	IDC_FADE_IN_31
};

uint FILTER_FADE_OUT[] =
{
	IDC_FADE_OUT_00,
	IDC_FADE_OUT_01,
	IDC_FADE_OUT_02,
	IDC_FADE_OUT_03,
	IDC_FADE_OUT_04,
	IDC_FADE_OUT_05,
	IDC_FADE_OUT_06,
	IDC_FADE_OUT_07,
	IDC_FADE_OUT_08,
	IDC_FADE_OUT_09,
	IDC_FADE_OUT_10,
	IDC_FADE_OUT_11,
	IDC_FADE_OUT_12,
	IDC_FADE_OUT_13,
	IDC_FADE_OUT_14,
	IDC_FADE_OUT_15,
	IDC_FADE_OUT_16,
	IDC_FADE_OUT_17,
	IDC_FADE_OUT_18,
	IDC_FADE_OUT_19,
	IDC_FADE_OUT_20,
	IDC_FADE_OUT_21,
	IDC_FADE_OUT_22,
	IDC_FADE_OUT_23,
	IDC_FADE_OUT_24,
	IDC_FADE_OUT_25,
	IDC_FADE_OUT_26,
	IDC_FADE_OUT_27,
	IDC_FADE_OUT_28,
	IDC_FADE_OUT_29,
	IDC_FADE_OUT_30,
	IDC_FADE_OUT_31,
};

map<uint, uint>	FILTER_FADE_IN_IDX;
map<uint, uint>	FILTER_FADE_OUT_IDX;

/////////////////////////////////////////////////////////////////////////////
// CPageBgFades property page

#undef new
IMPLEMENT_DYNCREATE(CPageBgFades, CPageBase)
#define new NL_NEW

CPageBgFades::CPageBgFades(NLGEORGES::CSoundDialog *soundDialog)
 : CPageBase(soundDialog, CPageBgFades::IDD)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//{{AFX_DATA_INIT(CPageBgFades)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	for (uint i=0; i<32; ++i)
	{
		FILTER_FADE_IN_IDX.insert(make_pair(FILTER_FADE_IN[i], i));
		FILTER_FADE_OUT_IDX.insert(make_pair(FILTER_FADE_OUT[i], i));
	}
}

CPageBgFades::~CPageBgFades()
{
}

void CPageBgFades::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageBgFades)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageBgFades, CPropertyPage)
	//{{AFX_MSG_MAP(CPageBgFades)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageBgFades message handlers

BOOL CPageBgFades::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// initialize the filter names and fade value
	for (uint i =0; i<32; ++i)
	{
		char tmp[128];
		GetDlgItem(FILTER_NAMES[i])->SetWindowText(SoundDialog->EnvNames[i].Name.c_str());
		sprintf(tmp, "%u", SoundDialog->FilterFades.FadeIns[i]);
		GetDlgItem(FILTER_FADE_IN[i])->SetWindowText(tmp);
		sprintf(tmp, "%u", SoundDialog->FilterFades.FadeOuts[i]);
		GetDlgItem(FILTER_FADE_OUT[i])->SetWindowText(tmp);
	}

	SoundDialog->getSoundPlugin()->getMixer()->setBackgroundFilterFades(SoundDialog->FilterFades);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPageBgFades::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (lParam != 0 && HIWORD(wParam) == EN_CHANGE)
	{
		int id = ::GetDlgCtrlID(HWND(lParam));
		char tmp[1024];

		if (FILTER_FADE_IN_IDX.find(id) != FILTER_FADE_IN_IDX.end())
		{
			// this is a fade in value modified !
			GetDlgItem(id)->GetWindowText(tmp, 1024);
			SoundDialog->FilterFades.FadeIns[FILTER_FADE_IN_IDX[id]] = atoi(tmp);
			SoundDialog->getSoundPlugin()->getMixer()->setBackgroundFilterFades(SoundDialog->FilterFades);
		}
		else if (FILTER_FADE_OUT_IDX.find(id) != FILTER_FADE_OUT_IDX.end())
		{
			// this is a fade in value modified !
			GetDlgItem(id)->GetWindowText(tmp, 1024);
			SoundDialog->FilterFades.FadeOuts[FILTER_FADE_OUT_IDX[id]] = atoi(tmp);
			SoundDialog->getSoundPlugin()->getMixer()->setBackgroundFilterFades(SoundDialog->FilterFades);
		}
	}
	
	return CPropertyPage::OnCommand(wParam, lParam);
}
