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

// vegetable_density_page.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "vegetable_density_page.h"
#include "vegetable_edit_tools.h"
#include "vegetable_noise_value_dlg.h"
#include "vegetable_dlg.h"
#include "nel/misc/noise_value.h"
#include "nel/3d/vegetable.h"



#define	NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE	256


/////////////////////////////////////////////////////////////////////////////
// CVegetableDensityPage property page

IMPLEMENT_DYNCREATE(CVegetableDensityPage, CPropertyPage)

CVegetableDensityPage::CVegetableDensityPage() : CPropertyPage(CVegetableDensityPage::IDD),
	_DensityDlg(NULL), _MaxDensityDlg(NULL)
{
	//{{AFX_DATA_INIT(CVegetableDensityPage)
	//}}AFX_DATA_INIT
}

CVegetableDensityPage::~CVegetableDensityPage()
{
	#define  REMOVE_WND(wnd) if (wnd) { wnd->DestroyWindow(); delete wnd; }
	REMOVE_WND(_DensityDlg);	
	REMOVE_WND(_MaxDensityDlg);	
}

void CVegetableDensityPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVegetableDensityPage)
	DDX_Control(pDX, IDC_STATIC_VEGETABLE_SHAPENAME, StaticVegetableShape);
	DDX_Control(pDX, IDC_STATIC_MAX_DENSITY, MaxDensityStaticText);
	DDX_Control(pDX, IDC_SLIDER_ANGLE_MIN, AngleMinSlider);
	DDX_Control(pDX, IDC_SLIDER_ANGLE_MAX, AngleMaxSlider);
	DDX_Control(pDX, IDC_EDIT_ANGLE_MIN, AngleMinEdit);
	DDX_Control(pDX, IDC_EDIT_ANGLE_MAX, AngleMaxEdit);
	DDX_Control(pDX, IDC_COMBO_DIST_TYPE, DistTypeCombo);
	DDX_Control(pDX, IDC_CHECK_MAX_DENSITY, MaxDensityCheckBox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVegetableDensityPage, CPropertyPage)
	//{{AFX_MSG_MAP(CVegetableDensityPage)
	ON_BN_CLICKED(IDC_CHECK_MAX_DENSITY, OnCheckMaxDensity)
	ON_CBN_SELCHANGE(IDC_COMBO_DIST_TYPE, OnSelchangeComboDistType)
	ON_BN_CLICKED(IDC_RADIO_ANGLE_CEILING, OnRadioAngleCeiling)
	ON_BN_CLICKED(IDC_RADIO_ANGLE_FLOOR, OnRadioAngleFloor)
	ON_BN_CLICKED(IDC_RADIO_ANGLE_WALL, OnRadioAngleWall)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_ANGLE_MAX, OnReleasedcaptureSliderAngleMax)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_ANGLE_MIN, OnReleasedcaptureSliderAngleMin)
	ON_EN_KILLFOCUS(IDC_EDIT_ANGLE_MIN, OnKillfocusEditAngleMin)
	ON_EN_KILLFOCUS(IDC_EDIT_ANGLE_MAX, OnKillfocusEditAngleMax)
	ON_EN_CHANGE(IDC_EDIT_ANGLE_MIN, OnChangeEditAngleMin)
	ON_EN_CHANGE(IDC_EDIT_ANGLE_MAX, OnChangeEditAngleMax)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_BROWSE, OnButtonVegetableBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CVegetableDensityPage::setVegetableToEdit(NL3D::CVegetable *vegetable)
{
	_Vegetable= vegetable;

	// If not NULL, init all controls.
	if(_Vegetable)
	{
		// Init ShapeName
		// ----------
		StaticVegetableShape.SetWindowText(_Vegetable->ShapeName.c_str());

		// init Creation Distance.
		// ----------
		// normalize creation distance for this editor.
		_Vegetable->DistType= std::min( (uint)_Vegetable->DistType, (uint)(DistTypeCombo.GetCount()-1) );
		// set Creation Distance.
		DistTypeCombo.SetCurSel( _Vegetable->DistType );

		// init _DensityDlg.
		// ----------
		_DensityDlg->setNoiseValue(&_Vegetable->Density, _VegetableDlg);

		// init MaxDensity
		// ----------
		if(_Vegetable->MaxDensity == -1)
		{
			// Disable the checkBox and the slider.
			MaxDensityCheckBox.SetCheck(0);
			static	float dummy;
			_MaxDensityDlg->setFloat(&dummy, _VegetableDlg);
			_MaxDensityDlg->EnableWindow(false);
			// Init with a default value
			_PrecMaxDensityValue= NL_VEGETABLE_EDIT_DEFAULT_MAX_DENSITY;
		}
		else
		{
			// Enable the checkBox and the slider
			MaxDensityCheckBox.SetCheck(1);
			_MaxDensityDlg->setFloat(&_Vegetable->MaxDensity, _VegetableDlg);
			_MaxDensityDlg->EnableWindow(true);
		}

		// init AngleSetup.
		// ----------
		NL3D::CVegetable::TAngleType	angType= _Vegetable->getAngleType();
		// disable 3 radio buttons.
		((CButton*)GetDlgItem(IDC_RADIO_ANGLE_FLOOR))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_ANGLE_WALL))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_ANGLE_CEILING))->SetCheck(0);
		// enable only the one of interest.
		switch(angType)
		{
		case NL3D::CVegetable::AngleGround:
			((CButton*)GetDlgItem(IDC_RADIO_ANGLE_FLOOR))->SetCheck(1);
			enableAngleEdit(IDC_RADIO_ANGLE_FLOOR);
			break;
		case NL3D::CVegetable::AngleWall:
			((CButton*)GetDlgItem(IDC_RADIO_ANGLE_WALL))->SetCheck(1);
			enableAngleEdit(IDC_RADIO_ANGLE_WALL);
			break;
		case NL3D::CVegetable::AngleCeiling:
			((CButton*)GetDlgItem(IDC_RADIO_ANGLE_CEILING))->SetCheck(1);
			enableAngleEdit(IDC_RADIO_ANGLE_CEILING);
			break;
		}
		// to avoid a strange bug if pos==0... (not correctly setuped the first time)
		AngleMinSlider.SetPos(10);
		AngleMaxSlider.SetPos(10);
		// Init sliders / edit.
		updateViewAngleMin();
		updateViewAngleMax();
	}
}


