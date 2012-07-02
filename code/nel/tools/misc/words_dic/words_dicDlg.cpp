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

#include "StdAfx.h"
#include "words_dic.h"
#include "words_dicDlg.h"
#include "DicSplashScreen.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "nel/misc/words_dictionary.h"

using namespace std;
using namespace NLMISC;

CWordsDictionary	Dico;
CDicSplashScreen	*SplashScreen;


/////////////////////////////////////////////////////////////////////////////
// CWords_dicDlg dialog

CWords_dicDlg::CWords_dicDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWords_dicDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWords_dicDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWords_dicDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWords_dicDlg)
	DDX_Control(pDX, IDC_LookUp, m_LookUp);
	DDX_Control(pDX, IDC_ResultList, m_Results);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWords_dicDlg, CDialog)
	//{{AFX_MSG_MAP(CWords_dicDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_LookUp, OnChangeLookUp)
	ON_BN_CLICKED(IDC_BUTTON1, OnBtnFind)
	ON_BN_CLICKED(IDC_BUTTON2, OnBtnClear)
	ON_LBN_DBLCLK(IDC_ResultList, OnDblclkResultList)
	ON_LBN_SELCHANGE(IDC_ResultList, OnSelchangeResultList)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_FileList, OnFileList)
	ON_BN_CLICKED(IDC_ShowAll, OnShowAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWords_dicDlg message handlers

BOOL CWords_dicDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	SplashScreen = new CDicSplashScreen();
	SplashScreen->Create( IDD_SplashScreen, NULL );
	SplashScreen->ShowWindow( SW_SHOW );
	SplashScreen->SetWindowPos( &wndTop, 400, 300, 0,0, SWP_NOSIZE );
	SplashScreen->GetDlgItem( IDC_SplashText )->SetWindowText( "Please wait while loading dictionary..." );
	if ( ! Dico.init() )
		AfxMessageBox( "Can't init dictionary, see reason in log.log" );
	SplashScreen->DestroyWindow();
	delete SplashScreen;
	GetDlgItem( IDC_Status )->SetWindowText( "Tip: ^ and $ can be used to represent the start and the end of string" );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWords_dicDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWords_dicDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


/*
 *
 */
void CWords_dicDlg::OnChangeLookUp() 
{
	CString inputStr;
	m_LookUp.GetWindowText( inputStr );

	// Ignore if input string too short
	if ( inputStr.GetLength() < 3 )
		return;

	lookUp( inputStr );
}


/*
 *
 */
void CWords_dicDlg::lookUp( const CString& inputStr )
{
	// Look up
	CVectorSString resultVec;
	Dico.lookup( CSString(inputStr), resultVec );

	// Display results
	clear();
	if ( resultVec.empty() )
	{
		m_Results.AddString( "<no result>" );
		return;
	}
	else
	{
		bool showAll = (((CButton*)(GetDlgItem( IDC_ShowAll )))->GetCheck() == 1);
		bool lvlRemoved = false;
		m_Results.SetRedraw( false );
		for ( CVectorSString::const_iterator ivs=resultVec.begin(); ivs!=resultVec.end(); ++ivs )
		{
			const CSString& res = (*ivs);
			if ( showAll || (res.find( "lvl" ) == string::npos) )
			{
				m_Results.AddString( res.c_str() );
			}
			else
				lvlRemoved = true;
		}
		m_Results.SetRedraw( true );
		CString s;
		s.Format( "%u results found for \"%s\".%s", resultVec.size(), inputStr, lvlRemoved?" Results containing \"lvl\" not shown":"" );
		GetDlgItem( IDC_Status )->SetWindowText( s );
	}
}

/*
 *
 */
void CWords_dicDlg::OnBtnFind() 
{
	CString inputStr;
	m_LookUp.GetWindowText( inputStr );

	lookUp( inputStr );
}

/*
 *
 */
void CWords_dicDlg::clear()
{
	GetDlgItem( IDC_Status )->SetWindowText( "" );
	m_Results.ResetContent();
}

/*
 *
 */
void CWords_dicDlg::OnBtnClear() 
{
	m_LookUp.SetWindowText( "" );
	clear();
}

/*
 *
 */
void CWords_dicDlg::OnShowAll() 
{
	OnBtnFind();
}

/*
 *
 */
void CWords_dicDlg::OnFileList() 
{
	clear();
	const vector<string>& fileList = Dico.getFileList();
	for ( vector<string>::const_iterator ifl=fileList.begin(); ifl!=fileList.end(); ++ifl )
	{
		m_Results.AddString( (*ifl).c_str() );
	}
}

/*
 *
 */
void CWords_dicDlg::OnDblclkResultList() 
{
}

/*
 *
 */
void CWords_dicDlg::OnSelchangeResultList() 
{
	// Get selection
	CString resStr;
	m_Results.GetText( m_Results.GetCurSel(), resStr );
	CSString key = Dico.getWordsKey( CSString(resStr) );

	// Copy the selection into the clipboard
	if ( OpenClipboard() )
	{
		HGLOBAL mem = GlobalAlloc (GHND|GMEM_DDESHARE, key.size()+1);
		if (mem)
		{
			char *pmem = (char*)GlobalLock( mem );
			strcpy( pmem, key.c_str() );
			GlobalUnlock( mem );
			EmptyClipboard();
			SetClipboardData( CF_TEXT, mem );
		}
		CloseClipboard();
		if ( mem )
		{
			CString s;
			s.Format( "\"%s\" copied into the clipboard", key.c_str() );
			GetDlgItem( IDC_Status )->SetWindowText( s );
		}
	}
	else
	{
		GetDlgItem( IDC_Status )->SetWindowText( "Cannot access the clipboard" );
	}	
}

/*
 *
 */
void CWords_dicDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	// Skip OnSize called before all child windows are ready
	if ( ! ::IsWindow( m_Results.m_hWnd ) )
		return;
	
	CRect mr, lr, sr;
	GetClientRect( &mr );
	m_Results.GetWindowRect( &lr );
	ScreenToClient( &lr );
	CWnd *status = GetDlgItem( IDC_Status );
	status->GetWindowRect( &sr );
	ScreenToClient( &sr );

	lr.right = mr.right - 12;
	lr.bottom = mr.bottom - 28;
	sr.top = mr.bottom - 20;

	m_Results.MoveWindow( &lr );
	status->MoveWindow( &sr );
}
