// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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


// log_analyserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "log_analyser.h"
#include "log_analyserDlg.h"
//#include <nel/misc/config_file.h>
#include <fstream>
#include <algorithm>

using namespace std;
//using namespace NLMISC;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CLog_analyserApp		theApp;
CString						LogDateString;


/*
 * Keyboard handler (in edit box)
 */
afx_msg void CLAEdit::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	switch( nChar )
	{
	// Ctrl+G: Go to selected line number
	case 'G':
		CViewDialog *view = ((CLog_analyserDlg*)(GetParent()))->getCurrentView();
		if ( view )
		{
			if ( (GetKeyState(VK_CONTROL) & 0x8000) != 0 )
			{
				// Get the selected line number
				CString str;
				GetWindowText(str);
				int start, end;
				GetSel( start, end );
				str = str.Mid( start, end-start );
				int lineNum = atoi(nlTStrToUtf8(str));
				if ( ! ((lineNum != 0) || (str == "0")) )
					break;

				// GoTo line
				view->scrollTo( lineNum );
			}
		}
		break;
	}

	// Transmit to Edit Box AND to main window
	CEdit::OnKeyDown( nChar, nRepCnt, nFlags );
	((CLog_analyserDlg*)(GetParent()))->OnKeyDown( nChar, nRepCnt, nFlags );
}


/*
 * Keyboard handler (everywhere)
 */
void CLog_analyserDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CViewDialog *view = getCurrentView();

	switch ( nChar )
	{
	// Bookmarks handling (TODO)
	case VK_F2:
		if ( view )
		{
			if ( (GetKeyState(VK_CONTROL) & 0x8000) != 0 )
				view->addBookmark();
			else
				view->recallNextBookmark();
		}
		break;

	// Ctrl+F, F3: Find
	case VK_F3:
	case 'F':
		if ( view )
		{
			if ( (nChar==VK_F3) || ((GetKeyState(VK_CONTROL) & 0x8000) != 0) )
				view->OnButtonFind();
		}
		break;

	// Ctrl+L: Display back the file list
	case 'L':
		if ( (GetKeyState(VK_CONTROL) & 0x8000) != 0 )
		{
			displayFileList();
		}
		break;
	}
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}


/////////////////////////////////////////////////////////////////////////////
// CLog_analyserDlg dialog

CLog_analyserDlg::CLog_analyserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLog_analyserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLog_analyserDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLog_analyserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLog_analyserDlg)
	DDX_Control(pDX, IDC_SCROLLBAR1, m_ScrollBar);
	DDX_Control(pDX, IDC_EDIT1, m_Edit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLog_analyserDlg, CDialog)
	//{{AFX_MSG_MAP(CLog_analyserDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_AddView, OnAddView)
	ON_BN_CLICKED(IDC_ADDTRACEVIEW, OnAddtraceview)
	ON_BN_CLICKED(IDC_ComputeTraces, OnComputeTraces)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_Reset, OnReset)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_HelpBtn, OnHelpBtn)
	ON_WM_LBUTTONUP()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_DispLineHeaders, OnDispLineHeaders)
	ON_BN_CLICKED(IDC_Analyse, OnAnalyse)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CLAEdit, CEdit)
	//{{AFX_MSG_MAP(CLAEdit)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLog_analyserDlg message handlers

