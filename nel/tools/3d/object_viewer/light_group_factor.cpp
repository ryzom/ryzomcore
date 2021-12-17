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

// light_group_factor.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "light_group_factor.h"

using namespace NL3D;

/////////////////////////////////////////////////////////////////////////////
// CLightGroupFactor dialog


CLightGroupFactor::CLightGroupFactor(CWnd* pParent /*=NULL*/)
	: CDialog(CLightGroupFactor::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLightGroupFactor)
	LightGroup1 = 256;
	LightGroup2 = 256;
	LightGroup3 = 256;
	//}}AFX_DATA_INIT
}


void CLightGroupFactor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightGroupFactor)
	DDX_Control(pDX, IDC_BUTTON_COLOR_START1, ColorStart1);
	DDX_Control(pDX, IDC_BUTTON_COLOR_START2, ColorStart2);
	DDX_Control(pDX, IDC_BUTTON_COLOR_START0, ColorStart0);
	DDX_Control(pDX, IDC_BUTTON_COLOR_END2, ColorEnd2);
	DDX_Control(pDX, IDC_BUTTON_COLOR_END1, ColorEnd1);
	DDX_Control(pDX, IDC_BUTTON_COLOR_END0, ColorEnd0);
	DDX_Control(pDX, IDC_LIGHT_GROUP3, LightGroup3Ctrl);
	DDX_Control(pDX, IDC_LIGHT_GROUP2, LightGroup2Ctrl);
	DDX_Control(pDX, IDC_LIGHT_GROUP1, LightGroup1Ctrl);
	DDX_Slider(pDX, IDC_LIGHT_GROUP1, LightGroup1);
	DDX_Slider(pDX, IDC_LIGHT_GROUP2, LightGroup2);
	DDX_Slider(pDX, IDC_LIGHT_GROUP3, LightGroup3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLightGroupFactor, CDialog)
	//{{AFX_MSG_MAP(CLightGroupFactor)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_COLOR_END0, OnButtonColorEnd0)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_END1, OnButtonColorEnd1)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_END2, OnButtonColorEnd2)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_START0, OnButtonColorStart0)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_START1, OnButtonColorStart1)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_START2, OnButtonColorStart2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightGroupFactor message handlers

BOOL CLightGroupFactor::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Light group
	LightGroup1Ctrl.SetRange (0, 256);
	LightGroup2Ctrl.SetRange (0, 256);
	LightGroup3Ctrl.SetRange (0, 256);
	LightGroup1 = 256;
	LightGroup2 = 256;
	LightGroup3 = 256;

	// Colors
	ColorStart0.setColor(CRGBA::Black);
	ColorStart1.setColor(CRGBA::Black);
	ColorStart2.setColor(CRGBA::Black);
	ColorEnd0.setColor(CRGBA::White);
	ColorEnd1.setColor(CRGBA::White);
	ColorEnd2.setColor(CRGBA::White);
	
	UpdateData (FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLightGroupFactor::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	handle ();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CLightGroupFactor::handle ()
{
	CRGBA	col;
	UpdateData ();

	// Set the light factors
	col.blendFromui(ColorStart0.getColor(), ColorEnd0.getColor(), LightGroup1);
	CNELU::Scene->setLightGroupColor (0, col);
	col.blendFromui(ColorStart1.getColor(), ColorEnd1.getColor(), LightGroup2);
	CNELU::Scene->setLightGroupColor (1, col);
	col.blendFromui(ColorStart2.getColor(), ColorEnd2.getColor(), LightGroup3);
	CNELU::Scene->setLightGroupColor (2, col);
}

// ***************************************************************************
void	CLightGroupFactor::chooseColor(CColorButton &colbut)
{
	CRGBA	col= colbut.getColor();

	CColorDialog	dlg;
	dlg.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT;
	dlg.m_cc.rgbResult = RGB(col.R,col.G,col.B);
	if(dlg.DoModal()==IDOK)
	{
		col.R= GetRValue(dlg.GetColor());
		col.G= GetGValue(dlg.GetColor());
		col.B= GetBValue(dlg.GetColor());
		col.A= 255;
		colbut.setColor(col);

		handle();
	}
}

// ***************************************************************************
void CLightGroupFactor::OnButtonColorEnd0() 
{
	chooseColor(ColorEnd0);
}

void CLightGroupFactor::OnButtonColorEnd1() 
{
	chooseColor(ColorEnd1);
}

void CLightGroupFactor::OnButtonColorEnd2() 
{
	chooseColor(ColorEnd2);
}

void CLightGroupFactor::OnButtonColorStart0() 
{
	chooseColor(ColorStart0);
}

void CLightGroupFactor::OnButtonColorStart1() 
{
	chooseColor(ColorStart1);
}

void CLightGroupFactor::OnButtonColorStart2() 
{
	chooseColor(ColorStart2);
}

