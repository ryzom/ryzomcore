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

// preset_dlg.cpp : implementation file
//

todo hulud remove 

#include "stdafx.h"
#include "client_config.h"
#include "preset_dlg.h"
#include "database.h"
#include "display_dlg.h"
#include "cfg_file.h"

#undef max

using namespace NLMISC;

/////////////////////////////////////////////////////////////////////////////
// CPresetDlg dialog


CPresetDlg::CPresetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPresetDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPresetDlg)
	CPUFrequency = _T("");
	SystemMemory = _T("");
	VideoCard = -1;
	VideoMemory = _T("");
	HardwareSoundBuffer = _T("");
	//}}AFX_DATA_INIT
}


void CPresetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPresetDlg)
	DDX_Control(pDX, IDOK, OkCtrl);
	DDX_Control(pDX, IDCANCEL, CancelCtrl);
	DDX_Control(pDX, IDC_VIDEO_CARD, VideoCardCtrl);
	DDX_Text(pDX, IDC_CPU_FREQUENCY, CPUFrequency);
	DDX_Text(pDX, IDC_SYSTEM_MEMORY, SystemMemory);
	DDX_CBIndex(pDX, IDC_VIDEO_CARD, VideoCard);
	DDX_Text(pDX, IDC_VIDEO_MEMORY, VideoMemory);
	DDX_Text(pDX, IDC_HARDWARE_SOUND_BUFFER, HardwareSoundBuffer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPresetDlg, CDialog)
	//{{AFX_MSG_MAP(CPresetDlg)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPresetDlg message handlers

void CPresetDlg::OnReset() 
{
	// TODO: Add your control notification handler code here
	
}

BOOL CPresetDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Set the video card list
	uint i;
	for (i=0; i<CVideoCard::CardCount; i++)
		VideoCardCtrl.AddString (GetString (VideoCards[i].DisplayString));

	// Select the good video card
	for (VideoCard=0; VideoCard<CVideoCard::CardCount; VideoCard++)
	{
		bool found = false;
		// For each keyword
		uint j;
		for (j=0; j<CVideoCard::TokenCount; j++)
		{
			// Check it is not an empty keyword
			if (strcmp (VideoCards[VideoCard].MatchingTokens[j], "") != 0)
			{
				// Look for the keyword in the tokens
				char str[512];
				strcpy (str, GLRenderer.c_str ());
				const char *token = strtok( str, " ");
				while( token != NULL )
				{
					if (stricmp (token, VideoCards[VideoCard].MatchingTokens[j]) == 0)
					{
						found = true;
						break;
					}

					// Get next token
					token = strtok( NULL, " ");
				}

				// Not found ?
				if (!token)
					break;
			}
		}

		// Found ?
		if (found && (j == CVideoCard::TokenCount))
			break;
	}

	// Found ?
	if (VideoCard == CVideoCard::CardCount)
		VideoCard = 0;

	UpdateData (FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPresetDlg::OnOK() 
{
	UpdateData ();
	// Choose a video card
	if (VideoCard == 0)
	{
		MessageBoxW (GetString (IDS_CHOOSE_A_CARD), CI18N::get ("uiConfigTitle"), MB_OK|MB_ICONEXCLAMATION);
	}
	else
	{
		// Reset the CFG
		ResetConfigFile ();
	
		// Reset the widgets
		GetFromConfigFile ();

		// Merge the config files
		MergeConfigFile (ConfigFilePreset[((::SystemMemory / (1024*1024)) < 500)?RAM256:RAM512]);
		MergeConfigFile (ConfigFilePreset[(::CPUFrequency < 1500)?CPU1:(::CPUFrequency < 2000)?CPU2:(::CPUFrequency < 2500)?CPU3:CPU4]);
		MergeConfigFile (ConfigFilePreset[VideoCards[VideoCard].Preset]);
		int vram = ::VideoMemory / (1024*1024);
		MergeConfigFile (ConfigFilePreset[(vram < 50)?VRAM32:(vram < 100)?VRAM64:VRAM128]);
		CConfigFile::CVar *var = ConfigFilePreset[SoundBuffer].getVarPtr ("MaxTrack");
		if (var && var->Type)
		{
			var->setAsInt (::HardwareSoundBuffer);
			MergeConfigFile (ConfigFilePreset[SoundBuffer]);
		}

		CDialog::OnOK();
	}
}

void CPresetDlg::OnCancel() 
{
	CDialog::OnCancel();
}
