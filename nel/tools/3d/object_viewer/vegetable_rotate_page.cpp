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

// vegetable_rotate_page.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "vegetable_rotate_page.h"
#include "vegetable_edit_tools.h"
#include "vegetable_noise_value_dlg.h"
#include "vegetable_dlg.h"
#include "nel/misc/noise_value.h"
#include "nel/3d/vegetable.h"


/////////////////////////////////////////////////////////////////////////////
// CVegetableRotatePage property page

IMPLEMENT_DYNCREATE(CVegetableRotatePage, CPropertyPage)

CVegetableRotatePage::CVegetableRotatePage() : CPropertyPage(CVegetableRotatePage::IDD),
	_RxDlg(NULL), _RyDlg(NULL), _RzDlg(NULL)
{
	//{{AFX_DATA_INIT(CVegetableRotatePage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CVegetableRotatePage::~CVegetableRotatePage()
{
	#define  REMOVE_WND(wnd) if (wnd) { wnd->DestroyWindow(); delete wnd; }
	REMOVE_WND(_RxDlg);
	REMOVE_WND(_RyDlg);
	REMOVE_WND(_RzDlg);
}

void CVegetableRotatePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVegetableRotatePage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVegetableRotatePage, CPropertyPage)
	//{{AFX_MSG_MAP(CVegetableRotatePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
void CVegetableRotatePage::setVegetableToEdit(NL3D::CVegetable *vegetable)
{
	_Vegetable= vegetable;

	// If not NULL, init all controls.
	if(_Vegetable)
	{
		// init all dlg.
		// ----------
		_RxDlg->setNoiseValue(&_Vegetable->Rx, _VegetableDlg);
		_RyDlg->setNoiseValue(&_Vegetable->Ry, _VegetableDlg);
		_RzDlg->setNoiseValue(&_Vegetable->Rz, _VegetableDlg);
	}

}


// ***************************************************************************
// ***************************************************************************
// CVegetableRotatePage message handlers
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
BOOL CVegetableRotatePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// position setup.
	uint	x= 5;
	// Position of the density DlgBox relative to CVegetableDensityPage.
	uint	y= 10;
	uint	spaceDy= 10;

	// Init _RxDlg Dialog.
	_RxDlg = new CVegetableNoiseValueDlg (std::string("Rotate X"));
	_RxDlg->setDefaultRangeAbs(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_RxDlg->setDefaultRangeRand(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_RxDlg->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);
	_RxDlg->init(x, y, this);
	y+= CVegetableNoiseValueDlg::ControlHeight + spaceDy;
	// Init _RyDlg Dialog.
	_RyDlg = new CVegetableNoiseValueDlg (std::string("Rotate Y"));
	_RyDlg->setDefaultRangeAbs(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_RyDlg->setDefaultRangeRand(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_RyDlg->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);
	_RyDlg->init(x, y, this);
	y+= CVegetableNoiseValueDlg::ControlHeight + spaceDy;
	// Init _RzDlg Dialog.
	_RzDlg = new CVegetableNoiseValueDlg (std::string("Rotate Z"));
	_RzDlg->setDefaultRangeAbs(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_RzDlg->setDefaultRangeRand(NL_VEGETABLE_ROTATE_RANGE_MIN, NL_VEGETABLE_ROTATE_RANGE_MAX);
	_RzDlg->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);
	_RzDlg->init(x, y, this);
	y+= CVegetableNoiseValueDlg::ControlHeight + spaceDy;

		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
