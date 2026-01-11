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

// vegetable_scale_page.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "vegetable_scale_page.h"
#include "vegetable_edit_tools.h"
#include "vegetable_noise_value_dlg.h"
#include "vegetable_dlg.h"
#include "nel/misc/noise_value.h"
#include "nel/3d/vegetable.h"



/////////////////////////////////////////////////////////////////////////////
// CVegetableScalePage property page

IMPLEMENT_DYNCREATE(CVegetableScalePage, CPropertyPage)


CVegetableScalePage::CVegetableScalePage() : CPropertyPage(CVegetableScalePage::IDD),
	_SxyDlg(NULL), _SzDlg(NULL), _BendFreqFactorDlg(NULL)
{
	//{{AFX_DATA_INIT(CVegetableScalePage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CVegetableScalePage::~CVegetableScalePage()
{
	#define  REMOVE_WND(wnd) if (wnd) { wnd->DestroyWindow(); delete wnd; }
	REMOVE_WND(_SxyDlg);
	REMOVE_WND(_SzDlg);
	REMOVE_WND(_BendFreqFactorDlg);	
}

void CVegetableScalePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVegetableScalePage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVegetableScalePage, CPropertyPage)
	//{{AFX_MSG_MAP(CVegetableScalePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
void CVegetableScalePage::setVegetableToEdit(NL3D::CVegetable *vegetable)
{
	_Vegetable= vegetable;

	// If not NULL, init all controls.
	if(_Vegetable)
	{
		// init all dlg.
		// ----------
		_SxyDlg->setNoiseValue(&_Vegetable->Sxy, _VegetableDlg);
		_SzDlg->setNoiseValue(&_Vegetable->Sz, _VegetableDlg);

		// init _BendFreqFactorDlg
		// ----------
		// Enable the checkBox and the slider
		_BendFreqFactorDlg->setFloat(&_Vegetable->BendFrequencyFactor, _VegetableDlg);
	}

}


// ***************************************************************************
// ***************************************************************************
// CVegetableScalePage message handlers
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
BOOL CVegetableScalePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// position setup.
	uint	x= 5;
	// Position of the density DlgBox relative to CVegetableDensityPage.
	uint	y= 10;
	uint	spaceDy= 10;

	// Init _SxyDlg Dialog.
	_SxyDlg = new CVegetableNoiseValueDlg (std::string("Scale X/Y"));
	_SxyDlg->setDefaultRangeAbs(NL_VEGETABLE_SCALE_RANGE_MIN, NL_VEGETABLE_SCALE_RANGE_MAX);
	_SxyDlg->setDefaultRangeRand(NL_VEGETABLE_SCALE_RANGE_MIN, NL_VEGETABLE_SCALE_RANGE_MAX);
	_SxyDlg->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);
	_SxyDlg->init(x, y, this);
	y+= CVegetableNoiseValueDlg::ControlHeight + spaceDy;
	// Init _SzDlg Dialog.
	_SzDlg = new CVegetableNoiseValueDlg (std::string("Scale Z"));
	_SzDlg->setDefaultRangeAbs(NL_VEGETABLE_SCALE_RANGE_MIN, NL_VEGETABLE_SCALE_RANGE_MAX);
	_SzDlg->setDefaultRangeRand(NL_VEGETABLE_SCALE_RANGE_MIN, NL_VEGETABLE_SCALE_RANGE_MAX);
	_SzDlg->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);
	_SzDlg->init(x, y, this);
	y+= CVegetableNoiseValueDlg::ControlHeight + spaceDy;


	// Init BendFreqFactorDlg Dialog.
	_BendFreqFactorDlg = new CDirectEditableRangeFloat (std::string("VEGET_BEND_FREQ_FACTOR"), 
		NL_VEGETABLE_BENDFREQ_RANGE_MIN, NL_VEGETABLE_BENDFREQ_RANGE_MAX, "FreqFactor");
	_BendFreqFactorDlg->enableLowerBound(0, false);
	y+= 25;
	_BendFreqFactorDlg->init(x+10, y, this);
	y+= CDirectEditableRangeFloat::ControlHeight + spaceDy;

		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
