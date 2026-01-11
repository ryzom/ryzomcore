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

// sound_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "base_dialog.h"
#include "client_config.h"
#include "sound_dlg.h"
#include "cfg_file.h"

// ***************************************************************************
#define	SOUND_BUFFER_STEP	8
#define	SOUND_BUFFER_NUM_STEP	(32/SOUND_BUFFER_STEP)

// ***************************************************************************
// CSoundDlg dialog
// ***************************************************************************

CSoundDlg::CSoundDlg(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CSoundDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoundDlg)
	SoundOn = FALSE;
	EAX = FALSE;
	FMod = FALSE;
	ForceSoundSoft = FALSE;
	SoundQualityInt = 1;
	//}}AFX_DATA_INIT
}

// ***************************************************************************

void CSoundDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundDlg)
	DDX_Control(pDX, IDC_SOUND_TEXT_SB, TextSB);
	DDX_Control(pDX, IDC_SOUND_TEXT_EAX, TextEAX);
	DDX_Control(pDX, IDC_SOUND_TEXT_FMOD, TextFMod);
	DDX_Control(pDX, IDC_SOUND_TEXT_FORCESOFT, TextForceSoundSoft);
	DDX_Control(pDX, IDC_EAX, EAXCtrl);
	DDX_Control(pDX, IDC_FMOD, FModCtrl);
	DDX_Control(pDX, IDC_FORCESOUNDSOFT, ForceSoundSoftCtrl);
	DDX_Check(pDX, IDC_SOUND_ON, SoundOn);
	DDX_Check(pDX, IDC_EAX, EAX);
	DDX_Check(pDX, IDC_FMOD, FMod);
	DDX_Check(pDX, IDC_FORCESOUNDSOFT, ForceSoundSoft);
	DDX_Control(pDX, IDC_SOUND_QUALITY, SoundQuality);
	DDX_Control(pDX, IDC_SOUND_QUALITY_VALUE, SoundQualityValue);
	DDX_Slider(pDX, IDC_SOUND_QUALITY, SoundQualityInt);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CSoundDlg, CDialog)
	//{{AFX_MSG_MAP(CSoundDlg)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_SOUND_ON, OnSoundOn)
	ON_BN_CLICKED(IDC_EAX, OnEax)
	ON_BN_CLICKED(IDC_FMOD, OnFMod)
	ON_BN_CLICKED(IDC_FORCESOUNDSOFT, OnForceSoundSoft)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CSoundDlg message handlers
// ***************************************************************************

void CSoundDlg::OnSoundOn() 
{
	UpdateData (TRUE);
	updateState ();
	UpdateData (FALSE);
	InvalidateConfig ();
}

// ***************************************************************************

void CSoundDlg::updateState ()
{
	FModCtrl.EnableWindow (SoundOn);
	ForceSoundSoftCtrl.EnableWindow (SoundOn);
	EAXCtrl.EnableWindow (SoundOn);
	SoundQuality.EnableWindow (SoundOn);
	TextSB.EnableWindow (SoundOn);
	TextEAX.EnableWindow (SoundOn);
	TextFMod.EnableWindow (SoundOn);
	TextForceSoundSoft.EnableWindow (SoundOn);
	SoundQualityValue.EnableWindow (SoundOn);

	// Yoyo: temporary avoid FMod ctrl, since always FMod....
	TextFMod.ShowWindow(SW_HIDE);
	FModCtrl.ShowWindow(SW_HIDE);

	ucstring str = NLMISC::toString (getAsNumSoundBuffer()) + " " + NLMISC::CI18N::get ("uiConfigTracks");
	setWindowText(SoundQualityValue, (WCHAR*)str.c_str());
}

// ***************************************************************************

BOOL CSoundDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SoundQuality.SetRange(1, SOUND_BUFFER_NUM_STEP);
	
	updateState ();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void CSoundDlg::OnEax() 
{
	InvalidateConfig ();
}

// ***************************************************************************

void CSoundDlg::OnFMod() 
{
	InvalidateConfig ();
}

// ***************************************************************************

void CSoundDlg::OnForceSoundSoft() 
{
	InvalidateConfig ();
}

// ***************************************************************************
void		CSoundDlg::setAsNumSoundBuffer(uint sb)
{
	SoundQualityInt= sb/SOUND_BUFFER_STEP;
	NLMISC::clamp(SoundQualityInt, 1, SOUND_BUFFER_NUM_STEP);
}

// ***************************************************************************
uint		CSoundDlg::getAsNumSoundBuffer() const
{
	return SoundQualityInt * SOUND_BUFFER_STEP;
}

// ***************************************************************************

void CSoundDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	UpdateData ();
	updateState ();
	UpdateData (FALSE);
	InvalidateConfig ();
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