BOOL CLog_analyserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CurrentView = NULL;
	Trace = false;
	ResizeViewInProgress = -1;
	((CButton*)GetDlgItem( IDC_CheckSessions ))->SetCheck( 1 );
	((CButton*)GetDlgItem( IDC_DispLineHeaders ))->SetCheck( 1 );
	((CButton*)GetDlgItem( IDC_DetectCorruptedLines ))->SetCheck( 1 );

	/*try
	{
		CConfigFile cf;
		cf.load( "log_analyser.cfg" );
		LogDateString = cf.getVar( "LogDateString" ).asString().c_str();
	}
	catch (const EConfigFile& )
	{*/
	LogDateString = "Log Starting [";
	AnalyseFunc = NULL;
	m_Edit.SetLimitText( ~0 );

	//}
	

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Add files given in command-line
	string cmdLine = NLMISC::tStrToUtf8(theApp.m_lpCmdLine);
	vector<CString> v;
	/*int pos = cmdLine.find_first_of(' '); // TODO: handle "" with blank characters
	while ( pos != string::npos )
	{
		v.push_back( cmdLine.substr( 0, pos ).c_str() );
		cmdLine = cmdLine.substr( pos );
		pos = cmdLine.find_first_of(' ');
	}*/
	if ( ! cmdLine.empty() )
	{
		v.push_back( cmdLine.c_str() );
		addView( v );
	}

	loadPluginConfiguration();

	DragAcceptFiles( true );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLog_analyserDlg::OnPaint() 
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
HCURSOR CLog_analyserDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


string getFileExtension (const string &filename)
{
	string::size_type pos = filename.find_last_of ('.');
	if (pos == string::npos)
		return "";
	else
		return filename.substr (pos + 1);
}


/*
 * Open in the same view
 */
void CLog_analyserDlg::OnDropFiles( HDROP hDropInfo )
{
	UINT nbFiles = DragQueryFile( hDropInfo, 0xFFFFFFFF, NULL, 0 );
	vector<CString> v;
	for ( UINT i=0; i!=nbFiles; ++i )
	{
		CString filename;
		DragQueryFile( hDropInfo, i, filename.GetBufferSetLength( 200 ), 200 );

		// Plug-in DLL or log file
		if ( getFileExtension( NLMISC::tStrToUtf8(filename) ) == "dll" )
		{
			if (addPlugIn(NLMISC::tStrToUtf8(filename)))
				AfxMessageBox( CString("Plugin added: ") + filename );
			else
				AfxMessageBox( CString("Plugin already registered: ") + filename );
		}
		else
		{
			v.push_back( filename );
		}
	}

	if ( ! v.empty() )
		addView( v );
}


/*
 *
 */
bool CLog_analyserDlg::addPlugIn( const std::string& dllName )
{
	int i = 0;
	char pluginN [10] = "Plugin0";
	CString pn = theApp.GetProfileString(_T(""), nlUtf8ToTStr(pluginN));
	while ( ! pn.IsEmpty() )
	{
		if (NLMISC::tStrToUtf8(pn) == dllName)
			return false; // already registered
		++i;
		smprintf( pluginN, 10, "Plugin%d", i );
		pn = theApp.GetProfileString(_T(""), nlUtf8ToTStr(pluginN));
	}
	theApp.WriteProfileString(_T(""), nlUtf8ToTStr(pluginN), nlUtf8ToTStr(dllName));
	Plugins.push_back( dllName.c_str() );
	return true;
}


/*
 *
 */
void CLog_analyserDlg::loadPluginConfiguration()
{
	// Read from the registry
	free( (void*)theApp.m_pszRegistryKey );
	theApp.m_pszRegistryKey = _tcsdup( _T("Nevrax") );

	CString pn = theApp.GetProfileString( _T(""), _T("Plugin0") );
	char pluginN [10];
	int i = 0;
	while ( ! pn.IsEmpty() )
	{
		Plugins.push_back( pn );
		++i;
		smprintf( pluginN, 10, "Plugin%d", i );
		pn = theApp.GetProfileString( _T(""), nlUtf8ToTStr(pluginN) );
	}
}


/*
 *
 */
bool	isNumberChar( char c )
{
	return (c >= '0') && (c <= '9');
}


/*
 *
 */
void CLog_analyserDlg::OnAddView()
{
	vector<CString> v;
	addView( v );
}


/*
 *	 
 */
