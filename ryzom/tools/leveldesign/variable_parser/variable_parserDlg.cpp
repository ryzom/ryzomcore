// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// VariableParserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "variable_parser.h"
#include "variable_parserDlg.h"

/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/

using namespace NLMISC;
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CVariableParserDlg dialog

CVariableParserDlg::CVariableParserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVariableParserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVariableParserDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CVariableParserDlg::~CVariableParserDlg()
{
	m_nomVariables.clear();
	for ( uint i=0; i<m_variables.size(); i++ )
	{
		m_variables[i].clear();
	}

	m_variables.clear();
}

void CVariableParserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVariableParserDlg)
	DDX_Control(pDX, IDC_VARDEF, m_varDefList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVariableParserDlg, CDialog)
	//{{AFX_MSG_MAP(CVariableParserDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_HDR_BROWSE, OnHdrBrowse)
	ON_BN_CLICKED(IDC_TMPL_BROWSE, OnTmplBrowse)
	ON_BN_CLICKED(IDC_FOOT_BROWSE, OnFootBrowse)
	ON_BN_CLICKED(IDC_OUTPUT_BROWSE, OnOutputBrowse)
	ON_BN_CLICKED(IDC_GENERATE, OnGenerate)
	ON_BN_CLICKED(IDC_GEN_BROWSE, OnGenBrowse)
	ON_BN_CLICKED(IDC_LUA_BROWSE, OnLUABrowse)
	ON_BN_CLICKED(IDC_ADDVARDEF, OnAddvardef)
	ON_BN_CLICKED(IDC_REMVARDEF, OnRemvardef)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVariableParserDlg message handlers

BOOL CVariableParserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVariableParserDlg::OnPaint() 
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
HCURSOR CVariableParserDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CVariableParserDlg::OnHdrBrowse() 
{
	CFileDialog fileDlg( TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, "Primitive file (*.primitive)|*.primitive||");
	
	if ( fileDlg.DoModal() == IDOK )
	{
		GetDlgItem( IDC_HDR_FILE )->SetWindowText( fileDlg.GetPathName() );
	}
}


void CVariableParserDlg::OnTmplBrowse() 
{
	CFileDialog fileDlg( TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, "Primitive file (*.primitive)|*.primitive||");
	
	if ( fileDlg.DoModal() == IDOK )
	{
		GetDlgItem( IDC_TMPL_FILE )->SetWindowText( fileDlg.GetPathName() );
	}
}


void CVariableParserDlg::OnFootBrowse() 
{
	CFileDialog fileDlg( TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, "Primitive file (*.primitive)|*.primitive||");
	
	if ( fileDlg.DoModal() == IDOK )
	{
		GetDlgItem( IDC_FOOT_FILE )->SetWindowText( fileDlg.GetPathName() );
	}
}


void CVariableParserDlg::OnOutputBrowse() 
{
	CFileDialog fileDlg( FALSE, NULL, NULL, OFN_OVERWRITEPROMPT, "Primitive file (*.primitive)|*.primitive||");
	
	if ( fileDlg.DoModal() == IDOK )
	{
		GetDlgItem( IDC_OUTPUT_FILE )->SetWindowText( fileDlg.GetPathName() );
	}
}


void CVariableParserDlg::OnGenBrowse() 
{
	CFileDialog fileDlg( TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, "Comma Separated Values (*.csv)|*.csv||");
	
	if ( fileDlg.DoModal() == IDOK )
	{
		GetDlgItem( IDC_GEN_FILE )->SetWindowText( fileDlg.GetPathName() );
	}
}

void CVariableParserDlg::OnLUABrowse() 
{
	CFileDialog fileDlg( TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, "Lua files (*.lua)|*.lua||");
	
	if ( fileDlg.DoModal() == IDOK )
	{
		GetDlgItem( IDC_LUA_FILE )->SetWindowText( fileDlg.GetPathName() );
	}
}

