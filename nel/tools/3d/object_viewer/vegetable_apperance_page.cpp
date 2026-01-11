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

// VegetableApperancePage.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "vegetable_apperance_page.h"
#include "vegetable_edit_tools.h"
#include "vegetable_noise_value_dlg.h"
#include "vegetable_select_dlg.h"
#include "vegetable_dlg.h"
#include "nel/misc/noise_value.h"
#include "nel/3d/vegetable.h"


/////////////////////////////////////////////////////////////////////////////
// CVegetableApperancePage property page

IMPLEMENT_DYNCREATE(CVegetableApperancePage, CPropertyPage)



CVegetableApperancePage::CVegetableApperancePage() : CPropertyPage(CVegetableApperancePage::IDD),
	_BendPhaseDlg(NULL), _BendFactorDlg(NULL), _ColorDlg(NULL)
{
	//{{AFX_DATA_INIT(CVegetableApperancePage)
	//}}AFX_DATA_INIT
}

CVegetableApperancePage::~CVegetableApperancePage()
{
	#define  REMOVE_WND(wnd) if (wnd) { wnd->DestroyWindow(); delete wnd; }
	REMOVE_WND(_BendPhaseDlg);
	REMOVE_WND(_BendFactorDlg);
	REMOVE_WND(_ColorDlg);
}

void CVegetableApperancePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVegetableApperancePage)
	DDX_Control(pDX, IDC_LIST_VEGETABLE_COLOR, ColorList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVegetableApperancePage, CPropertyPage)
	//{{AFX_MSG_MAP(CVegetableApperancePage)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_ADD, OnButtonVegetableAdd)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_GETOTHER, OnButtonVegetableGetother)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_INSERT, OnButtonVegetableInsert)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_REMOVE, OnButtonVegetableRemove)
	ON_LBN_DBLCLK(IDC_LIST_VEGETABLE_COLOR, OnDblclkListVegetableColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
void CVegetableApperancePage::setVegetableToEdit(NL3D::CVegetable *vegetable)
{
	_Vegetable= vegetable;

	// If not NULL, init all controls.
	if(_Vegetable)
	{
		// init noise dlg.
		// ----------
		_BendPhaseDlg->setNoiseValue(&_Vegetable->BendPhase, _VegetableDlg);
		_BendFactorDlg->setNoiseValue(&_Vegetable->BendFactor, _VegetableDlg);
		_ColorDlg->setNoiseValue(&_Vegetable->Color.NoiseValue, _VegetableDlg);

		// init color dlg.
		// ----------
		readFromVegetableColor(_Vegetable);

	}

}

// ***************************************************************************
void CVegetableApperancePage::readFromVegetableColor(NL3D::CVegetable *vegetable)
{
	// clear all
	ColorList.clear();
	// fill list.
	for(uint i=0; i<vegetable->Color.Gradients.size(); i++)
	{
		CRGBA	color= vegetable->Color.Gradients[i];
		ColorList.addValue(color);
	}
}

// ***************************************************************************
void CVegetableApperancePage::writeToVegetableColor(NL3D::CVegetable *vegetable)
{
	// setup the vegetable 
	vegetable->Color.Gradients.clear();
	for(uint i=0; i<ColorList.getColors().size(); i++)
	{
		CRGBA	color= ColorList.getColors()[i];
		vegetable->Color.Gradients.push_back(color);
	}
}


// ***************************************************************************
// ***************************************************************************
// CVegetableScalePage message handlers
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
BOOL CVegetableApperancePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// position setup.
	uint	x= 5;
	// Position of the density DlgBox relative to CVegetableDensityPage.
	uint	y= 10;
	uint	spaceDy= 10;

	// Init BendPhase Dialog.
	_BendPhaseDlg = new CVegetableNoiseValueDlg (std::string("BendPhase"));
	_BendPhaseDlg->setDefaultRangeAbs(NL_VEGETABLE_BENDPHASE_RANGE_MIN, NL_VEGETABLE_BENDPHASE_RANGE_MAX);
	_BendPhaseDlg->setDefaultRangeRand(NL_VEGETABLE_BENDPHASE_RANGE_MIN, NL_VEGETABLE_BENDPHASE_RANGE_MAX);
	_BendPhaseDlg->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);
	_BendPhaseDlg->init(x, y, this);
	y+= CVegetableNoiseValueDlg::ControlHeight + spaceDy;
	// Init BendFactor Dialog.
	_BendFactorDlg = new CVegetableNoiseValueDlg (std::string("BendFactor"));
	_BendFactorDlg->setDefaultRangeAbs(NL_VEGETABLE_BENDFACTOR_RANGE_MIN, NL_VEGETABLE_BENDFACTOR_RANGE_MAX);
	_BendFactorDlg->setDefaultRangeRand(NL_VEGETABLE_BENDFACTOR_RANGE_MIN, NL_VEGETABLE_BENDFACTOR_RANGE_MAX);
	_BendFactorDlg->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);
	_BendFactorDlg->init(x, y, this);
	y+= CVegetableNoiseValueDlg::ControlHeight + spaceDy;
	// Init Color Dialog.
	_ColorDlg = new CVegetableNoiseValueDlg (std::string("Color Noise"));
	_ColorDlg->setDefaultRangeAbs(NL_VEGETABLE_COLOR_RANGE_MIN, NL_VEGETABLE_COLOR_RANGE_MAX);
	_ColorDlg->setDefaultRangeRand(NL_VEGETABLE_COLOR_RANGE_MIN, NL_VEGETABLE_COLOR_RANGE_MAX);
	_ColorDlg->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);
	_ColorDlg->init(x, y, this);
	y+= CVegetableNoiseValueDlg::ControlHeight + spaceDy;

	// Init color part
	ColorList.setCtrlID(IDC_LIST_VEGETABLE_COLOR);
	uint	size= 28;
	ColorList.setupItem(size, size, size-10, size-10);

		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// ***************************************************************************