void CLog_analyserDlg::addView( std::vector<CString>& pathNames ) 
{
	if ( pathNames.empty() )
	{
		CFileDialog openDialog( true, NULL, _T("log.log"), OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, _T("Log files (*.log)|*.log|All files|*.*||"), this );
		CString filenameList;
		openDialog.m_ofn.lpstrFile = filenameList.GetBufferSetLength( 8192 );
		openDialog.m_ofn.nMaxFile = 8192;
		if ( openDialog.DoModal() == IDOK )
		{
			CWaitCursor wc;

			// Get the selected filenames
			CString pathName;
			POSITION it = openDialog.GetStartPosition();
			while ( it != NULL )
			{
				pathNames.push_back( openDialog.GetNextPathName( it ) );
			}
			if ( pathNames.empty() )
				return;
		}
		else
			return;
	}

	unsigned int i;
	if ( pathNames.size() > 1 )
	{
		// Sort the filenames
		for ( i=0; i!=pathNames.size(); ++i )
		{
			// Ensure that a log file without number comes *after* the ones with a number
			string name = NLMISC::tStrToUtf8(pathNames[i]);
			string::size_type dotpos = name.find_last_of('.');
			if ( (dotpos!=string::npos) && (dotpos > 2) )
			{
				if ( ! (isNumberChar(name[dotpos-1]) && isNumberChar(name[dotpos-2]) && isNumberChar(name[dotpos-3])) )
				{
					name = name.substr( 0, dotpos ) + "ZZZ" + name.substr( dotpos );
					pathNames[i] = name.c_str();
				}
			}
		}
		sort( pathNames.begin(), pathNames.end() );
		for ( i=0; i!=pathNames.size(); ++i )
		{
			// Set the original names back
			string name = NLMISC::tStrToUtf8(pathNames[i]);
			string::size_type tokenpos = name.find( "ZZZ." );
			if ( tokenpos != string::npos )
			{
				name = name.substr( 0, tokenpos ) + name.substr( tokenpos + 3 );
				pathNames[i] = name.c_str();
			}
		}
	}

	// Display the filenames
	string names;
	if ( isLogSeriesEnabled() )
		names += "Loading series corresponding to :\r\n";
	else
		names += "Loading files:\r\n";
	for ( i=0; i!=pathNames.size(); ++i )
		names += NLMISC::tStrToUtf8(pathNames[i]) + "\r\n";
	displayCurrentLine( names.c_str() );
	
	// Add view and browse sessions if needed
	CViewDialog *view = onAddCommon( pathNames );

	// Set filters
	FilterDialog.Trace = false;
	if ( FilterDialog.DoModal() == IDOK )
	{
		view->setFilters( FilterDialog.getPosFilter(), FilterDialog.getNegFilter() );

		// Load file
		view->reload();
	}
}


/*
 *
 */
void CLog_analyserDlg::OnAddtraceview() 
{
	CFileDialog openDialog( true, NULL, _T("log.log"), OFN_HIDEREADONLY, _T("Log files (*.log)|*.log|All files|*.*||"), this );
	if ( openDialog.DoModal() == IDOK )
	{
		vector<CString> pathNames;
		pathNames.push_back( openDialog.GetPathName() );
		CViewDialog *view = onAddCommon( pathNames );

		// Set filters
		FilterDialog.Trace = true;
		if ( FilterDialog.DoModal() == IDOK )
		{
			view->setFilters( FilterDialog.getPosFilter(), FilterDialog.getNegFilter() );
		}

		// Load file
		view->reloadTrace();
	}
}


/*
 * Precondition: !filenames.empty()
 */