void CleanString( CSString &str )
{
	int i = (int)str.size();
	bool ok = false;

	while ( !ok && ( i > 0 ) )
	{
		char c = str.c_str()[i-1];
		if ( !isalpha(c) && !isdigit(c) && ( c != '_' ) && ( c != ')' ) )
		{
			i--;
		}
		else 
			ok = true;
	}

	str = str.left( i );
}

void CVariableParserDlg::ProcessGeneratorFile( const string& generatorFile )
{
	CSString genData, ligne;
	int colonneVar = 0;

	genData.readFromFile( generatorFile ); 

	ligne = genData.splitTo( "\n", true );

	while ( ligne != "" )
	{
		CSString var = ligne.splitTo( ";", true );
		CleanString( var );

		if ( var != "" )
		{
			m_nomVariables.push_back( var ) ;
			vector<string> vec;
			m_variables.push_back( vec );
		}
	}
	
	while ( genData != "" )
	{
		ligne = genData.splitTo( "\n", true );
		
		if ( ligne != "" )
		{
			for ( uint i=0;i<m_variables.size(); i++)
			{
				CSString var = ligne.splitTo( ";", true );
				CleanString( var );

				if ( var != "" )
				{		
					m_variables[i].push_back( var );
				}
			}

		}
	}
	
	ParseParameters params;
	BuildParseParameters( params, 0, 0 );
}


void CVariableParserDlg::BuildParseParameters( ParseParameters& params, uint ligne, uint colonne )
{
	CSString left = CSString( m_nomVariables[ colonne ] ).left(2);
	
	if ( left == "@@" )
	{
		//if ( colonne+1 >= params.capacity() )
			params.push_back( "" );
		
		CString str = m_variables[colonne][ligne].c_str();
		params[colonne] = str;

		if ( colonne+1 == m_variables.size() )
		{
			ParseTemplate( params );
		}
		else
		{
			BuildParseParameters( params, ligne, colonne+1 );
		}
	}
	else if ( left == "##" )
	{
		params.push_back( "" );	
		CString str = m_variables[colonne][0].c_str();
			
		for ( uint j=(uint)m_nomVariables.size(); j>0; j-- )
		{
			str.Replace( toString( "C%d", j-1 ).c_str(), params[j-1].c_str() );
		}

		try 
		{
			m_luaState.executeScript( str.GetBuffer( str.GetLength() ) );
		}
		catch (ELuaError &e)
		{
			MessageBox( toString( "%s", e.luaWhat().c_str() ).c_str() );
			return;
		}
		
		
		m_luaState.push("VAL");
		m_luaState.getTable(LUA_GLOBALSINDEX);
		bool ok= false;
		sint type= m_luaState.type();

		if ( type == LUA_TBOOLEAN )
		{
			// get and pop
			bool val= m_luaState.toBoolean();
			m_luaState.pop();
			// set result
			if ( val )
				str = "TRUE";
			else
				str = "FALSE";
			ok = true;
		}
		else if ( type == LUA_TNUMBER )
		{
			// get and pop
			double	val= m_luaState.toNumber();
			m_luaState.pop();
			// set double or integer?
			if ( val == floor( val ) )
				str.Format( "%d", sint64( floor( val ) ) );
			else
				str.Format( "%.3f", val );
			ok = true;
		}
		else if ( type == LUA_TSTRING )
		{
			// get and pop
			std::string	val;
			m_luaState.toString(-1,  val);
			m_luaState.pop();
			// set result
			str = val.c_str();
			ok = true;
		}

		if ( !ok )
		{
			MessageBox( toString( "VAL is not defined on column %d", colonne ).c_str() );
		}
		else
		{
			params[colonne] = str;
			if ( colonne+1 == m_variables.size() )
			{
				ParseTemplate( params );
			}
			else
			{
				BuildParseParameters( params, ligne, colonne+1 );
			}
		}
	}
	else
	{
		for ( uint i=0; i<m_variables[colonne].size(); i++ )
		{
			//if ( colonne+1 >= params.capacity() )
				params.push_back( "" );
			
			CString str;
			
			str = m_variables[colonne][i].c_str();

			params[colonne] = str;

			if ( colonne+1 == m_variables.size() )
			{
				ParseTemplate( params );
			}
			else
			{
				BuildParseParameters( params, i, colonne+1 );
			}
		}
	}
}


