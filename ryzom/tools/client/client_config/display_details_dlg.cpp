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

// display_details_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "base_dialog.h"
#include "client_config.h"
#include "database.h"
#include "cfg_file.h"
#include "display_details_dlg.h"

// ***************************************************************************
// CDisplayDetailsDlg dialog
// ***************************************************************************

CDisplayDetailsDlg::CDisplayDetailsDlg(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDisplayDetailsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDisplayDetailsDlg)
	CharacterQualityInt = 0;
	FXQualityInt = 0;
	LandscapeQualityInt = 0;
	TextureQualityInt = 0;
	//}}AFX_DATA_INIT

	MaxTextureQuality= 1;
}

// ***************************************************************************

void CDisplayDetailsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisplayDetailsDlg)
	DDX_Control(pDX, IDC_LANDSCAPE_QUALITY, LandscapeQuality);
	DDX_Control(pDX, IDC_FX_QUALITY, FXQuality);
	DDX_Control(pDX, IDC_CHARACTER_QUALITY, CharacterQuality);
	DDX_Control(pDX, IDC_TEXTURE_QUALITY, TextureQuality);
	DDX_Control(pDX, IDC_CHARACTER_QUALITY_VALUE, CharacterQualityValue);
	DDX_Control(pDX, IDC_FX_QUALITY_VALUE, FXQualityValue);
	DDX_Control(pDX, IDC_LANDSCAPE_QUALITY_VALUE, LandscapeQualityValue);
	DDX_Control(pDX, IDC_TEXTURE_QUALITY_VALUE, TextureQualityValue);
	DDX_Slider(pDX, IDC_CHARACTER_QUALITY, CharacterQualityInt);
	DDX_Slider(pDX, IDC_FX_QUALITY, FXQualityInt);
	DDX_Slider(pDX, IDC_LANDSCAPE_QUALITY, LandscapeQualityInt);
	DDX_Slider(pDX, IDC_TEXTURE_QUALITY, TextureQualityInt);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CDisplayDetailsDlg, CDialog)
	//{{AFX_MSG_MAP(CDisplayDetailsDlg)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CDisplayDetailsDlg message handlers
// ***************************************************************************

void InitQuality (CSliderCtrl &slider)
{
	slider.SetRange (0, QUALITY_STEP-1);
}

// ***************************************************************************

BOOL CDisplayDetailsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	InitQuality (LandscapeQuality);
	InitQuality (CharacterQuality);
	InitQuality (FXQuality);
	TextureQuality.SetRange(0, MaxTextureQuality);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void UpdateQuality (int quality, HWND hwnd)
{
	static const char *qualityString[QUALITY_STEP] = { "uiConfigPoor", "uiConfigMedium", "uiConfigNormal", "uiConfigSuper"};
	setWindowText(hwnd, (WCHAR*)NLMISC::CI18N::get (qualityString[quality]).c_str());
}

// ***************************************************************************

void CDisplayDetailsDlg::updateState ()
{
	UpdateQuality (LandscapeQualityInt, LandscapeQualityValue);
	UpdateQuality (FXQualityInt, FXQualityValue);
	UpdateQuality (CharacterQualityInt, CharacterQualityValue);

	// If control has bad range, reset
	int		rMin, rMax;
	TextureQuality.GetRange(rMin, rMax);
	if(rMin!=0 || rMax!=MaxTextureQuality)
		TextureQuality.SetRange(0, MaxTextureQuality);
	
	// update texture quality text
	static const char *textureString[QUALITY_TEXTURE_STEP] = { "uiConfigTexturePoor", "uiConfigTextureNormal", "uiConfigTextureSuper"};
	setWindowText(TextureQualityValue, (WCHAR*)NLMISC::CI18N::get (textureString[TextureQualityInt]).c_str());
}

// ***************************************************************************

void CDisplayDetailsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	UpdateData ();
	updateState ();
	UpdateData (FALSE);
	InvalidateConfig ();
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

