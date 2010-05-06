// global_wind_dlg.cpp : implementation file
//

#include "std_afx.h"

#include "object_viewer.h"
#include "global_wind_dlg.h"


#define	NL_GLOBAL_WIND_SLIDER_RANGE		1000

/////////////////////////////////////////////////////////////////////////////
// CGlobalWindDlg dialog


CGlobalWindDlg::CGlobalWindDlg(CObjectViewer *objViewer, CWnd* pParent /*=NULL*/)
	: CDialog(CGlobalWindDlg::IDD, pParent), _ObjViewer(objViewer)
{
	//{{AFX_DATA_INIT(CGlobalWindDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGlobalWindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalWindDlg)
	DDX_Control(pDX, IDC_STATIC_GLOBAL_WIND_POWER, StaticPower);
	DDX_Control(pDX, IDC_SLIDER_GLOBAL_WIND_POWER, SliderPower);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalWindDlg, CDialog)
	//{{AFX_MSG_MAP(CGlobalWindDlg)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_GLOBAL_WIND_POWER, OnReleasedcaptureSliderGlobalWindPower)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ***************************************************************************
void	CGlobalWindDlg::updateView()
{
	float	a;
	char	str[256];

	// update Power.
	a= _ObjViewer->getGlobalWindPower();
	sprintf(str, "%.2f", a);
	StaticPower.SetWindowText(str);
	NLMISC::clamp(a, 0.f, 1.f);
	SliderPower.SetPos((sint)(a*NL_GLOBAL_WIND_SLIDER_RANGE));
}

// ***************************************************************************
// ***************************************************************************
// CVegetableWindDlg message handlers
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
BOOL CGlobalWindDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SliderPower.SetRange(0, NL_GLOBAL_WIND_SLIDER_RANGE);
	
	// Init them and static.
	updateView();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************
void CGlobalWindDlg::OnReleasedcaptureSliderGlobalWindPower(NMHDR* pNMHDR, LRESULT* pResult) 
{
	float	a;
	a= (float)SliderPower.GetPos() / NL_GLOBAL_WIND_SLIDER_RANGE;
	_ObjViewer->setGlobalWindPower(a);

	// refersh.
	updateView();
	
	*pResult = 0;
}

// ***************************************************************************
void CGlobalWindDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// test if one of my sliders.
	CSliderCtrl	*sliderCtrl= (CSliderCtrl*)pScrollBar;
	if( sliderCtrl==&SliderPower && nSBCode==SB_THUMBPOSITION || nSBCode==SB_THUMBTRACK)
	{
		float	a;
		char	str[256];

		a= (float)nPos / NL_GLOBAL_WIND_SLIDER_RANGE;
		_ObjViewer->setGlobalWindPower(a);
		sprintf(str, "%.2f", a);
		StaticPower.SetWindowText(str);
	}
	else
	{
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	}
}

void CGlobalWindDlg::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_OBJ_GLOBAL_WIND_DLG);

	CDialog::OnDestroy();
}
