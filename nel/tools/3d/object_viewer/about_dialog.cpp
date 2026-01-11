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
					if (VerQueryValue (pInfo, _T("\\"), (void**)&versionTab,  &versionSize))
					{
						// Get the pointer on the structure
						VS_FIXEDFILEINFO *info = (VS_FIXEDFILEINFO*)versionTab;

 						// Setup version number
						TCHAR version[512];
						_stprintf (version, _T("Version %d.%d.%d.%d"),
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