CViewDialog *CLog_analyserDlg::onAddCommon( const vector<CString>& filenames )
{
	CWaitCursor wc;
	
	// Create view
	CViewDialog *view = new CViewDialog();
	view->Create( IDD_View, this );
	view->Index = (int)Views.size();
	RECT editRect;
	m_Edit.GetWindowRect( &editRect );
	ScreenToClient( &editRect );
	RECT parentRect;
	GetClientRect( &parentRect );
	Views.push_back( view );
	int i, w = 0;
	for ( i=0; i!=(int)Views.size(); ++i )
	{
		Views[i]->WidthR = 1.0f/(float)Views.size();
		Views[i]->resizeView( (int)Views.size(), editRect.bottom+10, w );
		w += (int)(Views[i]->WidthR*(parentRect.right-32));
	}
	view->ShowWindow( SW_SHOW );

	// Set params
	if ( filenames.size() == 1 )
	{
		// One file or a whole log series
		view->Seriesname = filenames.front();
		getLogSeries( filenames.front(), view->Filenames );
	}
	else
	{
		// Multiple files
		view->Seriesname = filenames.front() + "...";
		view->Filenames = filenames;
	}

	view->LogSessionStartDate.Empty();
	LogSessionsDialog.clear();

	if ( ((CButton*)GetDlgItem( IDC_CheckSessions ))->GetCheck() == 1 )
	{
		LogSessionsDialog.addLogSession( "Beginning" );
		int nbsessions = 0;
		for ( i=0; i!=(int)(view->Filenames.size()); ++i )
		{
			// Scan file for log sessions dates
			ifstream ifs( view->Filenames[i] );
			if ( ! ifs.fail() )
			{
				char line [1024];
				while ( ! ifs.eof() )
				{
					ifs.getline( line, 1024 );
					if ( strstr( line, nlTStrToUtf8(LogDateString) ) != NULL )
					{
						LogSessionsDialog.addLogSession( line );
						++nbsessions;
					}
				}
			}
		}

		// Heuristic to bypass the session choice if not needed
		bool needToChooseSession;
		switch ( nbsessions )
		{
		case 0:
			// No 'Log Starting' in the file(s) => no choice needed
			needToChooseSession = false;
			break;
		case 1:
			{
			// 1 'Log Starting' => no choice if it's at the beginning (1st line, or 2nd line with blank 1st)
			ifstream ifs(view->Filenames[0]); // 1 session => ! Filename.empty()
			char line[1024];
			ifs.getline(line, 1024);
			if (!ifs.fail())
			{
				if (strstr(line, nlTStrToUtf8(LogDateString)) != NULL)
					needToChooseSession = false;
				else if ( string(line).empty() )
				{
					if (!ifs.fail())
					{
						ifs.getline(line, 1024);
						needToChooseSession = (strstr(line, nlTStrToUtf8(LogDateString)) == NULL);
					}
					else
						needToChooseSession = true;
				}
			}
			else
				needToChooseSession = true;
			}
		    break;
		default:
			// Several 'Log Starting' => always choice
			needToChooseSession = true;
		}

		// Let the user choose the session (if needed)
		if ( (needToChooseSession) && (LogSessionsDialog.DoModal() == IDOK) )
		{
			view->LogSessionStartDate = LogSessionsDialog.getStartDate();
		}
	}

	setCurrentView( view->Index );
	return view;
}


/*
 * Code from NeL misc
 */
int smprintf( char *buffer, size_t count, const char *format, ... )
{
	int ret;

	va_list args;
	va_start( args, format );
	ret = vsnprintf( buffer, count, format, args );
	if ( ret == -1 )
	{
		buffer[count-1] = '\0';
	}
	va_end( args );

	return( ret );
}


/*
 *
 */
void CLog_analyserDlg::getLogSeries( const CString& filenameStr, std::vector<CString>& filenameList )
{
	if ( isLogSeriesEnabled() )
	{
		string filename = NLMISC::tStrToUtf8(filenameStr);
		unsigned int dotpos = filename.find_last_of ('.');
		if ( dotpos != string::npos )
		{
			string start = filename.substr( 0, dotpos );
			string end = filename.substr( dotpos );
			char numchar [4];
			unsigned int i = 0;
			bool anymore = true;
			while ( anymore )
			{
				// If filename is my_service.log, try my_service001.log..my_service999.log
				string npath = start;
				smprintf( numchar, 4, "%03d", i );
				npath += numchar + end;
				if ( ! ! fstream( npath.c_str(), ios::in ) )
				{
					// File exists => add it
					filenameList.push_back( npath.c_str() );
					if ( i == 999 )
					{
						filenameList.push_back( "<Too many log files in the series>" );
						anymore = false;
					}
					++i;
				}
				else
				{
					// No more files
					anymore = false;
				}
			}
		}
	}

	// At last, add the filename
	filenameList.push_back( filenameStr );
}