// ***************************************************************************
void		CVegetableDensityPage::enableAngleEdit(uint radioId)
{
	// first disable all
	AngleMinSlider.EnableWindow(false);
	AngleMinEdit.EnableWindow(false);
	AngleMaxSlider.EnableWindow(false);
	AngleMaxEdit.EnableWindow(false);
	// Then enable only what needed.
	if(radioId == IDC_RADIO_ANGLE_WALL || radioId == IDC_RADIO_ANGLE_FLOOR)
	{
		AngleMinSlider.EnableWindow(true);
		AngleMinEdit.EnableWindow(true);
	}
	if(radioId == IDC_RADIO_ANGLE_WALL || radioId == IDC_RADIO_ANGLE_CEILING)
	{
		AngleMaxSlider.EnableWindow(true);
		AngleMaxEdit.EnableWindow(true);
	}
}


// ***************************************************************************
void		CVegetableDensityPage::updateViewAngleMin()
{
	double	angle= _Vegetable->getCosAngleMin();
	NLMISC::clamp(angle, -1, 1);
	angle= asin(angle);

	sint	pos= (sint)(angle/(NLMISC::Pi/2) * NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE);
	NLMISC::clamp(pos, -NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE, NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE);
	AngleMinSlider.SetPos(pos);
	char	stmp[256];
	sprintf(stmp, "%.2f", (double)(angle*180/NLMISC::Pi));
	AngleMinEdit.SetWindowText(stmp);
}

// ***************************************************************************
void		CVegetableDensityPage::updateViewAngleMax()
{
	double	angle= _Vegetable->getCosAngleMax();
	NLMISC::clamp(angle, -1, 1);
	angle= asin(angle);

	sint	pos= (sint)(angle/(NLMISC::Pi/2) * NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE);
	NLMISC::clamp(pos, -NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE, NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE);
	AngleMaxSlider.SetPos(pos);
	char	stmp[256];
	sprintf(stmp, "%.2f", (double)(angle*180/NLMISC::Pi));
	AngleMaxEdit.SetWindowText(stmp);
}


// ***************************************************************************
void		CVegetableDensityPage::updateAngleMinFromEditText()
{
	// get angles edited.
	char	stmp[256];
	AngleMinEdit.GetWindowText(stmp, 256);
	float	angleMin;
	NLMISC::fromString(stmp, angleMin);
	NLMISC::clamp(angleMin, -90, 90);
	// make a sinus, because 90 => 1, and -90 =>-1
	float	cosAngleMin= (float)sin(angleMin*NLMISC::Pi/180.f);

	// setup vegetable.
	if(_Vegetable->getAngleType()== NL3D::CVegetable::AngleWall)
		_Vegetable->setAngleWall(cosAngleMin, _Vegetable->getCosAngleMax());
	else if(_Vegetable->getAngleType()== NL3D::CVegetable::AngleGround)
		_Vegetable->setAngleGround(cosAngleMin);

	// update views
	updateViewAngleMin();

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();
}
// ***************************************************************************
void		CVegetableDensityPage::updateAngleMaxFromEditText()
{
	// get angles edited.
	char	stmp[256];
	AngleMaxEdit.GetWindowText(stmp, 256);
	float	angleMax;
	NLMISC::fromString(stmp, angleMax);
	NLMISC::clamp(angleMax, -90, 90);
	// make a sinus, because 90 => 1, and -90 =>-1
	float	cosAngleMax= (float)sin(angleMax*NLMISC::Pi/180.f);

	// setup vegetable.
	if(_Vegetable->getAngleType()== NL3D::CVegetable::AngleWall)
		_Vegetable->setAngleWall(_Vegetable->getCosAngleMin(), cosAngleMax);
	else if(_Vegetable->getAngleType()== NL3D::CVegetable::AngleCeiling)
		_Vegetable->setAngleCeiling(cosAngleMax);

	// update views
	updateViewAngleMax();

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();
}



