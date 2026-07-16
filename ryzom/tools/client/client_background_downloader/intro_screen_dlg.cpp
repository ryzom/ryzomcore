// intro_screen_dlg.cpp : implementation file
//

#include "stdpch.h"
#include "client_background_downloader.h"
#include "intro_screen_dlg.h"
#include "nel/misc/win32_util.h"
#include "install_logic/game_downloader.h"

/////////////////////////////////////////////////////////////////////////////
// CIntroScreenDlg dialog


CIntroScreenDlg::CIntroScreenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIntroScreenDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIntroScreenDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CIntroScreenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIntroScreenDlg)	
	DDX_Control(pDX, IDC_INTRO_TEXT, m_IntroText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIntroScreenDlg, CDialog)
	//{{AFX_MSG_MAP(CIntroScreenDlg)
	ON_BN_CLICKED(IDC_NEWINSTALL, OnNewinstall)
	ON_BN_CLICKED(ID_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_REPAIR, OnRepair)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntroScreenDlg message handlers

BOOL CIntroScreenDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetIcon(AfxGetApp()->LoadIcon(IDI_MAIN_ICON), TRUE);
	NLMISC::CWin32Util::localizeWindow(this->m_hWnd);	
	((CButton *) GetDlgItem(IDC_NEWINSTALL))->SetCheck(1);

	// if first install do not check 
	if (!IGameDownloader::getInstance()->getFirstInstall())
	{
		((CButton *) GetDlgItem(IDC_REPAIR))->SetCheck(1);
		((CButton *) GetDlgItem(IDC_NEWINSTALL))->SetCheck(0);		
		((CButton *) GetDlgItem(IDC_REPAIR))->EnableWindow(TRUE);//enable repair button
	}

	

	CHARFORMAT cf;
	ZeroMemory(&cf, sizeof(cf));
	cf.cbSize = sizeof(cf);

	m_IntroText.setFont(14, "Verdana", CFE_BOLD);
	m_IntroText.append(NLMISC::CI18N::get("uiBGD_RyzomUpdateRepair"));
	m_IntroText.append(ucstring(std::string("\n\n")));

	m_IntroText.setFont(12, "Verdana", 0);
	m_IntroText.append(NLMISC::CI18N::get("uiBGD_Welcome"));
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CIntroScreenDlg::OnCancel() 
{	
	// When the Cancel button is clicked we close the window
	IGameDownloader* gd = IGameDownloader::getInstance();
	gd->doAction("download_cancel");
	if (gd->getState() == IGameDownloader::InstallFinished)
	{
		EndDialog(IDCANCEL);
	}
}

void CIntroScreenDlg::OnNewinstall() 
{	
	// clicked on the Install buton
	IGameDownloader* gd = IGameDownloader::getInstance();
	gd->setFirstInstall(true);	
}

void CIntroScreenDlg::OnRepair() 
{
	//clicked on the Repair button
	IGameDownloader* gd = IGameDownloader::getInstance();
	gd->setFirstInstall(true);	
}