/*
 *
 */
void CLog_analyserDlg::displayCurrentLine( const CString& line )
{
	m_Edit.SetSel( 0, -1 );
	m_Edit.Clear();
	m_Edit.ReplaceSel( line, true );
}


/*
 *
 */
bool CLog_analyserDlg::selectText( int lineNum, int colNum, int length )
{
	int index = m_Edit.LineIndex( lineNum );
	if ( index != -1 )
	{
		index += colNum;
		m_Edit.SetSel( index, index + length );
		m_Edit.SetFocus();
		return true;
	}
	else
		return false;
}


/*
 *
 */
void CLog_analyserDlg::displayFileList()
{
	if ( ! MemorizedFileList.IsEmpty() )
	{
		displayCurrentLine( MemorizedFileList );
	}
}


/*
 *
 */
void CLog_analyserDlg::insertTraceLine( int index, char *traceLine )
{
	/*CString s0;
	s0.Format( "%s", traceLine );
	MessageBox( s0 );*/

	char *line = strchr( traceLine, ':' );
	char scycle [10];
	strncpy( scycle, traceLine, line-traceLine );
	int cycle = atoi(scycle);
	TStampedLine stampedLine;
	stampedLine.Index = index;
	stampedLine.Line = CString(traceLine);
	TraceMap.insert( make_pair( cycle, stampedLine ) );

	/*CString s;
	s.Format( "%d - %s", cycle, line );
	MessageBox( s );*/
}


/*
 *
 */
void CLog_analyserDlg::OnComputeTraces() 
{
	CWaitCursor wc;

	if ( Views.empty() )
		return;

	Trace = true;
	
	int j;
	for ( j=0; j!=(int)Views.size(); ++j )
	{
		Views[j]->clear();
		Views[j]->setRedraw( false );
	}

	multimap<int, TStampedLine>::iterator itm = TraceMap.begin(), itmU, itmC;
	while ( itm != TraceMap.end() )
	{
		// Fill all the views for one cycle
		itmU = TraceMap.upper_bound( (*itm).first );
		for ( itmC=itm; itmC!=itmU; ++itmC )
		{
			TStampedLine& stampedLine = (*itmC).second;
			Views[stampedLine.Index]->addLine( stampedLine.Line );
		}

		// Get the number of lines of the most filled view
		int i, maxNbLines=0;
		for ( i=0; i!=(int)Views.size(); ++i )
		{
			int vnb = Views[i]->getNbLines();
			if ( vnb > maxNbLines )
			{
				maxNbLines = vnb;
			}
		}

		// Fill the gaps with blank lines
		for ( i=0; i!=(int)Views.size(); ++i )
		{
			Views[i]->fillGaps( maxNbLines );
		}

		itm = itmU;
	}

	for ( j=0; j!=(int)Views.size(); ++j )
	{
		Views[j]->commitAddedLines();
		Views[j]->setRedraw( true );
	}

	m_ScrollBar.SetScrollRange( 0, Views[0]->getNbLines()-Views[0]->getNbVisibleLines() );
	m_ScrollBar.ShowWindow( SW_SHOW );
}


/*
 *
 */
void CLog_analyserDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( Trace )
	{
		int index;
		switch ( nSBCode )
		{
		case SB_TOP : index = 0; break;
		case SB_BOTTOM : index = Views[0]->getNbLines()-1; break;
		case SB_ENDSCROLL : index = -1; break;
		case SB_LINEDOWN : index = Views[0]->getScrollIndex()+1; break;
		case SB_LINEUP : index = Views[0]->getScrollIndex()-1; break;
		case SB_PAGEDOWN : index = Views[0]->getScrollIndex()+Views[0]->getNbVisibleLines(); break;
		case SB_PAGEUP : index = Views[0]->getScrollIndex()-Views[0]->getNbVisibleLines(); break;
		case SB_THUMBPOSITION :
		case SB_THUMBTRACK :
			index = nPos;
			break;
		}

		if ( index != -1 )
		{
			// Scroll the views
			for ( int i=0; i!=(int)Views.size(); ++i )
			{
				Views[i]->scrollTo( index );
			}

			pScrollBar->SetScrollPos( index );
		}
	}

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