// ***************************************************************************
// ***************************************************************************
// CVegetableDensityPage message handlers
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
BOOL CVegetableDensityPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	
	// position setup.
	uint	x= 5;
	// Position of the density DlgBox relative to CVegetableDensityPage.
	uint	yDensity= 130;
	// y relative to MaxDensity Group
	uint	yMaxDensity= 45;
	// get y of MaxDensityStaticText relative to CVegetableDensityPage.
	RECT	rectParent;
	RECT	rect;
	GetWindowRect(&rectParent);
	MaxDensityStaticText.GetWindowRect(&rect);
	// and apply.
	yMaxDensity+= rect.top - rectParent.top;


	// Init Density Dialog.
	_DensityDlg = new CVegetableNoiseValueDlg (std::string("Density"));
	_DensityDlg->setDefaultRangeAbs(NL_VEGETABLE_DENSITY_ABS_RANGE_MIN, NL_VEGETABLE_DENSITY_ABS_RANGE_MAX);
	_DensityDlg->setDefaultRangeRand(NL_VEGETABLE_DENSITY_RAND_RANGE_MIN, NL_VEGETABLE_DENSITY_RAND_RANGE_MAX);
	_DensityDlg->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);
	_DensityDlg->init(x, yDensity, this);

	
	// Init MaxDensity Dialog.
	_MaxDensityDlg = new CDirectEditableRangeFloat (std::string("VEGET_MAX_DENSITY"), 0, NL_VEGETABLE_EDIT_DEFAULT_MAX_DENSITY, 
		"MaxDensity");
	_MaxDensityDlg->enableLowerBound(0, false);
	_MaxDensityDlg->init(x+10, yMaxDensity, this);


	// Init angle sliders.
	AngleMinSlider.SetRange(-NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE, NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE);
	AngleMaxSlider.SetRange(-NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE, NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE);

	
	// Init ShapeName
	StaticVegetableShape.SetWindowText("");


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************
void CVegetableDensityPage::OnCheckMaxDensity() 
{
	if(MaxDensityCheckBox.GetCheck() == 1)
	{
		// check, restore maxDensity
		_Vegetable->MaxDensity= _PrecMaxDensityValue;
		// enable dlg.
		_MaxDensityDlg->setFloat(&_Vegetable->MaxDensity, _VegetableDlg);
		_MaxDensityDlg->EnableWindow(true);
	}
	else
	{
		// uncheck, bkup maxDenstiy
		_PrecMaxDensityValue= _Vegetable->MaxDensity;
		// disable dlg
		static	float dummy;
		_MaxDensityDlg->setFloat(&dummy, _VegetableDlg);
		_MaxDensityDlg->EnableWindow(false);
		// and setup vegetable (disable MaxDensity).
		_Vegetable->MaxDensity= -1;
	}

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();
}

// ***************************************************************************
void CVegetableDensityPage::OnSelchangeComboDistType() 
{
	// Get the DistType, and just copy to vegetable.
	_Vegetable->DistType= DistTypeCombo.GetCurSel();

	// Since used to display name in selection listBox, must update the name
	_VegetableDlg->updateCurSelVegetableName();

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();
}

// ***************************************************************************
void CVegetableDensityPage::OnRadioAngleCeiling() 
{
	// Enable just the AngleMax slider.
	enableAngleEdit(IDC_RADIO_ANGLE_CEILING);

	// Init value.
	_Vegetable->setAngleCeiling(0);

	// Update view Value
	updateViewAngleMax();

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();
}

// ***************************************************************************
void CVegetableDensityPage::OnRadioAngleFloor() 
{
	// Enable just the AngleMin slider.
	enableAngleEdit(IDC_RADIO_ANGLE_FLOOR);

	// Init value.
	_Vegetable->setAngleGround(0);

	// Update view Value
	updateViewAngleMin();

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();
}

