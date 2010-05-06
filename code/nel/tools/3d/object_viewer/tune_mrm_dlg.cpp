// tune_mrm_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "tune_mrm_dlg.h"
#include "main_frame.h"

/////////////////////////////////////////////////////////////////////////////
// CTuneMrmDlg dialog


#define NL_TMD_SLIDER_SIZE		10000
#define NL_TMD_SLIDER_MAX_STEP	20
#define NL_TMD_SLIDER_STEP_SIZE	5000


CTuneMrmDlg::CTuneMrmDlg(CObjectViewer *viewer, NL3D::CScene *scene, CWnd* pParent /*=NULL*/)
	: CDialog(CTuneMrmDlg::IDD, pParent), _ObjViewer(viewer), _Scene(scene)
{
	//{{AFX_DATA_INIT(CTuneMrmDlg)
	ViewCurrentMaxPoly = _T("");
	ViewMaxValue = _T("");
	//}}AFX_DATA_INIT
}

CTuneMrmDlg::~CTuneMrmDlg()
{
}

void CTuneMrmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTuneMrmDlg)
	DDX_Control(pDX, IDC_TMD_SLIDER_MAX, MaxValueSlider);
	DDX_Control(pDX, IDC_TMD_SLIDER, FaceSlider);
	DDX_Text(pDX, IDC_TDM_STATIC, ViewCurrentMaxPoly);
	DDX_Text(pDX, IDC_TDM_STATIC_MAX, ViewMaxValue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTuneMrmDlg, CDialog)
	//{{AFX_MSG_MAP(CTuneMrmDlg)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_TMD_SLIDER, OnReleasedcaptureTmdSlider)
	ON_WM_HSCROLL()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_TMD_SLIDER_MAX, OnReleasedcaptureTmdSliderMax)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTuneMrmDlg message handlers

BOOL CTuneMrmDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// init sliders ranges
	FaceSlider.SetRange(0, NL_TMD_SLIDER_SIZE);
	FaceSlider.SetPos(NL_TMD_SLIDER_SIZE);
	MaxValueSlider.SetRange(0, NL_TMD_SLIDER_MAX_STEP);
	MaxValueSlider.SetPos(NL_TMD_SLIDER_MAX_STEP);

	// init scene/texts
	applySlider(FaceSlider.GetPos(), MaxValueSlider.GetPos());
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTuneMrmDlg::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_TUNE_MRM_DLG);		
	CDialog::OnDestroy();	
}

void CTuneMrmDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	bool	handled= false;
	if(nSBCode==SB_THUMBPOSITION || nSBCode==SB_THUMBTRACK)
	{
		if(pScrollBar->GetDlgCtrlID()==FaceSlider.GetDlgCtrlID())
		{
			applySlider(nPos, MaxValueSlider.GetPos());
			handled= true;
		}
		else if(pScrollBar->GetDlgCtrlID()==MaxValueSlider.GetDlgCtrlID())
		{
			applySlider(FaceSlider.GetPos(), nPos);
			handled= true;
		}
	}

	if(!handled)
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CTuneMrmDlg::OnReleasedcaptureTmdSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Get value from slider
	applySlider(FaceSlider.GetPos(), MaxValueSlider.GetPos());
	*pResult = 0;
}

void CTuneMrmDlg::OnReleasedcaptureTmdSliderMax(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Get value from slider
	applySlider(FaceSlider.GetPos(), MaxValueSlider.GetPos());
	*pResult = 0;
}

void CTuneMrmDlg::applySlider(UINT sliderCur, UINT sliderMax)
{
	nlctassert(NL_TMD_SLIDER_SIZE*NL_TMD_SLIDER_MAX_STEP*NL_TMD_SLIDER_STEP_SIZE < ((uint)1<<31));
	// compute actual max polygon
	uint	actualValue= sliderCur*sliderMax*NL_TMD_SLIDER_STEP_SIZE / NL_TMD_SLIDER_SIZE;

	if(_Scene)
	{
		_Scene->setGroupLoadMaxPolygon("Skin", actualValue);
	}

	// refresh text views
	ViewCurrentMaxPoly= NLMISC::toString(actualValue).c_str();
	ViewMaxValue= NLMISC::toString(sliderMax*NL_TMD_SLIDER_STEP_SIZE).c_str();

	UpdateData(FALSE);
}



void CTuneMrmDlg::OnClose() 
{
	_ObjViewer->getMainFrame()->OnWindowTuneMRM();
}