/*
 *
 */
void CLog_analyserDlg::OnReset() 
{
	m_Edit.SetSel( 0, -1 );
	m_Edit.Clear();

	vector<CViewDialog*>::iterator iv;
	for ( iv=Views.begin(); iv!=Views.end(); ++iv )
	{
		(*iv)->DestroyWindow();
		delete (*iv);
	}
	Views.clear();
	CurrentView = NULL;

	Trace = false;
	TraceMap.clear();
	m_ScrollBar.ShowWindow( SW_HIDE );
}


/*
 * 
 */
void CLog_analyserDlg::OnDispLineHeaders() 
{
	vector<CViewDialog*>::iterator iv;
	for ( iv=Views.begin(); iv!=Views.end(); ++iv )
	{
		(*iv)->Invalidate();
	}		
}


/*
 *
 */
void CLog_analyserDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if ( ::IsWindow(m_Edit) )
	{
		RECT cltRect, editRect, sbRect;
		GetClientRect( &cltRect ),
		m_Edit.GetWindowRect( &editRect );
		m_ScrollBar.GetWindowRect( &sbRect );
		ScreenToClient( &editRect );
		ScreenToClient( &sbRect );
		editRect.right = cltRect.right-16;
		sbRect.right += cltRect.right-28-sbRect.left;
		sbRect.left = cltRect.right-28;
		sbRect.bottom = cltRect.bottom-12;
		m_Edit.MoveWindow( &editRect );
		m_ScrollBar.MoveWindow( &sbRect );

		resizeViews();
	}
}


/*
 *
 */
void CLog_analyserDlg::resizeViews()
{
	RECT editRect;
	m_Edit.GetWindowRect( &editRect );
	ScreenToClient( &editRect );
	RECT parentRect;
	GetClientRect( &parentRect );
	int i, w = 0;
	for ( i=0; i!=(int)Views.size(); ++i )
	{
		Views[i]->resizeView( (int)Views.size(), editRect.bottom+10, w );
		w += (int)(Views[i]->WidthR*(parentRect.right-32));
	}
}


/*
 *
 */
void CLog_analyserDlg::beginResizeView( int index )
{
	ResizeViewInProgress = index;
	SetCursor( theApp.LoadStandardCursor( IDC_SIZEWE ) );
	SetCapture();
}


/*
 * 
 */
void CLog_analyserDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ( ResizeViewInProgress != -1 )
	{
		if ( ResizeViewInProgress > 0 )
		{
			RECT viewRect, appClientRect;
			Views[ResizeViewInProgress]->GetWindowRect( &viewRect );
			ScreenToClient( &viewRect );
			GetClientRect( &appClientRect );
			if ( point.x < 0 )
				point.x = 10;
			int deltaPosX = point.x - viewRect.left;
			float deltaR = (float)deltaPosX / (float)(appClientRect.right-32);
			if ( -deltaR > Views[ResizeViewInProgress-1]->WidthR )
				deltaR = -Views[ResizeViewInProgress-1]->WidthR + 0.01f;
			if ( deltaR > Views[ResizeViewInProgress]->WidthR )
				deltaR = Views[ResizeViewInProgress]->WidthR - 0.01f;
			Views[ResizeViewInProgress-1]->WidthR += deltaR;
			Views[ResizeViewInProgress]->WidthR -= deltaR;
		}
		ResizeViewInProgress = -1;
		ReleaseCapture();
		SetCursor( theApp.LoadStandardCursor( IDC_ARROW ) );
		resizeViews();
	}

	CDialog::OnLButtonUp(nFlags, point);
}


/*
 *
 */
void CLog_analyserDlg::OnDestroy() 
{
	OnReset();

	CDialog::OnDestroy();
}