// ***************************************************************************
void CVegetableDensityPage::OnRadioAngleWall() 
{
	// Enable both sliders.
	enableAngleEdit(IDC_RADIO_ANGLE_WALL);

	// Init value.
	_Vegetable->setAngleWall(-1, 1);

	// Update view Value
	updateViewAngleMin();
	updateViewAngleMax();

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();
}

// ***************************************************************************
void CVegetableDensityPage::OnReleasedcaptureSliderAngleMax(NMHDR* pNMHDR, LRESULT* pResult) 
{
	float	angle= 90 * (float)AngleMaxSlider.GetPos() / (float)NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE;
	NLMISC::clamp(angle, -90, 90);
	// make a sinus, because 90 => 1, and -90 =>-1
	float	cosAngleMax= (float)sin(angle*NLMISC::Pi/180.f);

	// setup vegetable.
	if(_Vegetable->getAngleType()== NL3D::CVegetable::AngleWall)
		_Vegetable->setAngleWall(_Vegetable->getCosAngleMin(), cosAngleMax);
	else
		_Vegetable->setAngleCeiling(cosAngleMax);
	
	// update view
	updateViewAngleMax();

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();

	*pResult = 0;
}

// ***************************************************************************
void CVegetableDensityPage::OnReleasedcaptureSliderAngleMin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	float	angle= 90 * (float)AngleMinSlider.GetPos() / (float)NL_VEGETABLE_EDIT_ANGLE_SLIDER_SIZE;
	NLMISC::clamp(angle, -90, 90);
	// make a sinus, because 90 => 1, and -90 =>-1
	float	cosAngleMin= (float)sin(angle*NLMISC::Pi/180.f);

	// setup vegetable.
	if(_Vegetable->getAngleType()== NL3D::CVegetable::AngleWall)
		_Vegetable->setAngleWall(cosAngleMin, _Vegetable->getCosAngleMax());
	else
		_Vegetable->setAngleGround(cosAngleMin);
	
	// update view
	updateViewAngleMin();

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();

	*pResult = 0;
}

// ***************************************************************************
void CVegetableDensityPage::OnKillfocusEditAngleMin() 
{
	updateAngleMinFromEditText();
}
void CVegetableDensityPage::OnKillfocusEditAngleMax() 
{
	updateAngleMaxFromEditText();
}

static	void concatEdit2Lines(CEdit &edit)
{
	const	uint lineLen= 1000;
	uint	n;
	// retrieve the 2 lines.
	char	tmp0[2*lineLen];
	char	tmp1[lineLen];
	n= edit.GetLine(0, tmp0, lineLen);	tmp0[n]= 0;
	n= edit.GetLine(1, tmp1, lineLen);	tmp1[n]= 0;
	// concat and update the CEdit.
	edit.SetWindowText(strcat(tmp0, tmp1));
}

void CVegetableDensityPage::OnChangeEditAngleMin() 
{
	// Trick to track "Enter" keypress: CEdit are multiline. If GetLineCount()>1, then
	// user has press enter.
	if(AngleMinEdit.GetLineCount()>1)
	{
		// must ccat the 2 first lines.
		concatEdit2Lines(AngleMinEdit);
		// the text (and so the lineCount) is reseted in this method.
		updateAngleMinFromEditText();
	}
}
void CVegetableDensityPage::OnChangeEditAngleMax() 
{
	// Trick to track "Enter" keypress: CEdit are multiline. If GetLineCount()>1, then
	// user has press enter.
	if(AngleMaxEdit.GetLineCount()>1)
	{
		// must ccat the 2 first lines.
		concatEdit2Lines(AngleMinEdit);
		// the text (and so the lineCount) is reseted in this method.
		updateAngleMaxFromEditText();
	}
}



// ***************************************************************************
void CVegetableDensityPage::OnButtonVegetableBrowse() 
{
	CFileDialog fd(TRUE, "veget", "*.veget", 0, NULL, this) ;
	fd.m_ofn.lpstrTitle= "Open Vegetable Shape";
	if (fd.DoModal() == IDOK)
	{
		// Add to the path
		char drive[256];
		char dir[256];
		char path[256];

		// Add search path for the .veget
		_splitpath (fd.GetPathName(), drive, dir, NULL, NULL);
		_makepath (path, drive, dir, NULL, NULL);
		NLMISC::CPath::addSearchPath (path);

		try
		{
			// verify the file can be opened.
			NLMISC::CPath::lookup((const char*)fd.GetFileName());

			// update shapeName and view
			_Vegetable->ShapeName= std::string(fd.GetFileName());
			StaticVegetableShape.SetWindowText(fd.GetFileName());

			// update the name in the list-box
			_VegetableDlg->updateCurSelVegetableName();

			// update 3D view
			_VegetableDlg->refreshVegetableDisplay();
		}
		catch (NLMISC::EPathNotFound &ep)
		{
			MessageBox(ep.what(), "Can't open file");
		}
	}
}
