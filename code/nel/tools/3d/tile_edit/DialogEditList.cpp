// DialogEditList.cpp : implementation file
//

#include "stdafx.h"
#include "tile_edit_exe.h"
#include "DialogEditList.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogEditList dialog


CDialogEditList::CDialogEditList(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogEditList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogEditList)
	//}}AFX_DATA_INIT
}


void CDialogEditList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogEditList)
	DDX_Control(pDX, IDC_LIST1, m_ctrlList);
	DDX_Control(pDX, IDC_COMBO1, m_ctrlCombo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogEditList, CDialog)
	//{{AFX_MSG_MAP(CDialogEditList)
	ON_BN_CLICKED(ID_ADD, OnAdd)
	ON_BN_CLICKED(ID_DEL, OnDel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogEditList message handlers

void CDialogEditList::OnAdd() 
{
	// TODO: Add your control notification handler code here
	UpdateData ();
	CString str;
	m_ctrlCombo.GetWindowText (str);
	if (m_ctrlList.FindStringExact (0, str)==LB_ERR)
	{
		m_ctrlList.InsertString (-1, str);
	}
	UpdateData (FALSE);
}

BOOL CDialogEditList::OnInitDialog() 
{
	CDialog::OnInitDialog();

	OnInit ();
	UpdateData ();
	m_ctrlCombo.SetCurSel (0);
	UpdateData (FALSE);

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogEditList::OnOK() 
{
	// TODO: Add extra validation here
	OnOk ();
	
	CDialog::OnOK();
}

void CDialogEditList::OnDel() 
{
	// TODO: Add your control notification handler code here
	UpdateData ();
	if (m_ctrlList.GetSelCount())
	{
		std::vector<int> vect (m_ctrlList.GetSelCount());
		m_ctrlList.GetSelItems (m_ctrlList.GetSelCount(), &*vect.begin());
		for (int i=m_ctrlList.GetSelCount()-1; i>=0; i--)
			m_ctrlList.DeleteString (vect[i]);
		m_ctrlCombo.SetCurSel (0);
	}
	UpdateData (FALSE);	
}