/*
 *
 */
void CLog_analyserDlg::OnHelpBtn()
{
	CString s = "NeL Log Analyser v1.5.0\n(c) 2002-2003 Nevrax\n\n";
	s += "Simple Mode: open one or more log files using the button 'Add View...'.\n";
	s += "You can make a multiple selection, then the files will be sorted by log order.\n";
	s += "If the file(s) being opened contain(s) several log sessions, you can choose one or\n";
	s += "choose to display all sessions, if the checkbox 'Browse Log Sessions' is enabled. If the\n";
	s += "checkbox 'Browse All File Series' is checked and you choose my_service.log, all log\n";
	s += "files of the series beginning with my_service000.log up to the biggest number found,\n";
	s += "and ending with my_service.log, will be opened in the same view. If 'Detect corrupted\n";
	s += "files' is checked, the possibly malformed lines will be reported.\n";
	s += "You can add some positive/negative filters. Finally, you may click a log line to display\n";
	s += "it in its entirety in the top field.\n";
	s += "Another way to open a file is to pass its filename as an argument. An alternative way to\n";
	s += "open one or more files is to drag & drop them onto the main window!.\n";
	s += "To actualize a file (which may have changed if a program is still writing into it), just\n";
	s += "click 'Filter...' and OK.\n";
	s += "Resizing a view is done by dragging its left border.\n";
	s += "Line bookmarks: set/remove = Ctrl+F2, recall = F2. They are kept when changing the filter.\n\n";
	s += "Trace Mode: open several log files in Trace Format (see below) using the button\n";
	s += "'Add Trace View...', you can limit the lines loaded to the ones containing a\n";
	s += "specific service shortname (see below). Then click the button 'Compute Traces'\n";
	s += "to display the matching lines. The lines are sorted using their gamecycle and\n";
	s += "blank lines are filled so that different parallel views have the same timeline.\n";
	s += "Use the right scrollbar to scroll all the views at the same time.\n";
	s += "The logs in Trace Format should contains some lines that have a substring sTRACE:n:\n";
	s += "where s is an optional service name (e.g. FS) and n is the gamecycle of the action\n";
	s += "(an integer).\n\n";
	s += "Plug-in system: You can provide DLLs to perform some processing or analysis on the current\n";
	s += "view. To register a new plug-in, drag-n-drop the DLL on the main window of the Log Analyser.\n";
	s += "To unregister a plug-in, see HKEY_CURRENT_USER\\Software\\Nevrax\\log_analyser.INI in the\n";
	s += "registry.";
	MessageBox( s );	
}


/*
 * Plug-in activation
 */
void CLog_analyserDlg::OnAnalyse() 
{
	PlugInSelectorDialog.setPluginList( Plugins );
	if ( PlugInSelectorDialog.DoModal() == IDOK )
	{
		if ( Views.empty() )
		{
			AfxMessageBox(_T("This plug-in needs to be applied on the first open view"));
			return;
		}

		if ( ! PlugInSelectorDialog.AnalyseFunc )
		{
			AfxMessageBox(_T("Could not load function doAnalyse in dll"));
			return;
		}

		// Call the plug-in function and get results
		string resstr, logstr;
		PlugInSelectorDialog.AnalyseFunc( *(const std::vector<const char *>*)(void*)&(getCurrentView()->Buffer), resstr, logstr );
		if ( ! logstr.empty() )
		{
			vector<CString> pl;
			pl.push_back(_T("Analyse log"));
			onAddCommon( pl );
			Views.back()->addText( logstr.c_str() );
			Views.back()->commitAddedLines();
		}
		displayCurrentLine( resstr.c_str() );

		// Debug checks
		int nStartChar, nEndChar;
		m_Edit.GetSel( nStartChar, nEndChar );
		if ( nEndChar != (int)resstr.size() )
		{
			CString s;
			s.Format(_T("Error: plug-in returned %u characters, only %d displayed"), (uint)resstr.size(), nEndChar+1 );
			AfxMessageBox( s );
		}
	}
}
