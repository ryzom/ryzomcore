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

// DailogFlags.cpp : implementation file
//

#include "stdafx.h"
#include "DialogFlags.h"
#include <map>
#include "sound_plugin.h"
#include "nel/sound/u_listener.h"


using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

::CBitmap		PlayBitmap;
::CBitmap		StopBitmap;
::CBitmap		EarBitmap;
::CBitmap		EarMoveBitmap;

uint BG_FLAG_ID[] =
{
	IDC_CHECK_FLAG_00,
	IDC_CHECK_FLAG_01,
	IDC_CHECK_FLAG_02,
	IDC_CHECK_FLAG_03,
	IDC_CHECK_FLAG_04,
	IDC_CHECK_FLAG_05,
	IDC_CHECK_FLAG_06,
	IDC_CHECK_FLAG_07,
	IDC_CHECK_FLAG_08,
	IDC_CHECK_FLAG_09,
	IDC_CHECK_FLAG_10,
	IDC_CHECK_FLAG_11,
	IDC_CHECK_FLAG_12,
	IDC_CHECK_FLAG_13,
	IDC_CHECK_FLAG_14,
	IDC_CHECK_FLAG_15,
	IDC_CHECK_FLAG_16,
	IDC_CHECK_FLAG_17,
	IDC_CHECK_FLAG_18,
	IDC_CHECK_FLAG_19,
	IDC_CHECK_FLAG_20,
	IDC_CHECK_FLAG_21,
	IDC_CHECK_FLAG_22,
	IDC_CHECK_FLAG_23,
	IDC_CHECK_FLAG_24,
	IDC_CHECK_FLAG_25,
	IDC_CHECK_FLAG_26,
	IDC_CHECK_FLAG_27,
	IDC_CHECK_FLAG_28,
	IDC_CHECK_FLAG_29,
	IDC_CHECK_FLAG_30,
	IDC_CHECK_FLAG_31
};

map<uint, uint>	BG_FLAG_ID_IDX;


/////////////////////////////////////////////////////////////////////////////
// CDialogFlags dialog


CDialogFlags::CDialogFlags(UAudioMixer *mixer, CWnd* pParent /*=NULL*/)
	: CDialog(CDialogFlags::IDD, pParent),
	_Mixer(mixer)
{
	//{{AFX_DATA_INIT(CDialogFlags)
	SampleSize = _T("");
	//}}AFX_DATA_INIT

	// load initial bitmap into control
	PlayBitmap.LoadBitmap(IDB_BITMAP_PLAY);
	StopBitmap.LoadBitmap(IDB_BITMAP_STOP);
	EarBitmap.LoadBitmap(IDB_BITMAP_EAR);
	EarMoveBitmap.LoadBitmap(IDB_BITMAP_MOVE_EAR);

	// init the flags.

	ScrollPos = 0;

	_Flags = mixer->getBackgroundFlags();
	for (uint i=0; i<32; ++i)
	{
//		_Flags.Flags[i] = false;
		BG_FLAG_ID_IDX.insert(make_pair(BG_FLAG_ID[i], i));
//		BgControlId[i] = BG_FLAG_ID[i];
	}

//	mixer->setBackgroundFlags(_Flags);

}

void CDialogFlags::init(CSoundPlugin *plugin)
{
	_Plugin = plugin;

	BtnPlayStop.SetBitmap(PlayBitmap);
	CheckMoveEar.SetBitmap(EarBitmap);

	// preset the state of the flags
	const NLSOUND::UAudioMixer::TBackgroundFlags &flags = _Mixer->getBackgroundFlags();
	for (uint i =0; i<NLSOUND::UAudioMixer::TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
	{
		static_cast<CButton*>(GetDlgItem(BG_FLAG_ID[i]))->SetCheck(flags.Flags[i] ? 1 : 0);
		GetDlgItem(BG_FLAG_ID[i])->SetWindowText(_Mixer->getBackgroundFlagName(i).c_str());
	}


	// init the Sound bank list
	LVCOLUMN	col;
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	col.fmt = LVCFMT_LEFT;
	col.cx = 140;
	col.pszText = "bank names";
	col.cchTextMax = sizeof("bank names");
	_SbList.InsertColumn(0, &col);

	col.fmt = LVCFMT_RIGHT;
	col.cx = 50;
	col.pszText = "size";
	col.cchTextMax = sizeof("size");
	_SbList.InsertColumn(1, &col);

}


void CDialogFlags::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogFlags)
	DDX_Control(pDX, IDC_SB_LIST, _SbList);
	DDX_Control(pDX, IDC_CHECK_MOVE_EAR, CheckMoveEar);
	DDX_Control(pDX, IDC_BUTTON_PLAY_STOP, BtnPlayStop);
	DDX_Text(pDX, IDC_STATIC_SAMPLE_SIZE, SampleSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogFlags, CDialog)
	//{{AFX_MSG_MAP(CDialogFlags)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_STOP, OnButtonPlayStop)
	ON_BN_CLICKED(IDC_CHECK_MOVE_EAR, OnCheckMoveEar)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RELOAD, OnReload)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CDialogFlags::OnButtonPlayStop() 
{
	static bool	play = true;

	if (play)
	{
		_Plugin->play();
		play = false;
		BtnPlayStop.SetBitmap(StopBitmap);
//		SetTimer(1, 20, NULL);
	}
	else
	{
//		KillTimer(1);
		_Plugin->stop();
		play = true;
		BtnPlayStop.SetBitmap(PlayBitmap);
		
	}
	
}