void CVariableParserDlg::ParseTemplate( const ParseParameters& params )
{
	CString tmp = m_templateText.c_str();
	
	for ( uint i=0; i< m_nomVariables.size(); i++ )
	{
		CSString toFind;
		if ( CSString( m_nomVariables[ i ] ).left(2) == "@@" )
		{
			toFind = toString( "%s", m_nomVariables[i].c_str() );
		}
		else if ( CSString( m_nomVariables[ i ] ).left(2) == "##" )
		{
			toFind = toString( "%s", m_nomVariables[i].c_str() );
		}
		else
			toFind = toString( "$$%s", m_nomVariables[i].c_str() );

		tmp.Replace( toFind.c_str(), params[i].c_str() );
	}
	
	m_outputText += tmp;
}

void CVariableParserDlg::OnGenerate() 
{
	CString error = "";
	
	CSString hdr, foot;
	CString hdrFilename, templFileName, footFileName, 
		    outputFileName, luaFileName;

	
	GetDlgItem( IDC_HDR_FILE )->GetWindowText( hdrFilename );
	if ( hdrFilename == "" )
		error += "Header file name not set.\n";	
	
	GetDlgItem( IDC_FOOT_FILE )->GetWindowText( footFileName );
	if ( footFileName == "" )
		error += "Foot file name not set.\n";	
	
	GetDlgItem( IDC_TMPL_FILE )->GetWindowText( templFileName );
	if ( templFileName == "" )
		error += "Template file name not set.\n";

	if ( m_varDefList.GetCount() == 0 )
		error += "Generator file name not set.\n";

	GetDlgItem( IDC_OUTPUT_FILE )->GetWindowText( outputFileName );
	if ( outputFileName == "" )
		error += "Output file name not set.";

	if ( error != "" )
	{
		MessageBox( error, NULL, MB_OK );
	}
	else
	{
		GetDlgItem( IDC_LUA_FILE )->GetWindowText( luaFileName );
		if ( luaFileName != "" )
		{
			m_luaState.executeFile( std::string( luaFileName ) );
		}
	
		hdr.readFromFile( std::string( hdrFilename ) );
		foot.readFromFile( std::string( footFileName ) );
		m_templateText.readFromFile( std::string( templFileName ) );
		
		m_outputText = hdr;
		for ( int i=0; i<m_varDefList.GetCount(); i++ )
		{
			CString str;
			m_varDefList.GetText( i, str );
			ProcessGeneratorFile( std::string( str ) );
			m_variables.clear();
		}

		m_outputText += foot;
		m_outputText.writeToFile( std::string( outputFileName ) );

		MessageBox( "Done !" );
	}
}

void CVariableParserDlg::OnAddvardef() 
{
	CFileDialog fileDlg( TRUE, NULL, NULL, OFN_OVERWRITEPROMPT|OFN_ALLOWMULTISELECT, 
		                 "Comma Separated Values (*.csv)|*.csv||");
	
	if ( fileDlg.DoModal() == IDOK )
	{
		for ( POSITION pos = fileDlg.GetStartPosition(); pos != NULL; )
		{
			CString str = fileDlg.GetNextPathName(pos);
			int index = m_varDefList.FindStringExact( 0, str );
			if ( index == LB_ERR )
			{	
				m_varDefList.SetCurSel( m_varDefList.AddString( str ) );
			}
			else
				m_varDefList.SetCurSel( index );
		} 
	}
}

void CVariableParserDlg::OnRemvardef() 
{
	int index = m_varDefList.GetCurSel();
	if ( index != LB_ERR )
		m_varDefList.DeleteString( index );
}