void CVegetableApperancePage::OnButtonVegetableAdd() 
{
	// copy the current selected color
	CRGBA	color(255, 255, 255);
	int		id= ColorList.GetCount()-1;
	if(id!=-1)
		color= ColorList.getValue(id);
	// update view and vegetable
	ColorList.addValue(color);
	writeToVegetableColor(_Vegetable);

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();
}


// ***************************************************************************
void CVegetableApperancePage::OnButtonVegetableInsert() 
{
	// copy the current selected color
	CRGBA	color(255, 255, 255);
	int		id= ColorList.GetCurSel();
	if(id!=LB_ERR)
		color= ColorList.getValue(id);
	// update view and vegetable
	ColorList.insertValueBeforeCurSel(color);
	writeToVegetableColor(_Vegetable);

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();
}

// ***************************************************************************
void CVegetableApperancePage::OnButtonVegetableRemove() 
{
	// remove curSel from the list
	ColorList.removeCurSelValue();
	writeToVegetableColor(_Vegetable);

	// update 3D view
	_VegetableDlg->refreshVegetableDisplay();
}


// ***************************************************************************
void CVegetableApperancePage::OnButtonVegetableGetother() 
{
	// Open A dialog to select the other vegetable to copy color.
	CVegetableSelectDlg		dlg(_VegetableDlg);
	if( dlg.DoModal()==IDOK )
	{
		int	id= dlg.VegetableSelected;
		if(id != LB_ERR)
		{
			// read colors from this vegetable
			NL3D::CVegetable	*otherVegetable= _VegetableDlg->getVegetable(id);
			_Vegetable->Color.Gradients= otherVegetable->Color.Gradients;
			// update view
			readFromVegetableColor(_Vegetable);

			// update 3D view
			_VegetableDlg->refreshVegetableDisplay();
		}
	}
}

// ***************************************************************************
void CVegetableApperancePage::OnDblclkListVegetableColor() 
{
	CRGBA	color;

	// get the current color of the value.
	int		id= ColorList.GetCurSel();
	if(id!=LB_ERR)
	{
		color= ColorList.getValue(id);
		
		// Open a colorDialog.
		CColorDialog	colorDialog(RGB(color.R, color.G, color.B), CC_FULLOPEN);
		if( colorDialog.DoModal()==IDOK )
		{
			// update view
			COLORREF cref = colorDialog.GetColor();
			color.set(GetRValue(cref), GetGValue(cref), GetBValue(cref));
   			ColorList.changeCurSelValue(color);
			// update vegetable
			writeToVegetableColor(_Vegetable);

			// update 3D view
			_VegetableDlg->refreshVegetableDisplay();
		}
	}
}