void CDialogFlags::OnCheckMoveEar() 
{
	if (CheckMoveEar.GetCheck() == 1)
	{
		_Plugin->startMoveEar();
		CheckMoveEar.SetBitmap(EarMoveBitmap);
	}
	else
	{
		_Plugin->stopMoveEar();
		CheckMoveEar.SetBitmap(EarBitmap);
	}
}

void CDialogFlags::lostPositionControl()
{
	CheckMoveEar.SetCheck(0);
	CheckMoveEar.SetBitmap(EarBitmap);
}

void CDialogFlags::OnTimer(UINT_PTR nIDEvent) 
{
	CDialog::OnTimer(nIDEvent);

	_Plugin->update();
	std::string	txt = _Plugin->getStatusString();
//	uint size = _Plugin->getLoadedSampleSize();

	SampleSize = txt.c_str();

	// loaded sample banks size info
	_Plugin->getLoadedSampleBankInfo(SampleBanks);

	uint c, i;
	c = _SbList.GetItemCount();
	for (i=0; i<SampleBanks.size(); ++i)
	{
		if (i < c)
		{
			// update the item
			char temp[1024];
			sprintf(temp, "%6.2f", SampleBanks[i].second / (1024.0f*1024.0f));

			char temp2[1024];
			_SbList.GetItemText(i, 0, temp2, 1024);
			if (strcmp(SampleBanks[i].first.c_str(), temp2) != 0)
				_SbList.SetItemText(i, 0, SampleBanks[i].first.c_str());
			_SbList.GetItemText(i, 1, temp2, 1024);
			if (strcmp(temp, temp2) != 0)
				_SbList.SetItemText(i, 1, temp);
		}
		else
		{
			// insert item
			LVITEM lvi;
			lvi.mask =  LVIF_TEXT;
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.pszText = (char*) SampleBanks[i].first.c_str();
			_SbList.InsertItem(&lvi);

			lvi.iItem = i;
			lvi.iSubItem = 1;
			char temp[1024];
			sprintf(temp, "%6.2f", SampleBanks[i].second / (1024.0f*1024.0f));
			lvi.pszText = temp;
			_SbList.InsertItem(&lvi);
		}
	}

	for (; i<c; ++i)
	{
		// delete items
		_SbList.DeleteItem(_SbList.GetItemCount()-1);
	}

	UpdateData(FALSE);
}

int CDialogFlags::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	SetTimer(1, 20, NULL);
	
	return 0;
}

void CDialogFlags::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	KillTimer(1);
	
}

BOOL CDialogFlags::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	int id = ::GetDlgCtrlID((HWND)lParam);

	if (BG_FLAG_ID_IDX.find(id) != BG_FLAG_ID_IDX.end())
	{
		_Flags.Flags[BG_FLAG_ID_IDX[id]] = static_cast<CButton*>(GetDlgItem(id))->GetState() & (0x0003 != 0);

		_Mixer->setBackgroundFlags(_Flags);
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

void CDialogFlags::OnReload() 
{
	NLMISC::CVector pos(NLMISC::CVector::Null);
	if (_Mixer)
	{
		pos = _Mixer->getListener()->getPos();
	}

	_Plugin->ReInit();
	_Mixer = _Plugin->getMixer();

	if (_Mixer)
	{
		_Mixer->setBackgroundFlags(_Flags);
		_Mixer->setListenerPos(pos);
	}
}

void CDialogFlags::OnClose()
{
	_Plugin->closePlugin();

}


