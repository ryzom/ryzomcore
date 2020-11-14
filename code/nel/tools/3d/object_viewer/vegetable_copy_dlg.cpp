// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// vegetable_copy_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "vegetable_copy_dlg.h"
#include "vegetable_dlg.h"


/////////////////////////////////////////////////////////////////////////////
CVegetableCopyDlg::CLastSetup		CVegetableCopyDlg::_LastSetup;

CVegetableCopyDlg::CLastSetup::CLastSetup()
{
	SubsetCopy = FALSE;
	ScaleZ = FALSE;
	ScaleXY = FALSE;
	RotateZ = FALSE;
	RotateY = FALSE;
	RotateX = FALSE;
	Mesh = FALSE;
	MaxDensity = FALSE;
	Distance = FALSE;
	Density = FALSE;
	ColorSetup = FALSE;
	ColorNoise = FALSE;
	BendPhase = FALSE;
	BendFactor = FALSE;
	AngleSetup = FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CVegetableCopyDlg dialog


CVegetableCopyDlg::CVegetableCopyDlg(CVegetableDlg *vegetableDlg, CWnd* pParent /*=NULL*/)
	: CDialog(CVegetableCopyDlg::IDD, pParent), _VegetableDlg(vegetableDlg)
{
	//{{AFX_DATA_INIT(CVegetableCopyDlg)
	SubsetCopy = FALSE;
	ScaleZ = FALSE;
	ScaleXY = FALSE;
	RotateZ = FALSE;
	RotateY = FALSE;
	RotateX = FALSE;
	Mesh = FALSE;
	MaxDensity = FALSE;
	Distance = FALSE;
	Density = FALSE;
	ColorSetup = FALSE;
	ColorNoise = FALSE;
	BendPhase = FALSE;
	BendFactor = FALSE;
	AngleSetup = FALSE;
	VegetableSelected = -1;
	//}}AFX_DATA_INIT
}


void CVegetableCopyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVegetableCopyDlg)
	DDX_Control(pDX, IDC_LIST1, VegetableList);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_SUBSET_COPY, CheckSubsetCopy);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_SCALEZ, CheckScaleZ);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_SCALEXY, CheckScaleXY);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_ROTATEZ, CheckRotateZ);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_ROTATEY, CheckRotateY);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_ROTATEX, CheckRotateX);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_MESH, CheckMesh);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_MAXDENSITY, CheckMaxDensity);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_DISTANCE, CheckDistance);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_DENSITY, CheckDensity);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_COLORSETUP, CheckColorSetup);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_COLORNOISE, CheckColorNoise);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_BENDPHASE, CheckBendPhase);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_BENDFACTOR, CheckBendFactor);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_ANGLESETUP, CheckAngleSetup);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_SUBSET_COPY, SubsetCopy);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_SCALEZ, ScaleZ);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_SCALEXY, ScaleXY);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_ROTATEZ, RotateZ);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_ROTATEY, RotateY);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_ROTATEX, RotateX);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_MESH, Mesh);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_MAXDENSITY, MaxDensity);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_DISTANCE, Distance);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_DENSITY, Density);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_COLORSETUP, ColorSetup);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_COLORNOISE, ColorNoise);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_BENDPHASE, BendPhase);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_BENDFACTOR, BendFactor);
	DDX_Check(pDX, IDC_CHECK_VEGETABLE_ANGLESETUP, AngleSetup);
	DDX_LBIndex(pDX, IDC_LIST1, VegetableSelected);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVegetableCopyDlg, CDialog)
	//{{AFX_MSG_MAP(CVegetableCopyDlg)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_APPEARANCE_NONE, OnButtonVegetableAppearanceNone)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_APPERANCE_ALL, OnButtonVegetableApperanceAll)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_GENERAL_ALL, OnButtonVegetableGeneralAll)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_GENERAL_NONE, OnButtonVegetableGeneralNone)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_POSITION_ALL, OnButtonVegetablePositionAll)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_POSITION_NONE, OnButtonVegetablePositionNone)
	ON_BN_CLICKED(IDC_CHECK_VEGETABLE_SUBSET_COPY, OnCheckVegetableSubsetCopy)
	ON_WM_DESTROY()
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CVegetableCopyDlg::enableChecks(bool enable)
{
	CheckScaleZ.EnableWindow(enable);
	CheckScaleXY.EnableWindow(enable);
	CheckRotateZ.EnableWindow(enable);
	CheckRotateY.EnableWindow(enable);
	CheckRotateX.EnableWindow(enable);
	CheckMesh.EnableWindow(enable);
	CheckMaxDensity.EnableWindow(enable);
	CheckDistance.EnableWindow(enable);
	CheckDensity.EnableWindow(enable);
	CheckColorSetup.EnableWindow(enable);
	CheckColorNoise.EnableWindow(enable);
	CheckBendPhase.EnableWindow(enable);
	CheckBendFactor.EnableWindow(enable);
	CheckAngleSetup.EnableWindow(enable);
}


