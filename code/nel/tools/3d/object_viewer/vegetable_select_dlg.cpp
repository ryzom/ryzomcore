// vegetable_select_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "vegetable_select_dlg.h"
#include "vegetable_dlg.h"


/////////////////////////////////////////////////////////////////////////////
// CVegetableSelectDlg dialog


CVegetableSelectDlg::CVegetableSelectDlg(CVegetableDlg *vegetableDlg, CWnd* pParent /*=NULL*/)
	: CDialog(CVegetableSelectDlg::IDD, pParent), _VegetableDlg(vegetableDlg)
{
	//{{AFX_DATA_INIT(CVegetableSelectDlg)
	VegetableSelected = -1;
	//}}AFX_DATA_INIT
}


void CVegetableSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVegetableSelectDlg)
	DDX_Control(pDX, IDC_LIST1, VegetableList);
	DDX_LBIndex(pDX, IDC_LIST1, VegetableSelected);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVegetableSelectDlg, CDialog)
	//{{AFX_MSG_MAP(CVegetableSelectDlg)
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVegetableSelectDlg message handlers

BOOL CVegetableSelectDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Init the control list.
	uint	num= _VegetableDlg->getNumVegetables();
	for(uint i=0; i<num; i++)
	{
		VegetableList.AddString(_VegetableDlg->getVegetableName(i).c_str());
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVegetableSelectDlg::OnDblclkList1() 
{
	UpdateData();
	// DblClck select the name.
	EndDialog(IDOK);
}
