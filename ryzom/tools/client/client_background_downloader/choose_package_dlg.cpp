// choose_package_dlg.cpp : implementation file
//

#include "stdpch.h"
#include "client_background_downloader.h"
#include "choose_package_dlg.h"
#include "nel/misc/win32_util.h"
#include "install_logic/game_downloader.h"

/////////////////////////////////////////////////////////////////////////////
// CChoosePackageDlg dialog


CChoosePackageDlg::CChoosePackageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChoosePackageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChoosePackageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChoosePackageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChoosePackageDlg)
	DDX_Control(pDX, IDC_PACKAGE_TEXT, m_PackageText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChoosePackageDlg, CDialog)
	//{{AFX_MSG_MAP(CChoosePackageDlg)
	ON_BN_CLICKED(IDC_TORRENT, OnTorrent)
	ON_BN_CLICKED(IDC_FULL_PACKAGE, OnFullInstall)
	ON_BN_CLICKED(ID_CANCEL, OnCancel)	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChoosePackageDlg message handlers

BOOL CChoosePackageDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetIcon(AfxGetApp()->LoadIcon(IDI_MAIN_ICON), TRUE);
	NLMISC::CWin32Util::localizeWindow(this->m_hWnd);
	IGameDownloader* gd = IGameDownloader::getInstance();	
	gd->registerChoosePackageDlg(this);
	//set Labels initial values
	gd->updateStartProcessText();
	// set Check box initial value	
	((CButton *) GetDlgItem(IDC_FULL_PACKAGE))->SetCheck(gd->getUseFullVersion());	
	// Do not display the Use torrent lable anymore
	((CButton *) GetDlgItem(IDC_TORRENT))->SetCheck(gd->getUseTorrent());	
	((CButton *) GetDlgItem(IDC_TORRENT))->EnableWindow(  gd->getTorrentExist() );		
	((CButton *) GetDlgItem(IDC_TORRENT))->ShowWindow(SW_HIDE);

	
	NLMISC::CWin32Util::localizeWindow(this->m_hWnd);		

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChoosePackageDlg::OnCancel() 
{	
	// if on click on exit button or in the close button a widget is display. We check the current state in order to avoid cancel button -> onCancel->"do you want to quit Ryzom"->Yes->OnCancel ...
	IGameDownloader* gd = IGameDownloader::getInstance();
	if (gd->getState() != IGameDownloader::InstallFinished)
	{
		gd->doAction("download_cancel");	
		if (gd->getState() == IGameDownloader::InstallFinished)
		{
			EndDialog(IDCANCEL);
			//CDialog::OnClose();
		}		
	}
}

void CChoosePackageDlg::OnTorrent() 
{
	//activate the torrent mode (only active if button is display on Torrent Mode is enabled)	
	bool checked = static_cast<bool>(((CButton *) GetDlgItem(IDC_TORRENT))->GetCheck()==1);
	IGameDownloader* gd = IGameDownloader::getInstance();	
	gd->doAction("download_use_torrent", checked);
	
}

void CChoosePackageDlg::OnFullInstall() 
{	
	// activate the full install mode by clicking in the check box	
	bool checked = static_cast<bool>(((CButton *) GetDlgItem(IDC_FULL_PACKAGE))->GetCheck()==1);
	IGameDownloader* gd = IGameDownloader::getInstance();
	gd->doAction("download_full_version", checked);
}

void CChoosePackageDlg::OnMinimalInstall() 
{
	// obsolet code
	IGameDownloader* gd = IGameDownloader::getInstance();
	gd->doAction("download_full_version", false);	
}

void CChoosePackageDlg::setInstallText(const ucstring& title, const ucstring& content)
{
	// update labels 
	m_PackageText.Clear();
	m_PackageText.clear();
	m_PackageText.setFont(14, "Verdana", CFE_BOLD);
	m_PackageText.append(title);

	ucstring content2 = "\n\n";
	content2 += content;
	m_PackageText.setFont(12, "Verdana", 0);
	m_PackageText.append(content2);
}