// ***************************************************************************
// ***************************************************************************
// CVegetableCopyDlg message handlers
// ***************************************************************************
// ***************************************************************************

void CVegetableCopyDlg::OnButtonVegetableGeneralAll() 
{
	CheckMesh.SetCheck(1);
	CheckDistance.SetCheck(1);
	CheckDensity.SetCheck(1);
	CheckMaxDensity.SetCheck(1);
	CheckAngleSetup.SetCheck(1);
}

void CVegetableCopyDlg::OnButtonVegetableGeneralNone() 
{
	CheckMesh.SetCheck(0);
	CheckDistance.SetCheck(0);
	CheckDensity.SetCheck(0);
	CheckMaxDensity.SetCheck(0);
	CheckAngleSetup.SetCheck(0);
}


void CVegetableCopyDlg::OnButtonVegetableApperanceAll() 
{
	CheckBendPhase.SetCheck(1);
	CheckBendFactor.SetCheck(1);
	CheckColorNoise.SetCheck(1);
	CheckColorSetup.SetCheck(1);
}
void CVegetableCopyDlg::OnButtonVegetableAppearanceNone() 
{
	CheckBendPhase.SetCheck(0);
	CheckBendFactor.SetCheck(0);
	CheckColorNoise.SetCheck(0);
	CheckColorSetup.SetCheck(0);
}

void CVegetableCopyDlg::OnButtonVegetablePositionAll() 
{
	CheckScaleXY.SetCheck(1);
	CheckScaleZ.SetCheck(1);
	CheckRotateX.SetCheck(1);
	CheckRotateY.SetCheck(1);
	CheckRotateZ.SetCheck(1);
}

void CVegetableCopyDlg::OnButtonVegetablePositionNone() 
{
	CheckScaleXY.SetCheck(0);
	CheckScaleZ.SetCheck(0);
	CheckRotateX.SetCheck(0);
	CheckRotateY.SetCheck(0);
	CheckRotateZ.SetCheck(0);
}

void CVegetableCopyDlg::OnCheckVegetableSubsetCopy() 
{
	// enable checks if needed.
	enableChecks(CheckSubsetCopy.GetCheck()==1);
}

BOOL CVegetableCopyDlg::OnInitDialog() 
{
	// setup flags before the UpdateData
	SubsetCopy= _LastSetup.SubsetCopy;
	ScaleZ= _LastSetup.ScaleZ;
	ScaleXY= _LastSetup.ScaleXY;
	RotateZ= _LastSetup.RotateZ;
	RotateY= _LastSetup.RotateY;
	RotateX= _LastSetup.RotateX;
	Mesh= _LastSetup.Mesh;
	MaxDensity= _LastSetup.MaxDensity;
	Distance= _LastSetup.Distance;
	Density= _LastSetup.Density;
	ColorSetup= _LastSetup.ColorSetup;
	ColorNoise= _LastSetup.ColorNoise;
	BendPhase= _LastSetup.BendPhase;
	BendFactor= _LastSetup.BendFactor;
	AngleSetup= _LastSetup.AngleSetup;


	CDialog::OnInitDialog();
	
	// enable checks if needed.
	enableChecks(CheckSubsetCopy.GetCheck()==1);
	

	// Init the control list.
	uint	num= _VegetableDlg->getNumVegetables();
	for(uint i=0; i<num; i++)
	{
		VegetableList.AddString(nlUtf8ToTStr(_VegetableDlg->getVegetableName(i)));
	}
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVegetableCopyDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// save setup in _LastSetup.
	_LastSetup.SubsetCopy= SubsetCopy;
	_LastSetup.ScaleZ= ScaleZ;
	_LastSetup.ScaleXY= ScaleXY;
	_LastSetup.RotateZ= RotateZ;
	_LastSetup.RotateY= RotateY;
	_LastSetup.RotateX= RotateX;
	_LastSetup.Mesh= Mesh;
	_LastSetup.MaxDensity= MaxDensity;
	_LastSetup.Distance= Distance;
	_LastSetup.Density= Density;
	_LastSetup.ColorSetup= ColorSetup;
	_LastSetup.ColorNoise= ColorNoise;
	_LastSetup.BendPhase= BendPhase;
	_LastSetup.BendFactor= BendFactor;
	_LastSetup.AngleSetup= AngleSetup;
	
}

void CVegetableCopyDlg::OnOK() 
{
	UpdateData();
	if(VegetableSelected!=LB_ERR)
	{
		CDialog::OnOK();
	}
	else
	{
		MessageBox(_T("Select a  vegetable to copy first"), _T("Error"), MB_OK | MB_ICONWARNING);
	}
}

void CVegetableCopyDlg::OnDblclkList1() 
{
	UpdateData();
	// DblClck select the name.
	EndDialog(IDOK);
}
