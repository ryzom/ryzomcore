// about_dialog.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "about_dialog.h"

/////////////////////////////////////////////////////////////////////////////
// CAboutDialog dialog


CAboutDialog::CAboutDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAboutDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAboutDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAboutDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAboutDialog, CDialog)
	//{{AFX_MSG_MAP(CAboutDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutDialog message handlers

BOOL CAboutDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Get the module path
	HMODULE hModule = AfxGetInstanceHandle();
	nlassert(hModule); // shouldn't be null now anymore in any case
	nlassert(hModule != GetModuleHandle(NULL)); // if this is dll, the module handle can't be same as exe
	if (hModule)
	{
		// Find the verion resource
		HRSRC hRSrc=FindResource (hModule, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
		if (hRSrc)
		{
			HGLOBAL hGlobal=LoadResource (hModule, hRSrc);
			if (hGlobal)
			{
				void *pInfo=LockResource (hGlobal);
				if (pInfo)
				{
					uint *versionTab;
					uint versionSize;
					if (VerQueryValue (pInfo, "\\", (void**)&versionTab,  &versionSize))
					{
						// Get the pointer on the structure
						VS_FIXEDFILEINFO *info=(VS_FIXEDFILEINFO*)versionTab;

 						// Setup version number
						char version[512];
						sprintf (version, "Version %d.%d.%d.%d", 
							info->dwFileVersionMS>>16, 
							info->dwFileVersionMS&0xffff, 
							info->dwFileVersionLS>>16,  
							info->dwFileVersionLS&0xffff);
						GetDlgItem (IDC_VERSION)->SetWindowText (version);
					}
				}
			}
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

