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

// vegetable_noise_value_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "vegetable_noise_value_dlg.h"
#include "vegetable_edit_tools.h"
#include "nel/misc/noise_value.h"





#define	NL_VEGETABLE_EDIT_SLIDER_NVS_SIZE	1000
#define	NL_VEGETABLE_EDIT_SLIDER_NVS_SIZE_THRESHOLD		0.03f
#define	NL_VEGETABLE_EDIT_SLIDER_NVS_SCALE	3.0f


/////////////////////////////////////////////////////////////////////////////
// CVegetableNoiseValueDlg dialog


CVegetableNoiseValueDlg::CVegetableNoiseValueDlg(const std::string &noiseValueName)
	: _AbsValue(NULL), _RandValue(NULL), _Frequency(NULL), _NoiseValue(NULL)
{
	_TitleName= noiseValueName;
	_EnteringScalerSlider= false;


	// Default defaults.
	_DefAbsRangeMin=0; _DefAbsRangeMax=1;
	_DefRandRangeMin=0; _DefRandRangeMax=1;
	_DefFreqRangeMin=0; _DefFreqRangeMax=1;


	//{{AFX_DATA_INIT(CVegetableNoiseValueDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void	CVegetableNoiseValueDlg::setDefaultRangeAbs(float defRangeMin, float defRangeMax)
{
	_DefAbsRangeMin= defRangeMin; 
	_DefAbsRangeMax= defRangeMax; 
}
void	CVegetableNoiseValueDlg::setDefaultRangeRand(float defRangeMin, float defRangeMax)
{
	_DefRandRangeMin= defRangeMin; 
	_DefRandRangeMax= defRangeMax; 
}
void	CVegetableNoiseValueDlg::setDefaultRangeFreq(float defRangeMin, float defRangeMax)
{
	_DefFreqRangeMin= defRangeMin; 
	_DefFreqRangeMax= defRangeMax; 
}


BOOL CVegetableNoiseValueDlg::EnableWindow( BOOL bEnable)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	_AbsValue->EnableWindow(bEnable);
	_RandValue->EnableWindow(bEnable);
	_Frequency->EnableWindow(bEnable);
	SliderNoiseValue.EnableWindow(bEnable);

	UpdateData(FALSE);

	return CEditAttribDlg::EnableWindow(bEnable);
}

void CVegetableNoiseValueDlg::init(uint32 x, uint32 y, CWnd *pParent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	Create(IDD_VEGETABLE_NOISE_VALUE_DLG, pParent);
	RECT r;
	GetClientRect(&r);
	MoveWindow(x, y, r.right, r.bottom);
	ShowWindow(SW_SHOW);
}


void CVegetableNoiseValueDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVegetableNoiseValueDlg)
	DDX_Control(pDX, IDC_STATIC_VEGETABLE_SCALE, StaticScaleMarker);
	DDX_Control(pDX, IDC_SLIDER_VEGETABLE_SCALE_NOISE, SliderNoiseValue);
	DDX_Control(pDX, IDC_VEGETABLE_NOISE_VALUE_NAME, NoiseValueName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVegetableNoiseValueDlg, CDialog)
	//{{AFX_MSG_MAP(CVegetableNoiseValueDlg)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_VEGETABLE_SCALE_NOISE, OnReleasedcaptureSliderVegetableScaleNoise)
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CVegetableNoiseValueDlg message handlers


BOOL CVegetableNoiseValueDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// position setup for 3 sliders.
	uint	x= 10;
	uint	y= 8;
	uint	dy= CDirectEditableRangeFloat::ControlHeight;
	
	// Init AbsValue Slider.
	_AbsValue= new CDirectEditableRangeFloat (_TitleName + "_VEGET_" + std::string("AbsValue"), 
		_DefAbsRangeMin, _DefAbsRangeMax, "AbsValue:");
	_AbsValue->init(x, y, this);
	y+= dy;
	
	// Init RandValue Slider.
	_RandValue= new CDirectEditableRangeFloat (_TitleName + "_VEGET_" + std::string("RandValue"), 
		_DefRandRangeMin, _DefRandRangeMax, "RandValue:");
	_RandValue->init(x, y, this);
	y+= dy;
	
	// Init Frequency Slider.
	_Frequency= new CDirectEditableRangeFloat (_TitleName + "_VEGET_" + std::string("Frequency"), 
		_DefFreqRangeMin, _DefFreqRangeMax, "Frequency:");
	_Frequency->enableLowerBound(0, true);
	_Frequency->init(x, y, this);
	y+= dy;
	

	// Init the scaler of noiseValue
	SliderNoiseValue.SetRange(0, NL_VEGETABLE_EDIT_SLIDER_NVS_SIZE);
	SliderNoiseValue.SetPos(NL_VEGETABLE_EDIT_SLIDER_NVS_SIZE/2);


	// Set the name.
	NoiseValueName.SetWindowText(nlUtf8ToTStr(_TitleName));


	// if previously setuped, setup now the noiseValue.
	if(_NoiseValue)
		setNoiseValue(_NoiseValue, _VegetableRefresh);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


