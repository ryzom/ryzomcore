// InstallSuccessDlg.cpp : implementation file
//

#include "stdpch.h"
#include "client_background_downloader.h"
#include "install_success_dlg.h"
#include "nel/misc/win32_util.h"
#include "install_logic/game_downloader.h"
#include "nel/misc/i18n.h"


/////////////////////////////////////////////////////////////////////////////
// CInstallSuccessDlg dialog


CInstallSuccessDlg::CInstallSuccessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInstallSuccessDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInstallSuccessDlg)
	m_StartRyzomNow = TRUE;
	//}}AFX_DATA_INIT
}


void CInstallSuccessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInstallSuccessDlg)
	DDX_Control(pDX, IDC_CONGRATS, m_CongratText);
	DDX_Check(pDX, IDC_START_RYZOM_NOW, m_StartRyzomNow);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInstallSuccessDlg, CDialog)
	//{{AFX_MSG_MAP(CInstallSuccessDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_START_RYZOM_NOW, OnStartRyzomNow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInstallSuccessDlg message handlers

BOOL CInstallSuccessDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetIcon(AfxGetApp()->LoadIcon(IDI_MAIN_ICON), TRUE);
	NLMISC::CWin32Util::localizeWindow(this->m_hWnd);	
	((CButton *) GetDlgItem(IDC_START_RYZOM_NOW))->SetCheck(IGameDownloader::getInstance()->getStartRyzomAtEnd());
	// Set the title on the content of the window to its initial value
	m_CongratText.setFont(14, "Verdana", CFE_BOLD);
	m_CongratText.append(NLMISC::CI18N::get("uiBGD_TitleInstallSuccess"));
	m_CongratText.append(ucstring(std::string("\n\n")));
	m_CongratText.setFont(12, "Verdana", 0);
	m_CongratText.append(NLMISC::CI18N::get("uiBGD_ContentInstallSuccess"));


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInstallSuccessDlg::OnClose() 
{
	// when clicking on the cancel button or the close window button whe close the window	
	IGameDownloader::getInstance()->doAction("download_cancel");	
	if (IGameDownloader::getInstance()->getState() == IGameDownloader::InstallFinished)
	{
		CDialog::OnClose();
	}
}

void CInstallSuccessDlg::OnStartRyzomNow() 
{
	// update the "Start Ryzom At End Of Install" state with the check button
	bool state = ((CButton *) GetDlgItem(IDC_START_RYZOM_NOW))->GetCheck();
	IGameDownloader::getInstance()->doAction("launch_start_at_end", state);	
	
}
