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

#include "stdafx.h"
#include "log_analyser.h"
#include "PlugInSelector.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlugInSelector dialog


CPlugInSelector::CPlugInSelector(CWnd* pParent /*=NULL*/)
	: CDialog(CPlugInSelector::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlugInSelector)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPlugInSelector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlugInSelector)
	DDX_Control(pDX, IDC_LIST1, m_PlugInListBox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlugInSelector, CDialog)
	//{{AFX_MSG_MAP(CPlugInSelector)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlugInSelector message handlers

/*
 *
 */
BOOL CPlugInSelector::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if ( m_PlugInListBox.GetCount() == 0 )
	{
		for (unsigned int i=0; i!=Dlls->size(); ++i )
		{
			m_PlugInListBox.InsertString( i, (*Dlls)[i] ); // not sorted
		}
	}

	AnalyseFunc = NULL;
	LibInst = NULL;

	GetDlgItem( IDC_GROUP_INFO )->EnableWindow( ! Dlls->empty() );
	GetDlgItem( IDOK )->EnableWindow( ! Dlls->empty() );
	if ( ! Dlls->empty() )
	{
		m_PlugInListBox.SetCurSel( 0 );
		OnSelchangeList1();
	}

	return TRUE;
}


int getLastSeparator (const string &filename)
{
	string::size_type pos = filename.find_last_of ('/');
	if (pos == string::npos)
	{
		pos = filename.find_last_of ('\\');
		if (pos == string::npos)
		{
			pos = filename.find_last_of ('@');
		}
	}
	return pos;
}


string getFilename (const string &filename)
{
	string::size_type pos = getLastSeparator(filename);
	if (pos != string::npos)
		return filename.substr (pos + 1);
	else
		return filename;
}


/*
 *
 */
void CPlugInSelector::OnSelchangeList1() 
{
	CString dllName;
	m_PlugInListBox.GetText( m_PlugInListBox.GetCurSel(), dllName );

	// Release previous DLL if any
	if ( LibInst != NULL )
	{
		FreeLibrary( LibInst );
	}

	// Load DLL
	LibInst = LoadLibrary( dllName );
	if ( ! LibInst )
	{
		CString s;
		char msg [300];
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, 299, NULL );
		s.Format( "Can't load %s: %s", dllName, msg );
		AfxMessageBox( s );
		AnalyseFunc = NULL;
		return;
	}

	// Display info
	TInfoFunc infoFunc = (TInfoFunc)GetProcAddress( LibInst, "getInfoString" );
	if ( ! infoFunc )
	{
		AfxMessageBox( "Can't find function getInfoString in dll" );
		return;
	}
	GetDlgItem( IDC_GROUP_INFO )->SetWindowText( getFilename( string(dllName)).c_str() );
	GetDlgItem( IDC_PLUGIN_INFO )->SetWindowText( infoFunc().c_str() );

	// Prepare analyse func
	AnalyseFunc = (TAnalyseFunc)GetProcAddress( LibInst, "doAnalyse" );
	if ( ! AnalyseFunc )
	{
		AfxMessageBox( "Can't find function doAnalyse in dll" );
		return;
	}

	GetDlgItem( IDOK )->EnableWindow( m_PlugInListBox.GetCurSel() != LB_ERR );
}


/*
 *
 */
void CPlugInSelector::OnCancel() 
{
	AnalyseFunc = NULL;

	// Release previous DLL if any
	if ( LibInst != NULL )
	{
		FreeLibrary( LibInst );
		LibInst = NULL;
	}

	CDialog::OnCancel();
}