CVegetableNoiseValueDlg::~CVegetableNoiseValueDlg()
{
	#define  REMOVE_WND(wnd) if (wnd) { wnd->DestroyWindow(); delete wnd; }
	REMOVE_WND(_AbsValue);	
	REMOVE_WND(_RandValue);	
	REMOVE_WND(_Frequency);	
}


void			CVegetableNoiseValueDlg::setNoiseValue(NLMISC::CNoiseValue	*nv, IVegetableRefresh *vegetRefresh)
{
	_NoiseValue= nv;
	_VegetableRefresh= vegetRefresh;

	// Set pointer to edit value.
	if(_AbsValue)	_AbsValue->setFloat(&nv->Abs, vegetRefresh);
	if(_RandValue)	_RandValue->setFloat(&nv->Rand, vegetRefresh);
	if(_Frequency)	_Frequency->setFloat(&nv->Frequency, vegetRefresh);
}

void CVegetableNoiseValueDlg::OnReleasedcaptureSliderVegetableScaleNoise(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Get value from slider
	sint	value= SliderNoiseValue.GetPos();

	//applyScale
	if(!_EnteringScalerSlider)
	{
		_EnteringScalerSlider= true;
		_BkupAbsValue= _NoiseValue->Abs;
		_BkupRandValue= _NoiseValue->Rand;
	}
	applyScaleSlider(value);

	// And reset 
	SliderNoiseValue.SetPos(NL_VEGETABLE_EDIT_SLIDER_NVS_SIZE/2);
	_EnteringScalerSlider= false;
	StaticScaleMarker.SetWindowText(_T("100%"));
	
	// Must update display.
	_VegetableRefresh->refreshVegetableDisplay();


	*pResult = 0;
}


void CVegetableNoiseValueDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSliderCtrl	*sliderCtrl= (CSliderCtrl*)pScrollBar;
	if(sliderCtrl==&SliderNoiseValue && nSBCode==SB_THUMBPOSITION || nSBCode==SB_THUMBTRACK)
	{
		if(!_EnteringScalerSlider)
		{
			_EnteringScalerSlider= true;
			_BkupAbsValue= _NoiseValue->Abs;
			_BkupRandValue= _NoiseValue->Rand;
		}

		//applyScale
		applyScaleSlider(nPos);
	}
	else
		CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CVegetableNoiseValueDlg::applyScaleSlider(sint scrollValue)
{
	// get scale beetween -1 and 1.
	float	scale= (NL_VEGETABLE_EDIT_SLIDER_NVS_SIZE/2-scrollValue)/(float)(NL_VEGETABLE_EDIT_SLIDER_NVS_SIZE/2);
	float	factor;

	// scale down
	if(fabs(scale)<NL_VEGETABLE_EDIT_SLIDER_NVS_SIZE_THRESHOLD)
		factor=1;
	else if(scale<0)
	{
		float	minv= 1.0f / NL_VEGETABLE_EDIT_SLIDER_NVS_SCALE;
		factor= minv*(-scale) + 1.0f*(1+scale);
	}
	// scale up
	else
	{
		float	maxv= NL_VEGETABLE_EDIT_SLIDER_NVS_SCALE;
		factor= maxv*(scale) + 1.0f*(1-scale);
	}

	// Apply to noiseValue. NB: don't do a IPSWrapper::set(), because this cause a refreshVegetableDisplay
	// And we don't want it during sliding (too slow).
	_NoiseValue->Abs= _BkupAbsValue * factor;
	_NoiseValue->Rand= _BkupRandValue * factor;

	// udpate sliders view.
	_AbsValue->updateValueFromReader();
	_RandValue->updateValueFromReader();

	// update marker text
	StaticScaleMarker.SetWindowText(nlUtf8ToTStr(NLMISC::toString("%d%%", (sint)(factor * 100))));
}
